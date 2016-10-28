#ifndef VolViz_VisualizerImpl_h
#define VolViz_VisualizerImpl_h

#include "AtomicCache.h"
#include "GL/GL.h"
#include "GeometryFactory.h"
#include "Shaders.h"

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <VolViz/VolViz.h>
#include <concurrentqueue.h>

#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>
#include <unordered_map>

namespace VolViz {
namespace Private_ {

class VisualizerImpl {
  using InitQueueEntry =
      std::pair<Visualizer::GeometryName, Geometry::UniquePtr>;
  using GeometryList =
      std::unordered_map<Visualizer::GeometryName, Geometry::UniquePtr>;
  using GeometryInitQueue = moodycamel::ConcurrentQueue<InitQueueEntry>;

public:
  using Point2 = Eigen::Vector2f;
  using Size2 = Eigen::Vector2f;
  using Lights = std::unordered_map<Visualizer::LightName, Light>;

  VisualizerImpl(Visualizer *vis);

  void start();

  void enableMultithreading() noexcept;

  void renderOneFrame(bool block = true);

  operator bool() const noexcept;

  template <class T>
  void setVolume(VolumeDescriptor descriptor, gsl::span<T> data);

  Size3f volumeSize() const noexcept;

  template <class Descriptor,
            typename = std::enable_if_t<std::is_base_of<
                GeometryDescriptor, std::decay_t<Descriptor>>::value>>
  inline void addGeometry(Visualizer::GeometryName name,
                          Descriptor const &descriptor) {
    geometryInitQueue_.enqueue({name, geomFactory_.create(descriptor)});
  }

  template <class Descriptor,
            typename = std::enable_if_t<std::is_base_of<
                GeometryDescriptor, std::decay_t<Descriptor>>::value>>
  inline bool updateGeometry(Visualizer::GeometryName name,
                             Descriptor &&descriptor) {
    std::lock_guard<std::mutex> lock{geometriesMutex_};
    auto search = geometries_.find(name);
    if (search == geometries_.end()) {
      // There are two reasons for not finding a geometry:
      //   1. The geometry was not added (usage error). Nothing can be done to
      //      recover -> throw exception.
      //   2. Multithreading is enabled, in this case the geometry might have
      //      been added but was not initialized, yet. In this case the user
      //      can repeat the update later -> return false.
      if (multithreadingEnabled_) return false;
      throw std::logic_error("Geometry " + name + " not found");
    }

    search->second->enqueueUpdate(std::forward<Descriptor>(descriptor));
    return true;
  }

  /// Convenience method for easy camera access
  inline Camera const &camera() const noexcept { return visualizer_->camera; }
  inline Camera &camera() noexcept { return visualizer_->camera; }

  inline auto cameraClient() const noexcept { return camera().client(); }

  inline Shaders &shaders() noexcept { return shaders_; }

  void attachVolumeToShader(GL::ShaderProgram &shader) const;

  /// Issues an OpenGL draw call with a single vertex.
  /// This comes in handy if all the geometry is created by a geometry shader
  void drawSingleVertex() const noexcept;

  /// Bind the volume texture to texture unit i
  void bindVolume(GLuint unitIdx = 0) const noexcept;

  /// Returns a matrix that transforms world coordinates into texture
  /// coordinates
  Eigen::Matrix4f textureTransformationMatrix() const noexcept;

  /// Visualizer's scale is cached here, since it is accessed at least once per
  /// frame and is usually cahnged very rare. Since every access to Visualizer's
  /// scale property requires thread synchronization, a cache is necessary here.
  AtomicCache<Length> cachedScale{
      [this]() -> Length { return visualizer_->scale; }};

private:
  friend class ::VolViz::Visualizer;

  using Clock = std::chrono::steady_clock;
  using TimePoint = std::chrono::time_point<Clock>;
  using GeometryNameAndPosition = struct {
    Visualizer::GeometryName name;
    Position position;
    float depth;
  };

  /// IDs for the auxiliary textures used for the deferred rendering
  enum class TextureID : std::size_t {
    NormalsAndSpecular = 0,
    Albedo = 1,
    Depth = 2,
    RenderedImage = 3,
    FinalDepth = 4,
    VolumeTexture = 5,
    SelectionTexture = 6
  };

  /// Setup the required textures and frabebuffer objects for rendering
  void setupFBOs();

  /// Setup selection buffers
  void setupSelectionBuffers();

  /// Unprojects a point in screen coordinates and a given depth to a 3D point
  /// in world space
  Position unproject(Position2 const &screenPoint, float depth) const noexcept;

  /// Key input handler
  void handleKeyInput(int key, int scancode, int action, int mode);

  /// Renders the geometry
  void renderGeometry();

  /// Update the geometry
  void updateGeometries();

  /// Renders a grid
  void renderGrid();

  /// Renders a point
  void renderPoint(Position const &position, Color const &color, float size);

  /// Renderes a textured quad
  void renderQuad(Point2 const &topLeft, Size2 const &size, TextureID texture,
                  GL::ShaderProgram &prog);

  /// Renders a textured fullscreen quad
  void renderFullscreenQuad(TextureID texture, GL::ShaderProgram &quad);

  /// Renders all textures of the deferred lighting pass
  void renderLightingTextures();

  /// Defferred shading lighing pass
  void renderLights();

  void renderAmbientLighting();
  void renderDiffuseLighting();
  void renderSpecularLighting();

  void renderLightDiffuse(Light const &light);

  void renderLightSpecular(Light const &light);

  void renderSelectionIndexTexture();

  GeometryNameAndPosition getGeometryUnderCursor();

  void dragSelectedGeometry();

  /// Renders the final image to screen
  void renderFinalPass();

  /// Renders a bounding box
  void renderBoundingBox(Position const &position,
                         Orientation const &orientation, Size3f const &size,
                         Color const &color);

  void renderVolumeBBox();

  void addLight(Visualizer::LightName name, Light const &light);

  /// @defgroup privateMembers Private member variables
  /// @{

  Visualizer *visualizer_ = nullptr;
  GeometryFactory geomFactory_;

  struct DepthRange {
    float near, far;
  } depthRange_;

  GL::GLFW glfw_;

  /// Shader programs
  Shaders shaders_;

  /// Auxiliary textures use in the deferred shading process.
  struct TextureWrapper {
    inline GLuint operator[](TextureID id) const noexcept {
      return textures_.names[static_cast<std::size_t>(id)];
    }

  private:
    GL::Textures<7> textures_;
  } textures_;
  /// Frabebuffer used for the deferred shading
  GL::Framebuffer finalFbo_{0};
  GL::Framebuffer lightingFbo_{0};

  /// Pixel buffers used for mouse picking
  struct SelectionBuffer {
    using Buffers = std::array<GL::Buffer, 2>;
    using Handle = Buffers::iterator;

    Buffers buffers;
    Handle readBuffer{buffers.begin()}, writeBuffer{buffers.begin() + 1};
    inline void swap() noexcept {
      using std::swap;
      swap(readBuffer, writeBuffer);
    }
  } selectionBuffer_;

  /// @defgroup geomGroup Geometry processing related variables
  /// @{
  GeometryList geometries_;
  std::mutex geometriesMutex_;
  GeometryInitQueue geometryInitQueue_;
  ///@}

  /// Data representing a single vertex, required by the grid and fullscreen
  /// quad renderer
  struct SingleVertData {
    GL::Buffer vBuff{0};
    GL::VertexArray vao{0};
  } singleVertexData_;

  enum class ViewState {
    Scene3D,
    LightingComponents,
    SelectionIndices
  } viewState_{ViewState::Scene3D};

  bool inSelectionMode{false};
  Visualizer::GeometryName selectedGeometry_;
  Position selectedPoint_{Position::Zero()};

  /// Lights
  Lights lights_;
  std::mutex lightMutex_;

  ///@defgroup cameraRelated Camera related variables
  /// @{
  /// Camera state
  enum class MoveState {
    None,
    Rotating,
    Dragging
  } moveState_ = MoveState::None;

  /// Last position of the mouse cursor, used in the camera control code
  Position2 lastMousePos_ = Position2::Zero();
  Position2 lastMouseDelta_ = Position2::Zero();
  //@}

  VolumeDescriptor currentVolume_;

  bool multithreadingEnabled_{false};

  //@}
};

} // namespace Private_
} // namespace VolViz

#endif // VolViz_VisualizerImpl_h

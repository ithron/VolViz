#ifndef VolViz_VisualizerImpl_h
#define VolViz_VisualizerImpl_h

#include "GL/GL.h"

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <VolViz/VolViz.h>

#include <atomic>
#include <chrono>
#include <mutex>
#include <queue>
#include <unordered_map>

namespace VolViz {
namespace Private_ {

class VisualizerImpl {
  using RenderCommand = std::function<void(std::uint32_t)>;
  using InitCommand = std::function<RenderCommand()>;
  using InitQueueEntry = std::pair<Visualizer::GeometryName, InitCommand>;
  using GeometryList =
      std::unordered_map<Visualizer::GeometryName, RenderCommand>;
  using GeometryInitQueue = std::queue<InitQueueEntry>;

public:
  using Point2 = Eigen::Vector2f;
  using Size2 = Eigen::Vector2f;
  using Lights = std::unordered_map<Visualizer::LightName, Light>;

  VisualizerImpl(Visualizer *vis);

  void start();

  void renderOneFrame();

  operator bool() const noexcept;

  template <class T>
  void setVolume(VolumeDescriptor const &descriptor, gsl::span<T> data);

  template <class VertBase, class IdxBase>
  void setMesh(Eigen::MatrixBase<VertBase> const &V,
               Eigen::MatrixBase<IdxBase> const &I);

  void addGeometry(Visualizer::GeometryName name,
                   AxisAlignedPlane const &plane);

private:
  friend class ::VolViz::Visualizer;

  using Clock = std::chrono::steady_clock;
  using TimePoint = std::chrono::time_point<Clock>;
  using GeometryNameAndPosition = std::pair<Visualizer::GeometryName, Position>;

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

  /// Compiles and links all shader programs
  void setupShaders();

  /// Setup the required textures and frabebuffer objects for rendering
  void setupFBOs();

  /// Setup selection buffers
  void setupSelectionBuffers();

  /// Returns the projection matrix
  Eigen::Matrix4f projectionMatrix() const noexcept;

  /// Returns the view matrix (i.e. inverse camera transformation)
  Eigen::Matrix4f viewMatrix() const noexcept;

  /// Returns a matrix that transforms world coordinates into texture
  /// coordinates
  Eigen::Matrix4f textureTransformationMatrix() const noexcept;

  /// Unprojects a point in screen coordinates and a given depth to a 3D point
  /// in world space
  Position unproject(Position2 const &screenPoint, float depth) const noexcept;

  /// Key input handler
  void handleKeyInput(int key, int scancode, int action, int mode);

  /// Renders the set mesh if any
  void renderMeshes();

  /// Renders the geometry
  void renderGeometry();

  /// Renders a grid
  void renderGrid();

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

  /// @defgroup shaders Shader Programs
  /// @{
  GL::GLFW glfw_;
  GL::ShaderProgram ambientPassProgram_;
  GL::ShaderProgram bboxProgram_;
  GL::ShaderProgram depthQuadProgram_;
  GL::ShaderProgram diffuseLightingPassProgram_;
  GL::ShaderProgram geometryStageProgram_;
  GL::ShaderProgram gridProgram_;
  GL::ShaderProgram hdrQuadProgram_;
  GL::ShaderProgram normalQuadProgram_;
  GL::ShaderProgram planeProgram_;
  GL::ShaderProgram quadProgram_;
  GL::ShaderProgram selectionIndexVisualizationProgam_;
  GL::ShaderProgram specularLightingPassProgram_;
  GL::ShaderProgram specularQuadProgram_;
  /// @}

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

  /// Data required to render a mesh
  struct MeshData {
    /// the vertex array object
    GL::VertexArray vao{0};
    /// vertex buffer
    GL::Buffer vertices{0};
    /// index buffer
    GL::Buffer indices{0};
    /// number of primitives (i.e. triangles) to render
    std::size_t nTriangles = 0;
    /// The shininess of the mesh surface
    float shininess = 10.f;
  } mesh_;

  /// @defgroup geomGroup Geometry processing related variables
  /// @{
  GeometryList geometries_;
  GeometryInitQueue geometryInitQueue_;
  std::mutex geomInitQueueMutex_;
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

  /// Lights
  Lights lights_;
  std::mutex lightMutex_;

  ///@defgroup cameraRelated Camera related variables
  /// @{
  /// Camera state
  enum class MoveState { None, Rotating } moveState_ = MoveState::None;

  /// Last position of the mouse cursor, used in the camera control code
  Position2 lastMousePos_ = Position2::Zero();

  /// Camera field of view
  float fov = Visualizer::kDefaultFOV;
  Eigen::Vector3f cameraPosition_ = Eigen::Vector3f::Zero();
  Eigen::Quaternionf cameraOrientation_ = Eigen::Quaternionf::Identity();
  //@}

  VolumeDescriptor currentVolume_;

  //@}
};

} // namespace Private_
} // namespace VolViz

#endif // VolViz_VisualizerImpl_h

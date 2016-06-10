#ifndef VolViz_VisualizerImpl_h
#define VolViz_VisualizerImpl_h

#include "Binding.h"
#include "Buffer.h"
#include "Framebuffer.h"
#include "GLFW.h"
#include "ShaderProgram.h"
#include "Shaders.h"
#include "Textures.h"
#include "VertexArray.h"
#include "Visualizer.h"

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <atomic>

namespace VolViz {
namespace Private_ {

class VisualizerImpl {
public:
  using Point2 = Eigen::Vector2f;
  using Size2 = Eigen::Vector2f;

  VisualizerImpl(Visualizer *vis);

  void start();

  void renderOneFrame();

  operator bool() const noexcept;

  template <class VertBase, class IdxBase>
  void setMesh(Eigen::MatrixBase<VertBase> const &V,
               Eigen::MatrixBase<IdxBase> const &I);

private:
  friend class ::VolViz::Visualizer;

  /// IDs for the auxiliary textures used for the deferred rendering
  enum class TextureID : std::size_t {
    Normals = 0,
    Albedo = 1,
    Specular = 2,
    Depth = 3,
    RenderedImage = 4
  };

  /// Compiles and links all shader programs
  void setupShaders();

  /// Setup the required textures and frabebuffer objects for rendering
  void setupFBOs();

  /// Returns the projection matrix
  Eigen::Matrix4f projectionMatrix() const noexcept;

  /// Returns the view matrix (i.e. inverse camera transformation)
  Eigen::Matrix4f viewMatrix() const noexcept;

  /// Key input handler
  void handleKeyInput(int key, int scancode, int action, int mode);

  /// Renders the set mesh if any
  void renderMeshes();

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

  /// Renders the final image to screen
  void renderFinalPass();

  /// @defgroup privateMembers Private member variables
  /// @{

  Visualizer *visualizer_ = nullptr;

  /// @defgroup shaders Shader Programs
  /// @{
  GL::GLFW glfw_;
  GL::ShaderProgram geometryStageProgram_;
  GL::ShaderProgram quadProgram_;
  GL::ShaderProgram normalQuadProgram_;
  GL::ShaderProgram depthQuadProgram_;
  GL::ShaderProgram gridProgram_;
  GL::ShaderProgram ambientPassProgram_;
  GL::ShaderProgram lightingPassProgram_;
  /// @}

  /// Auxiliary textures use in the deferred shading process.
  struct TextureWrapper {
    inline GLuint operator[](TextureID id) const noexcept {
      return textures_.names[static_cast<std::size_t>(id)];
    }

  private:
    GL::Textures<5> textures_;
  } textures_;
  /// Frabebuffer used for the deferred shading
  GL::Framebuffer lightingFbo_{0};
  GL::Framebuffer finalFbo_{0};

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
    float shininess = 0.01f;
  } mesh_;

  /// Data representing a single vertex, required by the grid and fullscreen
  /// quad renderer
  struct SingleVertData {
    GL::Buffer vBuff{0};
    GL::VertexArray vao{0};
  } singleVertexData_;

  enum class ViewState {
    Scene3D,
    LightingComponents
  } viewState_{ViewState::Scene3D};

  ///@defgroup cameraRelated Camera related variables
  /// @{
  /// Camera state
  enum class MoveState { None, Rotating } moveState_ = MoveState::None;

  /// Last position of the mouse cursor, used in the camera control code
  Eigen::Vector2d lastMousePos_ = Eigen::Vector2d::Zero();

  /// Camera field of view
  float fov = Visualizer::kDefaultFOV;
  Eigen::Vector3f cameraPosition_ = Eigen::Vector3f::Zero();
  Eigen::Quaternionf cameraOrientation_ = Eigen::Quaternionf::Identity();
  //@}

  //@}
};

} // namespace Private_
} // namespace VolViz

#endif // VolViz_VisualizerImpl_h

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
  VisualizerImpl();

  void start();

  void renderOneFrame();

  operator bool() const noexcept;

  template <class VertBase, class IdxBase>
  void setMesh(Eigen::MatrixBase<VertBase> const &V,
               Eigen::MatrixBase<IdxBase> const &I);

  /// If true a grid is shown
  std::atomic<bool> gridEnabled{true};

private:
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
  void renderMeshes() const;

  /// Renders a grid
  void renderGrid() const;

  /// Renders the final image to screen
  void renderFinalPass() const;

  /// @defgroup privateMembers Private member variables
  /// @{

  /// @defgroup shaders Shader Programs
  /// @{
  GL::GLFW glfw_;
  GL::ShaderProgram geometryStageProgram_;
  GL::ShaderProgram displayStageProgram_;
  GL::ShaderProgram gridProgram_;
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
    std::size_t nTriangles = 0;
    /// number of primitives (i.e. triangles) to render
  } mesh_;

  /// Data representing a single vertex, required by the grid and fullscreen
  /// quad renderer
  struct SingleVertData {
    GL::Buffer vBuff{0};
    GL::VertexArray vao{0};
  } singleVertexData_;

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

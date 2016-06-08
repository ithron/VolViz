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
  void setupShaders();
  void setupFBOs();
  Eigen::Matrix4f projectionMatrix() const noexcept;
  Eigen::Matrix4f viewMatrix() const noexcept;
  void handleKeyInput(int key, int scancode, int action, int mode);

  void renderMeshes() const;

  void renderGrid() const;

  void renderFinalPass() const;

  GL::GLFW glfw_;
  GL::ShaderProgram geometryStageProgram_;
  GL::ShaderProgram displayStageProgram_;
  GL::ShaderProgram gridProgram_;
  GL::Textures<2> textures_{0};
  GL::Framebuffer fbo_{0};
  float fov = 150.f;

  struct MeshData {
    GL::VertexArray vao{0};
    GL::Buffer vertices{0};
    GL::Buffer indices{0};
    std::size_t nTriangles = 0;
  } mesh_;

  struct SingleVertData {
    GL::Buffer vBuff{0};
    GL::VertexArray vao{0};
  } singleVertexData_;

  enum class MoveState { None, Rotating } moveState_ = MoveState::None;

  Eigen::Vector2d lastMousePos_ = Eigen::Vector2d::Zero();

  Eigen::Vector3f cameraPosition_ = Eigen::Vector3f::Zero();
  Eigen::Quaternionf cameraOrientation_ = Eigen::Quaternionf::Identity();
};

} // namespace Private_
} // namespace VolViz

#endif // VolViz_VisualizerImpl_h

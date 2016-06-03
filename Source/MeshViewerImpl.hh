#ifndef SBSSegmentation_MeshViewerImpl_hh
#define SBSSegmentation_MeshViewerImpl_hh

#include "MeshViewer.hh"

#define GLFW_INCLUDE_GLEXT
#define GL_GLEXT_PROTOTYPES
#include <GLFW/glfw3.h>

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <array>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace MeshViewer {

/// Structure that handle GLFW state using RAII
struct GLFW {

  GLFW();
  GLFW(GLFW const &) = delete;
  GLFW(GLFW &&rhs);

  ~GLFW();

  void hide() noexcept;
  void show() noexcept;

  bool isHidden() const noexcept;

  void makeCurrent() noexcept;

  std::size_t width() const noexcept;
  std::size_t height() const noexcept;

  GLFWwindow *window = nullptr;
  std::function<void(int, int, int, int)> keyInputHandler;
  std::function<void(std::size_t, std::size_t)> windowResizeCallback;
  std::function<void(double, double)> scrollWheelInputHandler;
  std::function<void(int, int, int)> mouseButtonCallback;
  std::function<void(double, double)> mouseMoveCallback;
};

class ShaderProgram;

/// RAII Shader object
class Shader {
public:
  Shader(GLenum type, std::string const &source);

  ~Shader();

  Shader(Shader const &) = delete;
  Shader(Shader &&rhs);

  Shader &operator=(Shader &&);

private:
  void compile() const;

  friend class ShaderProgram;

  GLuint shader_;
};

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
/// RAII class for shader programs
class ShaderProgram {
public:
  ShaderProgram();

  ~ShaderProgram();

  ShaderProgram(ShaderProgram const &) = delete;
  ShaderProgram(ShaderProgram &&rhs);

  ShaderProgram &operator=(ShaderProgram &&rhs);

  ShaderProgram &attachShader(Shader const &shader);

  template <class Container> ShaderProgram &attachShaders(Container &&c) {
    for (auto const &s : std::forward<Container>(c)) attachShader(s);
  }

  ShaderProgram &link();

  void use() const;

private:
  void detachShaders();

  GLuint program_;
  std::vector<GLuint> attachedShaders_;
};

/// RAII Buffer wrapper
struct Buffer {
  inline Buffer(int) {}
  Buffer();

  ~Buffer();

  Buffer(Buffer const &) = delete;
  Buffer(Buffer &&rhs);

  Buffer &operator=(Buffer &&rhs);

  void bind(GLenum target) const;

  template <class T>
  void upload(GLenum target, std::size_t size, T const *data,
              GLbitfield flags) {
    bind(target);
    glBufferData(target, static_cast<GLsizeiptr>(size),
                 static_cast<GLvoid const *>(data), flags);
  }

  GLuint name;
};

/// RAII vertex array wrapper
struct VertexArray {
  inline VertexArray(int) {}
  VertexArray();

  ~VertexArray();

  VertexArray(VertexArray &&rhs);

  VertexArray &operator=(VertexArray &&rhs);

  VertexArray &bind();
  VertexArray &enableVertexAttribArray(GLuint idx);

  GLuint name = 0;
};

// RAII openGL texture wrapper
template <std::size_t N> struct Textures {
  GLuint names[N];

  inline Textures(int) {
    for (std::size_t i = 0; i < N; ++i) names[i] = 0;
  }
  inline Textures() { glGenTextures(N, names); }

  inline Textures(Textures &&rhs) {
    using std::swap;
    swap(names, rhs.names);
  }

  inline ~Textures() { glDeleteTextures(N, names); }

  inline Textures &operator=(Textures &&rhs) {
    using std::swap;
    swap(names, rhs.names);
    return *this;
  }
};

/// RAII OPenGL FBO wrapper
struct FramebufferObject {
  GLuint name = 0;

  inline FramebufferObject(int) {}
  inline FramebufferObject() { glGenFramebuffers(1, &name); }
  inline ~FramebufferObject() { glDeleteFramebuffers(1, &name); }
  inline FramebufferObject(FramebufferObject &&rhs) {
    using std::swap;
    swap(name, rhs.name);
  }
  inline FramebufferObject &operator=(FramebufferObject &&rhs) {
    using std::swap;
    swap(name, rhs.name);
    return *this;
  }
};

class MeshViewerImpl {
public:
  MeshViewerImpl();

  void start();

  void renderOneFrame();

  operator bool() const noexcept;

  template <class... Attrs> void setMesh(Viewer::Mesh<Attrs...> const &mesh);

private:
  void setupShaders();
  void setupFBOs();
  Eigen::Matrix4f projectionMatrix(std::size_t width, std::size_t height,
                                   float fov) const noexcept;
  void handleKeyInput(int key, int scancode, int action, int mode);

  GLFW glfw_;
  ShaderProgram geometryStageProgram_;
  ShaderProgram displayStageProgram_;
  ShaderProgram gridProgram_;
  Textures<2> textures_{0};
  FramebufferObject fbo_{0};

  struct {
    VertexArray vao{0};
    Buffer vertices{0};
    Buffer indices{0};
    std::size_t nTriangles = 0;
  } mesh_;

  enum class MoveState { None, Rotating } moveState_ = MoveState::None;

  Eigen::Vector2d lastMousePos_ = Eigen::Vector2d::Zero();

  Eigen::Vector3f cameraPosition_ = Eigen::Vector3f::UnitZ();
  Eigen::Quaternionf cameraOrientation_ = Eigen::Quaternionf::Identity();
};

} // namespace MeshViewer

#endif // SBSSegmentation_MeshViewerImpl_hh

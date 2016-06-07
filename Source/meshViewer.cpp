#include "MeshViewer.hh"
#include "MeshViewerImpl.hh"

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <Eigen/StdVector>

#include <array>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace MeshViewer {
namespace Shaders {

/// specify source codes of shader programs
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
static std::string const nullVertShaderSrc =
#include "Shaders/null.vert"
    ;
static std::string const fullscreenQuadGeomShaderSrc =
#include "Shaders/fullscreenQuad.geom"
    ;
static std::string const simpleTextureFragShaderSrc =
#include "Shaders/simpleTexture.frag"
    ;
static std::string const simpleVertShaderSrc =
#include "Shaders/simple.vert"
    ;
static std::string const passThroughFragShaderSrc =
#include "Shaders/passThrough.frag"
    ;

static std::string const gridGeometryShaderSrc =
#include "Shaders/grid.geom"
    ;
#pragma clang diagnostic pop

} // namespace Shaders

Viewer::Viewer() : impl_(std::make_unique<MeshViewerImpl>()) {}

Viewer::~Viewer() = default;

Viewer::Viewer(Viewer &&) = default;

Viewer &Viewer::operator=(Viewer &&) = default;

void Viewer::start() { impl_->start(); }

Viewer::operator bool() const noexcept { return *impl_; }

void Viewer::renderOneFrame() { impl_->renderOneFrame(); }

template <class VertBase, class IdxBase>
void Viewer::setMesh(Eigen::MatrixBase<VertBase> const &V,
                     Eigen::MatrixBase<IdxBase> const &I) {
  impl_->setMesh(V, I);
}

GLFW::GLFW() {
  if (!glfwInit()) throw(std::runtime_error("Failed to init GLFW"));

  glfwWindowHint(GLFW_VISIBLE, false);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  if (!(window = glfwCreateWindow(Viewer::kDefaultWidth, Viewer::kDefaultHeight,
                                  Viewer::kTitle, nullptr, nullptr))) {
    throw std::runtime_error("Failed to create GLFW window");
  }

  glfwSetWindowUserPointer(window, this);

  // setup key callback handler
  glfwSetKeyCallback(
      window, [](GLFWwindow *win, int key, int scancode, int action, int mode) {
        auto ptr = glfwGetWindowUserPointer(win);
        assert(ptr != nullptr && "Invalid user pointer");
        auto *const self = static_cast<GLFW *>(ptr);
        if (self->keyInputHandler)
          self->keyInputHandler(key, scancode, action, mode);
      });
  // setup window resize callback
  glfwSetWindowSizeCallback(window, [](GLFWwindow *win, int w, int h) {
    auto ptr = glfwGetWindowUserPointer(win);
    assert(ptr != nullptr && "Invalid user pointer");
    auto *const self = static_cast<GLFW *>(ptr);
    if (self->windowResizeCallback)
      self->windowResizeCallback(static_cast<std::size_t>(w),
                                 static_cast<std::size_t>(h));
  });

  // setup scroll wheel input
  glfwSetScrollCallback(window, [](GLFWwindow *win, double x, double y) {
    auto ptr = glfwGetWindowUserPointer(win);
    assert(ptr != nullptr && "Invalid user pointer");
    auto *const self = static_cast<GLFW *>(ptr);
    if (self->scrollWheelInputHandler) self->scrollWheelInputHandler(x, y);
  });

  // setup mouse button callback
  glfwSetMouseButtonCallback(window, [](GLFWwindow *win, int b, int a, int m) {
    auto ptr = glfwGetWindowUserPointer(win);
    assert(ptr != nullptr && "Invalid user pointer");
    auto *const self = static_cast<GLFW *>(ptr);
    if (self->mouseButtonCallback) self->mouseButtonCallback(b, a, m);
  });

  // setup mouse move callback
  glfwSetCursorPosCallback(window, [](GLFWwindow *win, double x, double y) {
    auto ptr = glfwGetWindowUserPointer(win);
    assert(ptr != nullptr && "Invalid user pointer");
    auto *const self = static_cast<GLFW *>(ptr);
    if (self->mouseMoveCallback) self->mouseMoveCallback(x, y);
  });

  makeCurrent();
  glfwSwapInterval(1);
}

GLFW::GLFW(GLFW &&rhs) : window(rhs.window) { rhs.window = nullptr; }

GLFW::~GLFW() {
  if (window) glfwDestroyWindow(window);
  glfwTerminate();
}

void GLFW::hide() noexcept { glfwHideWindow(window); }

void GLFW::show() noexcept { glfwShowWindow(window); }

bool GLFW::isHidden() const noexcept {
  return glfwGetWindowAttrib(window, GLFW_VISIBLE) == 0;
}

void GLFW::makeCurrent() noexcept { glfwMakeContextCurrent(window); }

std::size_t GLFW::width() const noexcept {
  int w, h;
  glfwGetWindowSize(window, &w, &h);
  if (!isHidden()) return static_cast<std::size_t>(w);
  return Viewer::kDefaultWidth;
}

std::size_t GLFW::height() const noexcept {
  int w, h;
  glfwGetWindowSize(window, &w, &h);
  if (!isHidden()) return static_cast<std::size_t>(h);
  return Viewer::kDefaultHeight;
}

Shader::Shader(GLenum type, std::string const &source) {
  shader_ = glCreateShader(type);
  assertGL("Failed to create shader");
  auto const src = source.c_str();
  glShaderSource(shader_, 1, &src, nullptr);
  assertGL("Failed to upload shader source");
  compile();
}

Shader::~Shader() { glDeleteShader(shader_); }

Shader::Shader(Shader &&rhs) {
  using std::swap;
  swap(shader_, rhs.shader_);
}

Shader &Shader::operator=(Shader &&rhs) {
  using std::swap;
  swap(shader_, rhs.shader_);
  return *this;
}

void Shader::compile() const {
  glCompileShader(shader_);
  assertGL("Shader compilation failed");

  GLint success = 0;
  glGetShaderiv(shader_, GL_COMPILE_STATUS, &success);
  assertGL("Failed to get shader compile status");

  if (success) {
    std::cout << "Sucessfully compiled shader " << shader_ << std::endl;
    return;
  }

  GLint logSize = 0;
  glGetShaderiv(shader_, GL_INFO_LOG_LENGTH, &logSize);
  assertGL("Failed to get shader info log length");
  std::vector<GLchar> log(static_cast<std::size_t>(logSize));

  glGetShaderInfoLog(shader_, logSize, &logSize, log.data());
  assertGL("Failed to get shader info log");

  std::cerr << "Failed to compile shader " << shader_ << ":" << std::endl
            << std::string(log.data()) << std::endl;
  throw std::runtime_error("Shader compilation failed");
}

ShaderProgram::ShaderProgram() {
  program_ = glCreateProgram();
  assert(program_ != 0 && "Shader program creationf ailed");
}

ShaderProgram::~ShaderProgram() {
  detachShaders();
  glDeleteProgram(program_);
}

ShaderProgram::ShaderProgram(ShaderProgram &&rhs)
    : program_(rhs.program_),
      attachedShaders_(std::move(rhs.attachedShaders_)) {
  rhs.program_ = 0;
}

ShaderProgram &ShaderProgram::operator=(ShaderProgram &&rhs) {
  using std::swap;
  swap(program_, rhs.program_);
  swap(attachedShaders_, rhs.attachedShaders_);
  return *this;
}

ShaderProgram &ShaderProgram::attachShader(Shader const &shader) {
  glAttachShader(program_, shader.shader_);
  assertGL("Shader attachment failed");
  attachedShaders_.push_back(shader.shader_);
  return *this;
}

ShaderProgram &ShaderProgram::link() {
  glLinkProgram(program_);
  assertGL("Failed to link program");

  // Check for linking errors
  GLint isLinked = GL_FALSE;
  glGetProgramiv(program_, GL_LINK_STATUS, &isLinked);

  if (!isLinked) {
    GLint maxLength = 0;
    glGetProgramiv(program_, GL_INFO_LOG_LENGTH, &maxLength);
    std::vector<GLchar> infoLog(static_cast<std::size_t>(maxLength));
    glGetProgramInfoLog(program_, maxLength, &maxLength, infoLog.data());

    throw std::runtime_error("Failed to link program: " +
                             std::string(infoLog.data()));
  }

  std::cout << "Sucessfully linked program " << program_ << std::endl;
  detachShaders();
  return *this;
}

void ShaderProgram::use() const {
  assertGL("Dirty OpenGL error stack");
  glUseProgram(program_);
  assertGL("Failed to use program");
}

void ShaderProgram::detachShaders() {
  for (auto s : attachedShaders_) glDetachShader(program_, s);
  attachedShaders_.clear();
}

Buffer::Buffer() {
  glGenBuffers(1, &name);
  assertGL("Buffer creation failed");
}

Buffer::~Buffer() { glDeleteBuffers(1, &name); }

Buffer::Buffer(Buffer &&rhs) : name(rhs.name) { rhs.name = 0; }

Buffer &Buffer::operator=(Buffer &&rhs) {
  name = rhs.name;
  rhs.name = 0;
  return *this;
}

void Buffer::bind(GLenum target) const {
  glBindBuffer(target, name);
  assertGL("Failed to bind buffer");
}

VertexArray::VertexArray() {
  glGenVertexArrays(1, &name);
  assertGL("Vertex array creation failed");
}

VertexArray::~VertexArray() { glDeleteVertexArrays(1, &name); };

VertexArray::VertexArray(VertexArray &&rhs) : name(rhs.name) { rhs.name = 0; }

VertexArray &VertexArray::operator=(VertexArray &&rhs) {
  name = rhs.name;
  rhs.name = 0;
  return *this;
}

VertexArray &VertexArray::bind() {
  glBindVertexArray(name);
  assertGL("Failed to bind vertex array");
  return *this;
}

VertexArray &VertexArray::enableVertexAttribArray(GLuint idx) {
  bind();
  glEnableVertexAttribArray(idx);
  assertGL("Failed to enabkle vertex attribute");
  return *this;
}

MeshViewerImpl::MeshViewerImpl() {

  // check OpenGL version
  GLint major, minor;
  glGetIntegerv(GL_MAJOR_VERSION, &major);
  glGetIntegerv(GL_MINOR_VERSION, &minor);

  std::cout << "OpenGL " << major << "." << minor << std::endl;
  if (major < 4 || !(major == 4 && minor >= 1)) {
    throw std::runtime_error("Need at least OpenGL version 4.1");
  }

  setupShaders();
  setupFBOs();
  glfw_.keyInputHandler =
      [this](int k, int s, int a, int m) { handleKeyInput(k, s, a, m); };

  glfw_.windowResizeCallback = [this](auto, auto) {
    setupFBOs();
    int w, h;
    glfwGetFramebufferSize(glfw_.window, &w, &h);
    glViewport(0, 0, w, h);
  };

  glfw_.scrollWheelInputHandler = [this](double, double y) {
    cameraPosition_(2) -= 2 * static_cast<float>(y);
  };

  glfw_.mouseButtonCallback = [this](int button, int action, int) {
    switch (moveState_) {
      case MoveState::None:
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
          moveState_ = MoveState::Rotating;
        break;
      case MoveState::Rotating:
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
          moveState_ = MoveState::None;
        break;
    }
  };

  glfw_.mouseMoveCallback = [this](double x, double y) {
    using Eigen::Vector2d;
    using Eigen::Vector3d;
    using Eigen::Vector3f;
    using std::sqrt;
    using std::acos;
    using std::min;
    auto const pos =
        Vector2d(2 * x / static_cast<double>(glfw_.width()) - 1.0,
                 -2 * y / static_cast<double>(glfw_.height()) + 1.0);
    auto const pos3 = Vector3d(pos(0), pos(1), sqrt(1.0 - pos.squaredNorm()));
    auto const lastPos3 = Vector3d(lastMousePos_(0), lastMousePos_(1),
                                   sqrt(1.0 - lastMousePos_.squaredNorm()));

    switch (moveState_) {
      case MoveState::Rotating: {
        auto const angle =
            acos(min(1.0, static_cast<double>(pos3.transpose() * lastPos3))) *
            2;
        Vector3f const axis = lastPos3.cross(pos3).normalized().cast<float>();
        if (angle > 1e-3) {
          cameraOrientation_ *=
              Eigen::Quaternionf(
                  Eigen::AngleAxisf(static_cast<float>(angle), axis)).inverse();
          cameraOrientation_.normalize();
        }
        break;
      }
      case MoveState::None:
        break;
    }

    lastMousePos_ = pos;
  };
}

void MeshViewerImpl::setupShaders() {
  auto dispProg = std::move(
      ShaderProgram()
          .attachShader(Shader(GL_VERTEX_SHADER, Shaders::nullVertShaderSrc))
          .attachShader(
               Shader(GL_GEOMETRY_SHADER, Shaders::fullscreenQuadGeomShaderSrc))
          .attachShader(
               Shader(GL_FRAGMENT_SHADER, Shaders::simpleTextureFragShaderSrc))
          .link());
  auto geomProg = std::move(
      ShaderProgram()
          .attachShader(Shader(GL_VERTEX_SHADER, Shaders::simpleVertShaderSrc))
          .attachShader(
               Shader(GL_FRAGMENT_SHADER, Shaders::passThroughFragShaderSrc))
          .link());

  auto gridProg = std::move(
      ShaderProgram()
          .attachShader(Shader(GL_VERTEX_SHADER, Shaders::nullVertShaderSrc))
          .attachShader(
               Shader(GL_GEOMETRY_SHADER, Shaders::gridGeometryShaderSrc))
          .attachShader(
               Shader(GL_FRAGMENT_SHADER, Shaders::passThroughFragShaderSrc))
          .link());

  geometryStageProgram_ = std::move(geomProg);
  displayStageProgram_ = std::move(dispProg);
  gridProgram_ = std::move(gridProg);
}

void MeshViewerImpl::setupFBOs() {

  // set up FBO
  Textures<2> textures;
  // color texture
  glBindTexture(GL_TEXTURE_2D, textures.names[0]);
  glTexStorage2D(GL_TEXTURE_2D, 1, GL_SRGB8_ALPHA8,
                 static_cast<GLsizei>(glfw_.width()),
                 static_cast<GLsizei>(glfw_.height()));
  // depth and stencil texture
  glBindTexture(GL_TEXTURE_2D, textures.names[1]);
  glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8,
                 static_cast<GLsizei>(glfw_.width()),
                 static_cast<GLsizei>(glfw_.height()));
  // setup FBO
  FramebufferObject fbo;
  glBindFramebuffer(GL_FRAMEBUFFER, fbo.name);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         textures.names[0], 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                         GL_TEXTURE_2D, textures.names[1], 0);
  // check FBO
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    throw std::runtime_error("Incomplete FBO");
  }

  textures_ = std::move(textures);
  fbo_ = std::move(fbo);

  // create dummy VAO for single single point rendering
  {
    auto vao = VertexArray{};
    vao.enableVertexAttribArray(0);

    auto const vert = Eigen::Vector3f::Zero().eval();
    auto vb = Buffer{};
    vb.upload(GL_ARRAY_BUFFER, 3 * sizeof(float), vert.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, nullptr);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    singleVertexData_.vBuff = std::move(vb);
    singleVertexData_.vao = std::move(vao);
  }
  // DEBUG END
}

Eigen::Matrix4f MeshViewerImpl::projectionMatrix(std::size_t width,
                                                 std::size_t height,
                                                 float fov) const noexcept {
  auto constexpr pi = static_cast<float>(M_PI);
  auto const fovy = 1.f / std::tan(fov * pi / 360.f);
  auto const aspect = static_cast<float>(width) / static_cast<float>(height);

  auto P = Eigen::Matrix4f();
  P << fovy / aspect, 0, 0, 0, 0, fovy, 0, 0, 0, 0, 0, -1, 0, 0, -1, 0;

  return P;
}

void MeshViewerImpl::start() { glfw_.show(); }

MeshViewerImpl::operator bool() const noexcept {
  return !glfwWindowShouldClose(glfw_.window);
}

template <class VertBase, class IdxBase>
void MeshViewerImpl::setMesh(Eigen::MatrixBase<VertBase> const &V,
                             Eigen::MatrixBase<IdxBase> const &I) {
  using Verts =
      std::vector<Eigen::Vector4f, Eigen::aligned_allocator<Eigen::Vector4f>>;
  using Triangle = Eigen::Matrix<std::uint32_t, 3, 1>;
  using Triangles = std::vector<Triangle>;

  auto verts = Verts();
  verts.reserve(static_cast<std::size_t>(V.rows()));
  auto indices = Triangles();
  indices.reserve(static_cast<std::size_t>(I.rows()));

  for (auto i = 0; i < V.rows(); ++i)
    verts.emplace_back(V.row(i).template cast<float>().homogeneous());
  for (auto i = 0; i < I.rows(); ++i)
    indices.emplace_back(I.row(i).template cast<std::uint32_t>());

  // setup VBO
  auto vertBuff = Buffer();
  vertBuff.upload(GL_ARRAY_BUFFER,
                  verts.size() * sizeof(typename decltype(verts)::value_type),
                  verts.data(), GL_STATIC_DRAW);
  auto idxBuff = Buffer();
  idxBuff.upload(GL_ELEMENT_ARRAY_BUFFER,
                 indices.size() *
                     sizeof(typename decltype(indices)::value_type),
                 indices.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  // setup VAO
  auto vao = VertexArray();
  vao.enableVertexAttribArray(0);
  //  vao.enableVertexAttribArray(1);
  vertBuff.bind(GL_ARRAY_BUFFER);
  glVertexAttribPointer(0, 4, GL_FLOAT, false, 0, nullptr);
  idxBuff.bind(GL_ELEMENT_ARRAY_BUFFER);
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  mesh_.vertices = std::move(vertBuff);
  mesh_.indices = std::move(idxBuff);
  mesh_.vao = std::move(vao);
  mesh_.nTriangles = indices.size();
}

template void Viewer::setMesh<>(Eigen::MatrixBase<Eigen::MatrixXd> const &,
                                Eigen::MatrixBase<Eigen::MatrixXi> const &);

template void
MeshViewerImpl::setMesh<>(Eigen::MatrixBase<Eigen::MatrixXd> const &,
                          Eigen::MatrixBase<Eigen::MatrixXi> const &);

void MeshViewerImpl::handleKeyInput(int key, int, int action, int) {
  if (action == GLFW_PRESS || action == GLFW_REPEAT) {
    switch (key) {
      case GLFW_KEY_DOWN:
        cameraPosition_(2) += 0.1;
        break;
      case GLFW_KEY_UP:
        cameraPosition_(2) -= 0.1;
        break;
    }
  }
}

void MeshViewerImpl::renderOneFrame() {

  glfw_.makeCurrent();

  auto const pMatrix =
      // projectionMatrix(glfw_.width(), glfw_.height(), Viewer::kDefaultFOV);
      projectionMatrix(glfw_.width(), glfw_.height(), 90);
  assert(glfw_.window && "invalid window handle");
  Eigen::Transform<float, 3, Eigen::Affine> camTrans =
      cameraOrientation_ * Eigen::Translation3f(cameraPosition_) *
      Eigen::AngleAxisf(0.f, Eigen::Vector3f::UnitZ());
  Eigen::Matrix4f const MVP = pMatrix * camTrans.inverse().matrix();

  assertGL("OpenGL Error stack not clear");
  glBindFramebuffer(GL_FRAMEBUFFER, fbo_.name);
  //    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  assertGL("Failed to bind framebuffer");
  glClearColor(0.f, 0.4f, 0.4f, 1.f);
  glClearDepth(1.0);
  glClearStencil(0);
  glEnable(GL_FRAMEBUFFER_SRGB);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);

  assert(glfwGetCurrentContext() == glfw_.window && "Context not current");
  // Render geometry to FBO
  geometryStageProgram_.use();
  assertGL("OpenGL Error stack not clean");

  glUniformMatrix4fv(
      glGetUniformLocation(gridProgram_.program_, "modelViewProjectionMatrix"),
      1, false, MVP.data());
  assertGL("Failed to upload uniform");
  if (mesh_.vao.name != 0) {
    std::cout << "Render mesh" << std::endl;
    mesh_.vao.bind();
    glDrawElements(GL_TRIANGLES, 3 * static_cast<GLsizei>(mesh_.nTriangles),
                   GL_UNSIGNED_INT, nullptr);
    assertGL("glDrawElements failed");
  }
  glBindVertexArray(0);

  // TODO: Draw grid if enabled
  std::cout << "grid shader: " << gridProgram_.program_ << std::endl;
  gridProgram_.use();
  assertGL("OpenGL Error stack not clear");
  glUniform1f(glGetUniformLocation(gridProgram_.program_, "scale"), 2.f);
  assertGL("Failed to upload scale uniform");
  glUniformMatrix4fv(
      glGetUniformLocation(gridProgram_.program_, "viewProjectionMatrix"), 1,
      false, MVP.data());
  assertGL("Failed to upload MVP matrix");
  singleVertexData_.vao.bind();
  glDrawArrays(GL_POINTS, 0, 1);
  assertGL("glDrawArrays failed");
  glBindVertexArray(0);

  // Render FBA color attachment to screen
  glDisable(GL_DEPTH_TEST);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  displayStageProgram_.use();
  auto const loc = glGetUniformLocation(displayStageProgram_.program_, "tex");
  glUniform1i(loc, 0);
  assertGL("glUniform1i failed");
  glActiveTexture(GL_TEXTURE0 + 0);
  glBindTexture(GL_TEXTURE_2D, textures_.names[0]);

  // draw fullscreen quad using the geometry shader
  singleVertexData_.vao.bind();
  assertGL("Dirty openGL state");
  glDrawArrays(GL_POINTS, 0, 1);
  assertGL("glDrawArrays failed");
  glBindVertexArray(0);

  glfwSwapBuffers(glfw_.window);
  glfwWaitEvents();
}

} // namespace MeshViewer

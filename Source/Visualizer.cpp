#include "VisualizerImpl.h"

#include "GL.h"
#include "Visualizer.h"

#include <iostream>

namespace VolViz {

////////////////////////////////
// VisualizerImpl
//

namespace Private_ {

VisualizerImpl::VisualizerImpl() {

  glfw_.makeCurrent();

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
    glViewport(0, 0, static_cast<GLsizei>(glfw_.width()),
               static_cast<GLsizei>(glfw_.height()));
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

void VisualizerImpl::setupShaders() {
  auto dispProg = std::move(
      GL::ShaderProgram()
          .attachShader(
               GL::Shader(GL_VERTEX_SHADER, GL::Shaders::nullVertShaderSrc))
          .attachShader(GL::Shader(GL_GEOMETRY_SHADER,
                                   GL::Shaders::fullscreenQuadGeomShaderSrc))
          .attachShader(GL::Shader(GL_FRAGMENT_SHADER,
                                   GL::Shaders::simpleTextureFragShaderSrc))
          .link());
  auto geomProg = std::move(
      GL::ShaderProgram()
          .attachShader(
               GL::Shader(GL_VERTEX_SHADER, GL::Shaders::simpleVertShaderSrc))
          .attachShader(GL::Shader(GL_FRAGMENT_SHADER,
                                   GL::Shaders::passThroughFragShaderSrc))
          .link());

  auto gridProg = std::move(
      GL::ShaderProgram()
          .attachShader(
               GL::Shader(GL_VERTEX_SHADER, GL::Shaders::nullVertShaderSrc))
          .attachShader(GL::Shader(GL_GEOMETRY_SHADER,
                                   GL::Shaders::gridGeometryShaderSrc))
          .attachShader(GL::Shader(GL_FRAGMENT_SHADER,
                                   GL::Shaders::passThroughFragShaderSrc))
          .link());

  geometryStageProgram_ = std::move(geomProg);
  displayStageProgram_ = std::move(dispProg);
  gridProgram_ = std::move(gridProg);
}

void VisualizerImpl::setupFBOs() {

  // set up FBO
  GL::Textures<2> textures;
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
  GL::Framebuffer fbo;
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
    auto vao = GL::VertexArray{};
    vao.enableVertexAttribArray(0);

    auto const vert = Eigen::Vector3f::Zero().eval();
    auto vb = GL::Buffer{};
    vb.upload(GL_ARRAY_BUFFER, 3 * sizeof(float), vert.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, nullptr);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    singleVertexData_.vBuff = std::move(vb);
    singleVertexData_.vao = std::move(vao);
  }
  // DEBUG END
}

Eigen::Matrix4f VisualizerImpl::projectionMatrix(std::size_t width,
                                                 std::size_t height,
                                                 float fov) const noexcept {
  auto constexpr pi = static_cast<float>(M_PI);
  auto const fovy = 1.f / std::tan(fov * pi / 360.f);
  auto const aspect = static_cast<float>(width) / static_cast<float>(height);

  auto P = Eigen::Matrix4f();
  P << fovy / aspect, 0, 0, 0, 0, fovy, 0, 0, 0, 0, 0, -1, 0, 0, -1, 0;

  return P;
}

void VisualizerImpl::start() { glfw_.show(); }

VisualizerImpl::operator bool() const noexcept { return glfw_; }

template <class VertBase, class IdxBase>
void VisualizerImpl::setMesh(Eigen::MatrixBase<VertBase> const &V,
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
  auto vertBuff = GL::Buffer();
  vertBuff.upload(GL_ARRAY_BUFFER,
                  verts.size() * sizeof(typename decltype(verts)::value_type),
                  verts.data(), GL_STATIC_DRAW);
  auto idxBuff = GL::Buffer();
  idxBuff.upload(GL_ELEMENT_ARRAY_BUFFER,
                 indices.size() *
                     sizeof(typename decltype(indices)::value_type),
                 indices.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  // setup VAO
  auto vao = GL::VertexArray();
  vao.enableVertexAttribArray(0);
  vertBuff.bind(GL_ARRAY_BUFFER);
  glVertexAttribPointer(0, 3, GL_FLOAT, false, 4 * sizeof(float), nullptr);
  idxBuff.bind(GL_ELEMENT_ARRAY_BUFFER);
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  mesh_.vertices = std::move(vertBuff);
  mesh_.indices = std::move(idxBuff);
  mesh_.vao = std::move(vao);
  mesh_.nTriangles = indices.size();
}

template void
VisualizerImpl::setMesh<>(Eigen::MatrixBase<Eigen::MatrixXd> const &,
                          Eigen::MatrixBase<Eigen::MatrixXi> const &);

void VisualizerImpl::handleKeyInput(int key, int, int action, int) {
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

void VisualizerImpl::renderOneFrame() {

  glfw_.makeCurrent();

  auto const pMatrix =
      // projectionMatrix(glfw_.width(), glfw_.height(), Viewer::kDefaultFOV);
      projectionMatrix(glfw_.width(), glfw_.height(), 90);

  Eigen::Transform<float, 3, Eigen::Affine> camTrans =
      cameraOrientation_ * Eigen::Translation3f(cameraPosition_) *
      Eigen::AngleAxisf(0.f, Eigen::Vector3f::UnitZ());
  Eigen::Matrix4f const MVP = pMatrix * camTrans.inverse().matrix();

  assertGL("OpenGL Error stack not clear");
  glBindFramebuffer(GL_FRAMEBUFFER, fbo_.name);
  assertGL("Failed to bind framebuffer");
  glClearColor(0.f, 0.4f, 0.4f, 1.f);
  glClearDepth(1.0);
  glClearStencil(0);
  glEnable(GL_FRAMEBUFFER_SRGB);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);

  // Render geometry to FBO
  geometryStageProgram_.use();
  assertGL("OpenGL Error stack not clean");

  geometryStageProgram_["modelViewProjectionMatrix"] = MVP;
  if (mesh_.vao.name != 0) {
    mesh_.vao.bind();
    glDrawElements(GL_TRIANGLES, 3 * static_cast<GLsizei>(mesh_.nTriangles),
                   GL_UNSIGNED_INT, nullptr);
    assertGL("glDrawElements failed");
  }
  glBindVertexArray(0);

  // TODO: Draw grid if enabled
  gridProgram_.use();
  gridProgram_["scale"] = 2.f;
  gridProgram_["viewProjectionMatrix"] = MVP;

  singleVertexData_.vao.bind();
  glDrawArrays(GL_POINTS, 0, 1);
  assertGL("glDrawArrays failed");
  glBindVertexArray(0);

  // Render FBA color attachment to screen
  glDisable(GL_DEPTH_TEST);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  displayStageProgram_.use();
  displayStageProgram_["tex"] = 0;
  glActiveTexture(GL_TEXTURE0 + 0);
  glBindTexture(GL_TEXTURE_2D, textures_.names[0]);

  // draw fullscreen quad using the geometry shader
  singleVertexData_.vao.bind();
  glDrawArrays(GL_POINTS, 0, 1);
  assertGL("glDrawArrays failed");
  glBindVertexArray(0);

  glfw_.swapBuffers();
  glfw_.waitEvents();
}

} // namespace Private_

/////////////////////////////
// Visualizer
//

Visualizer::Visualizer()
    : impl_(std::make_unique<Private_::VisualizerImpl>()) {}

Visualizer::~Visualizer() = default;

Visualizer::Visualizer(Visualizer &&) = default;

Visualizer &Visualizer::operator=(Visualizer &&) = default;

void Visualizer::start() { impl_->start(); }

Visualizer::operator bool() const noexcept { return *impl_; }

void Visualizer::renderOneFrame() { impl_->renderOneFrame(); }

template <class VertBase, class IdxBase>
void Visualizer::setMesh(Eigen::MatrixBase<VertBase> const &V,
                         Eigen::MatrixBase<IdxBase> const &I) {
  impl_->setMesh(V, I);
}

template void Visualizer::setMesh<>(Eigen::MatrixBase<Eigen::MatrixXd> const &,
                                    Eigen::MatrixBase<Eigen::MatrixXi> const &);

} // namespace VolViz

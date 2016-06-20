#include "VisualizerImpl.h"

#include "GL.h"
#include "Visualizer.h"

#include <Eigen/Core>
#include <gsl.h>

#include <iostream>
#include <mutex>

namespace VolViz {

////////////////////////////////
// VisualizerImpl
//

namespace Private_ {

VisualizerImpl::VisualizerImpl(Visualizer *vis) : visualizer_(vis) {

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
    this->setupFBOs(); // this is used explicitly here because gcc complains
                       // otherwise, although it's perectly standard conformant
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
  auto quadProg = std::move(
      GL::ShaderProgram()
          .attachShader(
               GL::Shader(GL_VERTEX_SHADER, GL::Shaders::nullVertShaderSrc))
          .attachShader(
               GL::Shader(GL_GEOMETRY_SHADER, GL::Shaders::quadGeomShaderSrc))
          .attachShader(GL::Shader(GL_FRAGMENT_SHADER,
                                   GL::Shaders::simpleTextureFragShaderSrc))
          .link());

  auto hdrQuadProg = std::move(
      GL::ShaderProgram()
          .attachShader(
               GL::Shader(GL_VERTEX_SHADER, GL::Shaders::nullVertShaderSrc))
          .attachShader(
               GL::Shader(GL_GEOMETRY_SHADER, GL::Shaders::quadGeomShaderSrc))
          .attachShader(GL::Shader(GL_FRAGMENT_SHADER,
                                   GL::Shaders::hdrTextureFragShaderSrc))
          .link());

  auto normalQuadProg =
      std::move(GL::ShaderProgram()
                    .attachShader(GL::Shader(GL_VERTEX_SHADER,
                                             GL::Shaders::nullVertShaderSrc))
                    .attachShader(GL::Shader(GL_GEOMETRY_SHADER,
                                             GL::Shaders::quadGeomShaderSrc))
                    .attachShader(GL::Shader(
                        GL_FRAGMENT_SHADER,
                        GL::Shaders::normalVisualizationFragShaderSrc))
                    .link());

  auto depthQuadProg = std::move(
      GL::ShaderProgram()
          .attachShader(
               GL::Shader(GL_VERTEX_SHADER, GL::Shaders::nullVertShaderSrc))
          .attachShader(
               GL::Shader(GL_GEOMETRY_SHADER, GL::Shaders::quadGeomShaderSrc))
          .attachShader(GL::Shader(
              GL_FRAGMENT_SHADER, GL::Shaders::depthVisualizationFragShaderSrc))
          .link());

  auto specularQuadProg =
      std::move(GL::ShaderProgram()
                    .attachShader(GL::Shader(GL_VERTEX_SHADER,
                                             GL::Shaders::nullVertShaderSrc))
                    .attachShader(GL::Shader(GL_GEOMETRY_SHADER,
                                             GL::Shaders::quadGeomShaderSrc))
                    .attachShader(GL::Shader(
                        GL_FRAGMENT_SHADER,
                        GL::Shaders::specularVisualizationFragShaderSrc))
                    .link());

  auto ambientPassProg = std::move(
      GL::ShaderProgram()
          .attachShader(
               GL::Shader(GL_VERTEX_SHADER, GL::Shaders::nullVertShaderSrc))
          .attachShader(
               GL::Shader(GL_GEOMETRY_SHADER, GL::Shaders::quadGeomShaderSrc))
          .attachShader(GL::Shader(GL_FRAGMENT_SHADER,
                                   GL::Shaders::ambientPassFragShaderSrc))
          .link());

  auto diffLightingPassProg =
      std::move(GL::ShaderProgram()
                    .attachShader(GL::Shader(GL_VERTEX_SHADER,
                                             GL::Shaders::nullVertShaderSrc))
                    .attachShader(GL::Shader(GL_GEOMETRY_SHADER,
                                             GL::Shaders::quadGeomShaderSrc))
                    .attachShader(GL::Shader(
                        GL_FRAGMENT_SHADER,
                        GL::Shaders::diffuseLightingPassFragShaderSrc))
                    .link());

  auto specLightingPassProg =
      std::move(GL::ShaderProgram()
                    .attachShader(GL::Shader(GL_VERTEX_SHADER,
                                             GL::Shaders::nullVertShaderSrc))
                    .attachShader(GL::Shader(GL_GEOMETRY_SHADER,
                                             GL::Shaders::quadGeomShaderSrc))
                    .attachShader(GL::Shader(
                        GL_FRAGMENT_SHADER,
                        GL::Shaders::specularLightingPassFragShaderSrc))
                    .link());

  auto geomProg = std::move(
      GL::ShaderProgram()
          .attachShader(GL::Shader(GL_VERTEX_SHADER,
                                   GL::Shaders::deferredVertexShaderSrc))
          .attachShader(
               GL::Shader(GL_FRAGMENT_SHADER,
                          GL::Shaders::deferredPassthroughFragShaderSrc))
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
  quadProgram_ = std::move(quadProg);
  gridProgram_ = std::move(gridProg);
  normalQuadProgram_ = std::move(normalQuadProg);
  depthQuadProgram_ = std::move(depthQuadProg);
  ambientPassProgram_ = std::move(ambientPassProg);
  diffuseLightingPassProgram_ = std::move(diffLightingPassProg);
  specularLightingPassProgram_ = std::move(specLightingPassProg);
  specularQuadProgram_ = std::move(specularQuadProg);
  hdrQuadProgram_ = std::move(hdrQuadProg);
}

void VisualizerImpl::setupFBOs() {

  auto const width = static_cast<GLsizei>(glfw_.width());
  auto const height = static_cast<GLsizei>(glfw_.height());
  // set up FBO

  { // textures and FBO for the geometry stage
    // normal and specular texture
    glBindTexture(GL_TEXTURE_2D, textures_[TextureID::NormalsAndSpecular]);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width, height);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // albedo texure
    glBindTexture(GL_TEXTURE_2D, textures_[TextureID::Albedo]);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_SRGB8_ALPHA8, width, height);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // depth and stencil texture
    glBindTexture(GL_TEXTURE_2D, textures_[TextureID::Depth]);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT24, width, height);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // setup FBO
    GL::Framebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo.name);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           textures_[TextureID::NormalsAndSpecular], 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
                           textures_[TextureID::Albedo], 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                           textures_[TextureID::Depth], 0);
    // check FBO
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      throw std::runtime_error(
          "Incomplete FBO: " +
          std::to_string(glCheckFramebufferStatus(GL_FRAMEBUFFER)));
    }

    lightingFbo_ = std::move(fbo);
  }

  { // textures and fbo for the lighting
    // final rendered image texture
    glBindTexture(GL_TEXTURE_2D, textures_[TextureID::RenderedImage]);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width, height);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // setup FBO
    GL::Framebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo.name);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           textures_[TextureID::RenderedImage], 0);
    // check FBO
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      throw std::runtime_error(
          "Incomplete FBO: " +
          std::to_string(glCheckFramebufferStatus(GL_FRAMEBUFFER)));
    }

    finalFbo_ = std::move(fbo);
  }

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
}

Eigen::Matrix4f VisualizerImpl::projectionMatrix() const noexcept {
  auto const width = glfw_.width();
  auto const height = glfw_.height();
  auto constexpr pi = static_cast<float>(M_PI);
  auto const fovy = 1.f / std::tan(fov * pi / 360.f);
  auto const aspect = static_cast<float>(width) / static_cast<float>(height);

  auto P = Eigen::Matrix4f();
  P << fovy / aspect, 0, 0, 0, 0, fovy, 0, 0, 0, 0, 0, -1, 0, 0, -1, 0;

  return P;
}

Eigen::Matrix4f VisualizerImpl::viewMatrix() const noexcept {
  Eigen::Transform<float, 3, Eigen::Affine> camTrans =
      (cameraOrientation_ * Eigen::Translation3f(cameraPosition_)).inverse();

  return camTrans.matrix();
}

void VisualizerImpl::start() { glfw_.show(); }

VisualizerImpl::operator bool() const noexcept { return glfw_; }

template <class VertBase, class IdxBase>
void VisualizerImpl::setMesh(Eigen::MatrixBase<VertBase> const &V,
                             Eigen::MatrixBase<IdxBase> const &I) {
  using Vertex = Eigen::Matrix<float, 8, 1>;
  using Verts = std::vector<Vertex, Eigen::aligned_allocator<Vertex>>;
  using Triangle = Eigen::Matrix<std::uint32_t, 3, 1>;
  using Triangles = std::vector<Triangle>;

  auto verts = Verts();
  verts.reserve(static_cast<std::size_t>(V.rows()));
  auto indices = Triangles();
  indices.reserve(static_cast<std::size_t>(I.rows()));

  for (auto i = 0; i < V.rows(); ++i) {
    Vertex v = Vertex::Zero();
    v.head<3>() = V.row(i).template cast<float>();
    verts.emplace_back(std::move(v));
  }
  for (auto i = 0; i < I.rows(); ++i) {
    Triangle t = I.row(i).template cast<std::uint32_t>();

    // compute normal
    auto &v0 = verts[t(0)];
    auto &v1 = verts[t(1)];
    auto &v2 = verts[t(2)];

    auto const normal =
        (v1 - v0).head<3>().cross((v2 - v0).head<3>()).normalized().eval();

    v0.block<3, 1>(4, 0) += normal;
    v1.block<3, 1>(4, 0) += normal;
    v2.block<3, 1>(4, 0) += normal;

    indices.emplace_back(std::move(t));
  }

  // normalize normals
  for (auto &v : verts) v.block<3, 1>(4, 0) = v.block<3, 1>(4, 0).normalized();

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
  {
    auto vaoBinding = GL::binding(vao);
    vao.enableVertexAttribArray(0);
    vao.enableVertexAttribArray(1);
    auto vbBinding =
        GL::binding(vertBuff, static_cast<GLenum>(GL_ARRAY_BUFFER));
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 8 * sizeof(float), nullptr);
    glVertexAttribPointer(1, 3, GL_FLOAT, false, 8 * sizeof(float),
                          reinterpret_cast<void const *>(4 * sizeof(float)));
    idxBuff.bind(GL_ELEMENT_ARRAY_BUFFER);
  }
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
      case GLFW_KEY_1:
        viewState_ = ViewState::Scene3D;
        break;
      case GLFW_KEY_2:
        viewState_ = ViewState::LightingComponents;
        break;
      case GLFW_KEY_G:
        visualizer_->showGrid = !visualizer_->showGrid;
        break;
    }
  }
}

void VisualizerImpl::renderOneFrame() {

  glfw_.makeCurrent();

  assertGL("OpenGL Error stack not clear");

  renderMeshes();

  switch (viewState_) {
    case ViewState::Scene3D:
      renderLights();
      if (visualizer_->showGrid) renderGrid();
      break;
    case ViewState::LightingComponents:
      renderLightingTextures();
      break;
  }

  renderFinalPass();

  glfw_.swapBuffers();
  glfw_.waitEvents();
}

void VisualizerImpl::renderMeshes() {
  auto const pMatrix = projectionMatrix();
  auto const vMatrix = viewMatrix();
  auto const MVP = (pMatrix * vMatrix).eval();
  auto const inverseModelViewMatrix =
      vMatrix.block<3, 3>(0, 0).inverse().eval();
  constexpr std::array<GLuint, 3> attachments{
      {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2}};

  // Bind FBO and set it up for MRT
  auto fboBinding = binding(lightingFbo_, static_cast<GLenum>(GL_FRAMEBUFFER));
  assertGL("Failed to bind framebuffer");
  glDrawBuffers(attachments.size(), attachments.data());

  glClearColor(0.f, 0.f, 0.f, 0.f);
  glClearDepth(1.0);
  glClearStencil(0);
  glDisable(GL_FRAMEBUFFER_SRGB);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
  glDepthMask(true);
  glColorMask(true, true, true, true);

  // Render geometry to FBO
  geometryStageProgram_.use();
  assertGL("OpenGL Error stack not clean");

  geometryStageProgram_["shininess"] = mesh_.shininess;
  geometryStageProgram_["modelViewProjectionMatrix"] = MVP;
  geometryStageProgram_["inverseModelViewMatrix"] = inverseModelViewMatrix;
  if (mesh_.vao.name != 0) {
    auto boundVao = GL::binding(mesh_.vao);
    glDrawElements(GL_TRIANGLES, 3 * static_cast<GLsizei>(mesh_.nTriangles),
                   GL_UNSIGNED_INT, nullptr);
    assertGL("glDrawElements failed");
  }

  // switch back to single render target
  glDrawBuffers(1, attachments.data());
}

void VisualizerImpl::renderGrid() {

  auto fboBinding = binding(finalFbo_, static_cast<GLenum>(GL_FRAMEBUFFER));
  // glClearColor(0.f, 0.f, 0.f, 0.f);
  // glClearDepth(1.0);
  // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);

  auto const pMatrix = projectionMatrix();
  auto const vMatrix = viewMatrix();

  gridProgram_.use();
  gridProgram_["scale"] = 2.f;
  gridProgram_["viewProjectionMatrix"] = (pMatrix * vMatrix).eval();

  auto boundVao = binding(singleVertexData_.vao);
  glDrawArrays(GL_POINTS, 0, 1);
  assertGL("glDrawArrays failed");
}

void VisualizerImpl::renderLightingTextures() {
  auto boundFBO = GL::binding(finalFbo_, static_cast<GLenum>(GL_FRAMEBUFFER));

  auto const halfWindowSize = (Size2{glfw_.width(), glfw_.height()} / 2).eval();

  glClearColor(1.0f, 1.f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  glDisable(GL_FRAMEBUFFER_SRGB);
  glDisable(GL_DEPTH_TEST);
  renderQuad(Point2::Zero(), halfWindowSize, TextureID::NormalsAndSpecular,
             normalQuadProgram_);
  renderQuad(Point2::Zero() + Point2(halfWindowSize(0), 0), halfWindowSize,
             TextureID::Depth, depthQuadProgram_);
  glEnable(GL_FRAMEBUFFER_SRGB);
  renderQuad(Point2::Zero() + Point2(0, halfWindowSize(1)), halfWindowSize,
             TextureID::Albedo, quadProgram_);
  renderQuad(Point2::Zero() + halfWindowSize, halfWindowSize,
             TextureID::NormalsAndSpecular, specularQuadProgram_);
}

void VisualizerImpl::renderFinalPass() {
  // Render FBA color attachment to screen
  GL::Framebuffer::unbind(GL_FRAMEBUFFER);
  // glDisable(GL_FRAMEBUFFER_SRGB);
  glEnable(GL_FRAMEBUFFER_SRGB);
  renderFullscreenQuad(TextureID::RenderedImage, quadProgram_);
}

void VisualizerImpl::renderFullscreenQuad(TextureID texture,
                                          GL::ShaderProgram &prog) {
  renderQuad(Point2::Zero(), Size2(glfw_.width(), glfw_.height()), texture,
             prog);
}

void VisualizerImpl::renderQuad(Point2 const &topLeft, Size2 const &size,
                                TextureID texture, GL::ShaderProgram &prog) {
  auto const windowSize = Size2(glfw_.width(), glfw_.height());
  glDisable(GL_DEPTH_TEST);

  // convert window coordinates (in pixel) to clip space coordinates
  auto const topLeftClipsSpace = (topLeft - windowSize / 2)
                                     .cwiseQuotient(windowSize / 2)
                                     .cwiseProduct(Point2(1, -1))
                                     .eval();
  auto const sizeClipSpace = size.cwiseQuotient(windowSize / 2).eval();

  prog.use();
  prog["topLeft"] = topLeftClipsSpace;
  prog["size"] = sizeClipSpace;
  prog["tex"] = 0;
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, textures_[texture]);

  auto boundVao = GL::binding(singleVertexData_.vao);

  // draw quad using the geometry shader
  glDrawArrays(GL_POINTS, 0, 1);
  assertGL("glDrawArrays failed");
}

void VisualizerImpl::renderLights() {

  auto depthBinding =
      GL::binding(lightingFbo_, static_cast<GLenum>(GL_READ_FRAMEBUFFER));
  auto fboBinding =
      GL::binding(finalFbo_, static_cast<GLenum>(GL_DRAW_FRAMEBUFFER));

  renderAmbientLighting();

  glBlendFunc(GL_ONE, GL_ONE);
  glEnable(GL_BLEND);

  renderDiffuseLighting();

  renderSpecularLighting();

  glDisable(GL_BLEND);

  // blit the depth attachment
  auto const w = static_cast<GLint>(glfw_.width());
  auto const h = static_cast<GLint>(glfw_.height());
  glBlitFramebuffer(0, 0, glfw_.width(), glfw_.height())
}

void VisualizerImpl::renderAmbientLighting() {
  std::lock_guard<std::mutex> lock(lightMutex_);
  Color ambientColor = Color::Zero();

  for (auto const &l : lights_) {
    ambientColor += l.second.ambientFactor * l.second.color;
  }

  glClearColor(0.f, 0.f, 0.f, 0.f);
  glClear(GL_COLOR_BUFFER_BIT);
  glDisable(GL_DEPTH_TEST);

  // Ambient pass
  ambientPassProgram_.use();
  ambientPassProgram_["lightColor"] = ambientColor;

  renderFullscreenQuad(TextureID::Albedo, ambientPassProgram_);
}

void VisualizerImpl::renderDiffuseLighting() {
  std::lock_guard<std::mutex> lock(lightMutex_);

  auto const viewMat = viewMatrix();

  diffuseLightingPassProgram_.use();
  diffuseLightingPassProgram_["normalAndSpecularTex"] = 0;
  diffuseLightingPassProgram_["albedoTex"] = 1;
  diffuseLightingPassProgram_["topLeft"] = Eigen::Vector2f(-1, 1);
  diffuseLightingPassProgram_["size"] = (2 * Eigen::Vector2f::Ones()).eval();

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, textures_[TextureID::NormalsAndSpecular]);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, textures_[TextureID::Albedo]);

  auto boundVao = GL::binding(singleVertexData_.vao);

  for (auto const &lightEntry : lights_) {
    auto const &light = lightEntry.second;

    Expects(std::abs(light.position(3)) < 1e-3);

    PositionH const lightPosition = (viewMat * light.position);

    diffuseLightingPassProgram_["lightPosition"] =
        lightPosition.head<3>().eval();
    diffuseLightingPassProgram_["lightColor"] = light.color;

    // draw quad using the geometry shader
    glDrawArrays(GL_POINTS, 0, 1);
    assertGL("glDrawArrays failed");
  }
}

void VisualizerImpl::renderSpecularLighting() {
  std::lock_guard<std::mutex> lock(lightMutex_);

  auto const viewMat = viewMatrix();

  specularLightingPassProgram_.use();
  specularLightingPassProgram_["normalAndSpecularTex"] = 0;
  specularLightingPassProgram_["albedoTex"] = 1;
  specularLightingPassProgram_["topLeft"] = Eigen::Vector2f(-1, 1);
  specularLightingPassProgram_["size"] = (2 * Eigen::Vector2f::Ones()).eval();

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, textures_[TextureID::NormalsAndSpecular]);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, textures_[TextureID::Albedo]);

  auto boundVao = GL::binding(singleVertexData_.vao);

  for (auto const &lightEntry : lights_) {
    auto const &light = lightEntry.second;

    Expects(std::abs(light.position(3)) < 1e-3);

    PositionH const lightPosition = (viewMat * light.position);

    specularLightingPassProgram_["lightPosition"] =
        lightPosition.head<3>().eval();
    specularLightingPassProgram_["lightColor"] = light.color;

    // draw quad using the geometry shader
    glDrawArrays(GL_POINTS, 0, 1);
    assertGL("glDrawArrays failed");
  }
}

void VisualizerImpl::addLight(Visualizer::LightName name, Light const &light) {
  std::lock_guard<std::mutex> lock(lightMutex_);

  lights_.emplace(name, light);
}

} // namespace Private_

/////////////////////////////
// Visualizer
//

Visualizer::Visualizer()
    : impl_(std::make_unique<Private_::VisualizerImpl>(this)) {}

Visualizer::~Visualizer() = default;

Visualizer::Visualizer(Visualizer &&rhs) : impl_(std::move(rhs.impl_)) {
  impl_->visualizer_ = this;
}

Visualizer &Visualizer::operator=(Visualizer &&rhs) {
  using std::swap;
  swap(impl_, rhs.impl_);
  impl_->visualizer_ = this;
  return *this;
}

void Visualizer::start() { impl_->start(); }

Visualizer::operator bool() const noexcept { return *impl_; }

void Visualizer::renderOneFrame() { impl_->renderOneFrame(); }

void Visualizer::addLight(LightName name, Light const &light) {
  impl_->addLight(name, light);
}

template <class VertBase, class IdxBase>
void Visualizer::setMesh(Eigen::MatrixBase<VertBase> const &V,
                         Eigen::MatrixBase<IdxBase> const &I) {
  impl_->setMesh(V, I);
}

template void Visualizer::setMesh<>(Eigen::MatrixBase<Eigen::MatrixXd> const &,
                                    Eigen::MatrixBase<Eigen::MatrixXi> const &);

} // namespace VolViz

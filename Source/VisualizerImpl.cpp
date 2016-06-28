#include "VisualizerImpl.h"

#include "GL/GL.h"
#include "Visualizer.h"

#include <Eigen/Core>
#include <gsl.h>

#include <iostream>
#include <mutex>

namespace VolViz {

////////////////////////////////
// VisualizerImpl
//
#pragma mark -
#pragma mark VisualizerImpl

namespace Private_ {

#pragma mark Constructor
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

#pragma mark Setup Code

void VisualizerImpl::setupShaders() {
  quadProgram_ = std::move(
      GL::ShaderProgram()
          .attachShader(
               GL::Shader(GL_VERTEX_SHADER, GL::Shaders::nullVertShaderSrc))
          .attachShader(
               GL::Shader(GL_GEOMETRY_SHADER, GL::Shaders::quadGeomShaderSrc))
          .attachShader(GL::Shader(GL_FRAGMENT_SHADER,
                                   GL::Shaders::simpleTextureFragShaderSrc))
          .link());

  hdrQuadProgram_ = std::move(
      GL::ShaderProgram()
          .attachShader(
               GL::Shader(GL_VERTEX_SHADER, GL::Shaders::nullVertShaderSrc))
          .attachShader(
               GL::Shader(GL_GEOMETRY_SHADER, GL::Shaders::quadGeomShaderSrc))
          .attachShader(GL::Shader(GL_FRAGMENT_SHADER,
                                   GL::Shaders::hdrTextureFragShaderSrc))
          .link());

  normalQuadProgram_ =
      std::move(GL::ShaderProgram()
                    .attachShader(GL::Shader(GL_VERTEX_SHADER,
                                             GL::Shaders::nullVertShaderSrc))
                    .attachShader(GL::Shader(GL_GEOMETRY_SHADER,
                                             GL::Shaders::quadGeomShaderSrc))
                    .attachShader(GL::Shader(
                        GL_FRAGMENT_SHADER,
                        GL::Shaders::normalVisualizationFragShaderSrc))
                    .link());

  depthQuadProgram_ = std::move(
      GL::ShaderProgram()
          .attachShader(
               GL::Shader(GL_VERTEX_SHADER, GL::Shaders::nullVertShaderSrc))
          .attachShader(
               GL::Shader(GL_GEOMETRY_SHADER, GL::Shaders::quadGeomShaderSrc))
          .attachShader(GL::Shader(
              GL_FRAGMENT_SHADER, GL::Shaders::depthVisualizationFragShaderSrc))
          .link());

  specularQuadProgram_ =
      std::move(GL::ShaderProgram()
                    .attachShader(GL::Shader(GL_VERTEX_SHADER,
                                             GL::Shaders::nullVertShaderSrc))
                    .attachShader(GL::Shader(GL_GEOMETRY_SHADER,
                                             GL::Shaders::quadGeomShaderSrc))
                    .attachShader(GL::Shader(
                        GL_FRAGMENT_SHADER,
                        GL::Shaders::specularVisualizationFragShaderSrc))
                    .link());

  ambientPassProgram_ = std::move(
      GL::ShaderProgram()
          .attachShader(
               GL::Shader(GL_VERTEX_SHADER, GL::Shaders::nullVertShaderSrc))
          .attachShader(
               GL::Shader(GL_GEOMETRY_SHADER, GL::Shaders::quadGeomShaderSrc))
          .attachShader(GL::Shader(GL_FRAGMENT_SHADER,
                                   GL::Shaders::ambientPassFragShaderSrc))
          .link());

  diffuseLightingPassProgram_ =
      std::move(GL::ShaderProgram()
                    .attachShader(GL::Shader(GL_VERTEX_SHADER,
                                             GL::Shaders::nullVertShaderSrc))
                    .attachShader(GL::Shader(GL_GEOMETRY_SHADER,
                                             GL::Shaders::quadGeomShaderSrc))
                    .attachShader(GL::Shader(
                        GL_FRAGMENT_SHADER,
                        GL::Shaders::diffuseLightingPassFragShaderSrc))
                    .link());

  specularLightingPassProgram_ =
      std::move(GL::ShaderProgram()
                    .attachShader(GL::Shader(GL_VERTEX_SHADER,
                                             GL::Shaders::nullVertShaderSrc))
                    .attachShader(GL::Shader(GL_GEOMETRY_SHADER,
                                             GL::Shaders::quadGeomShaderSrc))
                    .attachShader(GL::Shader(
                        GL_FRAGMENT_SHADER,
                        GL::Shaders::specularLightingPassFragShaderSrc))
                    .link());

  geometryStageProgram_ = std::move(
      GL::ShaderProgram()
          .attachShader(GL::Shader(GL_VERTEX_SHADER,
                                   GL::Shaders::deferredVertexShaderSrc))
          .attachShader(
               GL::Shader(GL_FRAGMENT_SHADER,
                          GL::Shaders::deferredPassthroughFragShaderSrc))
          .link());

  gridProgram_ = std::move(
      GL::ShaderProgram()
          .attachShader(
               GL::Shader(GL_VERTEX_SHADER, GL::Shaders::nullVertShaderSrc))
          .attachShader(GL::Shader(GL_GEOMETRY_SHADER,
                                   GL::Shaders::gridGeometryShaderSrc))
          .attachShader(GL::Shader(GL_FRAGMENT_SHADER,
                                   GL::Shaders::passThroughFragShaderSrc))
          .link());

  planeProgram_ =
      std::move(GL::ShaderProgram()
                    .attachShader(GL::Shader(GL_VERTEX_SHADER,
                                             GL::Shaders::nullVertShaderSrc))
                    .attachShader(GL::Shader(GL_GEOMETRY_SHADER,
                                             GL::Shaders::planeGeomShaderSrc))
                    .attachShader(GL::Shader(
                        GL_FRAGMENT_SHADER,
                        GL::Shaders::deferredPassthroughFragShaderSrc))
                    .link());

  bboxProgram_ = std::move(
      GL::ShaderProgram()
          .attachShader(
               GL::Shader(GL_VERTEX_SHADER, GL::Shaders::nullVertShaderSrc))
          .attachShader(GL::Shader(GL_GEOMETRY_SHADER,
                                   GL::Shaders::bboxGeometryShaderSrc))
          .attachShader(GL::Shader(GL_FRAGMENT_SHADER,
                                   GL::Shaders::passThroughFragShaderSrc))
          .link());
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

    // depth and stencil texture
    glBindTexture(GL_TEXTURE_2D, textures_[TextureID::FinalDepth]);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT24, width, height);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // setup FBO
    GL::Framebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo.name);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           textures_[TextureID::RenderedImage], 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                           textures_[TextureID::FinalDepth], 0);
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

#pragma mark Matrix Computation

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

Eigen::Matrix4f VisualizerImpl::textureTransformationMatrix() const noexcept {
  Length const refScale = visualizer_->scale;
  auto const invScale =
      (Size3f(static_cast<float>(refScale / currentVolume_.voxelSize[0]),
              static_cast<float>(refScale / currentVolume_.voxelSize[1]),
              static_cast<float>(refScale / currentVolume_.voxelSize[2]))
           .cwiseQuotient(currentVolume_.size.cast<float>())).eval();

  Eigen::Matrix4f const t = (Eigen::Translation3f(Position::Ones() / 2) *
                             invScale.asDiagonal()).matrix();

  return t;
}

#pragma mark Interface Methods

void VisualizerImpl::start() { glfw_.show(); }

VisualizerImpl::operator bool() const noexcept { return glfw_; }

template <>
void VisualizerImpl::setVolume(VolumeDescriptor const &descriptor,
                               gsl::span<float const> data) {
  auto const nVoxels =
      descriptor.size(0) * descriptor.size(1) * descriptor.size(2);
  auto const width = static_cast<GLsizei>(descriptor.size(0));
  auto const height = static_cast<GLsizei>(descriptor.size(1));
  auto const depth = static_cast<GLsizei>(descriptor.size(2));

  Expects(width > 0 && height > 0 && depth > 0);

  glBindTexture(GL_TEXTURE_3D, textures_[TextureID::VolumeTexture]);

  GLenum internalFormat = 0;
  GLenum format = 0;

  switch (descriptor.type) {
    case VolumeType::GrayScale:
      internalFormat = GL_R32F;
      format = GL_RED;
      Expects(static_cast<std::size_t>(data.size()) == nVoxels);
      break;
    case VolumeType::ColorRGB:
      internalFormat = GL_RGB32F;
      format = GL_RGB;
      Expects(static_cast<std::size_t>(data.size()) == 3 * nVoxels);
      break;
  }

  glTexStorage3D(GL_TEXTURE_3D, 1, internalFormat, width, height, depth);
  assertGL("Failed to allocate texture storage");
  glActiveTexture(GL_TEXTURE0);

  // upload texture data
  glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, width, height, depth, format,
                  GL_FLOAT, data.data());
  assertGL("Failed to upload texture data");

  assertGL("Failed to activate volume texte");
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
  glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR,
                   Colors::Black().eval().data());
  assertGL("Failed to set texture parameters");

  currentVolume_ = descriptor;
}

template <>
void VisualizerImpl::setVolume(VolumeDescriptor const &descriptor,
                               gsl::span<Color const> data) {
  auto const nVoxels =
      descriptor.size(0) * descriptor.size(1) * descriptor.size(2);

  Expects(descriptor.type == VolumeType::ColorRGB);
  Expects(nVoxels == static_cast<std::size_t>(data.size()));

  auto const *ptr = reinterpret_cast<float const *>(data.data());
  auto const size = static_cast<std::ptrdiff_t>(3 * data.size());
  setVolume(descriptor, gsl::as_span(ptr, size));
}

template void VisualizerImpl::setVolume(VolumeDescriptor const &,
                                        gsl::span<float const>);
template void VisualizerImpl::setVolume(VolumeDescriptor const &,
                                        gsl::span<Color const>);

void VisualizerImpl::addLight(Visualizer::LightName name, Light const &light) {
  std::lock_guard<std::mutex> lock(lightMutex_);

  lights_.emplace(name, light);
}

void VisualizerImpl::addGeometry(Visualizer::GeometryName name,
                                 AxisAlignedPlane const &plane) {
  using GL::MoveMask;
  using Eigen::AngleAxisf;
  using std::abs;
  Length const refScale = visualizer_->scale;
  auto geom = GL::Geometry{};
  auto constexpr d90 = static_cast<float>(M_PI / 2.0);

  float intercept = 1.f;

  if (abs(plane.intercept / refScale) < 1e-6) {
    geom.scale = refScale;
    intercept = 0.f;
  } else { geom.scale = plane.intercept; }

  geom.movable = plane.movable;
  geom.color = plane.color;

  switch (plane.axis) {
    case Axis::X:
      geom.position = intercept * Position::UnitX();
      geom.moveMask = MoveMask::X;
      geom.orientation = AngleAxisf(d90, -Position::UnitY());
      break;
    case Axis::Y:
      geom.position = intercept * Position::UnitY();
      geom.moveMask = MoveMask::Y;
      geom.orientation = AngleAxisf(d90, -Position::UnitX());
      break;
    case Axis::Z:
      geom.position = intercept * Position::UnitZ();
      geom.moveMask = MoveMask::Z;
      geom.orientation = Orientation::Identity();
      break;
  }

  auto init = [geom, this]() {
    return [geom, this]() {
      Length const rScale = visualizer_->scale;
      auto const viewMat = viewMatrix();
      auto const scale = static_cast<float>(geom.scale / rScale);
      auto const volSize =
          (Size3f(static_cast<float>(currentVolume_.voxelSize[0] / rScale),
                  static_cast<float>(currentVolume_.voxelSize[1] / rScale),
                  static_cast<float>(currentVolume_.voxelSize[2] / rScale))
               .cwiseProduct(currentVolume_.size.cast<float>()) /
           2.f).eval();

      auto const modelMat = (Eigen::Translation3f(geom.position * scale) *
                             geom.orientation * volSize.asDiagonal()).matrix();

      auto const modelViewMat = (viewMat * modelMat).eval();
      auto const inverseModelViewMatrix =
          modelViewMat.block<3, 3>(0, 0).inverse().eval();

      // bind volume texture
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_3D, textures_[TextureID::VolumeTexture]);

      planeProgram_.use();
      planeProgram_["volume"] = 0;
      planeProgram_["modelMatrix"] = modelMat;
      planeProgram_["shininess"] = 10.f;
      planeProgram_["color"] = geom.color;
      planeProgram_["modelViewProjectionMatrix"] =
          (projectionMatrix() * modelViewMat).eval();
      planeProgram_["inverseModelViewMatrix"] = inverseModelViewMatrix;
      planeProgram_["textureTransformMatrix"] = textureTransformationMatrix();

      auto boundVao = GL::binding(singleVertexData_.vao);
      glDrawArrays(GL_POINTS, 0, 1);
      assertGL("glDrawArrays failed");
    };
  };

  std::lock_guard<std::mutex> lock(geomInitQueueMutex_);
  geometryInitQueue_.emplace(name, init);
}

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
      case GLFW_KEY_B:
        visualizer_->showVolumeBoundingBox =
            !visualizer_->showVolumeBoundingBox;
        break;
    }
  }
}

#pragma mark Render Methods

void VisualizerImpl::renderOneFrame() {

  glfw_.makeCurrent();

  // Init all new geometry
  {
    InitQueueEntry entry;
    {
      std::lock_guard<std::mutex> lock(geomInitQueueMutex_);
      if (!geometryInitQueue_.empty()) {
        entry = geometryInitQueue_.front();
        geometryInitQueue_.pop();
      }
    } // release lock
    // Init and add geometry if queue was not empty
    if (entry.second) {
      std::cout << "Init geometry '" << entry.first << "'" << std::endl;
      geometries_.emplace(entry.first, entry.second());
    }
  }

  assertGL("OpenGL Error stack not clear");

  renderGeometry();

  renderMeshes();

  switch (viewState_) {
    case ViewState::Scene3D: {
      renderLights();
      if (visualizer_->showGrid) renderGrid();
      if (visualizer_->showVolumeBoundingBox && currentVolume_.size(0) > 0)
        renderVolumeBBox();
      break;
    }
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

  // glClearColor(0.f, 0.f, 0.f, 0.f);
  // glClearDepth(1.0);
  // glClearStencil(0);
  // glDisable(GL_FRAMEBUFFER_SRGB);
  // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
  glDepthMask(true);
  glColorMask(true, true, true, true);

  // Render geometry to FBO
  geometryStageProgram_.use();
  assertGL("OpenGL Error stack not clean");

  // bind volume texture
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_3D, textures_[TextureID::VolumeTexture]);

  geometryStageProgram_["volume"] = 0;
  geometryStageProgram_["shininess"] = mesh_.shininess;
  geometryStageProgram_["modelViewProjectionMatrix"] = MVP;
  geometryStageProgram_["inverseModelViewMatrix"] = inverseModelViewMatrix;
  geometryStageProgram_["textureTransformMatrix"] =
      textureTransformationMatrix();
  if (mesh_.vao.name != 0) {
    auto boundVao = GL::binding(mesh_.vao);
    glDrawElements(GL_TRIANGLES, 3 * static_cast<GLsizei>(mesh_.nTriangles),
                   GL_UNSIGNED_INT, nullptr);
    assertGL("glDrawElements failed");
  }

  // switch back to single render target
  glDrawBuffers(1, attachments.data());
}

void VisualizerImpl::renderGeometry() {
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

  // call render commands
  for (auto const &geom : geometries_) geom.second();

  // switch back to single render target
  glDrawBuffers(1, attachments.data());
}

void VisualizerImpl::renderGrid() {

  auto fboBinding = binding(finalFbo_, static_cast<GLenum>(GL_FRAMEBUFFER));

  auto const pMatrix = projectionMatrix();
  auto const vMatrix = viewMatrix();

  gridProgram_.use();
  gridProgram_["scale"] = 1.f;
  gridProgram_["viewProjectionMatrix"] = (pMatrix * vMatrix).eval();

  auto boundVao = binding(singleVertexData_.vao);

  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
  glDrawArrays(GL_POINTS, 0, 1);
  assertGL("glDrawArrays failed");
}

void VisualizerImpl::renderLightingTextures() {
  auto boundFBO =
      GL::binding(finalFbo_, static_cast<GLenum>(GL_DRAW_FRAMEBUFFER));

  auto const halfWindowSize = (Size2{glfw_.width(), glfw_.height()} / 2).eval();

  glDisable(GL_FRAMEBUFFER_SRGB);
  glDisable(GL_DEPTH_TEST);

  renderQuad(Point2::Zero(), halfWindowSize, TextureID::NormalsAndSpecular,
             normalQuadProgram_);
  renderQuad(Point2::Zero() + Point2(halfWindowSize(0), 0), halfWindowSize,
             TextureID::Depth, depthQuadProgram_);
  // glEnable(GL_FRAMEBUFFER_SRGB);
  renderQuad(Point2::Zero() + Point2(0, halfWindowSize(1)), halfWindowSize,
             TextureID::Albedo, quadProgram_);
  renderQuad(Point2::Zero() + halfWindowSize, halfWindowSize,
             TextureID::NormalsAndSpecular, specularQuadProgram_);
}

void VisualizerImpl::renderFinalPass() {
  // Render FBA color attachment to screen
  GL::Framebuffer::unbind(GL_FRAMEBUFFER);
  // glDisable(GL_FRAMEBUFFER_SRGB);
  // glEnable(GL_FRAMEBUFFER_SRGB);
  renderFullscreenQuad(TextureID::RenderedImage, hdrQuadProgram_);
  // auto readBinding =
  //     GL::binding(finalFbo_, static_cast<GLenum>(GL_READ_FRAMEBUFFER));
  // auto const w = static_cast<GLint>(glfw_.width());
  // auto const h = static_cast<GLint>(glfw_.height());
  // glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
  assertGL("Failed to blit framebuffer");
}

void VisualizerImpl::renderBoundingBox(Position const &position,
                                       Orientation const &orientation,
                                       Size3f const &size, Color const &color) {

  auto fboBinding = binding(finalFbo_, static_cast<GLenum>(GL_FRAMEBUFFER));

  auto const modelMat = (Eigen::Translation3f(position) * orientation *
                         size.asDiagonal()).matrix();
  auto const mvpMatrix = (projectionMatrix() * viewMatrix() * modelMat).eval();

  bboxProgram_.use();
  bboxProgram_["lineColor"] = color;
  bboxProgram_["modelViewProjectionMatrix"] = mvpMatrix;

  // draw quad using the geometry shader
  auto boundVao = GL::binding(singleVertexData_.vao);
  glEnable(GL_DEPTH_TEST);
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glDepthMask(GL_TRUE);
  glDrawArrays(GL_POINTS, 0, 1);
  assertGL("glDrawArrays failed");
}

void VisualizerImpl::renderVolumeBBox() {
  auto const vol = currentVolume_;
  Length const scale = visualizer_->scale;

  auto const voxelSize = Size3f(static_cast<float>(vol.voxelSize[0] / scale),
                                static_cast<float>(vol.voxelSize[1] / scale),
                                static_cast<float>(vol.voxelSize[2] / scale));

  auto const size = vol.size.cast<float>().cwiseProduct(voxelSize);

  renderBoundingBox(Size3f::Zero(), Orientation::Identity(), size,
                    Colors::Cyan());
}

void VisualizerImpl::renderFullscreenQuad(TextureID texture,
                                          GL::ShaderProgram &prog) {
  renderQuad(Point2::Zero(), Size2(glfw_.width(), glfw_.height()), texture,
             prog);
}

void VisualizerImpl::renderQuad(Point2 const &topLeft, Size2 const &size,
                                TextureID texture, GL::ShaderProgram &prog) {
  auto const windowSize = Size2(glfw_.width(), glfw_.height());

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
  glDisable(GL_DEPTH_TEST);
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glDrawArrays(GL_POINTS, 0, 1);
  assertGL("glDrawArrays failed");
}

void VisualizerImpl::renderLights() {

  auto depthBinding =
      GL::binding(lightingFbo_, static_cast<GLenum>(GL_READ_FRAMEBUFFER));
  auto fboBinding =
      GL::binding(finalFbo_, static_cast<GLenum>(GL_DRAW_FRAMEBUFFER));

  glClearColor(0.f, 0.f, 0.f, 0.f);
  glClear(GL_COLOR_BUFFER_BIT);

  renderAmbientLighting();

  glBlendFunc(GL_ONE, GL_ONE);
  glEnable(GL_BLEND);

  renderDiffuseLighting();

  renderSpecularLighting();

  glDisable(GL_BLEND);

  // blit the depth attachment
  auto const w = static_cast<GLint>(glfw_.width());
  auto const h = static_cast<GLint>(glfw_.height());
  glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
  assertGL("Failed to blit framebuffer");
}

void VisualizerImpl::renderAmbientLighting() {
  std::lock_guard<std::mutex> lock(lightMutex_);
  Color ambientColor = Color::Zero();

  for (auto const &l : lights_) {
    ambientColor += l.second.ambientFactor * l.second.color;
  }

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

} // namespace Private_
} // namespace VolViz

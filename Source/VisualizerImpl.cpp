#include "VisualizerImpl.h"

#include "GL/GL.h"
#include "GL/GLdefs.h"
#include "Visualizer.h"

#include <Eigen/Core>
#include <gsl.h>

#include <algorithm>
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
  // std::cout << "Extensions: " << std::endl;
  // for (auto e : glfw_.supportedExtensions())
  //   std::cout << "\t" << e << std::endl;

  setupShaders();
  setupFBOs();
  setupSelectionBuffers();

  // Check if glClipControl is available
  if (major > 4 || (major == 4 && minor >= 5) ||
      glfw_.supportsExtension("GL_ARB_clip_control")) {
    // glClipControl available, so we can use high precision DirectX like depth
    // ranges
    depthRange_.near = 1.f;
    depthRange_.far = 0.f;
    glDepthRange(0.0, 1.0);
    glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
  } else {
    // glClipControl is not available, have to stick to suboptimal depth ranges
    depthRange_.near = 1.f;
    depthRange_.far = -1.f;
    glDepthRange(0.0, 1.0);
  }
  glDepthFunc(GL_GREATER);

  glfw_.keyInputHandler =
      [this](int k, int s, int a, int m) { handleKeyInput(k, s, a, m); };

  glfw_.windowResizeCallback = [this](auto, auto) {
    this->setupFBOs(); // this is used explicitly here because gcc complains
                       // otherwise, although it's perectly standard conformant
    glViewport(0, 0, static_cast<GLsizei>(glfw_.width()),
               static_cast<GLsizei>(glfw_.height()));
    float const FOV =
        static_cast<float>(glfw_.width()) / static_cast<float>(glfw_.height());
    this->camera().verticalFieldOfView = FOV;
  };

  glfw_.scrollWheelInputHandler = [this](double, double y) {
    Length const scale = cachedScale_;
    PhysicalPosition pos = camera().position;
    pos(2) -= 2 * gsl::narrow_cast<float>(y) * scale;

    camera().position = pos;
  };

  glfw_.mouseButtonCallback = [this](int button, int action, int) {
    switch (moveState_) {
      case MoveState::None:
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
          if (!inSelectionMode)
            moveState_ = MoveState::Rotating;
          else
            moveState_ = MoveState::Dragging;
        }
        break;
      case MoveState::Rotating:
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
          moveState_ = MoveState::None;
        break;
      case MoveState::Dragging:
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
        Position2(2 * x / static_cast<double>(glfw_.width()) - 1.0,
                  -2 * y / static_cast<double>(glfw_.height()) + 1.0);
    auto const pos3 = Position(
        pos(0), pos(1), static_cast<float>(sqrt(1.0 - pos.squaredNorm())));
    auto const lastPos3 =
        Position(lastMousePos_(0), lastMousePos_(1),
                 static_cast<float>(sqrt(1.0 - lastMousePos_.squaredNorm())));

    switch (moveState_) {
      case MoveState::Rotating: {
        auto const angle =
            acos(min(1.0, static_cast<double>(pos3.transpose() * lastPos3))) *
            2;
        Vector3f const axis = lastPos3.cross(pos3).normalized().cast<float>();
        if (angle > 1e-3) {
          Orientation o = camera().orientation;
          o *= Eigen::Quaternionf(Eigen::AngleAxisf(static_cast<float>(angle),
                                                    axis)).inverse();
          camera().orientation = o.normalized();
        }
        break;
      }
      case MoveState::None:
        break;
      case MoveState::Dragging:
        break;
    }

    lastMouseDelta_ = pos - lastMousePos_;
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

  selectionIndexVisualizationProgam_ =
      std::move(GL::ShaderProgram()
                    .attachShader(GL::Shader(GL_VERTEX_SHADER,
                                             GL::Shaders::nullVertShaderSrc))
                    .attachShader(GL::Shader(GL_GEOMETRY_SHADER,
                                             GL::Shaders::quadGeomShaderSrc))
                    .attachShader(GL::Shader(
                        GL_FRAGMENT_SHADER,
                        GL::Shaders::selectionIndexVisualizationFragShaderSrc))
                    .link());

  pointProgram_ = std::move(
      GL::ShaderProgram()
          .attachShader(
               GL::Shader(GL_VERTEX_SHADER, GL::Shaders::pointVertShaderSrc))
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
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32, width, height);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Selection index texture
    glBindTexture(GL_TEXTURE_2D, textures_[TextureID::SelectionTexture]);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, width, height);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // setup FBO
    GL::Framebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo.name);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           textures_[TextureID::NormalsAndSpecular], 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
                           textures_[TextureID::Albedo], 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D,
                           textures_[TextureID::SelectionTexture], 0);
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
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32, width, height);
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

void VisualizerImpl::setupSelectionBuffers() {
  for (auto &buffer : selectionBuffer_.buffers) {
    buffer.upload(GL_PIXEL_PACK_BUFFER, sizeof(std::uint32_t) + sizeof(float),
                  static_cast<void *>(nullptr), GL_STREAM_READ);
  }
  GL::Buffer::unbind(GL_PIXEL_PACK_BUFFER);
}

#pragma mark Matrix Computation

Eigen::Matrix4f VisualizerImpl::textureTransformationMatrix() const noexcept {
  Length const refScale = cachedScale_;
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
  Length const refScale = cachedScale_;
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
    return [geom, this](std::uint32_t index, bool selected) {
      Length const rScale = cachedScale_;
      auto const viewMat = camera().client().viewMatrix(rScale);
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
      planeProgram_["index"] = index;
      planeProgram_["volume"] = 0;
      planeProgram_["modelMatrix"] = modelMat;
      planeProgram_["shininess"] = 10.f;
      planeProgram_["color"] =
          selected ? (geom.color * 1.5f).eval() : geom.color;
      planeProgram_["modelViewProjectionMatrix"] =
          (camera().client().projectionMatrix() * modelViewMat).eval();
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
      case GLFW_KEY_3:
        viewState_ = ViewState::SelectionIndices;
        break;
      case GLFW_KEY_G:
        visualizer_->showGrid = !visualizer_->showGrid;
        break;
      case GLFW_KEY_B:
        visualizer_->showVolumeBoundingBox =
            !visualizer_->showVolumeBoundingBox;
        break;
      case GLFW_KEY_LEFT_CONTROL:
        inSelectionMode = true;
        break;
    }
  } else if (action == GLFW_RELEASE) {
    switch (key) {
      case GLFW_KEY_LEFT_CONTROL:
        inSelectionMode = false;
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
    case ViewState::SelectionIndices:
      renderSelectionIndexTexture();
      break;
  }

  // auto const geomNameAndPos = getGeometryUnderCursor();
  // if (!geomNameAndPos.first.empty() && viewState_ == ViewState::Scene3D)
  //     renderPoint(geomNameAndPos.second, Colors::Cyan(), 2.5f);

  renderFinalPass();

  if (inSelectionMode && moveState_ != MoveState::Dragging) {
    auto const geomNameAndPos = getGeometryUnderCursor();
    selectedGeometry = geomNameAndPos.first;
  } else if (moveState_ != MoveState::Dragging) { selectedGeometry.clear(); }

  glfw_.swapBuffers();
  glfw_.waitEvents();
}

void VisualizerImpl::renderMeshes() {
  Length const scale = cachedScale_;
  auto const vMatrix = camera().client().viewMatrix(scale);
  auto const MVP = camera().client().viewProjectionMatrix(scale);
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
  glClearDepth(0.0);
  glClearStencil(0);
  glDisable(GL_FRAMEBUFFER_SRGB);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
  glDepthMask(true);
  glColorMask(true, true, true, true);

  // call render commands
  std::uint32_t idx = 0;
  for (auto const &geom : geometries_)
    geom.second(++idx, geom.first == selectedGeometry);

  // switch back to single render target
  glDrawBuffers(1, attachments.data());
}

void VisualizerImpl::renderGrid() {

  Length const scale = cachedScale_;
  auto fboBinding = binding(finalFbo_, static_cast<GLenum>(GL_FRAMEBUFFER));

  gridProgram_.use();
  gridProgram_["scale"] = 1.f;
  gridProgram_["viewProjectionMatrix"] =
      camera().client().viewProjectionMatrix(scale);

  auto boundVao = binding(singleVertexData_.vao);

  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
  glDrawArrays(GL_POINTS, 0, 1);
  assertGL("glDrawArrays failed");
}

void VisualizerImpl::renderPoint(Position const &position, Color const &color,
                                 float size) {

  Length const scale = cachedScale_;
  auto fboBinding =
      binding(finalFbo_, static_cast<GLenum>(GL_DRAW_FRAMEBUFFER));

  auto const viewProjMat = camera().client().viewProjectionMatrix(scale);
  PositionH projPos = viewProjMat * position.homogeneous();

  pointProgram_.use();
  pointProgram_["size"] = size;
  pointProgram_["position"] = projPos;
  pointProgram_["pointColor"] = color;

  auto boundVao = binding(singleVertexData_.vao);
  glDisable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);
  glEnable(GL_PROGRAM_POINT_SIZE);
  glDrawArrays(GL_POINTS, 0, 1);
  assertGL("glDrawArrays failed");
  glDepthMask(GL_TRUE);
  glDisable(GL_PROGRAM_POINT_SIZE);
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

void VisualizerImpl::renderSelectionIndexTexture() {
  auto boundFBO =
      GL::binding(finalFbo_, static_cast<GLenum>(GL_DRAW_FRAMEBUFFER));

  glDisable(GL_FRAMEBUFFER_SRGB);
  glDisable(GL_DEPTH_TEST);

  selectionIndexVisualizationProgam_.use();
  selectionIndexVisualizationProgam_["nObjects"] =
      static_cast<std::uint32_t>(geometries_.size());
  renderFullscreenQuad(TextureID::SelectionTexture,
                       selectionIndexVisualizationProgam_);
}

void VisualizerImpl::renderFinalPass() {
  // Render FBA color attachment to screen
  GL::Framebuffer::unbind(GL_FRAMEBUFFER);
  // glDisable(GL_FRAMEBUFFER_SRGB);
  glEnable(GL_FRAMEBUFFER_SRGB);
  renderFullscreenQuad(TextureID::RenderedImage, hdrQuadProgram_);
  // renderFullscreenQuad(TextureID::RenderedImage, quadProgram_);
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

  Length const scale = cachedScale_;
  auto fboBinding = binding(finalFbo_, static_cast<GLenum>(GL_FRAMEBUFFER));

  auto const modelMat = (Eigen::Translation3f(position) * orientation *
                         size.asDiagonal()).matrix();
  auto const mvpMatrix =
      (camera().client().viewProjectionMatrix(scale) * modelMat).eval();

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
  Length const scale = cachedScale_;

  auto const voxelSize = Size3f(static_cast<float>(vol.voxelSize[0] / scale),
                                static_cast<float>(vol.voxelSize[1] / scale),
                                static_cast<float>(vol.voxelSize[2] / scale));

  auto const size = vol.size.cast<float>().cwiseProduct(voxelSize);

  renderBoundingBox(Size3f::Zero(), Orientation::Identity(), size,
                    Colors::Cyan());
}

VisualizerImpl::GeometryNameAndPosition
VisualizerImpl::getGeometryUnderCursor() {
  using std::swap;
  // Bind FBO
  auto fboBinding =
      binding(lightingFbo_, static_cast<GLenum>(GL_READ_FRAMEBUFFER));
  // assertGL("Failed to bind framebuffer");

  auto const mousePos = lastMousePos_;
  // convert mouse position to texture screen coordinates
  auto const windowSize = Size2(glfw_.width(), glfw_.height()).cast<double>();
  auto const pos = Position2((mousePos(0) + 1.0) * windowSize(0) / 2.0,
                             (mousePos(1) + 1.0) * windowSize(1) / 2.0)
                       .cast<GLint>()
                       .eval();

  // Copy index and depth to selection(back) buffer
  auto const writeBinding = GL::binding(
      *selectionBuffer_.writeBuffer, static_cast<GLenum>(GL_PIXEL_PACK_BUFFER));
  glPixelStorei(GL_PACK_ALIGNMENT, 4);
  glReadBuffer(GL_COLOR_ATTACHMENT2);
  glReadPixels(pos(0), pos(1), 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, 0);
  glReadPixels(pos(0), pos(1), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT,
               reinterpret_cast<void *>(sizeof(std::uint32_t)));
  assertGL("Failed to read depth value under cursor");

  // map selection (front) buffer and read index and depth value
  auto const readBinding = GL::binding(
      *selectionBuffer_.readBuffer, static_cast<GLenum>(GL_PIXEL_PACK_BUFFER));
  void const *mappedMemory = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
  assertGL("Failed to map memory");
  Expects(mappedMemory != nullptr);

  auto const index = *reinterpret_cast<std::uint32_t const *>(mappedMemory);
  auto const normalizedDepth =
      *reinterpret_cast<float const *>(
          reinterpret_cast<std::uint8_t const *>(mappedMemory) +
          sizeof(std::uint32_t));
  glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
  selectionBuffer_.swap();

  // retrieve geometry name
  Visualizer::GeometryName name;
  if (index > 0) {
    auto const entry = std::next(geometries_.cbegin(), index - 1);
    name = entry->first;
  }

  // convert depth into camera depth
  auto const cameraDepthRange = camera().depthRange();
  auto const minCamDepth =
      std::min(cameraDepthRange.near, cameraDepthRange.far);
  auto const maxCamDepth =
      std::max(cameraDepthRange.near, cameraDepthRange.far);
  auto const minDepth = std::min(depthRange_.near, depthRange_.far);
  auto const maxDepth = std::max(depthRange_.near, depthRange_.far);
  auto const depthInWorld = normalizedDepth * (maxDepth - minDepth) + minDepth;
  auto const depthInCamera = std::clamp(depthInWorld, minCamDepth, maxCamDepth);

  // Compute 3D position using window coordinates and depth
  auto const posInWorld =
      camera().client().unproject(mousePos, depthInCamera, cachedScale_);

  return GeometryNameAndPosition(name, posInWorld);
}

void VisualizerImpl::dragSelectedGeometry() {
  if (selectedGeometry.empty()) return;

  Position const mousePos{lastMousePos_.x(), lastMousePos_.y(), 0};
  Position const mouseDelta{lastMouseDelta_.x(), lastMouseDelta_.y(), 0};
  Position const mouseDir = mouseDelta.normalized();
  auto const movedDistance = mouseDir.norm();
  Length const scale = visualizer_->scale;
  Matrix4 const invViewMat = camera().client().viewMatrix(scale).inverse();

  PositionH const moveDirectionInWorld =
      invViewMat * PositionH{mouseDir.x(), mouseDir.y(), mouseDir.z(), 0} *
      movedDistance;

  (void)&moveDirectionInWorld;
  // TODO: get geometry and move it along
}

void VisualizerImpl::renderFullscreenQuad(TextureID texture,
                                          GL::ShaderProgram &prog) {
  renderQuad(Point2::Zero(), Size2(glfw_.width(), glfw_.height()), texture,
             prog);
}

void VisualizerImpl::renderQuad(Point2 const &topLeft, Size2 const &size,
                                TextureID texture, GL::ShaderProgram &prog) {
  assertGL("Dirty openGL error stack");
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

  Length const scale = cachedScale_;
  auto const viewMat = camera().client().viewMatrix(scale);

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

  Length const scale = cachedScale_;
  auto const viewMat = camera().client().viewMatrix(scale);

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

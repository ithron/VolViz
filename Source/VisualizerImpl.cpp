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
VisualizerImpl::VisualizerImpl(Visualizer *vis)
    : visualizer_(vis), geomFactory_(*this) {

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

  shaders_.init();
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

  glfw_.keyInputHandler = [this](int k, int s, int a, int m) {
    handleKeyInput(k, s, a, m);
  };

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
    Length const scale = cachedScale;
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
          o *= Eigen::Quaternionf(
                   Eigen::AngleAxisf(static_cast<float>(angle), axis))
                   .inverse();
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
  Length const refScale = cachedScale;
  auto const invScale =
      (Size3f(static_cast<float>(refScale / currentVolume_.voxelSize[0]),
              static_cast<float>(refScale / currentVolume_.voxelSize[1]),
              static_cast<float>(refScale / currentVolume_.voxelSize[2]))
           .cwiseQuotient(currentVolume_.size.cast<float>()))
          .eval();

  Eigen::Matrix4f const t =
      (Eigen::Translation3f(Position::Ones() / 2) * invScale.asDiagonal())
          .matrix();

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

Size3f VisualizerImpl::volumeSize() const noexcept {
  Length const rScale = cachedScale;
  return (Size3f(static_cast<float>(currentVolume_.voxelSize[0] / rScale),
                 static_cast<float>(currentVolume_.voxelSize[1] / rScale),
                 static_cast<float>(currentVolume_.voxelSize[2] / rScale))
              .cwiseProduct(currentVolume_.size.cast<float>()) /
          2.f);
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

void VisualizerImpl::drawSingleVertex() const noexcept {
  auto boundVao = GL::binding(singleVertexData_.vao);
  glDrawArrays(GL_POINTS, 0, 1);
  assertGL("glDrawArrays failed");
}

void VisualizerImpl::bindVolume(GLuint unit) const noexcept {
  glActiveTexture(GL_TEXTURE0 + unit);
  glBindTexture(GL_TEXTURE_3D, textures_[TextureID::VolumeTexture]);
}

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
        entry = std::move(geometryInitQueue_.front());
        geometryInitQueue_.pop();
      }
    } // release lock
    // Init and add geometry if queue was not empty
    if (entry.second) {
      std::cout << "Init geometry '" << entry.first << "'" << std::endl;
      entry.second->init();
      geometries_.emplace(entry.first, std::move(entry.second));
    }
  }

  assertGL("OpenGL Error stack not clear");

  renderGeometry();

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
    selectedGeometry_ = geomNameAndPos.name;
    selectedPoint_ = geomNameAndPos.position;
  } else if (inSelectionMode && moveState_ == MoveState::Dragging) {
    dragSelectedGeometry();
  } else if (moveState_ != MoveState::Dragging) {
    selectedGeometry_.clear();
  }

  glfw_.swapBuffers();
  glfw_.waitEvents();
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
    geom.second->render(++idx, geom.first == selectedGeometry_);

  // switch back to single render target
  glDrawBuffers(1, attachments.data());
}

void VisualizerImpl::renderGrid() {

  Length const scale = cachedScale;
  auto fboBinding = binding(finalFbo_, static_cast<GLenum>(GL_FRAMEBUFFER));

  shaders_["grid"].use();
  shaders_["grid"]["scale"] = 1.f;
  shaders_["grid"]["viewProjectionMatrix"] =
      cameraClient().viewProjectionMatrix(scale);

  auto boundVao = binding(singleVertexData_.vao);

  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
  glDrawArrays(GL_POINTS, 0, 1);
  assertGL("glDrawArrays failed");
}

void VisualizerImpl::renderPoint(Position const &position, Color const &color,
                                 float size) {

  Length const scale = cachedScale;
  auto fboBinding =
      binding(finalFbo_, static_cast<GLenum>(GL_DRAW_FRAMEBUFFER));

  auto const viewProjMat = cameraClient().viewProjectionMatrix(scale);
  PositionH projPos = viewProjMat * position.homogeneous();

  shaders_["point"].use();
  shaders_["point"]["size"] = size;
  shaders_["point"]["position"] = projPos;
  shaders_["point"]["pointColor"] = color;

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
             shaders_["normalQuad"]);
  renderQuad(Point2::Zero() + Point2(halfWindowSize(0), 0), halfWindowSize,
             TextureID::Depth, shaders_["depthQuad"]);
  // glEnable(GL_FRAMEBUFFER_SRGB);
  renderQuad(Point2::Zero() + Point2(0, halfWindowSize(1)), halfWindowSize,
             TextureID::Albedo, shaders_["quad"]);
  renderQuad(Point2::Zero() + halfWindowSize, halfWindowSize,
             TextureID::NormalsAndSpecular, shaders_["specularQuad"]);
}

void VisualizerImpl::renderSelectionIndexTexture() {
  auto boundFBO =
      GL::binding(finalFbo_, static_cast<GLenum>(GL_DRAW_FRAMEBUFFER));

  glDisable(GL_FRAMEBUFFER_SRGB);
  glDisable(GL_DEPTH_TEST);

  shaders_["selectionIndexVisualization"].use();
  shaders_["selectionIndexVisualization"]["nObjects"] =
      static_cast<std::uint32_t>(geometries_.size());
  renderFullscreenQuad(TextureID::SelectionTexture,
                       shaders_["selectionIndexVisualization"]);
}

void VisualizerImpl::renderFinalPass() {
  // Render FBA color attachment to screen
  GL::Framebuffer::unbind(GL_FRAMEBUFFER);
  // glDisable(GL_FRAMEBUFFER_SRGB);
  glEnable(GL_FRAMEBUFFER_SRGB);
  renderFullscreenQuad(TextureID::RenderedImage, shaders_["hdrQuad"]);
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

  Length const scale = cachedScale;
  auto fboBinding = binding(finalFbo_, static_cast<GLenum>(GL_FRAMEBUFFER));

  auto const modelMat =
      (Eigen::Translation3f(position) * orientation * size.asDiagonal())
          .matrix();
  auto const mvpMatrix =
      (cameraClient().viewProjectionMatrix(scale) * modelMat).eval();

  shaders_["bbox"].use();
  shaders_["bbox"]["lineColor"] = color;
  shaders_["bbox"]["modelViewProjectionMatrix"] = mvpMatrix;

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
  Length const scale = cachedScale;

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

  auto const mousePos = lastMousePos_;
  // convert mouse position to texture screen coordinates
  auto const windowSize =
      Size2(glfw_.width(), glfw_.height()).cast<double>().eval();
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
  auto const normalizedDepth = *reinterpret_cast<float const *>(
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
      cameraClient().unproject(mousePos, depthInCamera, cachedScale);

  return GeometryNameAndPosition{name, posInWorld, depthInCamera};
}

void VisualizerImpl::dragSelectedGeometry() {
  if (selectedGeometry_.empty()) return;

  // Get mouse ray
  Position const pNear =
      cameraClient().unproject(lastMousePos_, 1.f, cachedScale);
  Position const pFar =
      cameraClient().unproject(lastMousePos_, 1e-12f, cachedScale);
  Position const d = (pFar - pNear).normalized();
  float const alpha = d.transpose() * (selectedPoint_ - pNear);

  // Compute the intersection point of the ray and a plane that is
  // perpendicular to d and on which the selected point lies.
  Position const targetPoint = pNear + alpha * d;

  // Compute movement delta
  Position const moveDelta = targetPoint - selectedPoint_;

  // Get selected geometry and compute a move mask vector
  auto &geometry = *geometries_[selectedGeometry_];
  auto const maskVector = maskToUnitVector(geometry.moveMask);
  auto const maskedMoveDelta = moveDelta.cwiseProduct(maskVector);

  selectedPoint_ += maskedMoveDelta;
  geometry.position += maskedMoveDelta;
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
  shaders_["ambientPass"].use();
  shaders_["ambientPass"]["lightColor"] = ambientColor;

  renderFullscreenQuad(TextureID::Albedo, shaders_["ambientPass"]);
}

void VisualizerImpl::renderDiffuseLighting() {
  std::lock_guard<std::mutex> lock(lightMutex_);

  Length const scale = cachedScale;
  auto const viewMat = cameraClient().viewMatrix(scale);

  shaders_["diffuseLightingPass"].use();
  shaders_["diffuseLightingPass"]["normalAndSpecularTex"] = 0;
  shaders_["diffuseLightingPass"]["albedoTex"] = 1;
  shaders_["diffuseLightingPass"]["topLeft"] = Eigen::Vector2f(-1, 1);
  shaders_["diffuseLightingPass"]["size"] =
      (2 * Eigen::Vector2f::Ones()).eval();

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, textures_[TextureID::NormalsAndSpecular]);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, textures_[TextureID::Albedo]);

  auto boundVao = GL::binding(singleVertexData_.vao);

  for (auto const &lightEntry : lights_) {
    auto const &light = lightEntry.second;

    Expects(std::abs(light.position(3)) < 1e-3);

    PositionH const lightPosition = (viewMat * light.position);

    shaders_["diffuseLightingPass"]["lightPosition"] =
        lightPosition.head<3>().eval();
    shaders_["diffuseLightingPass"]["lightColor"] = light.color;

    // draw quad using the geometry shader
    glDrawArrays(GL_POINTS, 0, 1);
    assertGL("glDrawArrays failed");
  }
}

void VisualizerImpl::renderSpecularLighting() {
  std::lock_guard<std::mutex> lock(lightMutex_);

  Length const scale = cachedScale;
  auto const viewMat = cameraClient().viewMatrix(scale);

  shaders_["specularLightingPass"].use();
  shaders_["specularLightingPass"]["normalAndSpecularTex"] = 0;
  shaders_["specularLightingPass"]["albedoTex"] = 1;
  shaders_["specularLightingPass"]["topLeft"] = Eigen::Vector2f(-1, 1);
  shaders_["specularLightingPass"]["size"] =
      (2 * Eigen::Vector2f::Ones()).eval();

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, textures_[TextureID::NormalsAndSpecular]);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, textures_[TextureID::Albedo]);

  auto boundVao = GL::binding(singleVertexData_.vao);

  for (auto const &lightEntry : lights_) {
    auto const &light = lightEntry.second;

    Expects(std::abs(light.position(3)) < 1e-3);

    PositionH const lightPosition = (viewMat * light.position);

    shaders_["specularLightingPass"]["lightPosition"] =
        lightPosition.head<3>().eval();
    shaders_["specularLightingPass"]["lightColor"] = light.color;

    // draw quad using the geometry shader
    glDrawArrays(GL_POINTS, 0, 1);
    assertGL("glDrawArrays failed");
  }
}

} // namespace Private_
} // namespace VolViz

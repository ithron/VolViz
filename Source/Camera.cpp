#include "Camera.h"

namespace VolViz {

Private_::CameraClient Camera::client() const noexcept {
  return Private_::CameraClient(*this);
}

Camera::Camera() noexcept {
  orientation.afterAction = [this](auto const &) {
    cachedViewMatrix_.markAsDirty();
    cachedViewProjectionMatrix_.markAsDirty();
  };

  position.afterAction = [this](auto const &) {
    cachedViewMatrix_.markAsDirty();
    cachedViewProjectionMatrix_.markAsDirty();
  };

  verticalFieldOfView.afterAction = [this](auto const &) {
    cachedVerticalFOV_.markAsDirty();
    cachedProjectionMatrix_.markAsDirty();
    cachedViewProjectionMatrix_.markAsDirty();
  };

  aspectRatio.afterAction = [this](auto const &) {
    cachedProjectionMatrix_.markAsDirty();
    cachedViewProjectionMatrix_.markAsDirty();
  };
}

Matrix4 Camera::projectionMatrix() const noexcept {
  float const aspect = aspectRatio;
  Angle const FOV = cachedVerticalFOV_;

  float const f = 1.f / gsl::narrow_cast<float>(std::tan(FOV / 2.0));

  Matrix4 P;
  P << f / aspect, 0, 0, 0, 0, f, 0, 0, 0, 0, 0, f, 0, 0, -1, f;

  return P;
}

Matrix4 Camera::viewMatrix() const noexcept {
  using namespace literals;

  Expects(cachedScale_ > 0_mm);

  PhysicalPosition const ppos = position;
  Orientation const ori = orientation;

  Position const scaledPosition{
      gsl::narrow_cast<float>(ppos(0) / cachedScale_),
      gsl::narrow_cast<float>(ppos(1) / cachedScale_),
      gsl::narrow_cast<float>(ppos(2) / cachedScale_)};

  return (ori * Eigen::Translation3f(scaledPosition)).inverse().matrix();
}

Matrix4 Camera::viewProjectionMatrix() const noexcept {
  Matrix4 const P = cachedProjectionMatrix_;
  Matrix4 const V = cachedViewMatrix_;

  return P * V;
}

Position Camera::unproject(Position2 const &screenPos, float depth,
                           Length ambientScale) const noexcept {
  Angle const FOV = cachedVerticalFOV_;
  float const f = 1.f / gsl::narrow_cast<float>(std::tan(FOV / 2.0));
  auto const w = f / depth;
  PositionH const p = PositionH(screenPos(0), screenPos(1), depth, 1) * w;
  cachedScale_ = ambientScale;

  return (viewProjectionMatrix().inverse() * p).head<3>();
}

} // namespace VolViz

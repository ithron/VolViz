#include "Camera.h"

namespace VolViz {

CameraClient Camera::client() const noexcept { return CameraClient(*this); }

Matrix4 Camera::projectionMatrix() const noexcept {
  float const aspect = aspectRatio;
  Angle const FOV = verticalFieldOfView;

  float const f = 1.f / gsl::narrow_cast<float>(std::tan(FOV / 2.0));

  Matrix4 P;
  P << f / aspect, 0, 0, 0, 0, f, 0, 0, 0, 0, 0, f, 0, 0, -1, f;

  return P;
}

Matrix4 Camera::viewMatrix() const noexcept {
  PhysicalPosition const ppos = position;
  Orientation const ori = orientation;

  Position const scaledPosition{
      gsl::narrow_cast<float>(ppos(0) / cachedScale_),
      gsl::narrow_cast<float>(ppos(1) / cachedScale_),
      gsl::narrow_cast<float>(ppos(2) / cachedScale_)};

  return (Eigen::Translation3f(scaledPosition) * ori).inverse().matrix();
}

Matrix4 Camera::viewProjectionMatrix() const noexcept {
  Matrix4 const P = cachedProjectionMatrix_;
  Matrix4 const V = cachedViewMatrix_;

  return P * V;
}

} // namespace VolViz

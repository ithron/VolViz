#include "AxisAlignedPlane.h"
#include "VisualizerImpl.h"

namespace VolViz {
namespace Private_ {

AxisAlignedPlane::AxisAlignedPlane(AxisAlignedPlaneDescriptor const &descriptor,
                                   VisualizerImpl &visualizer)
    : visualizer_(visualizer) {

  Length const refScale = visualizer_.scale;
  float intercept = 1.f;

  if (abs(descriptor.intercept / refScale) < 1e-6) {
    scale = refScale;
    intercept = 0.f;
  } else {
    scale = descriptor.intercept;
  }

  // set dependent properties
  switch (descriptor.axis) {
    case Axis::X:
      position = intercept * Position::UnitX();
      moveMask = MoveMask::X;
      orientation = AngleAxisf(d90, -Position::UnitY());
      break;
    case Axis::Y:
      position = intercept * Position::UnitY();
      moveMask = MoveMask::Y;
      orientation = AngleAxisf(d90, -Position::UnitX());
      break;
    case Axis::Z:
      position = intercept * Position::UnitZ();
      moveMask = MoveMask::Z;
      orientation = Orientation::Identity();
      break;
  }

  color = descriptor.color;
}

void AxisAlignedPlane::doInit() {}

void AxisAlignedPlane::doRender(std::uint32_t index, bool /*selected*/) {
  auto const &camera = visualizer_.camera();
  Length const rScale = visualizer_.cachedScale;

  auto const viewMat = camera.client().viewMatrix(rScale);
  auto const scale = static_cast<float>(scale / rScale);
  auto const volSize = visualizer_.volumeSize();

  auto const modelMat = (Eigen::Translation3f(position * scale) * orientation *
                         volSize.asDiagonal())
                            .matrix();

  auto const modelViewMat = (viewMat * modelMat).eval();
  auto const inverseModelViewMatrix =
      modelViewMat.block<3, 3>(0, 0).inverse().eval();

  auto &shaders = visualizer_.shaders();

  shaders["plane"].use();
  shaders["plane"]["index"] = index;
  shaders["plane"]["volume"] = 0;
  shaders["plane"]["modelMatrix"] = modelMat;
  shaders["plane"]["shininess"] = 10.f;
  shaders["plane"]["color"] = selected ? (color * 1.5f).eval() : color;
  shaders["plane"]["modelViewProjectionMatrix"] =
      (camera().client().projectionMatrix() * modelViewMat).eval();
  shaders["plane"]["inverseModelViewMatrix"] = inverseModelViewMatrix;
  shaders["plane"]["textureTransformMatrix"] = textureTransformationMatrix();

  auto boundVao = GL::binding(singleVertexData_.vao);
  glDrawArrays(GL_POINTS, 0, 1);
  assertGL("glDrawArrays failed");
}

} // namespace Private_
} // namespace VolViz

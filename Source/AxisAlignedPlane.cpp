#include "AxisAlignedPlane.h"
#include "Visualizer.h"

namespace VolViz {
namespace Private_ {

AxisAlignedPlane::AxisAlignedPlane(AxisAlignedPlaneDescriptor const &descriptor,
                                   Visualizer &visualizer)
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

void AxisAlignedPlane::doRender() {
  // TODO: implement
}

} // namespace Private_
} // namespace VolViz

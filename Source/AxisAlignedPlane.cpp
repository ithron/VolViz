#include "AxisAlignedPlane.h"

namespace VolViz {
namespace Private_ {

AxisAlignedPlane::AxisAlignedPlane(AxisAlignedPlaneDescriptor const &descriptor)
    : descriptor_(descriptor) {

  float intercept = 1.f;

  if (abs(descriptor.intercept / refScale) < 1e-6) {
    geom.scale = refScale;
    intercept = 0.f;
  } else {
    geom.scale = plane.intercept;
  }

  // set dependent properties
  switch (descriptor.axis) {
    case Axis::X:
      position = intercept * Position::UnitX();
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
}

void AxisAlignedPlane::doInit() {
  // TODO: implement
}

void AxisAlignedPlane::doRender() {
  // TODO: implement
}

} // namespace Private_
} // namespace VolViz

#pragma once

#include "Geometry.h"

namespace VolViz {
namespace Private_ {

class VisualizerImpl;

class GeometryFactory {
public:
  using GeometryPtr = Geometry::UniquePtr;

  GeometryFactory(VisualizerImpl &visualizer);

  GeometryPtr create(AxisAlignedPlaneDescriptor const &descriptor);

private:
  VisualizerImpl &visualizer_;
};

} // namespace Private_
} // namespace VolViz

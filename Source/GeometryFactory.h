#pragma once

#include "Geometry.h"

namespace VolViz {
namespace Private_ {

class Visualizer;

class GeometryFactory {
public:
  using GeometryPtr = Geometry::UniquePtr;

  GeometryFactory(Visualizer &visualizer);

  GeometryPtr create(AxisAlignedPlaneDescriptor const &descriptor);

private:
  VisualizerImpl &visualizer_;
};

} // namespace Private_
} // namespace VolViz

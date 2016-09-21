#include "GeometryFactory.h"

#include "AxisAlignedPlane.h"
#include "VisualizerImpl.h"

namespace VolViz {
namespace Private_ {

GeometryFactory::GeometryFactory(VisualizerImpl &visualizer)
    : visualizer_(visualizer) {}

GeometryFactory::GeometryPtr
GeometryFactory::create(AxisAlignedPlaneDescriptor const &descriptor) {
  return std::make_unique<AxisAlignedPlane>(descriptor, visualizer_);
}

} // namespace Private_
} // namespace VolViz

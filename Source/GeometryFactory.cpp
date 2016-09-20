#include "GeometryFactory.h"

#include "Visualizer.h"

namespace VolViz {
namespace Private_ {

GeometryFactory::GeometryFactory(Visualizer &visualizer)
    : visualizer_(visualizer) {}

GeometryFactory::GeometryPtr
GeometryFactory::create(AxisAlignedPlaneDescriptor const &descriptor) {
  return std::make_unique<AxisAlignedPlane>(descriptor, visualizer_);
}

} // namespace Private_
} // namespace VolViz

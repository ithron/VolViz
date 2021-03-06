#include "GeometryFactory.h"

#include "AxisAlignedPlane.h"
#include "Cube.h"
#include "Mesh.h"
#include "VisualizerImpl.h"

namespace VolViz {
namespace Private_ {

GeometryFactory::GeometryFactory(VisualizerImpl &visualizer)
    : visualizer_(visualizer) {}

GeometryFactory::GeometryPtr
GeometryFactory::create(AxisAlignedPlaneDescriptor const &descriptor) {
  return std::make_unique<AxisAlignedPlane>(descriptor, visualizer_);
}

GeometryFactory::GeometryPtr
GeometryFactory::create(CubeDescriptor const &descriptor) {
  return std::make_unique<Cube>(descriptor, visualizer_);
}

GeometryFactory::GeometryPtr
GeometryFactory::create(MeshDescriptor const &descriptor) {
  return std::make_unique<Mesh>(descriptor, visualizer_);
}

} // namespace Private_
} // namespace VolViz

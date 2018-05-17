#include "Cube.h"
#include "VisualizerImpl.h"

namespace VolViz {
namespace Private_ {

Cube::Cube(CubeDescriptor const &descriptor, VisualizerImpl &visualizer)
    : Geometry(descriptor, visualizer) {

  position = descriptor.position;
  scale = descriptor.scale;

  radius = descriptor.radius;
}

void Cube::doInit() {}

void Cube::doRender(std::uint32_t index, bool selected) {
  auto const &cameraClient = visualizer_.cameraClient();
  Length const rScale = visualizer_.cachedScale;

  auto const viewMat = cameraClient.viewMatrix(rScale);
  auto const destScale = static_cast<float>(scale / rScale);

  auto const modelMat = (Eigen::Translation3f(position) * orientation *
                         Eigen::Scaling(destScale * radius))
                            .matrix();

  auto const modelViewMat = (viewMat * modelMat).eval();
  auto const inverseModelViewMatrix =
      modelViewMat.block<3, 3>(0, 0).inverse().eval();

  auto &shaders = visualizer_.shaders();

  shaders["cube"].use();
  visualizer_.attachVolumeToShader(shaders["cube"]);
  shaders["cube"]["index"] = index;
  shaders["cube"]["modelMatrix"] = modelMat;
  shaders["cube"]["shininess"] = 10.f;
  shaders["cube"]["color"] = selected ? (color * 1.5f).eval() : color;
  shaders["cube"]["modelViewProjectionMatrix"] =
      (cameraClient.projectionMatrix() * modelViewMat).eval();
  shaders["cube"]["inverseModelViewMatrix"] = inverseModelViewMatrix;
  shaders["cube"]["textureTransformMatrix"] =
      visualizer_.textureTransformationMatrix();

  visualizer_.drawSingleVertex();
}

void Cube::doUpdate() {
  CubeDescriptor descriptor;
  if (!updateQueue_.try_dequeue(descriptor)) return;

  position = descriptor.position;
  radius = descriptor.radius;
  color = descriptor.color;
  scale = descriptor.scale;
}

void Cube::doEnqueueUpdate(GeometryDescriptor const &descriptor) {
  updateQueue_.enqueue(dynamic_cast<CubeDescriptor const &>(descriptor));
}

void Cube::doEnqueueUpdate(GeometryDescriptor &&descriptor) {
  updateQueue_.enqueue(std::move(dynamic_cast<CubeDescriptor &&>(descriptor)));
}

} // namespace Private_
} // namespace VolViz

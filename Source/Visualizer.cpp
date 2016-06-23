#include "Visualizer.h"
#include "VisualizerImpl.h"

#include <Eigen/Core>
#include <gsl.h>

#include <iostream>
#include <mutex>

namespace VolViz {

Visualizer::Visualizer()
    : impl_(std::make_unique<Private_::VisualizerImpl>(this)) {}

Visualizer::~Visualizer() = default;

Visualizer::Visualizer(Visualizer &&rhs) : impl_(std::move(rhs.impl_)) {
  impl_->visualizer_ = this;
}

Visualizer &Visualizer::operator=(Visualizer &&rhs) {
  using std::swap;
  swap(impl_, rhs.impl_);
  impl_->visualizer_ = this;
  return *this;
}

void Visualizer::start() { impl_->start(); }

Visualizer::operator bool() const noexcept { return *impl_; }

template <class T>
void Visualizer::setVolume(VolumeDescriptor const &descriptor,
                           gsl::span<T> data) {
  impl_->setVolume(descriptor, data);
}

template void Visualizer::setVolume(VolumeDescriptor const &,
                                    gsl::span<float const>);
template void Visualizer::setVolume(VolumeDescriptor const &,
                                    gsl::span<Color const>);

void Visualizer::renderOneFrame() { impl_->renderOneFrame(); }

void Visualizer::addLight(LightName name, Light const &light) {
  impl_->addLight(name, light);
}

void Visualizer::addGeometry(GeometryName name, AxisAlignedPlane const &plane) {
  impl_->addGeometry(name, plane);
}

template <class VertBase, class IdxBase>
void Visualizer::setMesh(Eigen::MatrixBase<VertBase> const &V,
                         Eigen::MatrixBase<IdxBase> const &I) {
  impl_->setMesh(V, I);
}

template void Visualizer::setMesh<>(Eigen::MatrixBase<Eigen::MatrixXd> const &,
                                    Eigen::MatrixBase<Eigen::MatrixXi> const &);

} // namespace VolViz

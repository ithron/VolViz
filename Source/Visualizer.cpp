#include "Visualizer.h"
#include "GeometryDescriptor.h"
#include "VisualizerImpl.h"

#include <Eigen/Core>
#include <gsl.h>

#include <iostream>
#include <mutex>
#include <type_traits>

namespace VolViz {

Visualizer::Visualizer()
    : impl_(std::make_unique<Private_::VisualizerImpl>(this)) {
  scale.afterAction = [this](auto const &) {
    impl_->cachedScale.markAsDirty();
  };
}

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

template <class Descriptor, typename>
void Visualizer::addGeometry(GeometryName name, Descriptor const &geom) {
  impl_->addGeometry(name, geom);
}

template void Visualizer::addGeometry<AxisAlignedPlaneDescriptor>(
    GeometryName name, AxisAlignedPlaneDescriptor const &);

template <class VertBase, class IdxBase>
void Visualizer::setMesh(Eigen::MatrixBase<VertBase> const &V,
                         Eigen::MatrixBase<IdxBase> const &I) {
  impl_->setMesh(V, I);
}

template void Visualizer::setMesh<>(Eigen::MatrixBase<Eigen::MatrixXd> const &,
                                    Eigen::MatrixBase<Eigen::MatrixXi> const &);

} // namespace VolViz

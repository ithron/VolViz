#include "Visualizer.h"
#include "GeometryDescriptor.h"
#include "VisualizerImpl.h"

#include <Eigen/Core>
#include <gsl.h>

#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
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

void Visualizer::renderOneFrame() { impl_->renderOneFrame(false); }

void Visualizer::renderOneFrameAndWaitForEvents() {
  impl_->renderOneFrame(true);
}

void Visualizer::renderAtFPS(double fps) {
  using Clock = std::chrono::steady_clock;
  using namespace std::chrono_literals;

  auto const frameDuration = 1.0 / fps * 1s;

  while (*this) {
    auto const t0 = Clock::now();

    renderOneFrame();

    auto const t1 = Clock::now();
    auto const deltaT = t1 - t0;

    if (deltaT < frameDuration)
      std::this_thread::sleep_for(frameDuration - deltaT);
  }
}

void Visualizer::renderOnUserInteraction(double maxFps) {
  using Clock = std::chrono::steady_clock;
  using namespace std::chrono_literals;

  auto const frameDuration = 1.0 / maxFps * 1s;

  while (*this) {
    auto const t0 = Clock::now();

    renderOneFrameAndWaitForEvents();

    auto const t1 = Clock::now();
    auto const deltaT = t1 - t0;

    if (deltaT < frameDuration)
      std::this_thread::sleep_for(frameDuration - deltaT);
  }
}

void Visualizer::addLight(LightName name, Light const &light) {
  impl_->addLight(name, light);
}

template <class Descriptor, typename>
void Visualizer::addGeometry(GeometryName name, Descriptor const &geom) {
  impl_->addGeometry(name, geom);
}

template void Visualizer::addGeometry<AxisAlignedPlaneDescriptor>(
    GeometryName name, AxisAlignedPlaneDescriptor const &);

template void Visualizer::addGeometry<MeshDescriptor>(GeometryName name,
                                                      MeshDescriptor const &);

template <class Descriptor, typename>
void Visualizer::updateGeometry(GeometryName name, Descriptor &&geom) {
  impl_->updateGeometry(name, std::forward<Descriptor>(geom));
}

template void Visualizer::updateGeometry<AxisAlignedPlaneDescriptor const &>(
    GeometryName name, AxisAlignedPlaneDescriptor const &);
template void Visualizer::updateGeometry<AxisAlignedPlaneDescriptor &&>(
    GeometryName name, AxisAlignedPlaneDescriptor &&);
template void Visualizer::updateGeometry<AxisAlignedPlaneDescriptor &>(
    GeometryName name, AxisAlignedPlaneDescriptor &);

template void
Visualizer::updateGeometry<MeshDescriptor const &>(GeometryName name,
                                                   MeshDescriptor const &);
template void Visualizer::updateGeometry<MeshDescriptor &&>(GeometryName name,
                                                            MeshDescriptor &&);
template void Visualizer::updateGeometry<MeshDescriptor &>(GeometryName name,
                                                           MeshDescriptor &);

} // namespace volviz

#include "Visualizer.h"
#include "GeometryDescriptor.h"
#include "VisualizerImpl.h"

#include <Eigen/Core>

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

void Visualizer::enableMultithreading() noexcept {
  impl_->enableMultithreading();
}

Visualizer::operator bool() const noexcept { return *impl_; }

template <class T>
void Visualizer::setVolume(VolumeDescriptor const &descriptor, span<T> data) {
  impl_->setVolume(descriptor, data);
}

template void Visualizer::setVolume<float const>(VolumeDescriptor const &,
                                                 span<float const>);
template void Visualizer::setVolume<Color const>(VolumeDescriptor const &,
                                                 span<Color const>);

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

template void Visualizer::addGeometry<CubeDescriptor>(GeometryName name,
                                                      CubeDescriptor const &);

template void Visualizer::addGeometry<MeshDescriptor>(GeometryName name,
                                                      MeshDescriptor const &);

template <class Descriptor, typename>
bool Visualizer::updateGeometry(GeometryName name, Descriptor &&geom) {
  return impl_->updateGeometry(name, std::forward<Descriptor>(geom));
}

template bool Visualizer::updateGeometry<AxisAlignedPlaneDescriptor const &>(
    GeometryName name, AxisAlignedPlaneDescriptor const &);
template bool Visualizer::updateGeometry<AxisAlignedPlaneDescriptor &&>(
    GeometryName name, AxisAlignedPlaneDescriptor &&);
template bool Visualizer::updateGeometry<AxisAlignedPlaneDescriptor &>(
    GeometryName name, AxisAlignedPlaneDescriptor &);

template bool
Visualizer::updateGeometry<CubeDescriptor const &>(GeometryName name,
                                                   CubeDescriptor const &);
template bool Visualizer::updateGeometry<CubeDescriptor &&>(GeometryName name,
                                                            CubeDescriptor &&);
template bool Visualizer::updateGeometry<CubeDescriptor &>(GeometryName name,
                                                           CubeDescriptor &);

template bool
Visualizer::updateGeometry<MeshDescriptor const &>(GeometryName name,
                                                   MeshDescriptor const &);
template bool Visualizer::updateGeometry<MeshDescriptor &&>(GeometryName name,
                                                            MeshDescriptor &&);
template bool Visualizer::updateGeometry<MeshDescriptor &>(GeometryName name,
                                                           MeshDescriptor &);

} // namespace VolViz

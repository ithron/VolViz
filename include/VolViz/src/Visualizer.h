#ifndef VolViz_Visualizer_h
#define VolViz_Visualizer_h

#include "AtomicWrapper.h"
#include "Camera.h"
#include "GeometryDescriptor.h"
#include "Light.h"
#include "Types.h"
#include "Volume.h"

#include <Eigen/Core>

#include <atomic>
#include <memory>
#include <functional>

namespace VolViz {

namespace Private_ {
class VisualizerImpl;
} // namespace Private_

class Visualizer {
  template <class T>
  using AtomicProperty = AtomicWrapper<T, SetAndNotifyPolicy>;

public:
  using LightName = std::uint16_t;
  using GeometryName = std::string;

  static auto constexpr kTitle = "Volume Visualizer";

  Visualizer();

  ~Visualizer();

  Visualizer(Visualizer const &) = delete;
  Visualizer(Visualizer &&);

  Visualizer &operator=(Visualizer const &) = delete;
  Visualizer &operator=(Visualizer &&);

  void start();

  void enableMultithreading() noexcept;

  void renderOneFrame();

  void renderOneFrameAndWaitForEvents();

  void renderAtFPS(double fps = 60.0);

  void renderOnUserInteraction(double maxFps = 60.0);

  operator bool() const noexcept;

  template <class T>
  void setVolume(VolumeDescriptor const &descriptor, span<T> data);

  void addLight(LightName name, Light const &light);

  template <class Descriptor,
            typename = std::enable_if_t<std::is_base_of<
                GeometryDescriptor, std::decay_t<Descriptor>>::value>>
  void addGeometry(GeometryName name, Descriptor const &geom);

  template <class Descriptor,
            typename = std::enable_if_t<std::is_base_of<
                GeometryDescriptor, std::decay_t<Descriptor>>::value>>
  bool updateGeometry(GeometryName name, Descriptor &&geom);

  std::atomic<bool> showGrid{true};
  std::atomic<bool> showVolumeBoundingBox{true};
  AtomicProperty<Length> scale{1 * milli * meter};
  AtomicProperty<Color> backgroundColor{Colors::Black()};

  /// The camera
  Camera camera;

private:
  std::unique_ptr<Private_::VisualizerImpl> impl_;
};

// Explicit template instaciation declarations
#pragma mark Explicit template instanciation declarations
extern template void Visualizer::addGeometry<AxisAlignedPlaneDescriptor>(
    GeometryName, AxisAlignedPlaneDescriptor const &);

extern template void
Visualizer::addGeometry<CubeDescriptor>(GeometryName, CubeDescriptor const &);

extern template void
Visualizer::addGeometry<MeshDescriptor>(GeometryName, MeshDescriptor const &);

extern template void
Visualizer::setVolume<float const>(VolumeDescriptor const &, span<float const>);
extern template void
Visualizer::setVolume<Color const>(VolumeDescriptor const &, span<Color const>);

extern template bool
Visualizer::updateGeometry<AxisAlignedPlaneDescriptor const &>(
    GeometryName name, AxisAlignedPlaneDescriptor const &);
extern template bool Visualizer::updateGeometry<AxisAlignedPlaneDescriptor &&>(
    GeometryName name, AxisAlignedPlaneDescriptor &&);
extern template bool Visualizer::updateGeometry<AxisAlignedPlaneDescriptor &>(
    GeometryName name, AxisAlignedPlaneDescriptor &);

extern template bool
Visualizer::updateGeometry<CubeDescriptor const &>(GeometryName name,
                                                   CubeDescriptor const &);
extern template bool
Visualizer::updateGeometry<CubeDescriptor &&>(GeometryName name,
                                              CubeDescriptor &&);
extern template bool
Visualizer::updateGeometry<CubeDescriptor &>(GeometryName name,
                                             CubeDescriptor &);

extern template bool
Visualizer::updateGeometry<MeshDescriptor const &>(GeometryName name,
                                                   MeshDescriptor const &);
extern template bool
Visualizer::updateGeometry<MeshDescriptor &&>(GeometryName name,
                                              MeshDescriptor &&);
extern template bool
Visualizer::updateGeometry<MeshDescriptor &>(GeometryName name,
                                             MeshDescriptor &);

} // namespace VolViz

#endif // VolViz_Visualizer_h

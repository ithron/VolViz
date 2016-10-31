#ifndef VolViz_Visualizer_h
#define VolViz_Visualizer_h

#include "AtomicWrapper.h"
#include "Camera.h"
#include "Geometry.h"
#include "Light.h"
#include "Types.h"
#include "Volume.h"

#include <Eigen/Core>

#include <atomic>
#include <memory>

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

  /// The camera
  Camera camera;

private:
  std::unique_ptr<Private_::VisualizerImpl> impl_;
};

} // namespace VolViz

#endif // VolViz_Visualizer_h

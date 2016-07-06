#ifndef VolViz_Visualizer_h
#define VolViz_Visualizer_h

#include "AtomicWrapper.h"
#include "Camera.h"
#include "Geometry.h"
#include "Light.h"
#include "Types.h"
#include "Volume.h"

#include <Eigen/Core>
#include <gsl.h>

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

  void renderOneFrame();

  operator bool() const noexcept;

  template <class T>
  void setVolume(VolumeDescriptor const &descriptor, gsl::span<T> data);

  template <class VertBase, class IdxBase>
  void setMesh(Eigen::MatrixBase<VertBase> const &V,
               Eigen::MatrixBase<IdxBase> const &I);

  void addLight(LightName name, Light const &light);

  void addGeometry(GeometryName name, AxisAlignedPlane const &plane);

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

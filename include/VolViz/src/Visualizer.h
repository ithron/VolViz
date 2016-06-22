#ifndef VolViz_Visualizer_h
#define VolViz_Visualizer_h

#include "AtomicWrapper.h"
#include "Light.h"
#include "Types.h"

#include <Eigen/Core>

#include <atomic>
#include <memory>

namespace VolViz {

namespace Private_ {
class VisualizerImpl;
} // namespace Private_

class Visualizer {
public:
  using LightName = std::uint16_t;

  static auto constexpr kDefaultFOV = 110;
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

  template <class VertBase, class IdxBase>
  void setMesh(Eigen::MatrixBase<VertBase> const &V,
               Eigen::MatrixBase<IdxBase> const &I);

  void addLight(LightName name, Light const &light);

  std::atomic<bool> showGrid{true};
  AtomicWrapper<Length> scale{1 * milli * meter};

private:
  std::unique_ptr<Private_::VisualizerImpl> impl_;
};

} // namespace VolViz

#endif // VolViz_Visualizer_h

#ifndef VolViz_Visualizer_h
#define VolViz_Visualizer_h

#include <Eigen/Core>

#include <memory>
#include <atomic>

namespace VolViz {

namespace Private_ {
class VisualizerImpl;
} // namespace Private_

class Visualizer {
public:
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

  std::atomic<bool> showGrid{true};

  std::atomic<float> ambientFactor{1.f};

private:
  std::unique_ptr<Private_::VisualizerImpl> impl_;
};

} // namespace VolViz

#endif // VolViz_Visualizer_h

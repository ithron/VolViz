#ifndef VolViz_Visualizer_h
#define VolViz_Visualizer_h

#include <Eigen/Core>
#include <gsl.h>

#include <atomic>
#include <memory>

namespace VolViz {

namespace Private_ {
class VisualizerImpl;
} // namespace Private_

/// Normalized RGB color
using Color = Eigen::Vector3f;
/// Position in 3D euclidean space
using Position = Eigen::Vector3f;
/// Position in homogenous coordinates
using PositionH = Eigen::Vector4f;

/// Point or directional light
class Light {
public:
  /// Color of the light
  Color color{Color::Ones()};
  /// Position of the light source. If the w (i.e. fourth) component is
  /// zero, the light position is a infinie point. This can be used to model
  /// directional light
  PositionH position{PositionH::Zero()};

  /// The light's attenuation factor. The attenuation is computes as
  /// 1 / (1 + factor * d^2), where d ist the distacne between a surface point
  /// and the light source.
  float attenuationFactor = 1.f;

  /// Factor that specifies how the light contributes to the ambient lighting
  float ambientFactor = 0.f;

  inline float distanceAtForAttenuation(float att) const noexcept {
    Expects(attenuationFactor > 0.f);
    auto const sqDist = (1.f / att - 1.f) / attenuationFactor;

    Ensures(sqDist >= 0.f);

    return std::sqrt(sqDist);
  }
};

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

private:
  std::unique_ptr<Private_::VisualizerImpl> impl_;
};

} // namespace VolViz

#endif // VolViz_Visualizer_h

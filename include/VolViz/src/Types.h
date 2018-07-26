#ifndef VolViz_Types_h
#define VolViz_Types_h

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <gsl/gsl>
#include <phys/units/quantity.hpp>

#include <array>

namespace VolViz {

using namespace gsl;

using Vector3f = Eigen::Vector3f;

using Matrix3 = Eigen::Matrix3f;
using Matrix4 = Eigen::Matrix4f;

/// Position in 3D euclidean space
using Position = Eigen::Vector3f;
/// Position in homogenous coordinates
using PositionH = Eigen::Vector4f;

/// Position in 2D space
using Position2 = Eigen::Vector2f;

using Size2 = Eigen::Matrix<std::size_t, 2, 1>;
using Size3 = Eigen::Matrix<std::size_t, 3, 1>;

using Size3f = Eigen::Vector3f;

/// An 1D range
template <class T> struct Range {
  T min, max;
  constexpr auto length() const noexcept { return max - min; }
};

/// Normalized RGB color
using Color = Eigen::Vector3f;

namespace Colors {
inline auto Black() noexcept { return Color::Zero(); }
inline auto White() noexcept { return Color::Ones(); }
inline auto Red() noexcept { return Color::UnitX(); }
inline auto Green() noexcept { return Color::UnitY(); }
inline auto Blue() noexcept { return Color::UnitZ(); }
inline auto Yellow() noexcept { return Red() + Green(); }
inline auto Magenta() noexcept { return Red() + Blue(); }
inline auto Cyan() noexcept { return Blue() + Green(); }
}

/// 6-DOF orientation, represented as a quaternion
using Orientation = Eigen::Quaternionf;

struct DepthRange {
  float near{1.f}, far{-1.f};
};

enum class Axis { X, Y, Z };

using Scale = float;

using Length = phys::units::quantity<phys::units::length_d>;
using Angle = double;
using PhysicalPosition = Eigen::Matrix<Length, 3, 1>;

using phys::units::meter;
using phys::units::rad;
using phys::units::degree_angle;

using phys::units::centi;
using phys::units::milli;
using phys::units::micro;
using phys::units::nano;

using phys::units::abs;

namespace literals = phys::units::literals;

using VoxelSize = std::array<Length, 3>;

/// @brief Conveniance wrapper for span<>
/// @{
template <class T>
inline constexpr auto as_span(T *ptr,
                              typename span<T>::index_type size) noexcept {
  return span<T>(ptr, size);
}

template <
    class Container,
    class = std::enable_if_t<
        !details::is_span<Container>::value &&
        !details::is_std_array<Container>::value &&
        std::is_convertible<typename Container::pointer,
                            decltype(std::declval<Container>().data())>::value>>
inline constexpr auto as_span(Container &c) {
  return span<typename Container::value_type>(c);
}

template <
    class Container,
    class = std::enable_if_t<
        !details::is_span<Container>::value &&
        !details::is_std_array<Container>::value &&
        std::is_convertible<typename Container::pointer,
                            decltype(std::declval<Container>().data())>::value>>
inline constexpr auto as_span(Container const &c) {
  return span<typename Container::value_type const>(c);
}
///@}

} // namespace VolViz

#if __cplusplus < 201703L

namespace std {

template<class T>
constexpr const T& clamp( const T& v, const T& lo, const T& hi ) {
    return clamp( v, lo, hi, std::less<>() );
}

template<class T, class Compare>
constexpr const T& clamp( const T& v, const T& lo, const T& hi, Compare comp ) {
      return assert( !comp(hi, lo) ),
             comp(v, lo) ? lo : comp(hi, v) ? hi : v;
  }

}

#endif

#endif // VolViz_Types_h

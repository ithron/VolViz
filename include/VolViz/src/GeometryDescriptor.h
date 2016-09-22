#ifndef VolViz_Geometry_h
#define VolViz_Geometry_h

#include "Types.h"

namespace VolViz {

enum MoveMask : uint8_t {
  None = 0x00,
  X = 0x01,
  Y = 0x02,
  Z = 0x04,
  All = 0x07
};

inline Vector3f maskToUnitVector(MoveMask mask) noexcept {
  Vector3f v = Vector3f::Zero();
  auto maskRep = static_cast<uint8_t>(mask);

  Expects(maskRep <= 0x07);

  int idx{0};
  while (maskRep != 0) {
    if (maskRep & 0x01) v(idx) = 1.f;
    maskRep >>= 1;
    ++idx;
  }

  return v;
}

struct GeometryDescriptor {
  bool movable{true};
  Color color{Colors::White()};
};

/// A geometry descriptor describing a axis aligned plane
struct AxisAlignedPlaneDescriptor : public GeometryDescriptor {
  Length intercept{0 * meter};
  Axis axis{Axis::X};
};

/// A geometry descriptor describing an arbitrary triangle mesh
struct MeshDescriptor : public GeometryDescriptor {
  Eigen::Matrix<float, Eigen::Dynamic, 3> vertices;
  Eigen::Matrix<std::uint32_t, Eigen::Dynamic, 3> indices;
};

} // namespace VolViz

#endif // VolViz_Geometry_h

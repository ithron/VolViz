#pragma once

#include "Types.h"

#pragma clang diagnostic ignored "-Wpadded"

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

class GeometryDescriptor {
public:
  bool movable{true};
  Color color{Colors::White()};

protected:
  GeometryDescriptor() = default;
  virtual ~GeometryDescriptor();

  GeometryDescriptor(GeometryDescriptor const &) = default;
  GeometryDescriptor(GeometryDescriptor &&) = default;

  GeometryDescriptor &operator=(GeometryDescriptor const &) = default;
  GeometryDescriptor &operator=(GeometryDescriptor &&) = default;
};

/// A geometry descriptor describing a axis aligned plane
class AxisAlignedPlaneDescriptor : public GeometryDescriptor {
public:
  Length intercept{0 * meter};
  Axis axis{Axis::X};

  AxisAlignedPlaneDescriptor() = default;
  AxisAlignedPlaneDescriptor(AxisAlignedPlaneDescriptor const &) = default;
  AxisAlignedPlaneDescriptor(AxisAlignedPlaneDescriptor &&) = default;

  virtual ~AxisAlignedPlaneDescriptor();

  AxisAlignedPlaneDescriptor &
  operator=(AxisAlignedPlaneDescriptor const &) = default;
  AxisAlignedPlaneDescriptor &
  operator=(AxisAlignedPlaneDescriptor &&) = default;
};

/// A geometry descriptor describing an arbitrary triangle mesh
class MeshDescriptor : public GeometryDescriptor {
public:
  virtual ~MeshDescriptor();

  MeshDescriptor() = default;
  MeshDescriptor(MeshDescriptor const &) = default;
  MeshDescriptor(MeshDescriptor &&) = default;

  MeshDescriptor &operator=(MeshDescriptor const &) = default;
  MeshDescriptor &operator=(MeshDescriptor &&) = default;

  Eigen::Matrix<float, Eigen::Dynamic, 3> vertices;
  Eigen::Matrix<std::uint32_t, Eigen::Dynamic, 3> indices;
  Length scale{1 * milli * meter};
};

/// A geomentry descriptor describing an axis aligned cube
class CubeDescriptor : public GeometryDescriptor {
public:
  virtual ~CubeDescriptor();

  CubeDescriptor() = default;
  CubeDescriptor(CubeDescriptor const &) = default;
  CubeDescriptor(CubeDescriptor &&) = default;

  CubeDescriptor &operator=(CubeDescriptor const &) = default;
  CubeDescriptor &operator=(CubeDescriptor &&) = default;

  Position position{Position::Zero()};
  Length scale{1 * milli * meter};

  Scale radius = 0.5f;
};

} // namespace VolViz

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

struct GeometryDescriptor {
  bool movable{true};
  Color color{Colors::White()};
};

struct AxisAlignedPlaneDescriptor : public GeometryDescriptor {
  Length intercept{0 * meter};
  Axis axis{Axis::X};
};

} // namespace VolViz

#endif // VolViz_Geometry_h

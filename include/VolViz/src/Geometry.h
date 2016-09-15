#ifndef VolViz_Geometry_h
#define VolViz_Geometry_h

#include "Types.h"

namespace VolViz {

struct Geometry {
  bool movable{true};
  Color color{Colors::White()};
};

struct AxisAlignedPlane : public Geometry {
  Length intercept{0 * meter};
  Axis axis{Axis::X};
};

} // namespace VolViz

#endif // VolViz_Geometry_h

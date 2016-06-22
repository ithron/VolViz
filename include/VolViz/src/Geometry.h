#ifndef VolViz_Geometry_h
#define VolViz_Geometry_h

#include "Types.h"

namespace VolViz {

struct AxisAlignedPlane {
  Length intercept{0 * meter};
  Axis axis{Axis::X};
  bool movable{true};
  Color color{Colors::White()};
};

} // namespace VolViz

#endif // VolViz_Geometry_h

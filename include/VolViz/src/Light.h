#ifndef VolViz_Light_h
#define VolViz_Light_h

#include "Types.h"

namespace VolViz {

/// Directional light
class Light {
public:
  /// Color of the light
  Color color{Color::Ones()};
  /// Position of the light source. Since only directional light are supported,
  /// the light is at an infinite position, therefore the position equals the
  /// direction to the light source
  PositionH position{PositionH::Zero()};

  /// Factor that specifies how the light contributes to the ambient lighting
  float ambientFactor = 0.f;
};

} // namespace VolViz

#endif // VolViz_Light_h

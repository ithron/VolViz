#ifndef VolViz_Geometry_h
#define VolViz_Geometry_h

#include "Types.h"

namespace VolViz {

  class Geometry {
    public:

      Position position{Position::Zero()};
      Orientation orientation{Orientation::Identity()};

      virtual ~Geometry() = default;
    protected:

  };

} // namespace VolViz

#endif // VolViz_Geometry_h

#ifndef VolViz_Volume_h
#define VolViz_Volume_h

#include "Types.h"

namespace VolViz {

enum class VolumeType { GrayScale, ColorRGB };

struct VolumeDescriptor {
  VolumeType type{VolumeType::GrayScale};

  VoxelSize voxelSize{{1 * milli * meter, 1 * milli *meter, 1 * milli *meter}};

  Size3 size{Size3::Zero()};
};

} // namespace VolViz

#endif // VolViz_Volume_h

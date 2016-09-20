#include "Geometry.h"

namespace VolViz {
namespace Private_ {

void Geometry::init() { doInit(); }

void Geometry::render(std::uint32_t index, bool selected) {
  doRender(index, selected);
}

void Geometry::doInit() {}

} // namespace Private_
} // namespace VolViz

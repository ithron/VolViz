#pragma once

#include "Geometry.h"
#include "Types.h"

#include <concurrentqueue.h>

namespace VolViz {
namespace Private_ {

class Cube : public Geometry {
public:
  Cube(CubeDescriptor const &descriptor, VisualizerImpl &visualizer);

protected:
  virtual void doInit() override;

  virtual void doRender(std::uint32_t index, bool selected) override;

  virtual void doUpdate() override;

  virtual void doEnqueueUpdate(GeometryDescriptor const &descriptor) override;
  virtual void doEnqueueUpdate(GeometryDescriptor &&descriptor) override;

private:
  using UpdateQueue = moodycamel::ConcurrentQueue<CubeDescriptor>;

  Scale radius;
  UpdateQueue updateQueue_;
};

} // namespace Private_
} // namespace VolViz

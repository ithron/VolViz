#pragma once

#include "GL/Buffer.h"
#include "GL/VertexArray.h"
#include "Geometry.h"
#include "Types.h"

#include <concurrentqueue.h>

namespace VolViz {
namespace Private_ {

class Mesh : public Geometry {
public:
  Mesh(MeshDescriptor const &descriptor, VisualizerImpl &visualizer);

protected:
  virtual void doInit() override;

  virtual void doRender(std::uint32_t index, bool selected) override;

  virtual void doUpdate() override;

  virtual void doEnqueueUpdate(GeometryDescriptor const &descriptor) override;
  virtual void doEnqueueUpdate(GeometryDescriptor &&descriptor) override;

private:
  using UpdateQueue = moodycamel::ConcurrentQueue<MeshDescriptor>;
  void uploadMesh();

  UpdateQueue updateQueue_;

  GL::Buffer vertexBuffer_;
  GL::Buffer indexBuffer_;
  GL::VertexArray vertexArrayObject_;
  std::size_t numTriangles_{0};
};

} // namespace Private_
} // namespace VolViz


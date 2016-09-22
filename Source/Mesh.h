#pragma once

#include "GL/Buffer.h"
#include "GL/VertexArray.h"
#include "Geometry.h"
#include "Types.h"

// clang-format off
#if __has_include(<optional>)
#  include <optional>
#elif __has_include(<experimental/optional>)
#  include <experimental/optional>
namespace std {
  using std::experimental::optional;
  using std::experimental::nullopt_t;
  using std::experimental::nullopt;
} // namespace std
#else
#  error "No optional type available"
#endif
// clang-format on

namespace VolViz {
namespace Private_ {

class Mesh : public Geometry {
public:
  Mesh(MeshDescriptor const &descriptor, VisualizerImpl &visualizer);

protected:
  virtual void doInit() override;

  virtual void doRender(std::uint32_t index, bool selected) override;

private:
  void uploadMesh();

  std::optional<MeshDescriptor> descriptor_;

  GL::Buffer vertexBuffer_;
  GL::Buffer indexBuffer_;
  GL::VertexArray vertexArrayObject_;
  std::size_t numTriangles_{0};
};

} // namespace Private_
} // namespace VolViz

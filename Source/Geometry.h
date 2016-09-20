#pragma once

namespace VolViz {
namespace Private_ {

class Geometry {
public:
  using UniquePtr = std::unique_ptr<Geometry>;

  Position position{Position::Zero()};
  Orientation orientation{Orientation::Identity()};

  virtual ~Geometry() = default;

  void init();

  void render();

protected:
  Geometry() = default;

  virtual void doInit();

  virtual void doRender() = 0;
};

} // namespace VolViz
} // namespace Private_

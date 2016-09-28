#pragma once

#include "GeometryDescriptor.h"

namespace VolViz {
namespace Private_ {

class VisualizerImpl;

class Geometry {
public:
  using UniquePtr = std::unique_ptr<Geometry>;

  Position position{Position::Zero()};
  Orientation orientation{Orientation::Identity()};
  Length scale{1 * milli * meter};
  MoveMask moveMask{MoveMask::All};
  Color color{Colors::White()};

  virtual ~Geometry() = default;

  void init();

  inline void update() { doUpdate(); }

  void render(std::uint32_t index, bool selected);

  template <class Descriptor,
            typename = std::enable_if_t<std::is_base_of<
                GeometryDescriptor, std::decay_t<Descriptor>>::value>>
  inline void enqueueUpdate(Descriptor &&descriptor) {
    doEnqueueUpdate(std::forward<Descriptor>(descriptor));
  }

protected:
  Geometry(VisualizerImpl &visualizer);
  Geometry(GeometryDescriptor const &descriptor, VisualizerImpl &visualizer);

  virtual void doInit();

  virtual void doRender(std::uint32_t index, bool selected) = 0;

  virtual void doUpdate();

  virtual void doEnqueueUpdate(GeometryDescriptor const &descriptor);
  virtual void doEnqueueUpdate(GeometryDescriptor &&descriptor);

  VisualizerImpl &visualizer_;
};

} // namespace VolViz
} // namespace Private_

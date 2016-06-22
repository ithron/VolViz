#include "Visualizer.h"
#include "VisualizerImpl.h"

#include <Eigen/Core>
#include <gsl.h>

#include <iostream>
#include <mutex>

namespace VolViz {

Visualizer::Visualizer()
    : impl_(std::make_unique<Private_::VisualizerImpl>(this)) {}

Visualizer::~Visualizer() = default;

Visualizer::Visualizer(Visualizer &&rhs) : impl_(std::move(rhs.impl_)) {
  impl_->visualizer_ = this;
}

Visualizer &Visualizer::operator=(Visualizer &&rhs) {
  using std::swap;
  swap(impl_, rhs.impl_);
  impl_->visualizer_ = this;
  return *this;
}

void Visualizer::start() { impl_->start(); }

Visualizer::operator bool() const noexcept { return *impl_; }

void Visualizer::renderOneFrame() { impl_->renderOneFrame(); }

void Visualizer::addLight(LightName name, Light const &light) {
  impl_->addLight(name, light);
}

template <class VertBase, class IdxBase>
void Visualizer::setMesh(Eigen::MatrixBase<VertBase> const &V,
                         Eigen::MatrixBase<IdxBase> const &I) {
  impl_->setMesh(V, I);
}

template void Visualizer::setMesh<>(Eigen::MatrixBase<Eigen::MatrixXd> const &,
                                    Eigen::MatrixBase<Eigen::MatrixXi> const &);

} // namespace VolViz

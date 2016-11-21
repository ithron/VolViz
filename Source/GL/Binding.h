#ifndef VolViz_Binding_h
#define VolViz_Binding_h

#include "GLdefs.h"

#include <tuple>
#include <utility>

namespace VolViz {
namespace Private_ {
namespace GL {

/// RAII wrapper for types than can be bound
///
/// If obj is an object of type T, calls obj.bind() on construction and
/// obj.unbind() on destruction.
template <class T> class Binding {
public:
  explicit Binding(T &&obj) noexcept : obj_(std::forward<T>(obj)) {
    obj_.bind();
  }

  Binding(Binding &&) = default;

  Binding &operator=(Binding &&) = default;

  ~Binding() { obj_.unbind(); }

private:
  T &&obj_;
};

template <class T, class Arg> class Binding2 {
public:
  explicit Binding2(T &&obj, Arg arg) noexcept
      : obj_(std::forward<T>(obj)), arg_(arg) {
    obj_.bind(arg_);
  }

  Binding2(Binding2 &&) = default;

  Binding2 &operator=(Binding2 &&) = default;

  ~Binding2() { obj_.unbind(arg_); }

private:
  T &&obj_;
  Arg arg_;
};

/// Convenience function to create Binding objects
template <class T> inline decltype(auto) binding(T &&obj) noexcept {
  return Binding<T>(std::forward<T>(obj));
}

template <class T, class A>
inline decltype(auto) binding(T &&obj, A &&arg) noexcept {
  return Binding2<T, std::remove_reference_t<A>>(std::forward<T>(obj),
                                                 std::forward<A>(arg));
}

} // namespace GL
} // namespace Private_
} // namespace VolViz

#endif // VolViz_Binding_h

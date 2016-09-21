#ifndef VolViz_AtomicWrapper_h
#define VolViz_AtomicWrapper_h

#include <mutex>

namespace VolViz {

/// Default set policy for AtomicWrapper class, justs sets dest to src
template <class T> class DefaultSetPolicy {
protected:
  inline void set(T &dest, T const &src) const noexcept { dest = src; }
  inline void set(T &dest, T &&src) const noexcept { dest = std::move(src); }
};

/// Notification set policy for AtomicWrapper
/// If beforeAction is set, it is called right before the set operation.
/// If afterAction is set, it is called right after the set operation
template <class T> class SetAndNotifyPolicy : public DefaultSetPolicy<T> {
public:
  /// action (or notification) that's called right before the set operation.
  /// The function is called with the value to be set as a parameter.
  std::function<void(T const &)> beforeAction;

  /// Action (or notification) that's called right after the set operation.
  /// The function is called with the new value as a parameter.
  std::function<void(T const &)> afterAction;

protected:
  template <class Arg> inline void set(T &dest, Arg &&src) const noexcept {
    if (beforeAction) beforeAction(src);
    DefaultSetPolicy<T>::set(dest, std::forward<Arg>(src));
    if (afterAction) afterAction(dest);
  }
};

/// Wrapper class for atomic-like access for types where no std::atomic
/// overload is available.
///
/// @tparam T wrapped type
/// @tparam SetPolicy policy class template, that specified the set operation
/// @note this wrapper is actually slower than std::atomic since it uses a
/// mutex and not only atomic operations. Also the wrapped object is copied
/// at read access, so only use this wrapper for fast to copy types.
template <class T, template <class> class SetPolicy = DefaultSetPolicy>
class AtomicWrapper : public SetPolicy<T> {
  using SetPol = SetPolicy<T>;

public:
  /// Initialized the wrapper with an object
  explicit AtomicWrapper(T const &obj) : obj_(obj) {}
  /// Initialized the wrapper with an object (move constructor)
  explicit AtomicWrapper(T &&obj) : obj_(std::move(obj)) {}

  AtomicWrapper(AtomicWrapper const &) = delete;
  AtomicWrapper &operator=(AtomicWrapper const &) = delete;

  /// Returns a copy of the wrapped object in a thread safe manner.
  operator T() const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto cpy = obj_;
    return cpy;
  }

  /// Reassign the wrapped object in a thread safe manner.
  AtomicWrapper &operator=(T const &rhs) {
    std::lock_guard<std::mutex> lock(mutex_);

    SetPol::set(obj_, rhs);

    return *this;
  }

  /// Reassign the wrapped object in a thread safe manner (move version).
  AtomicWrapper &operator=(T &&rhs) {
    std::lock_guard<std::mutex> lock(mutex_);

    SetPol::set(obj_, std::move(rhs));

    return *this;
  }

private:
  /// Thre wrapped object
  T obj_;

  /// Mutex preventing simultaneous access.
  mutable std::mutex mutex_;
};

} // namespace VolViz

#endif // VolViz_AtomicWrapper_h

#ifndef VolViz_AtomicWrapper_h
#define VolViz_AtomicWrapper_h

#include <mutex>

namespace VolViz {

template <class T> class AtomicWrapper {
public:
  explicit AtomicWrapper(T const &obj) : obj_(obj) {}
  explicit AtomicWrapper(T &&obj) : obj_(std::move(obj)) {}

  AtomicWrapper(AtomicWrapper const &) = delete;
  AtomicWrapper &operator=(AtomicWrapper const &) = delete;

  operator T() const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto cpy = obj_;
    return obj_;
  }

  AtomicWrapper &operator=(T const &rhs) {
    std::lock_guard<std::mutex> lock(mutex_);

    obj_ = rhs;

    return *this;
  }

  AtomicWrapper &operator=(T &&rhs) {
    std::lock_guard<std::mutex> lock(mutex_);

    obj_ = std::move(rhs);

    return *this;
  }

private:
  T obj_;
  std::mutex mutex_;
};

} // namespace VolViz

#endif // VolViz_AtomicWrapper_h

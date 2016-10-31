#ifndef VolViz_AtomicCache_h
#define VolViz_AtomicCache_h

#include <gsl>

#include <atomic>

namespace VolViz {

/// Class represening a cached value, that has a very basic thread safety.
/// The thread saftey is as follows: the cache'd value must not be accessed
/// from different threads (at least without external synchronization), but
/// the cache can be marked dirty from any thread.
/// A typical use case is to cache a derived property that depends on some
/// shared (beween threads) resources (e.g. AtomicWrapper<> objects) and the
/// cached value is frequently accessed but the dependent resource does not
/// change so often.
///
/// @tparam T the cached type
template <class T> class AtomicCache {
public:
  /// Creates a dirty cache with the given fetch operation.
  /// @tparam FetchOp the fetch operation. It takes no parameter and returns an
  /// object convertible to T.
  /// @note At construction no fetch operation is issued.
  template <class FetchOp>
  AtomicCache(FetchOp fetchOperation)
      : fetchOperation_(fetchOperation) {
    Expects(fetchOperation_);
    dirtyFlag_.clear();
  }

  /// Marks the cache as dirty, i.e. a fetch operations must be perfomed on the
  /// next read.
  /// @note this method is thread safe.
  inline void markAsDirty() noexcept { dirtyFlag_.clear(); }

  /// Returns the cached object. If the cache is dirty, a fetch operations is
  /// performed first.
  /// @note This method must not be called from more than one thread.
  inline operator T const &() const noexcept {
    if (dirtyFlag_.test_and_set()) return value_;

    Expects(fetchOperation_);
    value_ = std::move(fetchOperation_());
    return value_;
  }

private:
  /// The caced value
  mutable T value_;

  /// The atomic dirty flag
  mutable std::atomic_flag dirtyFlag_;

  /// The fetch operation
  std::function<T()> const fetchOperation_;
};

} // namespace VolViz

#endif // VolViz_AtomicCache_h

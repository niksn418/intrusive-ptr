#include <atomic>

template <typename T>
struct intrusive_ptr {
  using element_type = T;

  intrusive_ptr() noexcept : intrusive_ptr(nullptr) {}

  intrusive_ptr(T* p, bool add_ref = true) : pointer(p) {
    if (p != nullptr && add_ref) {
      intrusive_ptr_add_ref(p);
    }
  }

  intrusive_ptr(intrusive_ptr const& r) : pointer(r.get()) {
    if (pointer != nullptr) {
      intrusive_ptr_add_ref(pointer);
    }
  }

  template <class Y>
  requires std::is_convertible_v<Y*, T*>
  intrusive_ptr(intrusive_ptr<Y> const& r) : pointer(r.get()) {
    if (pointer != nullptr) {
      intrusive_ptr_add_ref(pointer);
    }
  }

  intrusive_ptr(intrusive_ptr&& r)
      : pointer(std::exchange(r.pointer, nullptr)) {}
  template <class Y>
  requires std::is_convertible_v<Y*, T*> intrusive_ptr(intrusive_ptr<Y>&& r)
      : pointer(std::exchange(r.pointer, nullptr)) {}

  ~intrusive_ptr() {
    if (pointer != nullptr) {
      intrusive_ptr_release(pointer);
    }
  }

  intrusive_ptr& operator=(intrusive_ptr const& r) {
    intrusive_ptr(r).swap(*this);
    return *this;
  }
  template <class Y>
  requires std::is_convertible_v<Y*, T*> intrusive_ptr&
  operator=(intrusive_ptr<Y> const& r) {
    intrusive_ptr(r).swap(*this);
    return *this;
  }
  intrusive_ptr& operator=(T* r) {
    intrusive_ptr(r).swap(*this);
    return *this;
  }

  intrusive_ptr& operator=(intrusive_ptr&& r) {
    intrusive_ptr(std::move(r)).swap(*this);
    return *this;
  }
  template <class Y>
  requires std::is_convertible_v<Y*, T*> intrusive_ptr&
  operator=(intrusive_ptr<Y>&& r) {
    intrusive_ptr(std::move(r)).swap(*this);
    return *this;
  }

  void reset() {
    reset(nullptr);
  }

  void reset(T* r) {
    reset(r, true);
  }

  void reset(T* r, bool add_ref) {
    intrusive_ptr(r, add_ref).swap(*this);
  }

  T& operator*() const noexcept {
    return *get();
  }

  T* operator->() const noexcept {
    return get();
  }

  T* get() const noexcept {
    return pointer;
  }

  T* detach() noexcept {
    return std::exchange(pointer, nullptr);
  }

  explicit operator bool() const noexcept {
    return get() != nullptr;
  }

  void swap(intrusive_ptr& b) noexcept {
    std::swap(pointer, b.pointer);
  }

private:
  template <class Y>
  friend struct intrusive_ptr;

  T* pointer;
};

template <class T, class U>
bool operator==(intrusive_ptr<T> const& a, intrusive_ptr<U> const& b) noexcept {
  return a.get() == b.get();
}

template <class T, class U>
bool operator!=(intrusive_ptr<T> const& a, intrusive_ptr<U> const& b) noexcept {
  return !(a == b);
}

template <class T, class U>
bool operator==(intrusive_ptr<T> const& a, U* b) noexcept {
  return a.get() == b;
}

template <class T, class U>
bool operator!=(intrusive_ptr<T> const& a, U* b) noexcept {
  return !(a == b);
}

template <class T, class U>
bool operator==(T* a, intrusive_ptr<U> const& b) noexcept {
  return b == a;
}

template <class T, class U>
bool operator!=(T* a, intrusive_ptr<U> const& b) noexcept {
  return b != a;
}

template <class T>
bool operator<(intrusive_ptr<T> const& a, intrusive_ptr<T> const& b) noexcept {
  return std::less<T*>()(a.get(), b.get());
}

template <class T>
void swap(intrusive_ptr<T>& a, intrusive_ptr<T>& b) noexcept {
  a.swap(b);
}

template <typename T>
struct intrusive_ref_counter {
  intrusive_ref_counter() noexcept {}
  intrusive_ref_counter(const intrusive_ref_counter& v) noexcept {}

  intrusive_ref_counter& operator=(const intrusive_ref_counter& v) noexcept {}

  unsigned int use_count() const noexcept {
    return counter.load(std::memory_order_relaxed);
  }

protected:
  ~intrusive_ref_counter() = default;

private:
  template <class Derived>
  friend void
  intrusive_ptr_add_ref(const intrusive_ref_counter<Derived>* p) noexcept;

  template <class Derived>
  friend void
  intrusive_ptr_release(const intrusive_ref_counter<Derived>* p) noexcept;

  mutable std::atomic<unsigned int> counter = 0;
};

template <class Derived>
void intrusive_ptr_add_ref(const intrusive_ref_counter<Derived>* p) noexcept {
  p->counter.fetch_add(1, std::memory_order_relaxed);
}

template <class Derived>
void intrusive_ptr_release(const intrusive_ref_counter<Derived>* p) noexcept {
  if (p->counter.fetch_sub(1, std::memory_order_acq_rel) == 1) {
    delete static_cast<const Derived*>(p);
  }
}

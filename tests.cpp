#include "gtest/gtest.h"

#include "solution.h"

#include <thread>

struct object : intrusive_ref_counter<object> {};
struct derived : object {};

TEST(correctness, default_ref_counter_ctor) {
  object o;
  ASSERT_EQ(o.use_count(), 0);
}

TEST(correctness, ref_counter_copy_ctor) {
  auto a = new object();
  intrusive_ptr<object> ptr(a);
  ASSERT_EQ(a->use_count(), 1);

  object b = *a;
  ASSERT_EQ(b.use_count(), 0);
}

TEST(correctness, ref_counter_assignment) {
  auto a = new object();
  intrusive_ptr<object> ptr1(a);
  intrusive_ptr<object> ptr2(a);
  ASSERT_EQ(a->use_count(), 2);

  object b;
  *a = b;
  ASSERT_EQ(a->use_count(), 2);
}

TEST(correctness, default_ptr_ctor) {
  intrusive_ptr<object> ptr;
  ASSERT_EQ(ptr.get(), nullptr);
  ASSERT_EQ(bool(ptr), false);
  ASSERT_EQ(!ptr, true);
}

TEST(correctness, ptr_ctor) {
  auto o = new object();
  intrusive_ptr<object> ptr3(o);
  ASSERT_EQ(o->use_count(), 1);
  { // intrusive_ptr ctor increments ref counter
    intrusive_ptr<object> ptr(o);
    ASSERT_EQ(ptr.get(), o);
    ASSERT_EQ(ptr->use_count(), 2);
    ASSERT_EQ(o->use_count(), 2);
  } // and decrements in destructor
  ASSERT_EQ(o->use_count(), 1);
  {
    intrusive_ptr<object> ptr(o);
    ASSERT_EQ(ptr->use_count(), 2);
    ASSERT_EQ(o->use_count(), 2);
    ptr.detach(); // doesn't decrement counter
  }
  ASSERT_EQ(o->use_count(), 2);
  { // doesn't decrement counter with add_ref = false
    intrusive_ptr<object> ptr(o, false);
    ASSERT_EQ(o->use_count(), 2);
  } // but does increment
  ASSERT_EQ(o->use_count(), 1);
}

TEST(correctness, ptr_copy_move_ctor) {
  auto o = new derived();
  intrusive_ptr<derived> a(o);
  ASSERT_EQ(o->use_count(), 1);
  auto b = a;
  ASSERT_EQ(o->use_count(), 2);
  intrusive_ptr<object> c = b;
  ASSERT_EQ(o->use_count(), 3);

  c = std::move(b);
  ASSERT_EQ(o->use_count(), 2);
}

TEST(correctness, thread_safety_check) {
  /// Num threads to run
  auto N = std::thread::hardware_concurrency();

  /// Number of copy ctor calls in a worker thread
  auto N_ctors = 16 * 1000000 / N;

  ASSERT_GT(N, 1);

  auto o = new derived();
  intrusive_ptr<object> ptr(o);
  std::vector<std::thread> workers;
  for (size_t i = 0; i < N; ++i) {
    workers.emplace_back([o, N_ctors]() {
      for (size_t j = 0; j < N_ctors; j++) {
        intrusive_ptr<object> ptr(o);
      }
    });
  }

  for (auto& W : workers)
    W.join();

  ASSERT_EQ(o->use_count(), 1);
}

static_assert(std::is_constructible_v<intrusive_ptr<object>,
                                      const intrusive_ptr<object>&>);
static_assert(
    std::is_constructible_v<intrusive_ptr<object>, intrusive_ptr<object>&&>);
static_assert(
    std::is_assignable_v<intrusive_ptr<object>, const intrusive_ptr<object>&>);
static_assert(
    std::is_assignable_v<intrusive_ptr<object>, intrusive_ptr<object>&&>);

static_assert(std::is_constructible_v<intrusive_ptr<object>,
                                      const intrusive_ptr<derived>&>);
static_assert(
    std::is_constructible_v<intrusive_ptr<object>, intrusive_ptr<derived>&&>);
static_assert(
    std::is_assignable_v<intrusive_ptr<object>, const intrusive_ptr<derived>&>);
static_assert(
    std::is_assignable_v<intrusive_ptr<object>, intrusive_ptr<derived>&&>);

static_assert(!std::is_constructible_v<intrusive_ptr<derived>,
                                       const intrusive_ptr<object>&>);
static_assert(
    !std::is_constructible_v<intrusive_ptr<derived>, intrusive_ptr<object>&&>);
static_assert(!std::is_assignable_v<intrusive_ptr<derived>,
                                    const intrusive_ptr<object>&>);
static_assert(
    !std::is_assignable_v<intrusive_ptr<derived>, intrusive_ptr<object>&&>);

static_assert(
    !std::is_constructible_v<intrusive_ptr<object>, const intrusive_ptr<int>&>);
static_assert(
    !std::is_constructible_v<intrusive_ptr<object>, intrusive_ptr<int>&&>);
static_assert(
    !std::is_assignable_v<intrusive_ptr<object>, const intrusive_ptr<int>&>);
static_assert(
    !std::is_assignable_v<intrusive_ptr<object>, intrusive_ptr<int>&&>);

static_assert(std::is_constructible_v<intrusive_ptr<const int>,
                                      const intrusive_ptr<int>&>);
static_assert(
    std::is_constructible_v<intrusive_ptr<const int>, intrusive_ptr<int>&&>);
static_assert(
    std::is_assignable_v<intrusive_ptr<const int>, const intrusive_ptr<int>&>);
static_assert(
    std::is_assignable_v<intrusive_ptr<const int>, intrusive_ptr<int>&&>);

static_assert(!std::is_constructible_v<intrusive_ptr<int>,
                                       const intrusive_ptr<const int>&>);
static_assert(
    !std::is_constructible_v<intrusive_ptr<int>, intrusive_ptr<const int>&&>);
static_assert(
    !std::is_assignable_v<intrusive_ptr<int>, const intrusive_ptr<const int>&>);
static_assert(
    !std::is_assignable_v<intrusive_ptr<int>, intrusive_ptr<const int>&&>);

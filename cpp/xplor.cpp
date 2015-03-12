
#include <gtest/gtest.h>

#include <map>
#include <mutex>
#include <string>
#include <utility>

namespace move_semantics {
// http://stackoverflow.com/questions/3106110/what-is-move-semantics

class C {
public:
  explicit C(int a, int b) : m_a(a), m_b(b) {}

  C(C &&) = default;
  C &operator=(C &&) = default;

  // forgetting the "const" in the definition of this copy-constructor and
  // assignment operator leads to a most cryptic compiler error;
  // an entire day was spent learning this (and learning it for good).
  C(const C &) = delete;
  C &operator=(const C &) = delete;

  int m_a;
  int m_b;
};

TEST(MoveSemantics, SimpleMap) {
  std::map<std::string, C> m;

  C c(13, 42);

  m.emplace("hey", std::move(c));

  C &cr = m.at("hey");
  ASSERT_EQ(42, cr.m_b);
}

class A {
public:
  explicit A(int a) : m_a(a) {}

  virtual int foo() const { return m_a; }

  A(A &&) = default;
  A &operator=(A &&) = default;

  A(const A &) = delete;
  A &operator=(const A &) = delete;

protected:
  int m_a;
};

class B : public A {
public:
  explicit B(int a, int b) : A(a), m_b(b) {}

  int foo() const override { return 7; }

  // all classes in the chain of inheritance must have move semantics
  // accessible;
  // granted, said semantics are often implicitly declared and defined by the
  // compiler.
  B(B &&) = default;
  B &operator=(B &&) = default;

  B(const B &) = delete;
  B &operator=(const B &) = delete;

  int m_b;
};

TEST(MoveSemantics, InheritanceMap) {
  std::map<std::string, B> m;

  B b(13, 42);

  m.emplace("hey", std::move(b));

  B &br = m.at("hey");
  ASSERT_EQ(42, br.m_b);
}

TEST(MoveSemantics, TypeSlicing) {
  std::map<std::string, A> m;

  B b(13, 42);

  m.emplace("hey", std::move(b));

  // this results in type slicing; the override fails.
  // A& ar is treated as an object of type A rather than as a pointer to virtual
  // base class A.
  A &ar = m.at("hey");
  ASSERT_EQ(13, ar.foo());
}

// @story
struct D {
  // constructor
  explicit D(int a) : data(a) {}

  // destructor
  ~D() { data = 0; }

  // copy constructor
  D(const D &other) {
    data = other.data;
    ++copy_count;
  }

  // move constructor
  D(D &&other) : D(0) { // calls standard constructor; initializes instance
    swap(*this, other);
    ++move_count;
  }

  // copy-and-swap idiom:
  // http://stackoverflow.com/questions/3279543/what-is-the-copy-and-swap-idiom
  //
  // Advantages:
  // * efficiency:
  //   * eliminates need to check for self assignment: if (this != other)
  //   * allows the compiler to optimize copy (pass-by-value);
  //     guideline: let the compiler perform copies for you whenever possible
  //
  // * conciseness
  //   * reduces code duplication: factors common bits into swap function;
  //     move constructor and assignment operator thus become trivial.
  //   * copy assignment operator and move assignment operator are fused into
  // one
  //
  // * correctness
  //   * strong exception guarantee
  //
  D &operator=(D d) {
    // if the operand is an l-value, it is copied into `d` using the copy
    // constructor
    // if the operand is an r-value, it is moved into `d` using the move
    // constructor
    swap(*this, d);
    return *this;
  }

  friend void swap(D &a, D &b) { std::swap(a.data, b.data); }

  // unrelated: equality operator
  friend bool operator==(const D &lhs, const D &rhs) {
    return lhs.data == rhs.data;
  }

  int data;

  static int copy_count;
  static int move_count;
};

int D::copy_count = 0;
int D::move_count = 0;

TEST(MoveSemantics, CopyAndSwap) {
  D::copy_count = 0;
  D::move_count = 0;

  D a(42);
  D b(a);

  EXPECT_EQ(1, D::copy_count);
  EXPECT_EQ(0, D::move_count);
  EXPECT_EQ(a, b);
}

TEST(MoveSemantics, MoveAndSwap) {
  D::copy_count = 0;
  D::move_count = 0;

  D a(42);
  D b(std::move(a));

  EXPECT_EQ(0, a.data);
  EXPECT_EQ(42, b.data);
  EXPECT_EQ(0, D::copy_count);
  EXPECT_EQ(1, D::move_count);
}

TEST(MoveSemantics, Assignment) {
  D::copy_count = 0;
  D::move_count = 0;

  D a(42);
  D b = a;

  EXPECT_EQ(1, D::copy_count);
  EXPECT_EQ(0, D::move_count);
  EXPECT_EQ(a, b);

  D c = std::move(a);
  EXPECT_EQ(1, D::copy_count);
  EXPECT_EQ(1, D::move_count);
  EXPECT_EQ(0, a.data);
  EXPECT_EQ(b, c);
}

} // namespace move_semantics

namespace lamdba_locks {

class LambdaLockFixture : public ::testing::Test {
public:
  std::mutex mutex;
};

TEST_F(LambdaLockFixture, ReferenceCapture) {
  std::function<void()> callback;

  {
    std::unique_lock<std::mutex> lk(mutex);
    callback = [&]() {
      std::unique_lock<std::mutex> lk2(mutex);
      ASSERT_TRUE(lk2.owns_lock());

      // if an entity is captured by reference, implicitly or explicitly, and
      // the function call operator of the closure object is invoked after the
      // entity's lifetime has ended, undefined behavior occurs. The C++
      // closures do not extend the lifetimes of captured references.

      // ASSERT_TRUE(lk.owns_lock()); // referencing lk causes undefined
      // behavior
    };
  }

  callback();
}

} // namespace lamda_locks

namespace vector_copy {

TEST(VectorCopy, Basic) {
  std::string str = "hello world";
  std::vector<uint8_t> vec;
  vec.resize(str.size());

  // note that str.data() has type char*
  // note that vec.data() has type uint8_t*
  std::copy(str.data(), str.data() + str.size(), vec.data());

  ASSERT_EQ(str.size(), vec.size());
  ASSERT_TRUE(std::equal(str.data(), str.data() + str.size(), vec.data()));
}

} // namespace vector_copy

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

#include <gtest/gtest.h>

#include <array>
#include <map>
#include <memory>
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

namespace more_move_semantics {

struct X {
    X(int v) : value(new int(v)) {
        ++constructor_count;
    }

    ~X() {
        ++destructor_count;
        delete value;
    }

    X(X& x) {
        ++copy_constructor_count;
        value = new int;
        *value = *x.value;
    }

    X& operator=(const X& other) {
        if (&other != this) {
            ++copy_assignment_operator_count;
            value = new int;
            *value = *other.value;
        }

        return *this;
    }

    X(X&& x) {
        ++move_constructor_count;
        value = x.value;
        x.value = nullptr;
    }

    X& operator=(X&& other) {
        if (&other != this) {
            ++move_assignment_operator_count;
            value = other.value;
            other.value = nullptr;
        }

        return *this;
    }

    int *value;

    static int constructor_count;
    static int destructor_count;
    static int copy_constructor_count;
    static int copy_assignment_operator_count;
    static int move_constructor_count;
    static int move_assignment_operator_count;
};

int X::constructor_count = 0;
int X::destructor_count = 0;
int X::copy_constructor_count = 0;
int X::copy_assignment_operator_count = 0;
int X::move_constructor_count = 0;
int X::move_assignment_operator_count = 0;

TEST(MoreMoveSemantics, Basic) {
    {
        X x(42);            // calls constructor
        X y(x);             // calls copy constructor
        X z(std::move(x));  // calls move constructor

        x = y;              // calls copy assignment operator
        y = std::move(z);   // calls move assignment operator


        EXPECT_EQ(42, *x.value);
        EXPECT_EQ(42, *y.value);
        EXPECT_EQ(nullptr, z.value);
    }

    // all three have now gone out of scope
    EXPECT_EQ(1, X::constructor_count);
    EXPECT_EQ(3, X::destructor_count);
    EXPECT_EQ(1, X::copy_constructor_count);
    EXPECT_EQ(1, X::copy_assignment_operator_count);
    EXPECT_EQ(1, X::move_constructor_count);
    EXPECT_EQ(1, X::move_assignment_operator_count);
}


} // namespace more_move_semantics

namespace even_more_move_semantics {

struct X {
    X() : value(42) {}

    // default copy semantics
    X(const X&) = default;
    X& operator=(const X&) = default;

    // move semantics disabled
    X(X&&) = delete;
    X& operator=(X&&) = delete;

    int value;
};

TEST(EvenMoreMoveSemantics, RValueRef) {
    X x;
    // X y(std::move(x)); // compiler error: move constructor has been deleted
}

} // namespace even_more_move_semantics


namespace unique_ptr {

struct X {
    X(int value) : value(value) {}
    int value;
};

struct M {
    void transfer(std::unique_ptr<X>&& external_x) {
        x_ptr = std::move(external_x);
    }

    std::unique_ptr<X> x_ptr;
};


TEST(UniquePtr, TransferOwnership) {
    M m;

    std::unique_ptr<X> x_ptr(new X(42));
    m.transfer(std::move(x_ptr));

    EXPECT_FALSE((bool)x_ptr);
    ASSERT_TRUE((bool)m.x_ptr);
    EXPECT_EQ(42, m.x_ptr->value);
}

} // namespace unique_ptr


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

namespace ternary_mischief {

TEST(TernaryMischief, Basic) {
    int i = 0;
    int j = 0;

    (true) ? ++i : ++j;

    EXPECT_EQ(1, i);
    EXPECT_EQ(0, j);
}

} // namespace ternary_mischief


namespace template_mischief {


template <int n>
struct factorial {
    enum { value = n * factorial<n - 1>::value };
};

template <>
struct factorial<0> {
    enum { value = 1 };
};


TEST(TemplateMischief, MetaFactorial) {
    EXPECT_EQ(factorial<3>::value, 6);
    EXPECT_EQ(factorial<9>::value, 362880);
}


struct Message {
    bool serialize(char* data_buffer, size_t buffer_size) { return true; }

    bool deserialize(const char* data_buffer, size_t buffer_size) { return true; }
};

struct InlineMessage {
    bool bmq_emplace(char *data_buffer, size_t buffer_size) { return true; }

    bool bmq_replace(char *data_buffer, size_t buffer_size) { return true; }
};

struct Other {};


template <typename MessageType>
struct Node {
    bool send(MessageType& msg) {
        return msg.serialize(nullptr, 0);
    }

    bool receive(MessageType& msg) {
        return msg.deserialize(nullptr, 0);
    }
};

TEST(TemplateMischief, StaticTypeChecks) {
    Message msg;
    Node<Message> node;
    EXPECT_TRUE(node.send(msg));
    EXPECT_TRUE(node.receive(msg));

    Other other_msg;
    Node<Other> other_node;
    // EXPECT_TRUE(other_node.send(other_msg));     // causes compile-time error
    // EXPECT_TRUE(other_node.receive(other_msg));  // causes compile-timp error
}


template <typename MessageType>
struct Publisher {

    typename std::enable_if<
        std::is_member_function_pointer<decltype(&MessageType::bmq_emplace)>::value &&
        std::is_member_function_pointer<decltype(&MessageType::bmq_replace)>::value,
        MessageType>::type
    *NewInplaceMessage() {
        return nullptr;
    }
};


TEST(TemplateMischief, SFINAE) {
    Publisher<InlineMessage> publisher;
    InlineMessage *msg = publisher.NewInplaceMessage();
}

} // namespace template_mischief


namespace template_specialization {

class A {};
class B {};

template <typename T>
class X final {
    // disallow instances templatized on generic type T
    // namely, only allow specializations
    X() = delete;
};

template <>
struct X<A> {
    int foo() { return 1; }
};

template <>
struct X<B> {
    int bar() { return 2; }
};

template <>
struct X<int> {
    int baz() { return 3; }
};

TEST(TemplateSpecialization, ClassLevel) {
    X<A> y;
    X<B> z;
    X<int> x;

    EXPECT_EQ(y.foo(), 1);
    EXPECT_EQ(z.bar(), 2);
    EXPECT_EQ(x.baz(), 3);
}

} // namespace template_specialization


namespace template_inheritance {

struct A {};
struct B : A {};

struct X {};
struct Y : X {};

struct C {};

template <typename MessageType, typename enabler=void>
struct Publisher {
    static_assert(std::is_void<MessageType>::value, "MessageType must inherit from A or X");
};

template<typename MessageType>
struct Publisher<MessageType,
                 typename std::enable_if<std::is_base_of<A, MessageType>::value>::type> {
    int foo() { return 42; }
};

template<typename MessageType>
struct Publisher<MessageType,
                 typename std::enable_if<std::is_base_of<X, MessageType>::value>::type> {
    int bar() { return 13; }
};


TEST(FOO, test) {
    Publisher<A> p1;
    Publisher<B> p2;
    EXPECT_EQ(42, p1.foo());
    EXPECT_EQ(42, p2.foo());

    Publisher<X> p3;
    Publisher<Y> p4;
    EXPECT_EQ(13, p3.bar());
    EXPECT_EQ(13, p4.bar());

    // Publisher<C> p5;  // compiler error
}

}  // namespace template_inheritance


namespace different_kind_of_template_inheritance {

struct X {};

template <typename T>
struct A : X {};

template <>
struct A<int> : X {};

} // different_kind_of_template_inheritance

namespace template_function_definition {

template <typename T>
struct X {
    void foo();
};

template <typename T>
inline void X<T>::foo() {}

} // template_function_definition

namespace protected_fields {

class A {
public:
    A() : a(42) {}
protected:
    int a;
};

template <typename T>
class B : public A {
public:
    B() {}
};

template <typename T>
class C : public B<T> {
public:
    int foo() { return B<T>::a; }
};

TEST(ProtectedFields, Basic) {
    C<int> c;
    EXPECT_EQ(42, c.foo());
}

} // namespace protected_fields


#include <poll.h>
#include <sys/socket.h>
#include <sys/un.h>

namespace socket_polling {

TEST(Poll, Basic) {
    int sv[2]; // pair of socket file descriptors

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == -1) {
        FAIL();
    }

    if (fork()) { /* parent */

    } else {

    }
}

} // namespace socket_polling

namespace enum_classes {

enum class Flags : uint8_t {
    FLAG_0 = 0,
    FLAG_1 = 1,
    FLAG_2 = 2,
};

Flags operator|(Flags lhs, Flags rhs) {
    return Flags(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs));
}

TEST(EnumClasses, Basic) {
    Flags flags = Flags::FLAG_1 | Flags::FLAG_2;
    ASSERT_EQ(3, static_cast<uint8_t>(flags));
}

} // namespace enum_classes


namespace array {

TEST(Array, Basic) {
    const int size = 10; // must be a constant expression
    std::array<int,size> array; // array size determined at compile-time
}

void foo(std::array<int,1>& array) {
    array[0] = 42;
}

TEST(Array, ValueSemantics) {
    std::array<int,1> array = { 0 };
    foo(array);
    ASSERT_EQ(42, array[0]);
}

} // namespace array

namespace polymorphic_references {

struct A {
    virtual int foo() = 0;
};

struct B : A {
    B(int value) : m_value(value) {}

    int foo() override {
        return m_value;
    }

    int m_value;
};

TEST(PolymorphicReferences, Basic) {
    B b(42);

    A& a = b;

    ASSERT_EQ(42, b.foo());
    ASSERT_EQ(42, a.foo());
}

} // namespace polymorphic_references

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}


#include <gtest/gtest.h>

#include <map>
#include <string>
#include <utility>

namespace move_semantics {
// http://stackoverflow.com/questions/3106110/what-is-move-semantics

class C {
public:
    explicit C(int a, int b) : m_a(a), m_b(b) {}

    C(C&&) = default;
    C& operator=(C&&) = default;

    // forgetting the "const" in the definition of this copy-constructor and
    // assignment operator leads to a most cryptic compiler error;
    // an entire day was spent learning this (and learning it for good).
    C(const C&) = delete;
    C& operator=(const C&) = delete;

    int m_a;
    int m_b;
};


TEST(MoveSemantics, SimpleMap) {
    std::map<std::string,C> m;

    C c(13, 42);

    m.emplace("hey", std::move(c));

    C& cr = m.at("hey");
    ASSERT_EQ(42, cr.m_b);
}


class A {
public:
    explicit A(int a) : m_a(a) {}

    virtual int foo() const { return m_a; }

    A(A&&) = default;
    A& operator=(A&&) = default;

    A(const A&) = delete;
    A& operator=(const A&) = delete;

protected:
    int m_a;
};

class B : public A {
public:
    explicit B(int a, int b) : A(a), m_b(b) {}

    int foo() const override { return 7; }

    // all classes in the chain of inheritance must have move semantics accessible;
    // granted, said semantics are often implicitly declared and defined by the compiler.
    B(B&&) = default;
    B& operator=(B&&) = default;

    B(const B&) = delete;
    B& operator=(const B&) = delete;

    int m_b;
};

TEST(MoveSemantics, InheritanceMap) {
    std::map<std::string,B> m;

    B b(13, 42);

    m.emplace("hey", std::move(b));

    B& br = m.at("hey");
    ASSERT_EQ(42, br.m_b);
}

TEST(MoveSemantics, TypeSlicing) {
    std::map<std::string,A> m;

    B b(13, 42);

    m.emplace("hey", std::move(b));

    // this results in type slicing; the override fails.
    // A& ar is treated as an object of type A rather than as a pointer to virtual base class A.
    A& ar = m.at("hey");
    ASSERT_EQ(13, ar.foo());
}

} // namespace move_semantics



int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

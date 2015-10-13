#include <gtest/gtest.h>

#include <array>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <utility>

namespace hello_world {

TEST(MoveSemantics, SimpleMap) {
    ASSERT_EQ(42, 21 * 2);
}

} // namespace hello_world


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

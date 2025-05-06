#include <gtest/gtest.h>

int main(int argc, char **argv) { // NOLINT
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    return ret;
}

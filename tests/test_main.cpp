#include <gtest/gtest.h>

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    std::cout << "We're in!!!\n";
    std::cout << "Welcome to my test cases!\n";
    std::cout << "rainy: let's get started!\n";
    RUN_ALL_TESTS();
    std::cout << "Now, we finally finish our test!\n";
    std::cout << "Thanks for you patience!\n";
    return 0;
}

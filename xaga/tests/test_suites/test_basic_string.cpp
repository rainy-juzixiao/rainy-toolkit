#include <gtest/gtest.h>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>
#include <rainy/collections/string.hpp>

using namespace rainy::collections;

// 测试默认构造函数
TEST(RainyToolkit_ConstructorTest, DefaultConstructor) {
    basic_string<char> str;
    EXPECT_EQ(str.size(), 0); // 空字符串
    EXPECT_TRUE(str.empty()); // 应为空
}

// 测试带allocator构造函数
TEST(RainyToolkit_ConstructorTest, AllocatorConstructor) {
    rainy::foundation::system::memory::allocator<char> alloc;
    basic_string<char> str(alloc);
    EXPECT_EQ(str.size(), 0); // 默认构造为空字符串
}

// 测试拷贝构造
TEST(RainyToolkit_ConstructorTest, CopyConstructor) {
    basic_string<char> str1("Test");
    basic_string<char> str2(str1); // 拷贝构造
    EXPECT_EQ(str1, str2); // str1 和 str2 内容相同
    EXPECT_EQ(str2.size(), 4); // 长度为4
}

// 测试移动构造
TEST(RainyToolkit_ConstructorTest, MoveConstructor) {
    basic_string<char> str1("MoveTest");
    basic_string<char> str2(std::move(str1)); // 移动构造
    EXPECT_EQ(str2, "MoveTest"); // str2 应该拥有内容
    EXPECT_TRUE(str1.empty()); // str1 应该为空，因为内容被转移了
}

// 测试带偏移量和计数的拷贝构造
TEST(RainyToolkit_ConstructorTest, CopyConstructorWithOffsetAndCount) {
    basic_string<char> str1("Hello, World!");
    basic_string<char> str2(str1, 7, 5); // 从位置7开始，取5个字符
    EXPECT_EQ(str2, "World"); // 字符串应为 "World"
}

// 测试带偏移量、计数和allocator的拷贝构造
TEST(RainyToolkit_ConstructorTest, CopyConstructorWithOffsetCountAndAlloc) {
    rainy::foundation::system::memory::allocator<char> alloc;
    basic_string<char> str1("Hello, World!");
    basic_string<char> str2(str1, 7, 5, alloc); // 从位置7开始，取5个字符
    EXPECT_EQ(str2, "World"); // 字符串应为 "World"
}

// 测试指针构造（字符数组）
TEST(RainyToolkit_ConstructorTest, PointerConstructor) {
    const char *ptr = "Hello, World!";
    basic_string<char> str(ptr);
    EXPECT_EQ(str, "Hello, World!"); // 字符串应为 "Hello, World!"
    EXPECT_EQ(str.size(), 13); // 长度应为13
}

// 测试指针构造（带计数）
TEST(RainyToolkit_ConstructorTest, PointerConstructorWithCount) {
    const char *ptr = "Hello, World!";
    basic_string<char> str(ptr, 5); // 只取前5个字符
    EXPECT_EQ(str, "Hello"); // 字符串应为 "Hello"
    EXPECT_EQ(str.size(), 5); // 长度应为5
}

// 测试带allocator的指针构造（字符数组）
TEST(RainyToolkit_ConstructorTest, PointerConstructorWithAlloc) {
    const char *ptr = "Hello, World!";
    rainy::foundation::system::memory::allocator<char> alloc;
    basic_string<char> str(ptr, alloc); // 使用allocator
    EXPECT_EQ(str, "Hello, World!");
}

// 测试带n个字符的构造
TEST(RainyToolkit_ConstructorTest, NCharConstructor) {
    basic_string<char> str(5, 'A'); // 创建5个'A'
    EXPECT_EQ(str, "AAAAA"); // 字符串应为 "AAAAA"
    EXPECT_EQ(str.size(), 5); // 长度应为5
}

// 测试带n个字符和allocator的构造
TEST(RainyToolkit_ConstructorTest, NCharConstructorWithAlloc) {
    rainy::foundation::system::memory::allocator<char> alloc;
    basic_string<char> str(5, 'A', alloc); // 使用allocator构造
    EXPECT_EQ(str, "AAAAA"); // 字符串应为 "AAAAA"
    EXPECT_EQ(str.size(), 5); // 长度应为5
}

// 测试迭代器构造
TEST(RainyToolkit_ConstructorTest, IteratorConstructor) {
    std::vector<char> vec = {'H', 'e', 'l', 'l', 'o'};
    basic_string<char> str(vec.begin(), vec.end()); // 使用迭代器构造字符串
    EXPECT_EQ(str, "Hello"); // 字符串应为 "Hello"
    EXPECT_EQ(str.size(), 5); // 长度应为5
}

// 测试带allocator的迭代器构造
TEST(RainyToolkit_ConstructorTest, IteratorConstructorWithAlloc) {
    rainy::foundation::system::memory::allocator<char> alloc;
    std::vector<char> vec = {'H', 'e', 'l', 'l', 'o'};
    basic_string<char> str(vec.begin(), vec.end(), alloc); // 使用allocator
    EXPECT_EQ(str, "Hello"); // 字符串应为 "Hello"
    EXPECT_EQ(str.size(), 5); // 长度应为5
}

// 测试const_pointer构造
TEST(RainyToolkit_ConstructorTest, ConstPointerConstructor) {
    const char *ptr = "World";
    basic_string<char> str(ptr, ptr + 5); // 使用指针构造
    EXPECT_EQ(str, "World"); // 字符串应为 "World"
    EXPECT_EQ(str.size(), 5); // 长度应为5
}

// 测试const_iterator构造
TEST(RainyToolkit_ConstructorTest, ConstIteratorConstructor) {
    basic_string<char> str("Hello");
    basic_string<char> str2(str.begin(), str.end()); // 使用const_iterator构造
    EXPECT_EQ(str, str2); // 应该相等
    EXPECT_EQ(str2.size(), 5); // 长度应为5
}

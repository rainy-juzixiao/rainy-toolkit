#include <gtest/gtest.h>
#include <rainy/text/format_wrapper.hpp>

using namespace rainy::utility;

TEST(RainyToolKit_FormatWrapperTest, BasicFormat) {
    std::string result;
    cstyle_format(result, "Hello, %s!", "world");
    EXPECT_EQ(result, "Hello, world!");
}

TEST(RainyToolKit_FormatWrapperTest, NumberFormat) {
    std::string result;
    cstyle_format(result, "The answer is %d.", 42);
    EXPECT_EQ(result, "The answer is 42.");
}

TEST(RainyToolKit_FormatWrapperTest, MixedFormat) {
    std::string result;
    cstyle_format(result, "Name: %s, Age: %d, Score: %.2f", "Alice", 25, 92.5);
    EXPECT_EQ(result, "Name: Alice, Age: 25, Score: 92.50");
}

TEST(RainyToolKit_FormatWrapperTest, MismatchPlaceholder) {
    std::string result;
    EXPECT_NO_THROW(cstyle_format(result, "This has %d placeholders %d", 20, 10)); // NOLINT
#if !RAINY_ENABLE_DEBUG
    EXPECT_THROW(cstyle_format(result, "This has %d placeholders %d", 20, 10, 40), // NOLINT
                 rainy::foundation::system::exceptions::wrapexcept<rainy::foundation::system::exceptions::runtime::runtime_error>);
    // 注意。实际上，cstyle_format抛出的异常是一个wrapper，因此，此处我们只需要就是，用wrapper指代即可
#endif
}

TEST(RainyToolKit_FormatWrapperTest, LargeOutputResize) {
    std::string result;
    std::string large_string(500, 'A');
    cstyle_format(result, "Large test: %s", large_string.data());
    std::string expect_string = "Large test: " + large_string;
    EXPECT_EQ(result, expect_string);
}

TEST(RainyToolKit_FormatWrapperTest, WideStringFormat) {
    std::wstring result;
    cstyle_format(result, L"Hello, %ls!", L"world");
    EXPECT_EQ(result, L"Hello, world!");
}

TEST(RainyToolKit_FormatWrapperTest, EmptyFormatString) {
    std::string result;
    cstyle_format(result, "");
    EXPECT_EQ(result, "");
}

TEST(RainyToolKit_FormatWrapperTest, SpecialCharacters) {
    std::string result;
    cstyle_format(result, "Tab:\t Newline:\n Percent:%%");
    EXPECT_EQ(result, "Tab:\t Newline:\n Percent:%");
}

TEST(RainyToolKit_FormatWrapperTest, NullStringInput) {
    std::string result;
    cstyle_format(result, "Null string: %s", static_cast<const char*>(nullptr));
    EXPECT_EQ(result, "Null string: (null)");
}

TEST(RainyToolKit_FormatWrapperTest, FloatingPointPrecision) {
    std::string result;
    cstyle_format(result, "Pi: %.5f", 3.14159);
    EXPECT_EQ(result, "Pi: 3.14159");
}

TEST(RainyToolKit_FormatWrapperTest, IntegerOverflow) {
    std::string result;
#if RAINY_USING_WINDOWS
    cstyle_format(result, "Max int: %d", INT_MAX);
#else
    cstyle_format(result, "Max int: %d", INT32_MAX);
#endif
    EXPECT_EQ(result, "Max int: 2147483647");
}

TEST(RainyToolKit_FormatWrapperTest, NegativeNumbers) {
    std::string result;
    cstyle_format(result, "Negative: %d", -42);
    EXPECT_EQ(result, "Negative: -42");
}

TEST(RainyToolKit_FormatWrapperTest, BooleanFormat) {
    std::string result;
    cstyle_format(result, "Boolean true: %d, false: %d", true, false);
    EXPECT_EQ(result, "Boolean true: 1, false: 0");
}
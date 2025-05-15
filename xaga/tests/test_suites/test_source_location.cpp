#include <gtest/gtest.h>
#include <rainy/core/core.hpp>

using namespace rainy::foundation::diagnostics;

TEST(RainyToolKit_SourceLocationTest, BasicTest) {
    auto sl = current_location();
    EXPECT_NE(sl.file_name(), nullptr);
    EXPECT_GT(sl.line(), 0);
    EXPECT_GT(sl.column(), 0);
    EXPECT_NE(sl.function_name(), nullptr);
    EXPECT_FALSE(sl.to_string().empty());
}

TEST(RainyToolKit_SourceLocationTest, LogicOperatorsTest) {
    auto sl1 = current_location();
    auto sl2 = current_location();
    auto sl_copy = sl1;
    EXPECT_EQ(sl1, sl_copy);
    EXPECT_NE(sl1, sl2);
}

TEST(RainyToolKit_SourceLocationTest, CompileTimeEvalTest) {
    constexpr auto sl_constexpr = current_location();
    EXPECT_NE(sl_constexpr.file_name(), nullptr);
    EXPECT_GT(sl_constexpr.line(), 0);
    EXPECT_GT(sl_constexpr.column(), 0);
    EXPECT_NE(sl_constexpr.function_name(), nullptr);
}

#if RAINY_HAS_CXX20
#include <source_location>
TEST(RainyToolKit_SourceLocationTest, Cpp20SupportTest) {
    auto std_sl = std::source_location::current();
    source_location rainy_sl = std_sl;

    EXPECT_NE(rainy_sl.file_name(), nullptr);
    EXPECT_GT(rainy_sl.line(), 0);
    EXPECT_GT(rainy_sl.column(), 0);
    EXPECT_NE(rainy_sl.function_name(), nullptr);
}
#endif

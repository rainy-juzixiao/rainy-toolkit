#include <gtest/gtest.h>
#include <rainy/foundation/functional/function_pointer.hpp>

using namespace rainy::foundation::functional;

int add(int a, int b) {
    return a + b;
}

void call_fn() {
}

int variadic_fn(int a, int b, ...) {
    return a + b;
}

TEST(RainyFunctionPointerTest, TestFunctionPointerCreation) {
    function_pointer<int(int, int)> fptr = &add;
    EXPECT_EQ(fptr(1, 2), 3);
}

TEST(RainyFunctionPointerTest, TestNullFunctionPointer) {
    function_pointer<int(int, int)> fptr = nullptr;
    EXPECT_TRUE(fptr.empty());
    EXPECT_THROW(fptr(1, 2), // NOLINT
                 rainy::foundation::exceptions::wrapexcept<rainy::foundation::exceptions::runtime::runtime_error>);
}

TEST(RainyFunctionPointerTest, TestFunctionPointerReset) {
    function_pointer<int(int, int)> fptr = &add;
    auto old_ptr = fptr.reset(nullptr);
    EXPECT_TRUE(fptr.empty());
    EXPECT_EQ(old_ptr(1, 2), 3);
}

TEST(RainyFunctionPointerTest, TestVoidFunctionPointer) {
    function_pointer<void()> fptr = &call_fn;
    // 此处不验证返回值，因为是 void 类型，检查是否能正常调用
    EXPECT_NO_THROW(fptr()); // NOLINT
}

TEST(RainyFunctionPointerTest, TestMakeFunctionObject) {
    function_pointer<int(int, int)> fptr = &add;
    // 使用 make_function_object 获取 std::function
    std::function<int(int, int)> func = fptr.make_function_object();
    EXPECT_EQ(func(2, 3), 5);
}

TEST(RainyFunctionPointerTest, TestFunctionPointerCTAD) {
    function_pointer fptr = &add;
    EXPECT_EQ(fptr(3, 4), 7); // 自动推导类型，检查调用是否正常
}

TEST(RainyFunctionPointerTest, TestFunctionPointerWithVariadic) {
    function_pointer<int(int, int, ...)> fptr = variadic_fn;
    EXPECT_EQ(fptr(1, 2), 3); // 验证调用
}

TEST(RainyFunctionPointerTest, TestEmptyPointerInvokeThrowsException) {
    function_pointer<int(int, int)> fptr = nullptr;
    EXPECT_THROW(fptr(1, 2),
                 rainy::foundation::exceptions::wrapexcept<rainy::foundation::exceptions::runtime::runtime_error>);
}

TEST(RainyFunctionPointerTest, TestResetPointerInvokeThrowsException) {
    function_pointer<int(int, int)> fptr = nullptr;
    EXPECT_THROW(fptr(1, 2),
                 rainy::foundation::exceptions::wrapexcept<rainy::foundation::exceptions::runtime::runtime_error>);
    fptr.reset(&add);
    EXPECT_NO_THROW(fptr(1, 2));
    fptr.reset(nullptr);
    EXPECT_THROW(fptr(1, 2),
                 rainy::foundation::exceptions::wrapexcept<rainy::foundation::exceptions::runtime::runtime_error>);
}

TEST(RainyFunctionPointerTest, TestNullptrAssignment) {
    function_pointer<int(int, int)> fptr = &add;
    fptr = nullptr;
    EXPECT_TRUE(fptr.empty());
}

TEST(RainyFunctionPointerTest, TestVariadicArgument) {
    function_pointer<int(int, int, ...)> fptr = variadic_fn;
    EXPECT_EQ(fptr(1, 2), 3);
}

TEST(RainyFunctionPointerTest, TestTypeSafety) {
    function_pointer<int(int, int)> fptr = &add;
    // 类型不匹配，编译时应该报错
    // function_pointer<void(int)> fptr2 = fptr;  // 编译错误
    // 不同签名的函数应该不能赋值给另一个
    // fptr = &print_hello;  // 编译时错误
}

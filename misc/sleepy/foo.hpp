#pragma once

#define rain_fn auto

template <bool Is>
struct FooImpl1 {
    /**
     * \lang chinese
     * @brief 草！
     *
     * \lang english
     * @brief Opps!
     */
    void impl1() const noexcept {
    }
};

template <>
struct FooImpl1<false> {};

/**
 * @notpublic
 * @hide
 */
class FooImpl {
public:
    /**
     * \lang chinese
     * @brief 草！
     *
     * \lang english
     * @brief Opps!
     */
    void impl() {
    }

private:
    /**
     * @notpublic
     *
     * @brief 草!
     */
    void not_public() {
    }
};

/// Example class
class Foo : private FooImpl, public FooImpl1<true> {
public:
    /**
     * \lang chinese
     * @brief 草！
     * @brief 1!
     *
     * \lang english
     * @brief Opps!
     */
    int add(int a, int b);

    /**
     * \lang chinese
     * @brief 草！
     * @brief 1!
     *
     * \lang english
     * @brief Opps!
     */
    rain_fn add(int a, int b, int c) -> int;

    /// \lang chinese
    /// @brief 草
    /// \lang english
    /// @brief fuck
    int add(int a, int b, char c);

    using FooImpl::impl;

    /**
     * \lang chinese
     * @brief 我
     *
     * \lang english
     * @brief I
     *
     * @overload_decl void impl1() const noexcept
     */
    using FooImpl1::impl1;
};

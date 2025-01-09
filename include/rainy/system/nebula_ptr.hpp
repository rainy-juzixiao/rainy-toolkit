/*
文件名: nebula_ptr.hpp
此头文件用于存放nebula_ptr智能指针的实现

nebula_ptr:
一个类模板类型。属于独占所有权语义指针。它的目标在于为用户提供一个比起std::unique_ptr更加灵活的API接口解决方案
*/

#ifndef RAINY_NEBULA_PTR
#define RAINY_NEBULA_PTR
#include <rainy/base.hpp>

namespace rainy::foundation::system::memory {
    template <typename Ty, typename Dx = default_deleter<Ty>>
    class nebula_ptr;

    template <typename Ty, typename Dx>
    class nebula_ptr_base {
    public:
        using element_type = Ty;
        using pointer = element_type *;
        using const_pointer = const element_type *;
        using deleter_type = Dx;

        template <typename, typename>
        friend class nebula_ptr;

        constexpr nebula_ptr_base() noexcept : pair(Dx{}, nullptr){};

        constexpr nebula_ptr_base(std::nullptr_t) noexcept : pair(Dx{}, nullptr) {
        }

        RAINY_CONSTEXPR20 nebula_ptr_base(element_type *pointer) noexcept : pair(Dx{}, pointer) {
        }

        template <type_traits::other_trans::enable_if_t<std::is_copy_constructible_v<deleter_type>, int> = 0>
        RAINY_CONSTEXPR20 nebula_ptr_base(element_type *pointer, deleter_type deleter) : pair(Dx{}, pointer) {
        }

        template <typename Uty,
                  type_traits::other_trans::enable_if_t<type_traits::type_relations::is_convertible_v<Uty *, pointer>, int> = 0>
        RAINY_CONSTEXPR20 nebula_ptr_base(Uty *pointer) noexcept : pair(Dx{}, pointer) {
        }

        template <typename Uty, type_traits::other_trans::enable_if_t<
                                    type_traits::type_relations::is_convertible_v<Uty *, pointer> && std::is_copy_constructible_v<deleter_type>, int> = 0>
        RAINY_CONSTEXPR20 nebula_ptr_base(Uty *pointer, deleter_type deleter) : pair(Dx{}, pointer) {
        }

        nebula_ptr_base(const nebula_ptr_base &) = delete;
        nebula_ptr_base(nebula_ptr_base &&) = default;

        template <typename Dx2 = Dx,
                  type_traits::other_trans::enable_if_t<
                      type_traits::logical_traits::conjunction_v<std::is_reference<Dx2>, std::is_constructible<Dx2, std::remove_reference_t<Dx2>>>, int> = 0>
        nebula_ptr_base(pointer, std::remove_reference_t<Dx> &&) = delete;

        RAINY_CONSTEXPR20 ~nebula_ptr_base() {
            reset(nullptr);
        };

        nebula_ptr_base &operator=(const nebula_ptr_base &) = delete;

        RAINY_NODISCARD constexpr bool empty() const noexcept {
            return !static_cast<bool>(pair.second);
        }

        RAINY_NODISCARD constexpr operator bool() const noexcept {
            return !static_cast<bool>(pair.second);
        }

        RAINY_CONSTEXPR20 void reset(element_type *pointer = nullptr) {
            if (!empty()) {
                pair.get_first()(pair.second);
            }
            pair.second = pointer;
        }

        RAINY_CONSTEXPR20 nebula_ptr_base &operator=(std::nullptr_t) {
            reset(nullptr);
            return *this;
        }

        RAINY_CONSTEXPR20 nebula_ptr_base &operator=(element_type *pointer) {
            reset(pointer);
            return *this;
        }

        constexpr nebula_ptr_base &operator=(nebula_ptr_base &&right) noexcept {
            pair = right.pair;
            right.pair.second = nullptr;
            return *this;
        }

        template <typename Reint, typename Dx_ = default_deleter<Reint>>
        nebula_ptr_base<Reint, Dx_> &reinterpret() noexcept {
            return reinterpret_cast<nebula_ptr_base<Reint, Dx_> &>(*this);
        }

        template <typename Reint, typename Dx_ = default_deleter<Reint>>
        const nebula_ptr_base<Reint, Dx_> &reinterpret() const noexcept {
            return reinterpret_cast<nebula_ptr_base<Reint, Dx_> &>(*this);
        }

        template <typename Cast, typename Dx_ = default_deleter<Cast>,
                  type_traits::other_trans::enable_if_t<type_traits::type_relations::is_convertible_v<Ty, Cast>, int> = 0>
        constexpr nebula_ptr_base<Cast, Dx_> &cast() noexcept {
            return static_cast<nebula_ptr_base<Cast, Dx_> &>(*this);
        }

        template <typename Cast, typename Dx_ = default_deleter<Cast>,
                  type_traits::other_trans::enable_if_t<type_traits::type_relations::is_convertible_v<Ty, Cast>, int> = 0>
        constexpr const nebula_ptr_base<Cast, Dx_> &cast() const noexcept {
            return static_cast<const nebula_ptr_base<Cast, Dx> &>(*this);
        }

        template <typename Base, type_traits::other_trans::enable_if_t<std::is_base_of_v<Base, Ty>, int> = 0>
        nebula_ptr_base<Base, Dx> &upcast() noexcept {
            // 我们无法使用RTTI的dynamic_cast功能来进行转换。因此，我们需要使用静态SFINAE来确保安全转换
            return reinterpret_cast<nebula_ptr_base<Base, Dx> &>(*this);
        }

        template <typename Base, type_traits::other_trans::enable_if_t<std::is_base_of_v<Base, Ty>, int> = 0>
        const nebula_ptr_base<Base, Dx> &upcast() const noexcept {
            return reinterpret_cast<nebula_ptr_base<Base, Dx> &>(*this);
        }

        RAINY_NODISCARD constexpr pointer *operator&() noexcept {
            return utility::addressof(pair.second);
        }

        RAINY_NODISCARD constexpr const pointer *operator&() const noexcept {
            return utility::addressof(pair.second);
        }

        RAINY_NODISCARD constexpr pointer release() noexcept {
            return utility::exchange(pair.second, nullptr);
        }

        RAINY_NODISCARD constexpr pointer get() noexcept {
            return pair.second;
        }

        RAINY_NODISCARD constexpr const_pointer get() const noexcept {
            return pair.second;
        }

        RAINY_NODISCARD constexpr deleter_type &get_deleter() noexcept {
            return pair.get_first();
        }

        RAINY_NODISCARD constexpr const deleter_type &get_deleter() const noexcept {
            return pair.get_first();
        }

        constexpr pointer operator->() noexcept {
            return pair.second;
        }

        constexpr const_pointer operator->() const noexcept {
            return pair.second;
        }

        constexpr type_traits::reference_modify::add_lvalue_reference_t<element_type> operator*() {
            return as_reference();
        }

        constexpr type_traits::reference_modify::add_lvalue_reference_t<const element_type> operator*() const {
            return as_reference();
        }

        constexpr type_traits::reference_modify::add_lvalue_reference_t<element_type> as_reference() {
            return *get();
        }

        constexpr type_traits::reference_modify::add_lvalue_reference_t<const element_type> as_reference() const {
            return *get();
        }

        constexpr void swap(nebula_ptr_base &right) noexcept {
            pair.swap(right.pair);
        }

    private:
        utility::compressed_pair<deleter_type, pointer> pair;
    };

    template <typename Ty, typename Dx>
    class nebula_ptr : public nebula_ptr_base<Ty, Dx> {
    public:
        using nebula_ptr_base<Ty, Dx>::nebula_ptr_base;
    };

    template <typename Ty, typename Dx>
    class nebula_ptr<Ty[], Dx> : public nebula_ptr_base<Ty, Dx> {
    public:
        using base = nebula_ptr_base<Ty, Dx>;
        using element_type = typename base::element_type;
        using size_type = std::size_t;
        using pointer = typename base::pointer;
        using base::base;

        nebula_ptr(element_type *ptr, const size_type length) : base(ptr), _length(length), local(false) {
        }

        template <size_type N>
        nebula_ptr(element_type (&arr)[N]) : base(arr), _length(N), local(true) {
        }

        RAINY_CONSTEXPR20 ~nebula_ptr() {
            reset(nullptr);
        }

        element_type &operator[](const size_type idx) noexcept {
            return this->pair.second[idx];
        }

        const element_type &operator[](const size_type idx) const noexcept {
            return this->pair.second[idx];
        }

        RAINY_NODISCARD bool pointer_to_local() const noexcept {
            return local;
        }

        element_type &at(const size_type idx) noexcept {
            range_check(idx);
            return this->pair.second[idx];
        }

        const element_type &at(const size_type idx) const noexcept {
            range_check(idx);
            return this->pair.second[idx];
        }

        constexpr pointer begin() noexcept {
            return this->pair.second;
        }

        constexpr pointer end() noexcept {
            return this->pair.second + length();
        }

        constexpr pointer begin() const noexcept {
            return this->pair.second;
        }

        constexpr pointer end() const noexcept {
            return this->pair.second + length();
        }

        RAINY_CONSTEXPR20 void fill(const Ty &val) noexcept(std::is_nothrow_constructible_v<Ty>) {
            std::fill_n(this->pair.second, length(), val);
        }

        RAINY_CONSTEXPR20 void fill_with_ilist(std::initializer_list<Ty> ilist) noexcept(std::is_nothrow_constructible_v<Ty>) {
            size_type count = ilist.size() > length() ? length() : ilist.size();
            std::copy_n(ilist.begin(), count, this->pair.second);
        }

        RAINY_CONSTEXPR20 void reset(element_type *pointer = nullptr) {
            if (!local) {
                base::reset(pointer);
                local = false;
            } else {
                (void) this->release();
                local = true;
                _length = 0;
            }
        }

        template <size_type N>
        RAINY_CONSTEXPR20 void reset(element_type (&arr)[N]) {
            base::reset(arr);
            this->_length = N;
            local = true;
        }

        RAINY_CONSTEXPR20 nebula_ptr &operator=(std::nullptr_t) {
            if (!local) {
                reset();
            }
            this->pair.second = nullptr;
            this->_length = 0;
            return *this;
        }

        RAINY_CONSTEXPR20 nebula_ptr &operator=(element_type *pointer) {
            reset(pointer);
            return *this;
        }

        template <size_type N>
        RAINY_CONSTEXPR20 nebula_ptr &operator=(element_type (*arr)[N]) {
            reset<N>(*arr);
            return *this;
        }

        constexpr nebula_ptr &operator=(nebula_ptr &&right) noexcept {
            static_cast<base &>(*this) = right;
            this->local = right.local;
            this->_length = right._length;
            return *this;
        }

        constexpr size_type has_range() const noexcept {
            return length() == 0;
        }

        constexpr size_type size() const noexcept {
            return _length;
        }

        constexpr size_type length() const noexcept {
            return _length;
        }

    private:
        constexpr void range_check(const size_type idx) const {
            if (length() == 0) {
                return;
            }
            if (idx >= length()) {
                system::exceptions::logic::throw_out_of_range("Invalid array subscript");
            }
        }

        bool local;
        size_type _length{0};
    };

    /**
     * @brief 创建一个 `nebula_ptr` 指针，用于管理一个非数组类型对象的生命周期。
     *
     * @tparam Ty 要创建的对象的类型，必须不是数组类型。
     * @tparam Args 要传递给对象构造函数的参数类型。
     * @param args 构造对象时需要的参数，可以是任意数量。
     * @return 返回一个 `nebula_ptr<Ty>` 指针，指向新创建的对象。
     *
     */
    template <typename Ty, typename... Args, type_traits::other_trans::enable_if_t<!std::is_array_v<Ty>, int> = 0>
    nebula_ptr<Ty> make_nebula(Args &&...args) noexcept(std::is_nothrow_constructible_v<Ty, Args...>) {
        return nebula_ptr<Ty>(new Ty(utility::forward<Args>(args)...));
    }

    /**
     * @brief 创建一个 `nebula_ptr` 指针，用于管理一个动态数组类型对象的生命周期。
     *
     * @tparam Ty 要创建的对象类型，必须是数组类型，并且尺寸必须为0（即动态大小数组）。
     * @tparam Args 要传递给对象构造函数的参数类型。
     * @param num 要创建的数组的大小。
     * @param args 构造对象需要的参数，可以是任意数量
     * @return 返回一个 `nebula_ptr<Ty[]>` 指针，指向新创建的动态数组。
     *
     * @remark
     * 该函数负责分配和初始化一个指定大小的动态数组，使用默认构造函数初始化每个元素。
     *
     */
    template <typename Ty, typename... Args,
              type_traits::other_trans::enable_if_t<std::is_array_v<Ty> && std::extent_v<Ty> == 0, int> = 0>
    nebula_ptr<Ty> make_nebula(const std::size_t num, Args... args) noexcept(std::is_nothrow_constructible_v<Ty>) {
        using elem = std::remove_extent_t<Ty>;
        return nebula_ptr<Ty>(new elem[num](args...), num);
    }

    template <typename Ty, typename... Args,
              type_traits::other_trans::enable_if_t<std::is_array_v<Ty> && std::extent_v<Ty> == 0, int> = 0>
    nebula_ptr<Ty> make_nebula(const std::size_t num, std::initializer_list<std::remove_extent_t<Ty>> ilist = {}) noexcept(
        std::is_nothrow_constructible_v<Ty>) {
        using elem = std::remove_extent_t<Ty>;
        auto data = new elem[num];
        std::size_t count = num > ilist.size() ? ilist.size() : num;
        std::copy_n(ilist.begin(), count, data);
        return nebula_ptr<Ty>(data, num);
    }

    template <typename Ty, typename Dx = default_deleter<Ty>>
    using unique_ptr = nebula_ptr<Ty, Dx>;

    template <typename Ty, typename... Args, type_traits::other_trans::enable_if_t<!std::is_array_v<Ty>, int> = 0>
    unique_ptr<Ty> make_unique(Args &&...args) noexcept(std::is_nothrow_constructible_v<Ty, Args...>) {
        return unique_ptr<Ty>(new Ty(utility::forward<Args>(args)...));
    }

    template <typename Ty, typename... Args,
              type_traits::other_trans::enable_if_t<std::is_array_v<Ty> && std::extent_v<Ty> == 0, int> = 0>
    unique_ptr<Ty> make_unique(const std::size_t num, Args... args) noexcept(std::is_nothrow_constructible_v<Ty>) {
        using elem = std::remove_extent_t<Ty>;
        return unique_ptr<Ty>(new elem[num](args...), num);
    }

    template <typename Ty, typename... Args,
              type_traits::other_trans::enable_if_t<std::is_array_v<Ty> && std::extent_v<Ty> == 0, int> = 0>
    unique_ptr<Ty> make_unique(const std::size_t num, std::initializer_list<std::remove_extent_t<Ty>> ilist = {}) noexcept(
        std::is_nothrow_constructible_v<Ty>) {
        using elem = std::remove_extent_t<Ty>;
        auto data = new elem[num];
        std::size_t count = num > ilist.size() ? ilist.size() : num;
        std::copy_n(ilist.begin(), count, data);
        return unique_ptr<Ty>(data, num);
    }
}

#endif // RAINY_NEBULA_PTR

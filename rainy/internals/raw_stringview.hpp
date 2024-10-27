#ifndef RAINY_RAW_STRINGVIEW
#define RAINY_RAW_STRINGVIEW

#ifndef RAINY_CORE_HPP
#include <rainy/core.hpp>
#endif

namespace rainy::information::internals {
    RAINY_INLINE int32_t ctz_avx2(const uint32_t x) noexcept {
        if (x == 0) {
            return 32; // 特殊情况：输入为0
        }
        const __m256i v = _mm256_set1_epi32(static_cast<int>(x)); // 使用AVX2加载32位整数
        __m256i bits = _mm256_set1_epi32(1);
        int i = 0;
        while (_mm256_testz_si256(v, bits)) {
            bits = _mm256_slli_epi32(bits, 1); // 每次左移一位
            ++i;
        }
        return i;
    }

    template <typename Ty>
    constexpr std::size_t string_length_compile_time(const Ty *string) {
        std::size_t length = 0;
        while (string[length++] != '\0') {
        }
        return length;
    }

    template <typename Ty>
    std::size_t string_length(const Ty *string) noexcept {
#if RAINY_USING_AVX2
        if (!string) {
            return 0;
        }
        const __m256i zero = _mm256_setzero_si256();
        std::size_t length = 0;
        rainy_let ptr = reinterpret_cast<const __m256i *>(string);
        while (true) {
            const __m256i chunk = _mm256_loadu_si256(ptr);
            const __m256i cmp_result = _mm256_cmpeq_epi8(chunk, zero);
            if (const int mask = _mm256_movemask_epi8(cmp_result); mask) {
                const int byte_index = ctz_avx2(mask);
                return length + byte_index;
            }
            length += 32;
            ++ptr;
        }
#else
        std::size_t length = 0;
        while (string[length] != '\0') {
            ++length;
        }
        return length;
#endif
    }

    template <typename Ty>
    int compare_string(const Ty *str1, const Ty *str2, std::size_t count) noexcept {
        using compare_type =
            foundation::type_traits::other_transformations::conditional_t<foundation::type_traits::type_relations::is_same_v<Ty, wchar_t>,
                                                                          wchar_t, unsigned char>;
        rainy_let ptr_left = reinterpret_cast<const compare_type *>(str1);
        rainy_let ptr_right = reinterpret_cast<const compare_type *>(str2);
        // Fast path for short strings
        switch (count) {
            case 1: {
                return (*ptr_left > *ptr_right) ? 1 : (*ptr_left < *ptr_right) ? -1 : 0;
            }
            case 2: {
                if (ptr_left[0] != ptr_right[0]) {
                    return ptr_left[0] > ptr_right[0] ? 1 : -1;
                }
                return ptr_left[1] > ptr_right[1] ? 1 : (ptr_left[1] < ptr_right[1]) ? -1 : 0;
            }
            case 3: {
                if (ptr_left[0] != ptr_right[0]) {
                    return ptr_left[0] > ptr_right[0] ? 1 : -1;
                }
                if (ptr_left[1] != ptr_right[1]) {
                    return ptr_left[1] > ptr_right[1] ? 1 : -1;
                }
                return (ptr_left[2] > ptr_right[2]) ? 1 : (ptr_left[2] < ptr_right[2]) ? -1 : 0;
            }
            case 4: {
                const uint32_t v1 = *reinterpret_cast<const uint32_t *>(ptr_left);
                const uint32_t v2 = *reinterpret_cast<const uint32_t *>(ptr_right);
                return v1 > v2 ? 1 : v1 < v2 ? -1 : 0;
            }
            default: {
                break;
            }
        }
        // Medium strings: use 64-bit comparisons
        while (count < 32) {
            if (const uint64_t v1 = *reinterpret_cast<const uint64_t *>(ptr_left), v2 = *reinterpret_cast<const uint64_t *>(ptr_right);
                v1 != v2) {
                return (v1 > v2) ? 1 : -1;
            }
            ptr_left += 8;
            ptr_right += 8;
            count -= 8;
        }
#if RAINY_USING_AVX2
        if (count >= 32) {
            while (count >= 32) {
                const __m256i x = _mm256_load_si256(reinterpret_cast<const __m256i *>(ptr_left));
                const __m256i y = _mm256_load_si256(reinterpret_cast<const __m256i *>(ptr_right));
                const __m256i cmp = _mm256_cmpeq_epi8(x, y);
                if (const uint32_t mask = _mm256_movemask_epi8(cmp); mask != 0xFFFFFFFF) {
                    // Find the first differing byte
                    uint32_t diff_index = ctz_avx2(~mask); // Find the first set bit
                    return (ptr_left[diff_index] > ptr_right[diff_index]) ? 1 : -1;
                }
                ptr_left += 32;
                ptr_right += 32;
                count -= 32;
            }
        }
#endif
        // Remaining bytes
        while (count--) {
            if (*ptr_left != *ptr_right) {
                return (*ptr_left > *ptr_right) ? 1 : -1;
            }
            ++ptr_left;
            ++ptr_right;
        }
        return 0;
    }

    template <typename Ty>
    constexpr int compare_string_compile_time(const Ty *str1, const Ty *str2, std::size_t count) noexcept {
        for (; 0 < count; --count, ++str1, ++str2) {
            if (*str1 != *str2) {
                return *str1 < *str2 ? -1 : +1;
            }
        }
        return 0;
    }
}

namespace rainy::information::internals {
    /**
     * @brief 用于存储字符串常量的简易容器，原则上只有rainy's toolkit的组件使用，因此，将其划分为internal，即内部实现
     * @attention 不建议使用本容器！
     * @tparam Ty 字符类型
     */
    template <typename Ty>
    class raw_string_view {
    public:
        using value_type = Ty;
        using iterator = const value_type *;
        using const_iterator = const value_type *;
        using reverse_iterator = const std::reverse_iterator<iterator>;
        using const_reverse_iterator = const std::reverse_iterator<iterator>;
        using pointer = const value_type *;
        using const_pointer = const value_type *;
        using size_type = std::size_t;

        /**
         * @brief 默认构造函数
         */
        constexpr raw_string_view() noexcept : view_data(nullptr), _size(0) {};

        /**
         * @brief 默认拷贝构造函数
         * @param right 另一个raw_string_view对象常量引用
         */
        constexpr raw_string_view(const raw_string_view &right) noexcept = default;

        /**
         * @brief 默认的视图构造函数
         * @tparam N 引用的reference_string大小（由模板自动推导，一般为引用对象的大小，非必要参数）
         * @param reference_string 引用的string（需确保引用对象生存期小于等于该视图生存期，必要参数）
         */
        template <std::size_t N>
        explicit constexpr raw_string_view(const value_type (&reference_string)[N]) noexcept : view_data(reference_string), _size(N) {
        }

        /**
         * @brief 自动计算传入的字符串的大小并构造对象.
         */
        explicit constexpr raw_string_view(const value_type *const string) noexcept :
            view_data(string), _size(string_length_compile_time(string)) {
        }

        /**
         *
         * @param reference_string 引用的string（需确保引用对象生存期小于等于该视图生存期，必要参数）
         * @param count 引用的reference_string的大小（一般是小于等于实际大小的，必要参数）
         */
        constexpr raw_string_view(const value_type* const reference_string, const size_type count) noexcept : view_data(reference_string), _size(count) {
        }

        /**
         * @brief 默认析构函数
         */
        rainy_constEXPR20 ~raw_string_view() noexcept = default;

        /**
         * @brief 获取视图指向字符串大小
         * @return 视图指向字符串的大小
         */
        RAINY_NODISCARD constexpr size_type size() const noexcept {
            return _size;
        }

        /**
         * @brief 获取视图指向字符串大小
         * @return 视图指向字符串的大小
         */
        RAINY_NODISCARD constexpr pointer data() const noexcept {
            return view_data;
        }

        /**
         * @brief 获取视图原始C风格字符串对象地址
         * @return 返回视图原始C风格字符串对象地址
         */
        RAINY_NODISCARD constexpr const_pointer c_str() const noexcept {
            return view_data;
        }

        /**
         * @brief 转换运算符
         */
        constexpr explicit operator const value_type*() const noexcept {
            return view_data;
        }

        /**
         * @brief 获取视图指向的字符串的起始迭代器
         * @return 返回起始迭代器
         */
        constexpr iterator begin() noexcept {
            return view_data;
        }

        /**
         * @brief 获取视图指向的字符串的常量起始迭代器
         * @return 返回常量起始迭代器
         */
        RAINY_NODISCARD constexpr const_iterator begin() const noexcept {
            return view_data;
        }

        /**
         * @brief 获取视图指向的字符串的末尾迭代器
         * @return 返回常量末尾迭代器
         */
        RAINY_NODISCARD constexpr iterator end() noexcept {
            return view_data;
        }

        /**
         * @brief 获取视图指向的字符串的常量末尾迭代器
         * @return 返回常量末尾迭代器
         */
        RAINY_NODISCARD constexpr const_iterator end() const noexcept {
            return view_data;
        }

        /**
         * @brief 获取视图指向的字符串的常量起始迭代器
         * @return 返回常量起始迭代器
         */
        RAINY_NODISCARD constexpr const_iterator cbegin() const noexcept {
            return view_data;
        }

        /**
         * @brief 获取视图指向的字符串的常量末尾迭代器
         * @return 返回常量末尾迭代器
         */
        RAINY_NODISCARD constexpr const_iterator cend() const noexcept {
            return view_data;
        }

        /**
         * @brief 交换两个raw_string_view对象的值
         * @param right 待交换对象
         */
        constexpr void swap(raw_string_view &right) noexcept {
            std::swap(this->view_data, right.view_data);
            std::swap(this->_size, right._size);
        }

        /**
         *
         * @param right 默认拷贝运算符
         * @return 返回this的引用
         */
        constexpr raw_string_view &operator=(const raw_string_view &right) = default;

        /**
         *
         * @param right 默认移动赋值运算符
         */
        constexpr raw_string_view &operator=(raw_string_view &&right) = default;

        constexpr raw_string_view &operator=(const value_type *const right) {
            view_data = right;
            _size = string_length(right);
            return *this;
        }

    private:
        pointer view_data;
        std::size_t _size;
    };

    class internal_source_location {
    public:
        RAINY_NODISCARD static constexpr internal_source_location current(const unsigned int line = __builtin_LINE(),
#ifdef RAINY_USING_GCC
                                                                             const unsigned int col = 0,
#else
                                                                             const unsigned int col = __builtin_COLUMN(),
#endif
                                                                             const char *const file = __builtin_FILE(),
                                                                             const char *const function = __builtin_FUNCTION()) noexcept {
            internal_source_location result;
            result.file_ = file;
            result.function_ = function;
            result.column_ = col;
            result.line_ = line;
            return result;
        }

        RAINY_NODISCARD constexpr const char *file_name() const noexcept {
            return static_cast<const char *>(file_);
        }

        RAINY_NODISCARD constexpr const char *function_name() const noexcept {
            return static_cast<const char *>(function_);
        }

        RAINY_NODISCARD constexpr unsigned int line() const noexcept {
            return line_;
        }

        RAINY_NODISCARD constexpr unsigned int column() const noexcept {
            return column_;
        }

    private:
        raw_string_view<char> file_;
        raw_string_view<char> function_;
        unsigned int line_{};
        unsigned int column_{};
    };
}
#endif

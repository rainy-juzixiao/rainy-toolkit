#ifndef RAINY_RAW_STRINGVIEW_HPP
#define RAINY_RAW_STRINGVIEW_HPP

#ifndef RAINY_CORE_HPP
#include <rainy/core/core.hpp>
#endif

namespace rainy::core::implements {
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
            view_data(string), _size(builtin::string_length(string)) {
        }

        /**
         * @param reference_string 引用的string（需确保引用对象生存期小于等于该视图生存期，必要参数）
         * @param count 引用的reference_string的大小（一般是小于等于实际大小的，必要参数）
         */
        constexpr raw_string_view(const value_type* const reference_string, const size_type count) noexcept : view_data(reference_string), _size(count) {
        }

        /**
         * @brief 默认析构函数
         */
        RAINY_CONSTEXPR20 ~raw_string_view() noexcept = default;

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
         * @param right 默认拷贝运算符
         * @return 返回this的引用
         */
        constexpr raw_string_view &operator=(const raw_string_view &right) = default;

        /**
         * @param right 默认移动赋值运算符
         */
        constexpr raw_string_view &operator=(raw_string_view &&right) = default;

        /**
         * @param right 赋值运算符
         * @return 返回this的引用
         */
        constexpr raw_string_view &operator=(const value_type *const right) {
            view_data = right;
            _size = builtin::string_length(right);
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

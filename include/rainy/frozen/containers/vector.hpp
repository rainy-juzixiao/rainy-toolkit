#ifndef RAINY_FROZEN_FROZEN_VECTOR_HPP
#define RAINY_FROZEN_FROZEN_VECTOR_HPP
#include <rainy/core/core.hpp>
#include <rainy/system/basic_exceptions.hpp>
#include <rainy/algorithm/modify_algorithm.hpp>
#include <rainy/utility/iterator.hpp>

namespace rainy::containers::frozen {
    template <typename Ty, std::size_t N>
    class frozen_vector {
    public:
        using value_type = Ty;
        using reference = value_type &;
        using const_reference = const value_type &;
        using pointer = value_type *;
        using const_pointer = const value_type *;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using iterator = utility::iterator<pointer>;
        using const_iterator = utility::const_iterator<const_pointer>;

        /**
         * @brief 构造空的 frozen_vector.
         */
        constexpr frozen_vector() noexcept(std::is_nothrow_constructible_v<Ty>) = default;

        /**
         * @brief 通过初始化列表,构造一个包含指定元素的 frozen_vector
         */
        RAINY_CONSTEXPR20 frozen_vector(std::initializer_list<Ty> ilist) noexcept(std::is_nothrow_copy_assignable_v<Ty>) {
            _size = ilist.size();
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                utility::expects(_size <= N);
                component::ranges::container_operater::copy_n(ilist, _size, elems);
            } else
#endif
            {
                rainy::utility::expects(_size <= N);
                if constexpr (type_traits::type_properties::is_pod_v<Ty>) {
                    std::uninitialized_copy_n(ilist.begin(), _size, elems);
                } else {
                    component::ranges::container_operater::copy_n(ilist, _size, elems);
                }
            }
        }

        /**
         * @brief 复制构造函数,复制一个 frozen_vector
         */
        frozen_vector(const frozen_vector &right) {
            component::ranges::container_operater::copy(right.elems, elems);
        }

        /**
         * @brief 获取当前frozen_vector已经使用的大小
         */
        constexpr std::size_t size() const noexcept {
            return _size;
        }

        /**
         * @brief 获取当前fronzen_vector的最大容量
         */
        constexpr size_type length() const noexcept {
            return N;
        }

        /**
         * @brief 获取当前frozen_vector容器中指定下表的元素引用
         * @param idx 下标
         * @return 元素引用
         */
        constexpr reference operator[](std::size_t idx) noexcept {
            return elems[idx];
        }

        /**
         * @brief 获取当前frozen_vector容器中指定下表的元素常量引用
         * @param idx 下标
         * @return 元素常量引用
         */
        constexpr const_reference operator[](std::size_t idx) const noexcept {
            return elems[idx];
        }

        /**
         * @brief 获取当前容器中指向第一个元素的迭代器
         * @return 迭代器
         */
        constexpr iterator begin() noexcept {
            return iterator(elems);
        }

        /**
         * @brief 获取当前容器中指向第一个元素的常量迭代器
         * @return 常量迭代器
         */
        constexpr const_iterator begin() const noexcept {
            return const_iterator(elems);
        }

        /**
         * @brief 获取当前容器中指向第一个元素的常量迭代器
         * @return 常量迭代器
         */
        constexpr const_iterator cbegin() const noexcept {
            return const_iterator(elems);
        }

         /**
         * @brief 获取当前容器中指向最后一个元素的迭代器
         * @attention 一般情况下，不建议直接读取，而是将其作为比较，例如
         * begin() != end()
         * 而不是类似如下
         * *end()
         * 此行为可能导致UB
         * @return 迭代器
         */
        constexpr iterator end() noexcept {
            return iterator(elems + _size);
        }

        /**
         * @brief 获取当前容器中指向最后一个元素的常量迭代器
         * @attention 一般情况下，不建议直接读取，而是将其作为比较，例如
         * begin() != end()
         * 而不是类似如下
         * *end()
         * 此行为可能导致UB
         * @return 常量迭代器
         */
        constexpr const_iterator end() const noexcept {
            return const_iterator(elems + _size);
        }

        /**
         * @brief 获取当前容器中指向最后一个元素的常量迭代器
         * @attention 一般情况下，不建议直接读取，而是将其作为比较，例如
         * begin() != cend()
         * 而不是类似如下
         * *end()
         * 此行为可能导致UB
         * @return 常量迭代器
         */
        constexpr const_iterator cend() const noexcept {
            return const_iterator(elems + _size);            
        }

        /**
         * @brief 获取当前容器中第一个元素的引用
         * @return 元素引用
         */
        constexpr reference front() noexcept {
            return elems[0];
        }

        /**
         * @brief 获取当前容器中第一个元素的常量引用
         * @return 元素常量引用
         */
        constexpr const_reference front() const noexcept {
            return elems[0];
        }

        /**
         * @brief 获取当前容器中最后一个元素的引用
         * @return 元素引用
         */
        constexpr reference back() noexcept {
            return elems[_size - 1];
        }

        /**
         * @brief 获取当前容器中最后一个元素的常量引用
         * @return 元素常量引用
         */
        constexpr const_reference back() const noexcept {
            return elems[_size - 1];
        }

        /**
         * @brief 向当前容器中添加一个元素
         * @param value 元素值
         */
        constexpr void push_back(const Ty &value) noexcept(std::is_nothrow_copy_assignable_v<Ty>) {
            range_check(_size + 1);
            if constexpr (type_traits::type_properties::is_pod_v<Ty>) {
                elems[_size++] = value;
            } else {
                utility::construct_at(elems + _size, value);
                _size++;
            }
        }

        constexpr void pop_back() noexcept {
            if (_size != 0) {
                range_check(_size - 1);
                if constexpr (!type_traits::type_properties::is_pod_v<Ty>) {
                    elems[_size].~Ty();
                }
                --_size;
            }
        }

        constexpr void clear() noexcept {
            _size = 0;
        }

        constexpr void resize(std::size_t new_size) {
            range_check(new_size);
            _size = new_size;
        }

        constexpr void swap(frozen_vector &other) noexcept(std::is_nothrow_swappable_v<Ty>) {
            std::swap(_size, other._size);
            std::swap(elems, other.elems);
        }

        constexpr reference at(std::size_t idx) {
            return elems[idx];
        }

        constexpr const_reference at(std::size_t idx) const {
            return elems[idx];
        }

        constexpr std::size_t empty() const noexcept {
            return _size == 0;
        }

        template <typename... Args>
        void emplace_back(Args... args) noexcept {
            range_check(_size + 1);
        }

    private:
        void range_check(const size_type idx) {
            if (idx <= _size) {
                foundation::system::exceptions::logic::throw_out_of_range("Invalid frozen_vector subscript");
            }
        }

        static void check_zero_length_error() {
            if (N == 0) {
                std::terminate();
            }
        }

        std::size_t _size;
        value_type elems[N];
    };

    template <typename Ty, std::size_t N>
    using vector = frozen_vector<Ty, N>;
}

#endif
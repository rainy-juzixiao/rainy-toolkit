/*
 * Copyright 2026 rainy-juzixiao
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef RAINY_COLLECTIONS_VECTOR_HPP
#define RAINY_COLLECTIONS_VECTOR_HPP
#include <rainy/core/core.hpp>

namespace rainy::collections {
    template <typename Ty, typename Alloc = std::allocator<Ty>>
    class vector {
    public:
        using value_type = Ty;
        using allocator_type = Alloc;
        using pointer = Ty *;
        using const_pointer = const Ty *;
        using reference = value_type &;
        using const_reference = const value_type &;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using iterator = Ty *;
        using const_iterator = const Ty *;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        static_assert(type_traits::composite_types::is_object_v<Ty>, "Ty must be a object");
        static_assert(type_traits::type_properties::is_complete_v<Ty>, "Ty require that Ty must a complete type");
        static_assert(type_traits::type_properties::is_destructible_v<Ty>, "Ty must be destructible");
        static_assert(!type_traits::type_properties::is_abstract_v<Ty>, "Ty cannot be a abstract type");
        static_assert(!type_traits::composite_types::is_reference_v<Ty>, "Ty cannot be a reference type");

        RAINY_CONSTEXPR20 vector() noexcept(noexcept(allocator_type())) : vector(allocator_type()) {
        }

        RAINY_CONSTEXPR20 explicit vector(const allocator_type &alloc) noexcept : pair(alloc, {}) {
        }

        RAINY_CONSTEXPR20 explicit vector(size_type count, const allocator_type &alloc = allocator_type()) : pair(alloc, {}) {
            if (count == 0) {
                return;
            }
            auto &allocator = get_al();
            auto &object = vec_object();
            object.start = std::allocator_traits<allocator_type>::allocate(allocator, count);
            object.finish = object.start;
            object.end_of_storage = object.start + count;
            for (size_type i = 0; i < count; ++i) {
                std::allocator_traits<allocator_type>::construct(allocator, object.finish);
                ++object.finish;
            }
        }

        RAINY_CONSTEXPR20 vector(size_type count, const_reference &value, const allocator_type &alloc = allocator_type()) :
            pair(alloc, {}) {
            if (count == 0) {
                return;
            }
            auto &allocator = get_al();
            auto &object = vec_object();
            object.start = std::allocator_traits<allocator_type>::allocate(allocator, count);
            object.finish = object.start;
            object.end_of_storage = object.start + count;
            for (size_type i = 0; i < count; ++i) {
                std::allocator_traits<allocator_type>::construct(allocator, object.finish, value);
                ++object.finish;
            }
        }

        template <typename InputIter>
        RAINY_CONSTEXPR20 vector(InputIter first, InputIter last, const allocator_type &alloc = allocator_type()) : pair(alloc, {}) {
            auto &allocator = get_al();
            auto &object = vec_object();
            const size_type count = static_cast<size_type>(std::distance(first, last));
            if (count == 0) {
                return;
            }
            object.start = std::allocator_traits<allocator_type>::allocate(allocator, count);
            object.finish = object.start;
            object.end_of_storage = object.start + count;
            for (auto it = first; it != last; ++it) {
                std::allocator_traits<allocator_type>::construct(allocator, object.finish, *it);
                ++object.finish;
            }
        }

        RAINY_CONSTEXPR20 vector(const vector &right) :
            pair(std::allocator_traits<allocator_type>::select_on_container_copy_construction(right.get_allocator()), {}) {
            auto &allocator = get_al();
            auto &object = vec_object();
            const size_type count = right.size();
            if (rainy_unlikely(count == 0)) {
                return;
            }
            object.start = std::allocator_traits<allocator_type>::allocate(allocator, count);
            object.finish = object.start;
            object.end_of_storage = object.start + count;
            for (auto it = right.begin(); it != right.end(); ++it) {
                std::allocator_traits<allocator_type>::construct(allocator, object.finish, *it);
                ++object.finish;
            }
        }

        RAINY_CONSTEXPR20 vector(vector &&right) noexcept : pair(utility::move(right.get_allocator()), right.vec_object()) {
            right.vec_object() = {};
        }

        RAINY_CONSTEXPR20 vector(const vector &right, const type_traits::helper::identity_t<allocator_type> &alloc) : pair(alloc, {}) {
            auto &allocator = get_al();
            auto &object = vec_object();
            const size_type count = right.size();
            if (rainy_unlikely(count == 0)) {
                return;
            }
            object.start = std::allocator_traits<allocator_type>::allocate(allocator, count);
            object.finish = object.start;
            object.end_of_storage = object.start + count;
            for (auto it = right.begin(); it != right.end(); ++it) {
                std::allocator_traits<allocator_type>::construct(allocator, object.finish, *it);
                ++object.finish;
            }
        }

        RAINY_CONSTEXPR20 vector(vector &&right, const type_traits::helper::identity_t<allocator_type> &alloc) : pair(alloc, {}) {
            auto &allocator = get_al();
            auto &object = vec_object();
            if (allocator == right.get_allocator()) { // 相同，那就直接窃取资源
                object = right.vec_object();
                right.vec_object() = {};
            } else {
                const size_type count = right.size(); // 分配器不同，我们必须逐元素 move 构造
                if (count == 0) {
                    return;
                }
                object.start = std::allocator_traits<allocator_type>::allocate(allocator, count);
                object.finish = object.start;
                object.end_of_storage = object.start + count;
                for (auto it = right.begin(); it != right.end(); ++it) {
                    std::allocator_traits<allocator_type>::construct(allocator, object.finish, utility::move(*it));
                    ++object.finish;
                }
            }
        }

        RAINY_CONSTEXPR20 vector(std::initializer_list<value_type> ilist, const allocator_type &alloc = allocator_type()) :
            vector(ilist.begin(), ilist.end(), alloc) {
        }

        RAINY_CONSTEXPR20 ~vector() {
            auto &allocator = get_al();
            auto &object = vec_object();
            if (object.start) {
                if constexpr (!type_traits::type_properties::is_trivially_destructible_v<value_type>) {
                    for (pointer p = object.start; p != object.finish; ++p) {
                        std::allocator_traits<allocator_type>::destroy(allocator, p);
                    }
                }
                std::allocator_traits<allocator_type>::deallocate(allocator, object.start,
                                                                  static_cast<size_type>(object.end_of_storage - object.start));
                object = {};
            }
        }

        RAINY_CONSTEXPR20 vector &operator=(const vector &right) {
            if (this == &right) {
                return *this;
            }
            auto &allocator = get_al();
            auto &rim = right.vec_object();
            if constexpr (std::allocator_traits<allocator_type>::propagate_on_container_copy_assignment::value) {
                if (allocator != right.get_allocator()) {
                    clear();
                    std::allocator_traits<allocator_type>::deallocate(
                        allocator, vec_object().start, static_cast<size_type>(vec_object().end_of_storage - vec_object().start));
                    vec_object() = {};
                    allocator = right.get_allocator();
                }
            }
            const size_type new_size = right.size();
            if (new_size > capacity()) {
                // 重新分配
                vector tmp(right, allocator);
                swap(tmp);
                return *this;
            }
            auto &object = vec_object();
            // 覆盖已有元素
            const size_type cur_size = size();
            if (new_size <= cur_size) {
                std::copy(rim.start, rim.start + new_size, object.start); // copy 覆盖前 new_size 个，然后销毁多余的
                for (pointer p = object.start + new_size; p != object.finish; ++p) {
                    std::allocator_traits<allocator_type>::destroy(allocator, p);
                }
            } else {
                std::copy(rim.start, rim.start + cur_size, object.start); // copy 覆盖已有的，然后 construct 新增的
                for (pointer p = rim.start + cur_size; p != rim.finish; ++p) {
                    std::allocator_traits<allocator_type>::construct(allocator, object.start + (p - rim.start), *p);
                }
            }
            object.finish = object.start + new_size;
            return *this;
        }

        RAINY_CONSTEXPR20 vector &operator=(vector &&right) noexcept(
            std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value ||
            std::allocator_traits<allocator_type>::is_always_equal::value) {
            if (rainy_unlikely(this == &right)) {
                return *this;
            }
            auto &allocator = get_al();
            auto &object = vec_object();
            for (pointer p = object.start; p != object.finish; ++p) {
                std::allocator_traits<allocator_type>::destroy(allocator, p);
            }
            if (object.start) {
                std::allocator_traits<allocator_type>::deallocate(allocator, object.start,
                                                                  static_cast<size_type>(object.end_of_storage - object.start));
            }
            if constexpr (std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value) {
                allocator = utility::move(right.get_al());
            }
            object = right.vec_object();
            right.vec_object() = {};
            return *this;
        }

        RAINY_CONSTEXPR20 vector &operator=(std::initializer_list<value_type> ilist) {
            assign(ilist.begin(), ilist.end());
            return *this;
        }

        template <typename InputIter,
                  type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_input_iterator_v<InputIter>, int> = 0>
        RAINY_CONSTEXPR20 void assign(InputIter first, InputIter last) {
            const size_type count = static_cast<size_type>(utility::distance(first, last));
            auto &allocator = get_al();
            auto &object = vec_object();
            if (count > capacity()) {
                vector tmp(first, last, allocator);
                swap(tmp);
                return;
            }
            const size_type cur = size();
            if (count <= cur) {
                auto new_finish = std::copy(first, last, object.start);
                for (pointer p = new_finish; p != object.finish; ++p) {
                    std::allocator_traits<allocator_type>::destroy(allocator, p);
                }
                object.finish = new_finish;
            } else {
                auto mid = first;
                std::advance(mid, cur);
                std::copy(first, mid, object.start);
                for (auto it = mid; it != last; ++it) {
                    std::allocator_traits<allocator_type>::construct(allocator, object.finish, *it);
                    ++object.finish;
                }
            }
        }

        RAINY_CONSTEXPR20 void assign(size_type count, const_reference &elem) {
            auto &allocator = get_al();
            auto &object = vec_object();
            if (count > capacity()) {
                vector tmp(count, elem, allocator);
                swap(tmp);
                return;
            }
            const size_type cur = size();
            if (count <= cur) {
                std::fill_n(object.start, count, elem);
                for (pointer p = object.start + count; p != object.finish; ++p) {
                    std::allocator_traits<allocator_type>::destroy(allocator, p);
                }
                object.finish = object.start + count;
            } else {
                std::fill_n(object.start, cur, elem);
                for (size_type i = cur; i < count; ++i) {
                    std::allocator_traits<allocator_type>::construct(allocator, object.finish, elem);
                    ++object.finish;
                }
            }
        }

        RAINY_CONSTEXPR20 void assign(std::initializer_list<value_type> ilist) {
            assign(ilist.begin(), ilist.end());
        }

        RAINY_CONSTEXPR20 allocator_type get_allocator() const noexcept {
            return pair.get_first();
        }

        RAINY_CONSTEXPR20 iterator begin() noexcept {
            return vec_object().start;
        }

        RAINY_CONSTEXPR20 const_iterator begin() const noexcept {
            return vec_object().start;
        }

        RAINY_CONSTEXPR20 iterator end() noexcept {
            return vec_object().finish;
        }

        RAINY_CONSTEXPR20 const_iterator end() const noexcept {
            return vec_object().finish;
        }

        RAINY_CONSTEXPR20 reverse_iterator rbegin() noexcept {
            return reverse_iterator(end());
        }

        RAINY_CONSTEXPR20 const_reverse_iterator rbegin() const noexcept {
            return const_reverse_iterator(end());
        }

        RAINY_CONSTEXPR20 reverse_iterator rend() noexcept {
            return reverse_iterator(begin());
        }

        RAINY_CONSTEXPR20 const_reverse_iterator rend() const noexcept {
            return const_reverse_iterator(begin());
        }

        RAINY_CONSTEXPR20 const_iterator cbegin() const noexcept {
            return begin();
        }

        RAINY_CONSTEXPR20 const_iterator cend() const noexcept {
            return end();
        }

        RAINY_CONSTEXPR20 const_reverse_iterator crbegin() const noexcept {
            return rbegin();
        }

        RAINY_CONSTEXPR20 const_reverse_iterator crend() const noexcept {
            return rend();
        }

        RAINY_CONSTEXPR20 bool empty() const noexcept {
            return vec_object().start == vec_object().finish;
        }

        RAINY_CONSTEXPR20 size_type size() const noexcept {
            return static_cast<size_type>(vec_object().finish - vec_object().start);
        }

        RAINY_CONSTEXPR20 size_type max_size() const noexcept {
            return std::allocator_traits<allocator_type>::max_size(get_allocator());
        }

        RAINY_CONSTEXPR20 size_type capacity() const noexcept {
            return static_cast<size_type>(vec_object().end_of_storage - vec_object().start);
        }

        RAINY_CONSTEXPR20 void resize(size_type new_size) {
            auto &allocator = get_al();
            auto &object = vec_object();
            const size_type cur = size();
            if (new_size < cur) {
                for (pointer p = object.start + new_size; p != object.finish; ++p) {
                    std::allocator_traits<allocator_type>::destroy(allocator, p);
                }
                object.finish = object.start + new_size;
            } else if (new_size > cur) {
                reserve(new_size);
                for (size_type i = cur; i < new_size; ++i) {
                    std::allocator_traits<allocator_type>::construct(allocator, object.finish);
                    ++object.finish;
                }
            }
        }

        RAINY_CONSTEXPR20 void resize(size_type new_size, const_reference elem) {
            auto &allocator = get_al();
            auto &object = vec_object();
            const size_type cur = size();
            if (new_size < cur) {
                for (pointer p = object.start + new_size; p != object.finish; ++p) {
                    std::allocator_traits<allocator_type>::destroy(allocator, p);
                }
                object.finish = object.start + new_size;
            } else if (new_size > cur) {
                reserve(new_size);
                for (size_type i = cur; i < new_size; ++i) {
                    std::allocator_traits<allocator_type>::construct(allocator, object.finish, elem);
                    ++object.finish;
                }
            }
        }

        RAINY_CONSTEXPR20 void reserve(size_type count) {
            if (count <= capacity()) {
                return;
            }
            auto &allocator = get_al();
            auto &object = vec_object();
            const size_type cur_size = size();
            pointer new_start = std::allocator_traits<allocator_type>::allocate(allocator, count);
            pointer new_finish = new_start;
            for (pointer p = object.start; p != object.finish; ++p) {
                std::allocator_traits<allocator_type>::construct(allocator, new_finish, utility::move_if_noexcept(*p));
                ++new_finish;
            }
            // 销毁旧元素并释放旧内存
            for (pointer p = object.start; p != object.finish; ++p) {
                std::allocator_traits<allocator_type>::destroy(allocator, p);
            }
            if (object.start) {
                std::allocator_traits<allocator_type>::deallocate(allocator, object.start,
                                                                  static_cast<size_type>(object.end_of_storage - object.start));
            }
            object.start = new_start;
            object.finish = new_finish;
            object.end_of_storage = new_start + count;
        }

        RAINY_CONSTEXPR20 void shrink_to_fit() {
            auto &allocator = get_al();
            auto &object = vec_object();
            const size_type cur_size = size();
            if (cur_size == capacity()) {
                return;
            }
            if (cur_size == 0) {
                if (object.start) {
                    std::allocator_traits<allocator_type>::deallocate(allocator, object.start,
                                                                      static_cast<size_type>(object.end_of_storage - object.start));
                }
                object = {};
                return;
            }
            pointer new_start = std::allocator_traits<allocator_type>::allocate(allocator, cur_size);
            pointer new_finish = new_start;
            for (pointer p = object.start; p != object.finish; ++p) {
                std::allocator_traits<allocator_type>::construct(allocator, new_finish, std::move_if_noexcept(*p));
                ++new_finish;
            }
            for (pointer p = object.start; p != object.finish; ++p) {
                std::allocator_traits<allocator_type>::destroy(allocator, p);
            }
            std::allocator_traits<allocator_type>::deallocate(allocator, object.start,
                                                              static_cast<size_type>(object.end_of_storage - object.start));
            object.start = new_start;
            object.finish = new_finish;
            object.end_of_storage = new_start + cur_size;
        }

        RAINY_CONSTEXPR20 reference operator[](size_type index) {
            return vec_object().start[index];
        }

        RAINY_CONSTEXPR20 const_reference operator[](size_type index) const {
            return vec_object().start[index];
        }

        RAINY_CONSTEXPR20 const_reference at(size_type index) const {
            if (index >= size()) {
                throw std::out_of_range("vector::at — index out of range");
            }
            return vec_object().start[index];
        }

        RAINY_CONSTEXPR20 reference at(size_type index) {
            if (index >= size()) {
                throw std::out_of_range("vector::at — index out of range");
            }
            return vec_object().start[index];
        }

        RAINY_CONSTEXPR20 reference front() {
            return *vec_object().start;
        }

        RAINY_CONSTEXPR20 const_reference front() const {
            return *vec_object().start;
        }

        RAINY_CONSTEXPR20 reference back() {
            return *(vec_object().finish - 1);
        }

        RAINY_CONSTEXPR20 const_reference back() const {
            return *(vec_object().finish - 1);
        }

        RAINY_CONSTEXPR20 pointer data() noexcept {
            return vec_object().start;
        }

        RAINY_CONSTEXPR20 const_pointer data() const noexcept {
            return vec_object().start;
        }

        template <typename... Args>
        RAINY_CONSTEXPR20 reference emplace_back(Args &&...args) {
            auto &allocator = get_al();
            auto &object = vec_object();
            if (object.finish == object.end_of_storage) {
                const size_type old_cap = capacity();
                const size_type new_cap = old_cap == 0 ? 1 : old_cap * 2;
                reserve(new_cap);
            }
            std::allocator_traits<allocator_type>::construct(allocator, object.finish, utility::forward<Args>(args)...);
            return *object.finish++;
        }

        RAINY_CONSTEXPR20 void push_back(const_reference right) {
            emplace_back(right);
        }

        RAINY_CONSTEXPR20 void push_back(value_type &&right) {
            emplace_back(utility::move(right));
        }

        RAINY_CONSTEXPR20 void pop_back() {
            auto &allocator = get_al();
            auto &object = vec_object();
            --object.finish;
            std::allocator_traits<allocator_type>::destroy(allocator, object.finish);
        }

        template <typename... Args>
        RAINY_CONSTEXPR20 iterator emplace(const_iterator position, Args &&...args) {
            auto &allocator = get_al();
            auto &object = vec_object();
            const size_type offset = static_cast<size_type>(position - object.start);
            if (object.finish == object.end_of_storage) {
                const size_type old_cap = capacity();
                const size_type new_cap = old_cap == 0 ? 1 : old_cap * 2;
                reserve(new_cap);
            }
            // reserve 可能使指针失效，重新算插入位置
            pointer pos = object.start + offset;
            if (pos != object.finish) {
                // 在末尾 construct 最后一个元素的移动拷贝，然后向后移位
                std::allocator_traits<allocator_type>::construct(allocator, object.finish, utility::move(*(object.finish - 1)));
                std::move_backward(pos, object.finish - 1, object.finish);
                // 销毁旧元素后 placement-construct 新值
                std::allocator_traits<allocator_type>::destroy(allocator, pos);
            }
            std::allocator_traits<allocator_type>::construct(allocator, pos, utility::forward<Args>(args)...);
            ++object.finish;
            return pos;
        }

        RAINY_CONSTEXPR20 iterator insert(const_iterator position, const_reference right) {
            return emplace(position, right);
        }

        RAINY_CONSTEXPR20 iterator insert(const_iterator position, value_type &&right) {
            return emplace(position, utility::move(right));
        }

        RAINY_CONSTEXPR20 iterator insert(const_iterator position, size_type count, const_reference right) {
            if (count == 0) {
                return const_cast<iterator>(position);
            }
            auto &allocator = get_al();
            auto &object = vec_object();
            const size_type offset = static_cast<size_type>(position - object.start);
            const size_type cur_size = size();
            if (cur_size + count > capacity()) {
                const size_type new_cap = std::max(cur_size + count, capacity() * 2);
                reserve(new_cap);
            }
            pointer pos = object.start + offset;
            const size_type elems_after = static_cast<size_type>(object.finish - pos);
            if (elems_after > count) {
                // 尾部有足够多的已有元素，先移动尾部 count 个到未初始化区域
                pointer old_finish = object.finish;
                for (pointer p = old_finish - count; p != old_finish; ++p) {
                    std::allocator_traits<allocator_type>::construct(allocator, p + count, utility::move(*p));
                }
                object.finish += count;
                core::algorithm::move_backward(pos, old_finish - count, old_finish);
                std::fill_n(pos, count, right);
            } else {
                // count >= elems_after
                pointer old_finish = object.finish;
                // 先 construct 超出原 finish 的新元素
                for (size_type i = elems_after; i < count; ++i) {
                    std::allocator_traits<allocator_type>::construct(allocator, object.finish, right);
                    ++object.finish;
                }
                // 将原 [pos, old_finish) 移动到新末尾
                for (pointer p = old_finish - 1; p >= pos; --p) {
                    std::allocator_traits<allocator_type>::construct(allocator, p + count, utility::move(*p));
                    std::allocator_traits<allocator_type>::destroy(allocator, p);
                }
                object.finish = object.start + cur_size + count;
                core::algorithm::fill_n(pos, elems_after, right);
            }
            return pos;
        }

        template <typename InputIter,
                  type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_input_iterator_v<InputIter>, int> = 0>
        RAINY_CONSTEXPR20 iterator insert(const_iterator position, InputIter first, InputIter last) {
            const size_type offset = static_cast<size_type>(position - vec_object().start);
            for (auto it = first; it != last; ++it) {
                // 每次 emplace 后 position 可能因 reserve 失效，用 offset 重新定位
                emplace(vec_object().start + offset + static_cast<size_type>(it - first), *it);
            }
            return vec_object().start + offset;
        }

        RAINY_CONSTEXPR20 iterator insert(const_iterator position, std::initializer_list<value_type> ilist) {
            return insert(position, ilist.begin(), ilist.end());
        }

        RAINY_CONSTEXPR20 iterator erase(const_iterator position) {
            auto &allocator = get_al();
            auto &object = vec_object();
            pointer pos = const_cast<pointer>(position);
            core::algorithm::move(pos + 1, object.finish, pos);
            --object.finish;
            std::allocator_traits<allocator_type>::destroy(allocator, object.finish);
            return pos;
        }

        RAINY_CONSTEXPR20 iterator erase(const_iterator first, const_iterator last) {
            auto &allocator = get_al();
            auto &object = vec_object();
            pointer f = const_cast<pointer>(first);
            pointer l = const_cast<pointer>(last);
            pointer new_finish = core::algorithm::move(l, object.finish, f);
            for (pointer p = new_finish; p != object.finish; ++p) {
                std::allocator_traits<allocator_type>::destroy(allocator, p);
            }
            object.finish = new_finish;
            return f;
        }

        RAINY_CONSTEXPR20 void swap(vector &right) noexcept(
            std::allocator_traits<allocator_type>::propagate_on_container_swap::value ||
            std::allocator_traits<allocator_type>::is_always_equal::value) {
            using std::swap;
            if constexpr (std::allocator_traits<allocator_type>::propagate_on_container_swap::value) {
                swap(get_al(), right.get_al());
            }
            swap(vec_object(), right.vec_object());
        }

        RAINY_CONSTEXPR20 void clear() noexcept {
            auto &allocator = get_al();
            auto &object = vec_object();
            for (pointer p = object.start; p != object.finish; ++p) {
                std::allocator_traits<allocator_type>::destroy(allocator, p);
            }
            object.finish = object.start;
        }

        /**
         * @brief Equality comparison operator for vectors.
         *        判断两个向量是否相等。
         *
         * @param left Left vector
         *             左侧向量
         * @param right Right vector
         *              右侧向量
         * @return true if all corresponding elements are equal, false otherwise.
         * @return 若两个向量元素一一对应且相等则返回 true，否则返回 false。
         */
        friend constexpr rain_fn operator==(const vector &left, const vector &right) noexcept -> bool {
            return core::algorithm::equal(left.begin(), left.end(), right.begin(), right.end());
        }

        /**
         * @brief Inequality comparison operator for vectors.
         *        判断两个向量是否不相等。
         *
         * @param left Left vector
         *             左侧向量
         * @param right Right vector
         *              右侧向量
         * @return true if the vectors are not equal, false otherwise.
         * @return 若两个向量不相等则返回 true，否则返回 false。
         */
        friend constexpr rain_fn operator!=(const vector &left, const vector &right) noexcept -> bool {
            return !(left == right);
        }

        /**
         * @brief Less-than comparison operator for vectors (lexicographical).
         *        判断左侧向量是否小于右侧向量（字典序比较）。
         *
         * @param left Left vector
         *             左侧向量
         * @param right Right vector
         *              右侧向量
         * @return true if left is lexicographically less than right, false otherwise.
         * @return 若左侧向量在字典序上小于右侧向量则返回 true，否则返回 false。
         */
        friend constexpr rain_fn operator<(const vector &left, const vector &right) noexcept -> bool {
            return core::algorithm::lexicographical_compare(left.begin(), left.end(), right.begin(), right.end());
        }

        /**
         * @brief Greater-than comparison operator for vectors (lexicographical).
         *        判断左侧向量是否大于右侧向量（字典序比较）。
         *
         * @param left Left vector
         *             左侧向量
         * @param right Right vector
         *              右侧向量
         * @return true if left is lexicographically greater than right, false otherwise.
         * @return 若左侧向量在字典序上大于右侧向量则返回 true，否则返回 false。
         */
        friend constexpr rain_fn operator>(const vector &left, const vector &right) noexcept -> bool {
            return right < left;
        }

        /**
         * @brief Less-than-or-equal comparison operator for vectors (lexicographical).
         *        判断左侧向量是否小于等于右侧向量（字典序比较）。
         *
         * @param left Left vector
         *             左侧向量
         * @param right Right vector
         *              右侧向量
         * @return true if left is lexicographically less than or equal to right, false otherwise.
         * @return 若左侧向量小于或等于右侧向量则返回 true，否则返回 false。
         */
        friend constexpr rain_fn operator<=(const vector &left, const vector &right) noexcept -> bool {
            return !(left > right);
        }

        /**
         * @brief Greater-than-or-equal comparison operator for vectors (lexicographical).
         *        判断左侧向量是否大于等于右侧向量（字典序比较）。
         *
         * @param left Left vector
         *             左侧向量
         * @param right Right vector
         *              右侧向量
         * @return true if left is lexicographically greater than or equal to right, false otherwise.
         * @return 若左侧向量大于或等于右侧向量则返回 true，否则返回 false。
         */
        friend constexpr rain_fn operator>=(const vector &left, const vector &right) noexcept -> bool {
            return !(left < right);
        }

        friend constexpr rain_fn swap(vector &left, vector &right) noexcept -> void {
            left.swap(right);
        }

    private:
        struct impl {
            Ty *start{};
            Ty *finish{};
            Ty *end_of_storage{};
        };

        constexpr impl &vec_object() noexcept {
            return pair.second;
        }

        constexpr const impl &vec_object() const noexcept {
            return pair.second;
        }

        constexpr allocator_type &get_al() noexcept {
            return pair.get_first();
        }

        foundation::container::compressed_pair<allocator_type, impl> pair;
    };
}

#endif

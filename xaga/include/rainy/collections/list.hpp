/*
 * Copyright 2025 rainy-juzixiao
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
#ifndef RAINY_COLLECTIONS_LIST_HPP
#define RAINY_COLLECTIONS_LIST_HPP
#include <rainy/core/core.hpp>
#include <rainy/utility/pair.hpp>
#include <rainy/utility/iterator.hpp>
#include <rainy/foundation/functional/functor.hpp>
#include <rainy/foundation/memory/allocator.hpp>

#if RAINY_USING_MSVC
#pragma warning(push)
#pragma warning(disable: 26439)
#endif

namespace rainy::collections {
    /**
     * @brief 
     */
    template <typename Ty, typename Alloc>
    class list_node;

    /**
     * @brief 适用于rainy-toolkit的链表数据结构，类似于std::list
     * @brief 但该链表被用于实现更复杂的功能
     * @tparam Ty 表示要存储的值类型
     * @tparam Allocator 表示要存储值类型所对应的分配器
     */
    template <typename Ty, typename Allocator = foundation::memory::allocator<Ty>>
    class list {
    public:
        struct node {
            Ty value;
            node *prev;
            node *next;

            template <typename... Args>
            node(Args &&...args) : value(utility::forward<Args>(args)...), prev(this), next(this) {
            }
        };

        using node_allocator_type = typename foundation::memory::allocator<node>;
        using node_allocator_traits = foundation::memory::allocator_traits<node_allocator_type>;

        using value_type = Ty;
        using allocator_type = Allocator;
        using pointer = typename foundation::memory::allocator_traits<Allocator>::pointer;
        using const_pointer = typename foundation::memory::allocator_traits<Allocator>::const_pointer;
        using reference = value_type &;
        using const_reference = const value_type &;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;

        template <bool IsConst>
        class list_iterator {
        public:
            using iterator_category = std::bidirectional_iterator_tag;
            using value_type = Ty;
            using difference_type = std::ptrdiff_t;
            using pointer = type_traits::other_trans::conditional_t<IsConst, const Ty *, Ty *>;
            using reference = type_traits::other_trans::conditional_t<IsConst, const Ty &, Ty &>;
            using node_pointer = type_traits::other_trans::conditional_t<IsConst, const node *, node *>;

            friend class list;
            friend class list_iterator<!IsConst>;

            list_iterator() : node_ptr(nullptr) {
            }
            explicit list_iterator(node_pointer ptr) : node_ptr(ptr) {
            }

            template <bool WasConst, type_traits::other_trans::enable_if_t<IsConst && !WasConst, int> = 0>
            list_iterator(const list_iterator<WasConst> &right) : node_ptr(right.node_ptr) {
            }

            reference operator*() const {
                return node_ptr->value;
            }

            pointer operator->() const {
                return &(node_ptr->value);
            }

            list_iterator &operator++() {
                node_ptr = node_ptr->next;
                return *this;
            }

            list_iterator operator++(int) {
                list_iterator tmp = *this;
                ++(*this);
                return tmp;
            }

            list_iterator &operator--() {
                node_ptr = node_ptr->prev;
                return *this;
            }

            list_iterator operator--(int) {
                list_iterator tmp = *this;
                --(*this);
                return tmp;
            }

            bool operator==(const list_iterator<false> &right) const {
                return node_ptr == right.node_ptr;
            }

            bool operator==(const list_iterator<true> &right) const {
                return node_ptr == right.node_ptr;
            }

            bool operator!=(const list_iterator<false> &right) const {
                return node_ptr != right.node_ptr;
            }

            bool operator!=(const list_iterator<true> &right) const {
                return node_ptr != right.node_ptr;
            }

        private:
            node_pointer node_ptr;
        };

        using iterator = list_iterator<false>;
        using const_iterator = list_iterator<true>;
        using reverse_iterator = utility::reverse_iterator<iterator>;
        using const_reverse_iterator = utility::reverse_iterator<const_iterator>;

        list() : list(allocator_type()) {
        }

        explicit list(const allocator_type &alloc) : allocator_(alloc), size_{} {
            create_sentinel();
        }

        explicit list(size_type count, const allocator_type &alloc = allocator_type()) : allocator_(alloc), size_{} {
            create_sentinel();
            for (size_type i = 0; i < count; ++i) {
                emplace_back();
            }
        }

        list(size_type count, const Ty &value, const allocator_type &alloc = allocator_type()) : allocator_(alloc), size_{} {
            create_sentinel();
            for (size_type i = 0; i < count; ++i) {
                push_back(value);
            }
        }

        template <typename InputIter, type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_iterator_v<InputIter>,int> = 0>
        list(InputIter first, InputIter last, const allocator_type &alloc = allocator_type()) : allocator_(alloc), size_{} {
            create_sentinel();
            for (auto it = first; it != last; ++it) {
                push_back(*it);
            }
        }

        list(const list &right) : allocator_(node_allocator_traits::select_on_container_copy_construction(right.allocator_)), size_{} {
            create_sentinel();
            for (const auto &item: right) {
                push_back(item);
            }
        }

        list(list &&right) : allocator_(utility::move(right.allocator_)), sentinel_(right.sentinel_), size_(right.size_) { // NOLINT
            right.sentinel_ = nullptr;
            right.size_ = 0;
            right.create_sentinel();
        }

        list(const list &right, const std::type_identity_t<allocator_type> &alloc) : allocator_(alloc), size_(0) {
            create_sentinel();
            for (const auto &item: right) {
                push_back(item);
            }
        }

        list(list &&right, const std::type_identity_t<allocator_type> &alloc) : allocator_(alloc), size_(0) {
            create_sentinel();
            if (allocator_ == right.allocator_) {
                std::swap(sentinel_, right.sentinel_);
                std::swap(size_, right.size_);
            } else {
                for (auto &item: right) {
                    push_back(utility::move(item));
                }
            }
        }

        list(std::initializer_list<value_type> ilist, const allocator_type &alloc = allocator_type()) : allocator_(alloc), size_(0) {
            create_sentinel();
            for (const auto &item: ilist) {
                push_back(item);
            }
        }

        ~list() {
            clear();
            if (sentinel_) {
                node_allocator_traits::deallocate(allocator_, sentinel_, 1);
            }
        }

        list &operator=(const list &right) {
            if (this != &right) {
                if (node_allocator_traits::propagate_on_container_copy_assignment::value) {
                    if (allocator_ != right.allocator_) {
                        clear();
                        allocator_ = right.allocator_;
                    }
                }
                assign(right.begin(), right.end());
            }
            return *this;
        }

        list &operator=(list &&right) noexcept(std::allocator_traits<Allocator>::is_always_equal::value) {
            if (this != &right) {
                clear();
                if constexpr (node_allocator_traits::propagate_on_container_move_assignment::value) {
                    allocator_ = utility::move(right.allocator_);
                }
                if (allocator_ == right.allocator_) {
                    std::swap(sentinel_, right.sentinel_);
                    std::swap(size_, right.size_);
                } else {
                    for (auto &item: right) {
                        push_back(utility::move(item));
                    }
                }
            }
            return *this;
        }

        list &operator=(std::initializer_list<value_type> ilist) {
            assign(ilist);
            return *this;
        }

        template <typename InputIter>
        void assign(InputIter first, InputIter last) {
            clear();
            for (auto it = first; it != last; ++it) {
                push_back(*it);
            }
        }

        void assign(size_type count, const value_type &value) {
            clear();
            for (size_type i = 0; i < count; ++i) {
                push_back(value);
            }
        }

        void assign(std::initializer_list<value_type> ilist) {
            assign(ilist.begin(), ilist.end());
        }

        allocator_type get_allocator() const noexcept {
            return allocator_type(allocator_);
        }

        iterator begin() noexcept {
            return iterator(sentinel_->next);
        }

        const_iterator begin() const noexcept {
            return const_iterator(sentinel_->next);
        }

        iterator end() noexcept {
            return iterator(sentinel_);
        }

        const_iterator end() const noexcept {
            return const_iterator(sentinel_);
        }

        reverse_iterator rbegin() noexcept {
            return reverse_iterator(end());
        }

        const_reverse_iterator rbegin() const noexcept {
            return const_reverse_iterator(end());
        }

        reverse_iterator rend() noexcept {
            return reverse_iterator(begin());
        }

        const_reverse_iterator rend() const noexcept {
            return const_reverse_iterator(begin());
        }

        const_iterator cbegin() const noexcept {
            return begin();
        }

        const_iterator cend() const noexcept {
            return end();
        }

        const_reverse_iterator crbegin() const noexcept {
            return rbegin();
        }

        const_reverse_iterator crend() const noexcept {
            return rend();
        }

        bool empty() const noexcept {
            return size_ == 0;
        }

        size_type size() const noexcept {
            return size_;
        }

        size_type max_size() const noexcept {
            return node_allocator_traits::max_size(allocator_);
        }

        void resize(size_type sz) {
            while (size_ > sz) {
                pop_back();
            }
            while (size_ < sz) {
                emplace_back();
            }
        }

        void resize(size_type sz, const Ty &c) {
            while (size_ > sz) {
                pop_back();
            }
            while (size_ < sz) {
                push_back(c);
            }
        }

        reference front() {
            return sentinel_->next->value;
        }

        const_reference front() const {
            return sentinel_->next->value;
        }

        reference back() {
            return sentinel_->prev->value;
        }

        const_reference back() const {
            return sentinel_->prev->value;
        }

        template <typename... Args>
        reference emplace_front(Args &&...args) {
            node *new_node = create_node(utility::forward<Args>(args)...);
            link_before(sentinel_->next, new_node);
            ++size_;
            return new_node->value;
        }

        template <typename... Args>
        reference emplace_back(Args &&...args) {
            node *new_node = create_node(utility::forward<Args>(args)...);
            link_before(sentinel_, new_node);
            ++size_;
            return new_node->value;
        }

        void push_front(const Ty &left) {
            emplace_front(left);
        }

        void push_front(Ty &&left) {
            emplace_front(utility::move(left));
        }

        void pop_front() {
            node *list_node = sentinel_->next;
            unlink(list_node);
            destroy_node(list_node);
            --size_;
        }

        void push_back(const Ty &left) {
            emplace_back(left);
        }

        void push_back(Ty &&left) {
            emplace_back(utility::move(left));
        }

        void pop_back() {
            node *list_node = sentinel_->prev;
            unlink(list_node);
            destroy_node(list_node);
            --size_;
        }

        template <typename... Args>
        iterator emplace(const_iterator position, Args &&...args) {
            node *new_node = create_node(utility::forward<Args>(args)...);
            link_before(const_cast<node *>(position.node_ptr), new_node);
            ++size_;
            return iterator(new_node);
        }

        iterator insert(const_iterator position, const Ty &left) {
            return emplace(position, left);
        }

        iterator insert(const_iterator position, Ty &&left) {
            return emplace(position, utility::move(left));
        }

        iterator insert(const_iterator position, size_type count, const Ty &left) {
            iterator result = iterator(const_cast<node *>(position.node_ptr));
            for (size_type i = 0; i < count; ++i) {
                result = insert(position, left);
            }
            return result;
        }

        template <typename InputIter>
        iterator insert(const_iterator position, InputIter first, InputIter last) {
            iterator result = iterator(const_cast<node *>(position.node_ptr));
            for (auto it = first; it != last; ++it) {
                result = insert(position, *it);
            }
            return result;
        }

        iterator insert(const_iterator position, std::initializer_list<Ty> ilist) {
            return insert(position, ilist.begin(), ilist.end());
        }

        iterator insert(const_iterator position, list_node<value_type, allocator_type> &&new_node) {
            if (new_node.empty()) {
                return {};
            }
            auto new_node_ptr = new_node.release();
            link_before(const_cast<node *>(position.node_ptr), new_node_ptr);
            ++size_;
            return iterator(new_node_ptr);
        }

        iterator erase(const_iterator position) {
            node *list_node = const_cast<node *>(position.node_ptr);
            node *next = list_node->next;
            unlink(list_node);
            destroy_node(list_node);
            --size_;
            return iterator(next);
        }

        iterator erase(const_iterator first, const_iterator last) {
            auto it = first;
            while (it != last) {
                it = erase(it);
            }
            return iterator(const_cast<node *>(last.node_ptr));
        }

        void swap(list &right) noexcept(foundation::memory::allocator_traits<Allocator>::is_always_equal::value) {
            std::swap(sentinel_, right.sentinel_);
            std::swap(size_, right.size_);
            if constexpr (node_allocator_traits::propagate_on_container_swap::value) {
                std::swap(allocator_, right.allocator_);
            }
        }

        void clear() noexcept {
            while (!empty()) {
                pop_front();
            }
        }

        void splice(const_iterator position, list &right) {
            splice(position, right, right.begin(), right.end());
        }

        void splice(const_iterator position, list &&right) {
            splice(position, right);
        }

        void splice(const_iterator position, list &right, const_iterator i) {
            const_iterator j = i;
            ++j;
            splice(position, right, i, j);
        }

        void splice(const_iterator position, list &&right, const_iterator i) {
            splice(position, right, i);
        }

        void splice(const_iterator position, list &right, const_iterator first, const_iterator last) {
            if (first == last) {
                return;
            }
            node *pos = const_cast<node *>(position.node_ptr);
            node *f = const_cast<node *>(first.node_ptr);
            node *l = const_cast<node *>(last.node_ptr)->prev;
            size_type count = 0;
            for (auto it = first; it != last; ++it) {
                ++count;
            }
            // 从right中断开[first, last)
            f->prev->next = l->next;
            l->next->prev = f->prev;
            // 插入到position之前
            f->prev = pos->prev;
            l->next = pos;
            pos->prev->next = f;
            pos->prev = l;
            right.size_ -= count;
            size_ += count;
        }

        void splice(const_iterator position, list &&right, const_iterator first, const_iterator last) {
            splice(position, right, first, last);
        }

        size_type remove(const value_type &value) {
            return remove_if([&value](const value_type &left) { return left == value; });
        }

        template <typename Predicate>
        size_type remove_if(Predicate pred) {
            size_type count = 0;
            auto it = begin();
            while (it != end()) {
                if (pred(*it)) {
                    it = erase(it);
                    ++count;
                } else {
                    ++it;
                }
            }
            return count;
        }

        size_type unique() {
            return unique(std::equal_to<Ty>());
        }

        template <typename BinaryPredicate>
        size_type unique(BinaryPredicate&& binary_pred) {
            if (empty()) {
                return 0;
            }
            size_type count = 0;
            auto it = begin();
            auto next = it;
            ++next;
            while (next != end()) {
                if (binary_pred(*it, *next)) {
                    next = erase(next);
                    ++count;
                } else {
                    it = next;
                    ++next;
                }
            }
            return count;
        }

        void merge(list &right) {
            merge(right, foundation::functional::less<Ty>());
        }

        void merge(list &&right) {
            merge(right);
        }

        template <typename Compare>
        void merge(list &right, Compare&& comp) {
            if (this == &right) {
                return;
            }
            auto it1 = begin();
            auto it2 = right.begin();
            while (it1 != end() && it2 != right.end()) {
                if (comp(*it2, *it1)) {
                    auto next = it2;
                    ++next;
                    splice(it1, right, it2);
                    it2 = next;
                } else {
                    ++it1;
                }
            }
            if (it2 != right.end()) {
                splice(end(), right, it2, right.end());
            }
        }

        template <typename Compare>
        void merge(list &&right, Compare comp) {
            merge(right, comp);
        }

        void sort() {
            sort(std::less<Ty>());
        }

        template <typename Compare>
        void sort(Compare comp) {
            if (size_ <= 1) {
                return;
            }
            // 归并排序
            list carry;
            list counter[64];
            int fill = 0;
            while (!empty()) {
                carry.splice(carry.begin(), *this, begin());
                int i = 0;
                while (i < fill && !counter[i].empty()) {
                    counter[i].merge(carry, comp);
                    carry.swap(counter[i++]);
                }
                carry.swap(counter[i]);
                if (i == fill) {
                    ++fill;
                }
            }
            for (int i = 1; i < fill; ++i) {
                counter[i].merge(counter[i - 1], comp);
            }
            swap(counter[fill - 1]);
        }

        void reverse() noexcept {
            if (size_ <= 1) {
                return;
            }
            node *current = sentinel_->next;
            do {
                std::swap(current->prev, current->next);
                current = current->prev; // 因为已经交换，所以prev是原来的next
            } while (current != sentinel_->next);
        }

        list_node<value_type, node_allocator_type> extract_node(const_iterator position) {
            node *node_ptr = const_cast<node *>(position.node_ptr);
            // 从链表中断开该节点
            unlink(node_ptr);
            --size_;
            // 返回包装的独立节点
            return list_node<value_type, node_allocator_type>{node_ptr, allocator_};
        }

        list take_front(size_type count) {
            if (count == 0 || empty()) {
                return {};
            }
            list out;
            if (count >= size()) {
                out.splice(out.end(), *this);
                return out;
            }
            auto mid = begin();
            std::advance(mid, count);
            out.splice(out.end(), *this, begin(), mid);
            return out;
        }

        list take_back(size_type n) {
            if (n == 0 || empty()) {
                return {};
            }
            list out;
            if (n >= size()) {
                out.splice(out.end(), *this);
                return out;
            }
            auto mid = std::prev(end(), n);
            out.splice(out.end(), *this, mid, end());
            return out;
        }

        friend class list_node<value_type, node_allocator_type>;

    private:
        void create_sentinel() {
            sentinel_ = node_allocator_traits::allocate(allocator_, 1);
            sentinel_->prev = sentinel_;
            sentinel_->next = sentinel_;
        }

        template <typename... Args>
        node *create_node(Args &&...args) {
            node *new_node = node_allocator_traits::allocate(allocator_, 1);
            try {
                node_allocator_traits::construct(allocator_, new_node, utility::forward<Args>(args)...);
            } catch (...) {
                node_allocator_traits::deallocate(allocator_, new_node, 1);
                throw;
            }
            return new_node;
        }

        void destroy_node(node *list_node) {
            node_allocator_traits::destroy(allocator_, list_node);
            node_allocator_traits::deallocate(allocator_, list_node, 1);
        }

        void link_before(node *position, node *new_node) {
            new_node->next = position;
            new_node->prev = position->prev;
            position->prev->next = new_node;
            position->prev = new_node;
        }

        void unlink(node *list_node) {
            list_node->prev->next = list_node->next;
            list_node->next->prev = list_node->prev;
        }

        node *sentinel_; // 哨兵节点（头尾相连的环形链表）
        size_type size_;
        node_allocator_type allocator_;
    };

    template <typename Ty, typename Alloc>
    class list_node {
    public:
        using node = typename list<Ty>::node;
        using allocator_type = Alloc;

        friend class list<Ty, Alloc>;

        list_node() : ptr{nullptr} {
        }

        explicit list_node(node *p, Alloc alloc = Alloc()) : ptr{p}, allocator{alloc} {
            if (ptr) {
                ptr->prev = ptr;
                ptr->next = ptr;
            }
        }

        list_node(list_node &&right) noexcept : ptr(right.ptr), allocator{right.allocator} {
            right.ptr = nullptr;
            right.allocator = {};
        }

        list_node &operator=(list_node &&right) noexcept {
            if (this != &right) {
                deallocate_this_node();
                ptr = right.ptr;
                right.ptr = nullptr;
            }
            return *this;
        }

        list_node(const list_node &) = delete;
        list_node &operator=(const list_node &) = delete;

        ~list_node() {
            deallocate_this_node();
        }

        Ty &value() {
            return ptr->value;
        }

        const Ty &value() const {
            return ptr->value;
        }

        bool empty() const noexcept {
            return ptr == nullptr;
        }

        node *release() noexcept {
            node *temp = ptr;
            ptr = nullptr;
            return temp;
        }

        void deallocate_this_node() {
            if (ptr) {
                std::allocator_traits<Alloc>::destroy(this->allocator, ptr);
                std::allocator_traits<Alloc>::deallocate(this->allocator, ptr, 1);            
            }
            ptr = nullptr;
        }

    private:
        node *ptr{};
        Alloc allocator{};
    };

    template <typename Ty, typename Allocator>
    bool operator==(const list<Ty, Allocator> &left, const list<Ty, Allocator> &right) {
        if (left.size() != right.size()) {
            return false;
        }
        return core::algorithm::equal(left.begin(), left.end(), right.begin());
    }

    template <typename Ty, typename Allocator>
    bool operator!=(const list<Ty, Allocator> &left, const list<Ty, Allocator> &right) {
        return !(left == right);
    }

    template <typename Ty, typename Allocator>
    bool operator<(const list<Ty, Allocator> &left, const list<Ty, Allocator> &right) {
        return core::algorithm::lexicographical_compare(left.begin(), left.end(), right.begin(), right.end());
    }

    template <typename Ty, typename Allocator>
    bool operator>(const list<Ty, Allocator> &left, const list<Ty, Allocator> &right) {
        return right < left;
    }

    template <typename Ty, typename Allocator>
    bool operator<=(const list<Ty, Allocator> &left, const list<Ty, Allocator> &right) {
        return !(right < left);
    }

    template <typename Ty, typename Allocator>
    bool operator>=(const list<Ty, Allocator> &left, const list<Ty, Allocator> &right) {
        return !(left < right);
    }

    template <typename Ty, typename Allocator>
    void swap(list<Ty, Allocator> &left, list<Ty, Allocator> &right) noexcept(noexcept(left.swap(right))) {
        left.swap(right);
    }
}

#if RAINY_USING_MSVC
#pragma warning(pop)
#endif

#endif
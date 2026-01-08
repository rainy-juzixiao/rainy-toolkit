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
#ifndef RAINY_COLLECTIONS_CONCURRENCY_FORWARD_LIST_HPP
#define RAINY_COLLECTIONS_CONCURRENCY_FORWARD_LIST_HPP
#include <rainy/core/core.hpp>
#include <rainy/foundation/functional/functor.hpp>
#include <rainy/foundation/memory/allocator.hpp>
#include <rainy/foundation/memory/hazard_pointer.hpp>

#if RAINY_USING_MSVC
#pragma warning(push)
#pragma warning(disable : 26439)
#pragma warning(disable : 26495)
#endif

namespace rainy::collections::concurrency {
    /**
     * @brief 用于并发安全的单向链表，用于保证数据安全插入和删除.
     * @attention 在插入或弹出时，可能产生乱序，仅保证操作是原子性的（x86 only）
     * @tparam Ty 值类型
     * @tparam Allocator 需要该分配器能安全分配内存的，不应出现如下情况：内存复用、使用非线程安全的池化分配器
     */
    template <typename Ty, typename Allocator = foundation::memory::allocator<Ty>>
    class forward_list {
    public:
        struct node;

        using node_allocator_type = foundation::memory::allocator<node>;

        struct node_deleter {
            explicit node_deleter(node_allocator_type *allocator) : allocator(allocator) {
            }

            void operator()(node *p) const {
                if (p && allocator) {
                    foundation::memory::allocator_traits<node_allocator_type>::destroy(*allocator, p);
                    foundation::memory::allocator_traits<node_allocator_type>::deallocate(*allocator, p, 1);
                }
            }

            node_allocator_type *allocator;
        };

        struct node {
            Ty value;
            std::atomic<node *> next;

            template <typename... Args>
            explicit node(Args &&...args) : value(utility::forward<Args>(args)...), next(nullptr) {
            }

            node(const node &) = delete;
            node &operator=(const node &) = delete;
        };

        using value_type = Ty;
        using allocator_type = Allocator;
        using allocator_traits = foundation::memory::allocator_traits<Allocator>;
        using pointer = typename allocator_traits::pointer;
        using const_pointer = typename allocator_traits::const_pointer;
        using reference = value_type &;
        using const_reference = const value_type &;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;

        class iterator;

        class const_iterator {
        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = Ty;
            using difference_type = std::ptrdiff_t;
            using pointer = const Ty *;
            using reference = const Ty &;

            const_iterator() : node_ptr(nullptr) {
            }
            explicit const_iterator(const node *ptr) : node_ptr(ptr) {
            }
            const_iterator(const iterator &it) : node_ptr(it.node_ptr) {
            }

            reference operator*() const {
                return node_ptr->value;
            }
            pointer operator->() const {
                return &node_ptr->value;
            }

            const_iterator &operator++() {
                if (node_ptr) {
                    node_ptr = node_ptr->next.load(std::memory_order_acquire);
                }
                return *this;
            }

            const_iterator operator++(int) {
                const_iterator tmp = *this;
                ++(*this);
                return tmp;
            }

            bool operator==(const const_iterator &other) const {
                return node_ptr == other.node_ptr;
            }
            bool operator!=(const const_iterator &other) const {
                return node_ptr != other.node_ptr;
            }

        private:
            const node *node_ptr;
            friend class forward_list;
        };

        class iterator {
        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = Ty;
            using difference_type = std::ptrdiff_t;
            using pointer = Ty *;
            using reference = Ty &;

            iterator() : node_ptr(nullptr) {
            }
            explicit iterator(node *ptr) : node_ptr(ptr) {
            }

            reference operator*() const {
                return node_ptr->value;
            }
            pointer operator->() const {
                return &node_ptr->value;
            }

            iterator &operator++() {
                if (node_ptr) {
                    node_ptr = node_ptr->next.load(std::memory_order_acquire);
                }
                return *this;
            }

            iterator operator++(int) {
                iterator tmp = *this;
                ++(*this);
                return tmp;
            }

            bool operator==(const iterator &other) const {
                return node_ptr == other.node_ptr;
            }
            bool operator!=(const iterator &other) const {
                return node_ptr != other.node_ptr;
            }

        private:
            node *node_ptr;
            friend class forward_list;
            friend class const_iterator;
        };

        forward_list() : forward_list(allocator_type()) {
        }

        explicit forward_list(const allocator_type &alloc) : allocator_(alloc), head_(allocate_node_dummy()), size_(0) {
        }

        explicit forward_list(size_type n, const allocator_type &alloc = allocator_type()) :
            allocator_(alloc), head_(allocate_node_dummy()), size_(0) {
            for (size_type i = 0; i < n; ++i) {
                emplace_front();
            }
        }

        forward_list(size_type n, const Ty &value, const allocator_type &alloc = allocator_type()) :
            allocator_(alloc), head_(allocate_node_dummy()), size_(0) {
            for (size_type i = 0; i < n; ++i) {
                push_front(value);
            }
        }

        template <typename InputIter>
        forward_list(InputIter first, InputIter last, const allocator_type &alloc = allocator_type()) :
            allocator_(alloc), head_(allocate_node_dummy()), size_(0) {
            for (auto it = first; it != last; ++it) {
                push_front(*it);
            }
        }

        forward_list(const forward_list &) = delete;
        forward_list(forward_list &&) = delete;

        forward_list(std::initializer_list<Ty> ilist, const allocator_type &alloc = allocator_type()) :
            allocator_(alloc), head_(allocate_node_dummy()), size_(0) {
            for (auto it = ilist.begin(); it != ilist.end(); ++it) {
                push_front(*it);
            }
        }

        ~forward_list() {
            clear();
            deallocate_node_dummy(head_);
        }

        forward_list &operator=(const forward_list &) = delete;
        forward_list &operator=(forward_list &&) = delete;

        forward_list &operator=(std::initializer_list<Ty> ilist) {
            assign(ilist.begin(), ilist.end());
            return *this;
        }

        template <typename InputIter,
                  type_traits::other_trans::enable_if_t<type_traits::extras::iterators::is_iterator_v<InputIter>, int> = 0>
        void assign(InputIter first, InputIter last) {
            clear();
            for (auto it = first; it != last; ++it) {
                push_front(*it);
            }
        }

        void assign(size_type n, const Ty &t) {
            clear();
            for (size_type i = 0; i < n; ++i) {
                push_front(t);
            }
        }

        void assign(std::initializer_list<Ty> ilist) {
            assign(ilist.begin(), ilist.end());
        }

        allocator_type get_allocator() const noexcept {
            return allocator_type(allocator_);
        }

        iterator before_begin() noexcept {
            return iterator(head_);
        }
        const_iterator before_begin() const noexcept {
            return const_iterator(head_);
        }

        iterator begin() noexcept {
            return iterator(head_->next.load(std::memory_order_acquire));
        }
        const_iterator begin() const noexcept {
            return const_iterator(head_->next.load(std::memory_order_acquire));
        }

        iterator end() noexcept {
            return iterator(nullptr);
        }
        const_iterator end() const noexcept {
            return const_iterator(nullptr);
        }

        const_iterator cbegin() const noexcept {
            return begin();
        }
        const_iterator cbefore_begin() const noexcept {
            return before_begin();
        }
        const_iterator cend() const noexcept {
            return end();
        }

        RAINY_NODISCARD bool empty() const noexcept {
            return head_->next.load(std::memory_order_acquire) == nullptr;
        }

        RAINY_NODISCARD size_type max_size() const noexcept {
            return std::allocator_traits<allocator_type>::max_size(allocator_);
        }

        RAINY_NODISCARD size_type size() const noexcept {
            return size_.load(std::memory_order_relaxed);
        }

        reference front() {
            node *f = head_->next.load(std::memory_order_acquire);
            return f->value;
        }

        const_reference front() const {
            const node *f = head_->next.load(std::memory_order_acquire);
            return f->value;
        }

        template <typename... Args>
        reference emplace_front(Args &&...args) {
            node *new_node = allocate_node(utility::forward<Args>(args)...);
            struct node_guard {
                node *n;
                node_allocator_type *alloc;
                bool released = false;
                ~node_guard() {
                    if (!released && n) {
                        std::allocator_traits<node_allocator_type>::destroy(*alloc, n);
                        alloc->deallocate(n, 1);
                    }
                }
            } guard{new_node, &allocator_};
            node *old_head = head_->next.load(std::memory_order_acquire);
            do {
                new_node->next.store(old_head, std::memory_order_relaxed);
            } while (!head_->next.compare_exchange_weak(old_head, new_node, std::memory_order_release, std::memory_order_acquire));
            guard.released = true;
            size_.fetch_add(1, std::memory_order_relaxed);
            return new_node->value;
        }

        void push_front(const Ty &x) {
            emplace_front(x);
        }

        void push_front(Ty &&x) {
            emplace_front(utility::move(x));
        }

        void pop_front() noexcept {
            auto hp = foundation::memory::make_hazard_pointer();
            node *first;

            do {
                first = hp.protect(head_->next.load());
                if (!first)
                    return;
            } while (!head_->next.compare_exchange_strong(first, first->next.load(std::memory_order_acquire),
                                                          std::memory_order_release, std::memory_order_acquire));
            hp.reset_protection();
            // 使用自定义删除器
            foundation::memory::hazard_pointer_domain<node>::global().retire(first);
            size_.fetch_sub(1, std::memory_order_relaxed);
        }

        template <typename... Args>
        iterator emplace_after(const_iterator position, Args &&...args) {
            node *new_node = allocate_node(utility::forward<Args>(args)...);
            node *pos = const_cast<node *>(position.node_ptr);

            auto hp = foundation::memory::make_hazard_pointer();
            node *next;

            do {
                next = hp.protect(pos->next.load());
                new_node->next.store(next, std::memory_order_relaxed);
            } while (!pos->next.compare_exchange_weak(next, new_node, std::memory_order_release, std::memory_order_acquire));

            size_.fetch_add(1, std::memory_order_relaxed);
            return iterator(new_node);
        }

        iterator insert_after(const_iterator position, const Ty &x) {
            return emplace_after(position, x);
        }

        iterator insert_after(const_iterator position, Ty &&x) {
            return emplace_after(position, utility::move(x));
        }

        iterator insert_after(const_iterator position, size_type n, const Ty &x) {
            iterator result = iterator(const_cast<node *>(position.node_ptr));
            for (size_type i = 0; i < n; ++i) {
                result = insert_after(const_iterator(result.node_ptr), x);
            }
            return result;
        }

        template <typename InputIter>
        iterator insert_after(const_iterator position, InputIter first, InputIter last) {
            iterator result = iterator(const_cast<node *>(position.node_ptr));
            for (auto it = first; it != last; ++it) {
                result = insert_after(const_iterator(result.node_ptr), *it);
            }
            return result;
        }

        iterator insert_after(const_iterator position, std::initializer_list<Ty> il) {
            return insert_after(position, il.begin(), il.end());
        }

        iterator erase_after(const_iterator position) noexcept {
            node *pos = const_cast<node *>(position.node_ptr);
            auto hp = foundation::memory::make_hazard_pointer();
            node *to_erase;

            do {
                to_erase = hp.protect(pos->next.load());
                if (!to_erase)
                    return iterator(nullptr);
            } while (!pos->next.compare_exchange_weak(to_erase, to_erase->next.load(std::memory_order_acquire),
                                                      std::memory_order_release, std::memory_order_acquire));

            hp.reset_protection();
            foundation::memory::hazard_pointer_domain<node>::global().retire(to_erase);
            size_.fetch_sub(1, std::memory_order_relaxed);
            return iterator(pos->next.load(std::memory_order_acquire));
        }

        iterator erase_after(const_iterator position, const_iterator last) noexcept {
            node *pos = const_cast<node *>(position.node_ptr);
            node *l = const_cast<node *>(last.node_ptr);

            node *to_erase = pos->next.load(std::memory_order_acquire);
            size_type count = 0;

            while (to_erase && to_erase != l) {
                node *next = to_erase->next.load(std::memory_order_acquire);
                // 使用自定义删除器
                foundation::memory::hazard_pointer_domain<node>::global().retire(to_erase);
                to_erase = next;
                ++count;
            }

            pos->next.store(l, std::memory_order_release);
            size_.fetch_sub(count, std::memory_order_relaxed);
            return iterator(l);
        }

        void resize(size_type sz) {
            size_type current = size_.load(std::memory_order_relaxed);
            if (sz < current) {
                auto it = before_begin();
                std::advance(it, static_cast<difference_type>(sz));
                erase_after(it, end());
            } else if (sz > current) {
                auto it = before_begin();
                std::advance(it, static_cast<difference_type>(current));
                for (size_type i = current; i < sz; ++i) {
                    it = emplace_after(const_iterator(it.node_ptr));
                }
            }
        }

        void resize(size_type sz, const value_type &c) {
            size_type current = size_.load(std::memory_order_relaxed);
            if (sz < current) {
                auto it = before_begin();
                std::advance(it, static_cast<difference_type>(sz));
                erase_after(it, end());
            } else if (sz > current) {
                auto it = before_begin();
                std::advance(it, static_cast<difference_type>(current));
                for (size_type i = current; i < sz; ++i) {
                    it = insert_after(const_iterator(it.node_ptr), c);
                }
            }
        }

        void clear() noexcept {
            node *curr = head_->next.exchange(nullptr, std::memory_order_acquire);
            while (curr) {
                node *next = curr->next.load(std::memory_order_acquire);
                // 使用自定义删除器
                foundation::memory::hazard_pointer_domain<node>::global().retire(curr);
                curr = next;
            }
            size_.store(0, std::memory_order_relaxed);
            foundation::memory::hazard_pointer_domain<node>::global().reclaim();
        }

        void splice_after(const_iterator position, forward_list &x) {
            splice_after(position, x, x.before_begin(), x.end());
        }

        void splice_after(const_iterator position, forward_list &&x) {
            splice_after(position, x);
        }

        void splice_after(const_iterator position, forward_list &x, const_iterator i) {
            const_iterator j = i;
            ++j;
            splice_after(position, x, i, j);
        }

        void splice_after(const_iterator position, forward_list &&x, const_iterator i) {
            splice_after(position, x, i);
        }

        void splice_after(const_iterator position, forward_list &x, const_iterator first, const_iterator last) {
            if (first == last) {
                return;
            }
            node *pos = const_cast<node *>(position.node_ptr);
            node *f = const_cast<node *>(first.node_ptr);
            node *l = const_cast<node *>(last.node_ptr);
            node *f_next = f->next.load(std::memory_order_acquire);
            if (f_next == l) {
                return;
            }
            // 找到最后一个节点（last 前的节点），同时用 CAS 防止并发修改
            node *before_last = f_next;
            while (true) {
                node *next = before_last->next.load(std::memory_order_acquire);
                if (next == l) {
                    break;
                }
                before_last = next;
            }
            // 将 f..before_last 从 x 中取出
            node *old_pos_next = nullptr;
            do {
                old_pos_next = pos->next.load(std::memory_order_acquire);
                before_last->next.store(old_pos_next, std::memory_order_release);
            } while (!pos->next.compare_exchange_weak(old_pos_next, f_next, std::memory_order_release, std::memory_order_relaxed));
            f->next.store(l, std::memory_order_release);
            // 更新大小
            size_type count = 0;
            node *it = f_next;
            while (it != l) {
                ++count;
                it = it->next.load(std::memory_order_acquire);
            }
            x.size_.fetch_sub(count, std::memory_order_relaxed);
            size_.fetch_add(count, std::memory_order_relaxed);
        }


        void splice_after(annotations::lifetime::in<const_iterator> position, forward_list &&x,
                          annotations::lifetime::in<const_iterator> first, annotations::lifetime::in<const_iterator> last) {
            splice_after(position, x, first, last);
        }

        size_type remove(const value_type &value) {
            return remove_if([&value](const value_type &x) { return x == value; });
        }

        template <typename Predicate>
        size_type remove_if(Predicate pred) {
            size_type count = 0;
            auto it = before_begin();
            while (true) {
                node *next_node = it.node_ptr->next.load(std::memory_order_acquire);
                if (!next_node) {
                    break;
                }
                if (pred(next_node->value)) {
                    if (erase_after(const_iterator(it.node_ptr)) != end()) {
                        ++count;
                    } else {
                        // 如果 erase_after 失败，重试同一位置
                        continue;
                    }
                } else {
                    it.node_ptr = next_node;
                }
            }
            return count;
        }

        size_type unique() {
            return unique(std::equal_to<Ty>());
        }

        template <typename BinaryPredicate>
        size_type unique(BinaryPredicate binary_pred) {
            if (empty()) {
                return 0;
            }
            size_type count = 0;
            auto it = begin();
            while (it.node_ptr->next.load(std::memory_order_acquire) != nullptr) {
                node *next_node = it.node_ptr->next.load(std::memory_order_acquire);
                if (binary_pred(it.node_ptr->value, next_node->value)) {
                    if (erase_after(const_iterator(it.node_ptr)) != end()) {
                        ++count;
                    } else {
                        continue; // 重试同一位置
                    }
                } else {
                    it.node_ptr = next_node;
                }
            }
            return count;
        }

        void merge(forward_list &x) {
            merge(x, foundation::functional::less<Ty>());
        }

        void merge(forward_list &&x) {
            merge(x);
        }

        template <typename Compare>
        void merge(forward_list &x, Compare comp) {
            if (this == &x || x.empty()) {
                return;
            }
            auto pos = before_begin();
            auto x_it = x.begin();
            while (x_it != x.end()) {
                node *pos_next = pos.node_ptr->next.load(std::memory_order_acquire);
                if (!pos_next) {
                    break;
                }
                if (comp(x_it.node_ptr->value, pos_next->value)) {
                    auto x_next = x_it;
                    ++x_next;
                    // splice_after 内部是无锁 CAS 安全版本
                    splice_after(const_iterator(pos.node_ptr), x, const_iterator(x_it.node_ptr), const_iterator(x_next.node_ptr));
                    x_it = x_next;
                } else {
                    ++pos;
                }
            }
            // 如果 x 还有剩余节点
            if (x_it != x.end()) {
                splice_after(const_iterator(pos.node_ptr), x, const_iterator(x_it.node_ptr), x.end());
            }
        }

        template <typename Compare>
        void merge(forward_list &&x, Compare comp) {
            merge(x, comp);
        }

        void reverse() noexcept {
            if (size_.load(std::memory_order_relaxed) <= 1) {
                return;
            }
            node *prev = nullptr;
            node *current = head_->next.load(std::memory_order_acquire);
            while (current) {
                node *next = current->next.load(std::memory_order_acquire);
                // CAS 循环确保安全修改
                while (!current->next.compare_exchange_weak(next, prev, std::memory_order_release, std::memory_order_relaxed)) {
                    // 如果修改失败，next 被其他线程改变，重新读取
                }
                prev = current;
                current = next;
            }
            // CAS 更新 head_->next
            node *old_head_next = nullptr;
            do {
                old_head_next = head_->next.load(std::memory_order_acquire);
            } while (!head_->next.compare_exchange_weak(old_head_next, prev, std::memory_order_release, std::memory_order_relaxed));
        }

        void swap(forward_list &right) noexcept {
            node *old_head = head_;
            head_ = right.head_;
            right.head_ = old_head;
            const size_type old_size = size_.load(std::memory_order_relaxed);
            size_.store(right.size_.load(std::memory_order_relaxed), std::memory_order_relaxed);
            right.size_.store(old_size, std::memory_order_relaxed);
        }

    private:
        template <typename... Args>
        node *allocate_node(Args &&...args) {
            node *p = new node(utility::forward<Args>(args)...);
            try {
            } catch (...) {
                allocator_.deallocate(p, 1);
                throw;
            }
            return p;
        }

        node *allocate_node_dummy() {
            node *p = new node;
            try {
                p->next.store(nullptr, std::memory_order_relaxed);
            } catch (...) {
                allocator_.deallocate(p, 1);
                throw;
            }
            return p;
        }

        static void deallocate_node_dummy(node *p) noexcept {
            if (!p) {
                return;
            }
            delete p;
        }

        node_allocator_type allocator_;
        node *head_;
        std::atomic<size_t> size_;
    };

    template <typename Ty, typename Allocator>
    void swap(forward_list<Ty, Allocator> &left, forward_list<Ty, Allocator> &right) noexcept {
        left.swap(right);
    }

    template <typename Ty, typename Allocator>
    bool operator==(const forward_list<Ty, Allocator> &x, const forward_list<Ty, Allocator> &y) {
        auto ix = x.begin();
        auto iy = y.begin();
        while (ix != x.end() && iy != y.end()) {
            if (!(*ix == *iy)) {
                return false;
            }
            ++ix;
            ++iy;
        }
        return ix == x.end() && iy == y.end();
    }

    template <typename Ty, typename Allocator>
    bool operator!=(const forward_list<Ty, Allocator> &x, const forward_list<Ty, Allocator> &y) {
        return !(x == y);
    }

    template <typename Ty, typename Allocator>
    bool operator<(const forward_list<Ty, Allocator> &x, const forward_list<Ty, Allocator> &y) {
        return std::lexicographical_compare(x.begin(), x.end(), y.begin(), y.end());
    }

    template <typename Ty, typename Allocator>
    bool operator>(const forward_list<Ty, Allocator> &x, const forward_list<Ty, Allocator> &y) {
        return y < x;
    }

    template <typename Ty, typename Allocator>
    bool operator<=(const forward_list<Ty, Allocator> &x, const forward_list<Ty, Allocator> &y) {
        return !(y < x);
    }

    template <typename Ty, typename Allocator>
    bool operator>=(const forward_list<Ty, Allocator> &x, const forward_list<Ty, Allocator> &y) {
        return !(x < y);
    }

}

#if RAINY_USING_MSVC
#pragma warning(pop)
#endif

#endif

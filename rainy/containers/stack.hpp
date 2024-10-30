#ifndef RAINY_STACK_HPP
#define RAINY_STACK_HPP
#include <rainy/base.hpp>
#include <deque>

namespace rainy::foundation::containers {
    template <typename Ty, typename container = std::deque<Ty>>
    class stack {
    public:
        using container_type = container;
        using value_type = Ty;
        using size_type = std::size_t;

        stack() = default;

        RAINY_NODISCARD bool empty() const {
            return container_.empty();
        }

        value_type &top() {
            return container_.back();
        }

        const value_type &top() const {
            return container_.back();
        }

        RAINY_NODISCARD size_type size() const {
            return container_.size();
        }

        void pop() {
            container_.pop_back();
        }

        void push(const value_type &vals) {
            container_.push_back(vals);
        }

        void push(value_type &&vals) {
            container_.emplace_back(std::move(vals));
        }

        template <typename... Args>
        void emplace(Args... args) {
            container_.emplace_back(std::forward<Args>(args)...);
        }

        void swap(stack &right) noexcept {
            this->container_.swap(right.container_);
        }

        void clear() noexcept {
            container_.clear();
        }

    private:
        container_type container_;
    };

    template <typename Ty, std::size_t N, typename array_container = array<Ty, N>>
    class solid_stack {
    public:
        static_assert(std::is_standard_layout_v<Ty> && std::is_trivial_v<Ty>, "solid_stack only support POD types");

        using container_type = array_container;
        using value_type = Ty;
        using size_type = std::size_t;

        solid_stack() = default;

        RAINY_NODISCARD bool is_full() const noexcept {
            return ptr == N;
        }

        RAINY_NODISCARD bool empty() const noexcept {
            return ptr == 0;
        }

        value_type &top() {
            if (ptr == 0) {
                utility::throw_exception(std::out_of_range("Stack is empty"));
            }
            return container_[ptr];
        }

        const value_type &top() const {
            if (ptr == 0) {
                utility::throw_exception(std::out_of_range("Stack is empty"));
            }
            return container_[ptr];
        }

        RAINY_NODISCARD size_type size() const {
            return ptr;
        }

        void pop() {
            if (ptr == 0) {
                utility::throw_exception(std::out_of_range("Stack is empty"));
            }
            --ptr;
        }

        void push(const value_type &vals) {
            range_check();
            container_[ptr] = vals;
            ++ptr;
        }

        void push(value_type &&vals) {
            range_check();
            container_[ptr] = std::move(vals);
            ++ptr;
        }

        void swap(solid_stack &right) noexcept {
            this->container_.swap(right.container_);
            std::swap(this->ptr, right.ptr);
        }

        void clear() noexcept {
            ptr = 0;
        }

    private:
        void range_check() const {
            if (is_full()) {
                foundation::system::exceptions::logic::throw_out_of_range("We can't push more elements");
            }
        }

        size_type ptr{0};
        container_type container_;
    };

    template <typename Ty, typename _alloc = system::memory::allocator<Ty>>
    class stack_container {
    public:
        using value_type = Ty;
        using size_type = std::size_t;
        using allocator_type = _alloc;
        using pointer = value_type *;
        using const_reference = const value_type &;

        constexpr stack_container() noexcept = default;

        RAINY_CONSTEXPR20 stack_container(stack_container &&right) noexcept :
            container_pair_(utility::exchange(right.container_pair_, {})) {
        }
        RAINY_CONSTEXPR20 stack_container(const stack_container &) = default;

        stack_container &operator=(stack_container &&right) noexcept {
            container_pair_ = utility::exchange(right.container_pair_, {});
            return *this;
        }

        stack_container &operator=(const stack_container &) = delete;


        ~stack_container() {
            auto &data = container_pair_.second;
            data.tidy();
        }

        RAINY_NODISCARD constexpr bool empty() const {
            return container_pair_.second.empty();
        }

        void push(const_reference value) {
            auto &data = container_pair_.second;
            if (empty()) {
                buy_zero_(5);
            } else {
                buy_non_zero_(capacity());
            }
            data.push(value);
        }

        void push(value_type &&value) {
            auto &data = container_pair_.second;
            if (empty()) {
                buy_zero_(5);
            } else if (size() + 1 >= capacity()) {
                buy_non_zero_(capacity());
            }
            data.push(value);
        }

        void pop() {
            auto &data = container_pair_.second;
            if (!empty()) {
                --data.last;
                std::destroy_at(data.last);
            }
            if (data.first == data.last) {
                data.tidy();
            }
        }

        const_reference top() const {
            auto &data = container_pair_.second;
            if (empty()) {
                utility::throw_exception(std::range_error("This Container is empty!"));
            }
            return data.top();
        }

        RAINY_NODISCARD size_type size() const noexcept {
            auto &data = container_pair_.second;
            return data.size();
        }

        RAINY_NODISCARD size_type capacity() const noexcept {
            auto &data = container_pair_.second;
            return data.capa_size();
        }

    private:
        struct internal_ {
            constexpr internal_() : first(nullptr), end(nullptr), last(nullptr){};

            void push(const_reference value) {
                ::new (last++) Ty(value);
            }

            void push(value_type &&right) {
                ::new (last++) Ty(std::move(right));
            }

            const_reference top() const {
                return *(last - 1);
            }

            RAINY_NODISCARD size_type size() const {
                return std::distance(first, last);
            }

            RAINY_NODISCARD size_type capa_size() const {
                return std::distance(first, end);
            }

            RAINY_NODISCARD bool empty() const {
                return first == nullptr;
            }

            void tidy() {
                if (first) {
                    allocator_type allocator{};
                    allocator.deallocate(first, capa_size());
                    first = end = last = nullptr;
                }
            }

            pointer first;
            pointer end;
            pointer last;
        };

        static size_type calculate_new_size_(const size_type new_size) {
            // 当 new_size
            // 大于20时，我们将new_size加上new_size的一半，否则检查new_size是否大于4，若大于则将new_size加上2，否则加上6
            return new_size > 20 ? new_size + new_size / 2 : new_size > 4 ? new_size + 2 : new_size + 6;
        }

        void buy_zero_(const size_type alloc_size) {
            auto &allocator = container_pair_.get_first();
            auto &data = container_pair_.second;
            if (empty()) {
                try {
                    data.first = allocator.allocate(alloc_size);
                    data.last = data.first;
                    data.end = data.first + alloc_size;
                } catch (const std::bad_alloc &except) {
                    (void) std::fwrite(except.what(), 1, std::strlen(except.what()), stderr);
                }
            } else {
                buy_non_zero_(alloc_size);
            }
        }

        void buy_non_zero_(const size_type alloc_size) {
            auto &allocator = container_pair_.get_first();
            auto &data = container_pair_.second;
            pointer old_first = data.first;
            pointer old_last = data.last;
            size_type old_size = data.capa_size();
            try {
                size_type new_size = calculate_new_size_(alloc_size);
                data.first = allocator.allocate(new_size);
                data.end = data.first + new_size;
                data.last = std::uninitialized_move(old_first, old_last, data.first);
                allocator.deallocate(old_first, old_size);
            } catch (const std::bad_alloc &except) {
                (void) std::fwrite(except.what(), 1, std::strlen(except.what()), stderr);
            }
        }

        utility::compressed_pair<allocator_type, internal_> container_pair_;
    };
}

#endif

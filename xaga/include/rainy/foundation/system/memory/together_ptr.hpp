#ifndef RAINY_TOGETHER_PTR
#define RAINY_TOGETHER_PTR
#include <rainy/base.hpp>

namespace rainy::foundation::system::memory::implements {
    template <typename Type, typename Dx>
    class ctrl_block {
    public:
        using pointer = Type *;
        using const_pointer = const Type *;
        using deleter_type = Dx;

        ctrl_block() noexcept = default;

        ctrl_block(std::nullptr_t) : resources({}, nullptr) {
        }

        ctrl_block(deleter_type deleter) : resources(deleter, nullptr) {
        }

        ctrl_block(std::nullptr_t, deleter_type deleter) : resources(deleter, nullptr) {
        }

        template <typename Uty,
                  type_traits::other_trans::enable_if_t<type_traits::type_relations::is_convertible_v<Uty *, pointer>, int> = 0>
        ctrl_block(Uty *const ptr) : resources({}, nullptr) {
            if (ptr) {
                construct(ptr, deleter_type{}, allocator<Type>{});
            }
        }

        template <typename Uty,
                  type_traits::other_trans::enable_if_t<type_traits::type_relations::is_convertible_v<Uty *, pointer>, int> = 0>
        ctrl_block(Uty *const ptr, deleter_type deleter) {
            if (ptr) {
                construct(ptr, deleter, allocator<Type>{});
            }
        }

        ctrl_block(const ctrl_block &right) : resources(right.resources) {
            if (right.hold_ownership()) {
                // 先确保right是具有所有权的
                incr_use_count();
            }
        }

        ctrl_block(ctrl_block &&right) noexcept : resources(utility::exchange(right.resources, {})) {
        }

        ctrl_block &operator=(const ctrl_block &right) {
            if (this == utility::addressof(right)) {
                return *this;
            }
            if (right.hold_ownership()) {
                if (this->hold_ownership()) {
                    try_release_if_used_eq_0();
                }
                this->resources = right.resources;
                incr_use_count();
            }
            return *this;
        }

        ctrl_block &operator=(ctrl_block &&right) noexcept {
            if (this == utility::addressof(right)) {
                return *this;
            }
            if (this->hold_ownership()) {
                try_release_if_used_eq_0();
            }
            this->resources = utility::exchange(right.resources, {});
            return *this;
        }

        ~ctrl_block() {
            try_release_if_used_eq_0();
        }

        bool hold_ownership() const noexcept {
            const res *block = resources.get_second();
            return block != nullptr; // 将检查持有的块是否为空指针，若为空指针，意味着它没有持有任何一个资源的所有权
        }

        void relinquish() {
            try_release_if_used_eq_0();
        }

        long use_count() const noexcept {
            const res *block = resources.get_second();
            return hold_ownership() ? block->use_count : 0;
        }

        pointer get() noexcept {
            res *block = resources.get_second();
            return hold_ownership() ? block->ptr : nullptr;
        }

        const_pointer get() const noexcept {
            const res *block = resources.get_second();
            return hold_ownership() ? block->ptr : nullptr;
        }

        void swap(ctrl_block &right) noexcept {
            this->resources.swap(right.resources);
        }

    private:
        struct res {
            res(pointer ptr) noexcept : ptr(ptr) {
            }

            res(const res &) = delete; // 一切通过指针间接引用块

            pointer ptr{nullptr};
            long use_count{0};
        };

        template <typename PointerOrNullptrType, typename Deleter, typename Alloc>
        void construct(const PointerOrNullptrType ptr, Deleter deleter, Alloc alloc) {
            using rebind_alloc_t = typename Alloc::template rebind_alloc<res>;
            rebind_alloc_t rebind_alloc{};
            res *block = rebind_alloc.allocate(1);
            resources.get_first() = deleter;
            resources.get_second() = utility::construct_at(block, ptr);
        }

        void try_release_if_used_eq_0() {
            using rebind_alloc_t = typename allocator<Type>::template rebind_alloc<res>;
            if (!hold_ownership()) {
                // 没有所有权意味着什么？意味着我们不应当对其操作
                return;
            }
            res *block = resources.get_second();
            if (decr_use_count() == 0) {
                auto &deleter = resources.get_first();
                deleter(block->ptr);
                {
                    rebind_alloc_t rebind_alloc{};
                    rebind_alloc.deallocate(block, 1);
                }
                resources.get_second() = nullptr;
            }
        }

        long incr_use_count() noexcept {
            if (hold_ownership()) {
                res *block = resources.get_second();
                return core::pal::interlocked_increment(static_cast<volatile long *>(&block->use_count));
            }
            return 0;
        }

        long decr_use_count() noexcept {
            if (hold_ownership()) {
                res *block = resources.get_second();
                return core::pal::interlocked_decrement(static_cast<volatile long *>(&block->use_count));
            }
            return 0;
        }

        utility::compressed_pair<Dx, res *> resources{};
    };
}

namespace rainy::foundation::system::memory {
    template <typename Ty>
    class together_ptr;

    template <typename Ty>
    class weak_ptr;
}


namespace rainy::foundation::system::memory::implements {

    struct atomic_ref_count_base {
        atomic_ref_count_base(const atomic_ref_count_base &) = delete;
        atomic_ref_count_base &operator=(const atomic_ref_count_base &) = delete;

        virtual ~atomic_ref_count_base() noexcept {
        }

        bool incref_if_not_zero() noexcept {
            rainy_ref volatile_uses = reinterpret_cast<volatile long &>(uses);
            long count = core::pal::iso_volatile_load32(reinterpret_cast<volatile int *>(&volatile_uses));
            while (count != 0) {
            }
        }


        unsigned long uses = 1;
        unsigned long weaks = 1;
    };
}

#endif //RAINY_TOGETHER_PTR_HPP

#ifndef RAINY_BASE_HPP
#define RAINY_BASE_HPP
/*
文件名: base.hpp
此头文件用于存放基础设施实现，提供对C语言的部分函数封装以及少量模块
*/
#include <rainy/core.hpp>
#include <rainy/diagnostics/source_location.hpp>
#include <rainy/system/basic_exceptions.hpp>
#include <rainy/text/format_wrapper.hpp>
#include <rainy/functional/function_pointer.hpp>
#include <rainy/diagnostics/contract.hpp>

/* standard-libray header */
#include <algorithm>
#include <array>
#include <bitset>
#include <cerrno>
#include <cstdarg>
#include <cstring>
#include <deque>
#include <functional>
#include <future>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <shared_mutex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#if RAINY_HAS_CXX20
#include <format>
#include <source_location>
#endif

#if RAINY_USING_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <sstream>
#include <windows.h>
#endif

namespace rainy::information {
    static const std::size_t max_threads = std::thread::hardware_concurrency();
}

#if RAINY_HAS_CXX20
namespace rainy::foundation::type_traits::concepts {
    template <typename Ty1, typename Ty2>
    concept same_as = internals::_is_same_v<Ty1, Ty2>;

    template <typename base, typename derived>
    concept derived_from = __is_base_of(base, derived) && internals::_is_convertible_v<const volatile derived *, const volatile base *>;

    template <typename Ty, typename Context = std::format_context,
              typename Formatter = Context::template formatter_type<std::remove_const_t<Ty>>>
    concept formattable_with =
        std::semiregular<Formatter> && requires(Formatter &formatter, const Formatter &const_formatter, Ty &&type, Context format_context,
                                                std::basic_format_parse_context<typename Context::char_type> parse_context) {
            { formatter.parse(parse_context) } -> same_as<typename decltype(parse_context)::iterator>;
            { const_formatter.format(type, format_context) } -> same_as<typename Context::iterator>;
        };

    template <typename Ty, typename Context = std::format_context,
              typename Formatter = Context::template formatter_type<std::remove_const_t<Ty>>>
    concept formattable_with_non_const =
        std::semiregular<Formatter> && requires(Formatter &formatter, Ty &&type, Context format_context,
                                                std::basic_format_parse_context<typename Context::char_type> parse_context) {
            { formatter.parse(format_context) } -> std::same_as<typename decltype(parse_context)::iterator>;
            { formatter.format(type, format_context) } -> std::same_as<typename Context::iterator>;
        };
}
#endif

namespace rainy::component {
    template <typename Class>
    class object {
    public:
        using object_type = Class;
        using super_type = object<Class>;

        static_assert(std::is_class_v<object_type> && !std::is_array_v<object_type>);

        RAINY_NODISCARD const std::type_info &type_info() const noexcept {
            return typeid(object_type);
        }

        RAINY_NODISCARD std::size_t hash_code() const noexcept {
            rainy_let object_address = reinterpret_cast<const unsigned char *>(&object_dummy_unused);
            return utility::internals::fnv1a_append_bytes(utility::internals::fnv_offset_basis, object_address,
                                                          sizeof(object<object_type>));
        }

        RAINY_NODISCARD const void *addressof() const noexcept {
            return static_cast<const void *>(this);
        }

        RAINY_NODISCARD virtual const super_type &as_super() const noexcept {
            return *this;
        }

        RAINY_NODISCARD virtual std::shared_ptr<object_type> clone() const noexcept(std::is_nothrow_copy_constructible_v<object_type>) {
            if (std::is_copy_constructible_v<object_type>) {
                return std::shared_ptr<object_type>(new object_type(*static_cast<const object_type *>(this)));
            }
            foundation::system::exceptions::runtime::throw_runtime_error("We can't create a copy from this class");
            return nullptr;
        }

    protected:
        constexpr object() noexcept = default;
        virtual RAINY_CONSTEXPR20 ~object() = default;

    private:
        union {
            std::max_align_t object_dummy_unused{};
        };
    };
}

namespace rainy::utility {
    namespace internals {
        using type_name_prober = void;

        constexpr std::string_view type_name_prober_ = "void";

        template <typename Ty>
        constexpr std::string_view wrapped_type_name() {
            constexpr bool always_false = rainy::foundation::type_traits::internals::always_false<Ty>;
            static_assert(!always_false);
#if RAINY_USING_CLANG || RAINY_USING_GCC
            return __PRETTY_FUNCTION__;
#elif RAINY_USING_MSVC
            return __FUNCSIG__;
#else
            static_assert(always_false, "unsuporrted complier");
#endif
        }

        static constexpr std::size_t wrapped_type_name_prefix_length() {
            return wrapped_type_name<type_name_prober>().find(type_name_prober_);
        }

        static constexpr std::size_t wrapped_type_name_suffix_length() {
            return wrapped_type_name<type_name_prober>().length() - wrapped_type_name_prefix_length() - type_name_prober_.length();
        }

        template <auto Variable>
        constexpr std::string_view wrapped_variable_name() {
#if RAINY_USING_CLANG || RAINY_USING_GCC
            return __PRETTY_FUNCTION__;
#elif RAINY_USING_MSVC
            return __FUNCSIG__;
#else
            static_assert(false, "unsupported compiler");
#endif
        }
    }

    template <typename Ty>
    constexpr std::string_view type_name() {
        constexpr auto wrapped_name = internals::wrapped_type_name<Ty>();
        constexpr auto prefix_length = internals::wrapped_type_name_prefix_length();
        constexpr auto suffix_length = internals::wrapped_type_name_suffix_length();
        constexpr auto type_name_length = wrapped_name.length() - prefix_length - suffix_length;
        return wrapped_name.substr(prefix_length, type_name_length);
    }

    template <auto Variable>
    constexpr std::string_view variable_name() {
        constexpr std::string_view wrapped_name = internals::wrapped_variable_name<Variable>();
        constexpr std::string_view prober_prefix = internals::wrapped_variable_name<0>();
        constexpr auto prefix_length = prober_prefix.find('0');
        constexpr std::string_view remove_prefix = wrapped_name.substr(prefix_length);
#if RAINY_USING_CLANG
        constexpr auto suffix_length = remove_prefix.find("]");
#elif RAINY_USING_GCC
        constexpr auto suffix_length = remove_prefix.rfind(';');
#elif RAINY_USING_MSVC
        constexpr auto suffix_length = remove_prefix.rfind(">(void)");
#else
        static_assert(false, "unsupported compiler");
#endif
        return remove_prefix.substr(0, suffix_length);
    }

    enum class type_flags {
        is_fundamental,
        is_integral,
        is_floating_point,
        is_trivially_copyable,
        size
    };

    class type_info final {
    public:
        template <typename Ty>
        static type_info create() noexcept {
            type_info info{};

            info._name = type_name<Ty>();
            info._size = sizeof(Ty);
            info._align = alignof(Ty);
            info.flags[static_cast<int>(type_flags::is_trivially_copyable)] = std::is_trivially_copyable_v<Ty>;
            info.flags[static_cast<int>(type_flags::is_integral)] = std::is_integral_v<Ty>;
            info.flags[static_cast<int>(type_flags::is_floating_point)] = std::is_floating_point_v<Ty>;
            info.flags[static_cast<int>(type_flags::is_fundamental)] = std::is_fundamental_v<Ty>;
            info._hash_code = typeid(Ty).hash_code();
            return info;
        }

        RAINY_NODISCARD constexpr uint64_t hash_code() const noexcept {
            return _hash_code;
        }

        RAINY_NODISCARD std::string_view name() const noexcept {
            return _name;
        }

        RAINY_NODISCARD uint32_t size() const noexcept {
            return _size;
        }

        RAINY_NODISCARD uint32_t alignment() const noexcept {
            return _align;
        }

        RAINY_NODISCARD bool is_fundamental() const noexcept {
            return flags[static_cast<int>(type_flags::is_fundamental)];
        }

        RAINY_NODISCARD bool is_integral() const noexcept {
            return flags[static_cast<int>(type_flags::is_integral)];
        }

        RAINY_NODISCARD bool is_floating_point() const noexcept {
            return flags[static_cast<int>(type_flags::is_floating_point)];
        }

        RAINY_NODISCARD bool is_trivially_copyable() const noexcept {
            return flags[static_cast<int>(type_flags::is_trivially_copyable)];
        }

        RAINY_NODISCARD std::string to_string() const {
            std::string buffer;
            buffer.reserve(64);
#if RAINY_USING_CXX20
            utility::format(buffer, "type_name={} : size={},align={},hash_code={}", _name.data(), _size, _align, _hash_code);
#else
            buffer += "typename=";
            buffer += _name;
            std::string temp;
            utility::cstyle_format(temp, " : size=%d,align=%d,hash_code=%d", _size, _align, _hash_code);
            buffer += temp;
#endif
            return buffer;
        }

    private:
        std::string_view _name;
        uint32_t _size{};
        uint32_t _align{};
        std::size_t _hash_code{};
        std::bitset<static_cast<int>(type_flags::size)> flags;
    };
}

#if RAINY_HAS_CXX20
template <>
class std::formatter<rainy::utility::type_info, char> {
public:
    explicit formatter() noexcept = default;

    static auto parse(format_parse_context &ctx) noexcept {
        return ctx.begin();
    }

    static auto format(const rainy::utility::type_info &value, std::format_context fc) noexcept {
        return std::format_to(fc.out(), "{}", value.to_string());
    }
};
#endif

namespace rainy::foundation::system::memory {
    enum class allocation_method {
        std_allocator,
        cstd_allocator,
        RAINY_allocator,
        RAINY_allocator_no_check
    };

    template <typename Ptr, typename SizeType = std::size_t>
    struct allocation_result {
        Ptr ptr;
        SizeType count;
    };

    template <typename Ty, allocation_method Method = allocation_method::RAINY_allocator>
    class allocator final {
    public:
        using value_type = Ty;
        using pointer = value_type *;
        using size_type = std::size_t;
        using reference = value_type &;
        using const_reference = const value_type &;

        static_assert(!type_traits::internals::_is_reference_v<value_type>);

        static constexpr std::size_t align = alignof(value_type);
        static constexpr std::size_t element_size = sizeof(value_type);

        RAINY_CONSTEXPR20 allocator() noexcept = default;

        RAINY_CONSTEXPR20 allocator(const allocator &) noexcept = default;

        template <typename U>
        RAINY_CONSTEXPR20 explicit allocator(const allocator<U, Method> &) noexcept {
        }

        RAINY_CONSTEXPR20 ~allocator() = default;

        constexpr allocator(allocator &&) noexcept = default;
        constexpr allocator &operator=(const allocator &) noexcept = default;
        constexpr allocator &operator=(allocator &&) noexcept = default;

        constexpr allocator &operator=(const std::allocator<value_type> &) noexcept {
            return *this;
        }

        constexpr allocator &operator=(std::allocator<value_type> &&) noexcept {
            return *this;
        }

        RAINY_NODISCARD_RAW_PTR_ALLOC RAINY_CONSTEXPR20 pointer allocate(const size_type count) const {
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                return std::allocator<value_type>{}.allocate(count);
            }
#endif
            using exceptions::runtime::throw_bad_alloc;
            if constexpr (Method == allocation_method::std_allocator) {
                try {
                    rainy_let ret = std::allocator<value_type>{}.allocate(count);
                    return ret;
                } catch (std::bad_alloc &) {
                    throw_bad_alloc();
                }
            } else if constexpr (Method == allocation_method::cstd_allocator) {
                rainy_let allocated_memory = static_cast<pointer>(std::malloc(element_size * count));
                if (!allocated_memory) {
                    throw_bad_alloc();
                }
                return allocated_memory;
            } else if constexpr (Method == allocation_method::RAINY_allocator_no_check) {
                rainy_let allocated_memory =
                    static_cast<value_type *>(information::system_call::allocate(element_size * count, align));
                if (!allocated_memory) {
                    throw_bad_alloc();
                }
                return allocated_memory;
            }
            // 使用rainy's toolkit的内存管理函数
            rainy_let allocated_memory =
                static_cast<value_type *>(information::system_call::aligned_malloc_withcheck(element_size * count, align));
            if (!allocated_memory) {
                throw_bad_alloc();
            }
            return allocated_memory;
        }

        RAINY_CONSTEXPR20 void deallocate(value_type *block, const size_type count) const {
            if (!block || count == 0) {
                return;
            }
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                std::allocator<value_type>{}.deallocate(block, count);
            } else
#endif
            {
                if constexpr (Method == allocation_method::std_allocator) {
                    std::allocator<value_type>{}.deallocate(block, count);
                } else if constexpr (Method == allocation_method::cstd_allocator) {
                    std::free(block);
                } else if constexpr (Method == allocation_method::RAINY_allocator_no_check) {
                    information::system_call::deallocate(block, element_size * count, align);
                } else {
                    // 使用rainy's toolkit的内存管理函数
                    information::system_call::aligned_free_withcheck(block, align);
                }
            }
        }

        RAINY_NODISCARD_RAW_PTR_ALLOC RAINY_CONSTEXPR20 allocation_result<pointer> allocate_at_least(
            const size_type count) const {
            return {allocate(count), count};
        }

        constexpr static allocation_method current_method() noexcept {
            return Method;
        }

        template <typename... Args>
        RAINY_CONSTEXPR20 void construct(value_type *const ptr, Args &&...args) const
            noexcept(std::is_nothrow_constructible_v<value_type, Args...>) {
            utility::construct_at(ptr, utility::forward<Args>(args)...);
        }

        RAINY_CONSTEXPR20 void destory(value_type *const ptr) const noexcept(std::is_nothrow_destructible_v<value_type>) {
            ptr->~value_type();
        }
    };

    namespace pmr {
        class memory_resource : public std::pmr::memory_resource {
        public:
            using std::pmr::memory_resource::memory_resource;

            /*
            rainy's toolkit将在std::pmr::memory_resource的基础上拓展一个current_method以表示对应的分配器策略
            */

            RAINY_NODISCARD virtual allocation_method current_method() const noexcept = 0;
        };

        class cstd_memory_resource : public memory_resource {
        public:
            RAINY_NODISCARD allocation_method current_method() const noexcept override {
                return allocation_method::cstd_allocator;
            }

            static memory_resource *instance() {
                static cstd_memory_resource instance;
                return &instance;
            }

        private:
            void *do_allocate(std::size_t bytes, std::size_t) override {
                void *ptr = std::malloc(bytes);
                if (!ptr) {
                    exceptions::runtime::throw_bad_alloc();
                }
                return ptr;
            }

            void do_deallocate(void *block, std::size_t bytes, std::size_t alignment) override {
                if (bytes == 0 || !information::internals::is_pow_2(alignment) || !block) {
                    return;
                }
                std::free(block);
            }

            RAINY_NODISCARD bool do_is_equal(const std::pmr::memory_resource &right) const noexcept override {
                return this == utility::addressof(right);
            }
        };

        class std_memory_resource final : public memory_resource {
        public:
            RAINY_NODISCARD allocation_method current_method() const noexcept override {
                return allocation_method::std_allocator;
            }

            static memory_resource *instance() {
                static std_memory_resource instance;
                return &instance;
            }

        private:
            void *do_allocate(std::size_t bytes, std::size_t alignment) override {
#ifdef __cpp_aligned_new
                return ::operator new[](bytes, std::align_val_t{alignment});
#else
                return information::system_call::allocate(bytes, alignment);
#endif
            }

            void do_deallocate(void *block, std::size_t bytes, std::size_t alignment) override {
                if (bytes == 0 || !information::internals::is_pow_2(alignment) || !block) {
                    return;
                }
#ifdef __cpp_aligned_new
                ::operator delete[](block, bytes, std::align_val_t{alignment});
#else
                information::system_call::deallocate(block, bytes, alignment);
#endif
            }

            RAINY_NODISCARD bool do_is_equal(const std::pmr::memory_resource &right) const noexcept override {
                return this == utility::addressof(right);
            }
        };

        class RAINY_memory_resource final : public memory_resource {
        public:
            RAINY_NODISCARD allocation_method current_method() const noexcept override {
                return allocation_method::RAINY_allocator;
            }

            static memory_resource *instance() {
                static RAINY_memory_resource instance;
                return &instance;
            }

        private:
            void *do_allocate(std::size_t bytes, std::size_t alignment) override {
                if (bytes == 0 || !information::internals::is_pow_2(alignment)) {
                    return nullptr;
                }
                return information::system_call::aligned_malloc_withcheck(bytes, alignment);
            }

            void do_deallocate(void *block, std::size_t bytes, std::size_t alignment) override {
                if (bytes == 0 || !information::internals::is_pow_2(alignment) || !block) {
                    return;
                }
                information::system_call::aligned_free_withcheck(block, alignment);
            }

            RAINY_NODISCARD bool do_is_equal(const std::pmr::memory_resource &right) const noexcept override {
                return this == utility::addressof(right);
            }
        };

        class RAINY_nocheck_memory_resource final : public memory_resource {
        public:
            RAINY_NODISCARD allocation_method current_method() const noexcept override {
                return allocation_method::RAINY_allocator_no_check;
            }

            static memory_resource *instance() {
                static RAINY_nocheck_memory_resource instance;
                return &instance;
            }

        private:
            void *do_allocate(std::size_t bytes, std::size_t alignment) override {
                if (bytes == 0 || !information::internals::is_pow_2(alignment)) {
                    return nullptr;
                }
                return information::system_call::allocate(bytes, alignment);
            }

            void do_deallocate(void *block, std::size_t bytes, std::size_t alignment) override {
                if (bytes == 0 || !information::internals::is_pow_2(alignment) || !block) {
                    return;
                }
                information::system_call::deallocate(block, bytes, alignment);
            }

            RAINY_NODISCARD bool do_is_equal(const std::pmr::memory_resource &right) const noexcept override {
                return this == utility::addressof(right);
            }
        };

        RAINY_INLINE memory_resource *get_memory_resource(allocation_method method) noexcept {
            switch (method) {
                case allocation_method::std_allocator:
                    return std_memory_resource::instance();
                case allocation_method::cstd_allocator:
                    return cstd_memory_resource::instance();
                case allocation_method::RAINY_allocator:
                    return RAINY_memory_resource::instance();
                case allocation_method::RAINY_allocator_no_check:
                    return RAINY_nocheck_memory_resource::instance();
                default:
                    break;
            }
            return nullptr;
        }


#if RAINY_HAS_CXX20
        template <typename Ty = information::byte_t, allocation_method Method = allocation_method::std_allocator>
#else
        template <typename Ty, allocation_method Method = allocation_method::std_allocator>
#endif
        class polymorphic_allocator {
        public:
            using value_type = Ty;
            static constexpr std::size_t align = alignof(value_type);
            static constexpr std::size_t element_size = sizeof(value_type);

            polymorphic_allocator() noexcept = default;

            polymorphic_allocator(memory_resource *const resource) noexcept : _resource{resource} {
                utility::expects(static_cast<bool>(resource), "Cannot initialize polymorphic_allocator with null resource");
            }

            polymorphic_allocator(const polymorphic_allocator &) = default;

            template <typename Uty>
            polymorphic_allocator(const polymorphic_allocator<Uty> &that) noexcept : _resource{that._resource} {
            }

            polymorphic_allocator &operator=(const polymorphic_allocator &) = delete;

            RAINY_NODISCARD_RAW_PTR_ALLOC value_type *allocate(const size_t count) {
                void *const resource = _resource->allocate(information::internals::get_size_of_n<Ty>(count), align);
                return static_cast<value_type *>(resource);
            }

            void deallocate(Ty *const ptr, const size_t count) noexcept {
                _resource->deallocate(ptr, information::internals::get_size_of_n<Ty>(count), align);
            }

            RAINY_NODISCARD_RAW_PTR_ALLOC void *allocate_bytes(const size_t bytes,
                                                                  const size_t align_ = alignof(std::max_align_t)) {
                return _resource->allocate(bytes, align_);
            }

            void deallocate_bytes(void *const ptr, const size_t bytes, const size_t align_ = alignof(std::max_align_t)) noexcept {
                _resource->deallocate(ptr, bytes, align_);
            }

            RAINY_NODISCARD_RAW_PTR_ALLOC value_type *allocate_object(const size_t count = 1) {
                void *const resource = allocate_bytes(information::internals::get_size_of_n<value_type>(count), align);
                return static_cast<value_type *>(resource);
            }

            void deallocate_object(value_type *const ptr, const size_t count = 1) noexcept {
                deallocate_bytes(ptr, information::internals::get_size_of_n<value_type>(count), align);
            }

            template <typename... Args>
            RAINY_NODISCARD_RAW_PTR_ALLOC value_type *new_object(Args &&...args) {
                value_type *const ptr = allocate_object();
                struct deallocate_bytes_guard {
                    ~deallocate_bytes_guard() noexcept {
                        if (mem_res) {
                            mem_res->deallocate(pointer, element_size, align);
                        }
                    }

                    deallocate_bytes_guard &operator=(const deallocate_bytes_guard &) = delete;
                    deallocate_bytes_guard &operator=(deallocate_bytes_guard &&) = delete;

                    memory_resource *mem_res;
                    void *pointer;
                };

                deallocate_bytes_guard guard{_resource, ptr};
                construct(ptr, utility::forward<Args>(args)...);
                guard.mem_res = nullptr;
                return ptr;
            }

            void delete_object(value_type *const ptr) noexcept {
                ptr->~value_type();
                deallocate_object(ptr);
            }

            template <typename... Args>
            void construct(value_type *const ptr, Args &&...args) {
                utility::construct_at(ptr, utility::forward<Args>(args)...);
            }

            void destroy(value_type *const ptr) noexcept {
                ptr->~value_type();
            }

            RAINY_NODISCARD polymorphic_allocator select_on_container_copy_construction() const noexcept {
                return {};
            }

            RAINY_NODISCARD memory_resource *resource() const noexcept {
                return _resource;
            }

            RAINY_NODISCARD_FRIEND bool operator==(const polymorphic_allocator &left, const polymorphic_allocator &right) noexcept {
                return *left._resource == *right.resource;
            }

        private:
            memory_resource *_resource = get_memory_resource(Method);
        };

        /* 为STL容器提供具有rainy's toolkit的polymorphic_allocator分配器别名 */
        namespace stl {
            template <typename Elem, typename Traits = std::char_traits<Elem>>
            using basic_string = std::basic_string<Elem, Traits, polymorphic_allocator<Elem>>;

            using string = basic_string<char>;
#ifdef __cpp_lib_char8_t
            using u8string = basic_string<char8_t>;
#endif
            using u16string = basic_string<char16_t>;
            using u32string = basic_string<char32_t>;
            using wstring = basic_string<wchar_t>;

            template <typename Ty>
            using vector = std::vector<Ty, polymorphic_allocator<Ty>>;

            template <typename Ty>
            using deque = std::deque<Ty, polymorphic_allocator<Ty>>;

            template <typename Ty>
            using list = std::list<Ty, polymorphic_allocator<Ty>>;

            template <typename Key, typename Ty, typename Pred = std::less<Key>>
            using map = std::map<Key, Ty, Pred, polymorphic_allocator<Ty>>;

            template <typename Key, typename Ty, typename Hasher = std::hash<Key>, typename KeyEqual = std::equal_to<Key>>
            using unordered_map = std::unordered_map<Key, Ty, Hasher, KeyEqual, polymorphic_allocator<Ty>>;

            template <typename Key, typename Ty, typename Hasher = std::hash<Key>, typename KeyEqual = std::equal_to<Key>>
            using unordered_multimap = std::unordered_multimap<Key, Ty, Hasher, KeyEqual, polymorphic_allocator<Ty>>;

            template <typename Ty, typename Hasher = std::hash<Ty>, typename KeyEqual = std::equal_to<Ty>>
            using unordered_set = std::unordered_set<Ty, Hasher, KeyEqual, polymorphic_allocator<Ty>>;
        }
    }
}

namespace rainy::utility {
    template <typename Ty1, typename Ty2>
    class compressed_pair;

    namespace internals {
        template <typename Ty, bool = std::is_final_v<Ty>>
        struct compressed_pair_empty : std::false_type {};

        template <typename Ty>
        struct compressed_pair_empty<Ty, false> : std::is_empty<Ty> {};

        template <typename Ty1, typename Ty2, bool is_same, bool first_empty, bool second_empty>
        struct compressed_pair_switch;

        template <typename Ty1, typename Ty2>
        struct compressed_pair_switch<Ty1, Ty2, false, false, false> {
            RAINY_CONSTEXPR static int value = 0;
        };

        template <typename Ty1, typename Ty2>
        struct compressed_pair_switch<Ty1, Ty2, false, true, true> {
            RAINY_CONSTEXPR static int value = 3;
        };

        template <typename Ty1, typename Ty2>
        struct compressed_pair_switch<Ty1, Ty2, false, true, false> {
            RAINY_CONSTEXPR static int value = 1;
        };

        template <typename Ty1, typename Ty2>
        struct compressed_pair_switch<Ty1, Ty2, false, false, true> {
            RAINY_CONSTEXPR static int value = 2;
        };

        template <typename Ty1, typename Ty2>
        struct compressed_pair_switch<Ty1, Ty2, true, true, true> {
            RAINY_CONSTEXPR static int value = 4;
        };

        template <typename Ty1, typename Ty2>
        struct compressed_pair_switch<Ty1, Ty2, true, false, false> {
            RAINY_CONSTEXPR static int value = 5;
        };

        template <typename Ty1, typename Ty2, int Version>
        class compressed_pair_impl;

        template <class Ty1, typename Ty2>
        class compressed_pair_impl<Ty1, Ty2, 0> {
        public:
            using first_type = Ty1;
            using second_type = Ty2;
            using first_param_type = typename foundation::type_traits::internals::_call_traits<first_type>::param_type;
            using second_param_type = typename foundation::type_traits::internals::_call_traits<second_type>::param_type;
            using first_reference = typename foundation::type_traits::internals::_call_traits<first_type>::reference;
            using second_reference = typename foundation::type_traits::internals::_call_traits<second_type>::reference;
            using first_const_reference = typename foundation::type_traits::internals::_call_traits<first_type>::const_reference;
            using second_const_reference = typename foundation::type_traits::internals::_call_traits<second_type>::const_reference;

            constexpr compressed_pair_impl() = default;

            constexpr compressed_pair_impl(first_param_type x, second_param_type y) : first(x), second(y) {
            }

            constexpr explicit compressed_pair_impl(first_param_type x) : first(x) {
            }

            constexpr explicit compressed_pair_impl(second_param_type y) : second(y) {
            }

            constexpr first_reference get_first() {
                return first;
            }
            
            constexpr first_const_reference get_first() const {
                return first;
            }

            constexpr second_reference get_second() {
                return second;
            }

            constexpr second_const_reference get_second() const {
                return second;
            }

            constexpr void swap(compressed_pair<Ty1, Ty2> &pair) {
                std::swap(first, pair.second);
                std::swap(second, pair.second);
            }

            first_type first;
            second_type second;
        };

        template <typename Ty1, typename Ty2>
        class compressed_pair_impl<Ty1, Ty2, 1> : protected foundation::type_traits::internals::remove_cv_t<Ty1> {
        public:
            using first_type = Ty1;
            using second_type = Ty2;
            using first_param_type = typename foundation::type_traits::internals::_call_traits<first_type>::param_type;
            using second_param_type = typename foundation::type_traits::internals::_call_traits<second_type>::param_type;
            using first_reference = typename foundation::type_traits::internals::_call_traits<first_type>::reference;
            using second_reference = typename foundation::type_traits::internals::_call_traits<second_type>::reference;
            using first_const_reference = typename foundation::type_traits::internals::_call_traits<first_type>::const_reference;
            using second_const_reference = typename foundation::type_traits::internals::_call_traits<second_type>::const_reference;

            constexpr compressed_pair_impl() = default;

            constexpr explicit compressed_pair_impl(first_param_type first, second_param_type second) : first_type(first), second(second) {
            }

            constexpr explicit compressed_pair_impl(first_param_type first) : first_type(first) {
            }

            explicit compressed_pair_impl(second_param_type second) : second(second) {
            }

            constexpr first_reference get_first() {
                return *this;
            }

            constexpr first_const_reference get_first() const {
                return *this;
            }

            constexpr second_reference get_second() {
                return second;
            }

            constexpr second_const_reference get_second() const {
                return second;
            }

            constexpr void swap(compressed_pair<Ty1, Ty2> &pair) {
                std::swap(second, pair.second);
            }

            second_type second;
        };

        template <typename Ty1, typename Ty2>
        class compressed_pair_impl<Ty1, Ty2, 2> : protected foundation::type_traits::internals::remove_cv_t<Ty2>::type {
        public:
            using first_type = Ty1;
            using second_type = Ty2;
            using first_param_type = typename foundation::type_traits::internals::_call_traits<first_type>::param_type;
            using second_param_type = typename foundation::type_traits::internals::_call_traits<second_type>::param_type;
            using first_reference = typename foundation::type_traits::internals::_call_traits<first_type>::reference;
            using second_reference = typename foundation::type_traits::internals::_call_traits<second_type>::reference;
            using first_const_reference = typename foundation::type_traits::internals::_call_traits<first_type>::const_reference;
            using second_const_reference = typename foundation::type_traits::internals::_call_traits<second_type>::const_reference;

            constexpr compressed_pair_impl() = default;

            constexpr compressed_pair_impl(first_param_type x, second_param_type y) : second_type(y), first(x) {
            }

            constexpr explicit compressed_pair_impl(first_param_type x) : first(x) {
            }

            constexpr explicit compressed_pair_impl(second_param_type y) : second_type(y) {
            }

            constexpr first_reference get_first() {
                return first;
            }

            constexpr first_const_reference get_first() const {
                return first;
            }

            constexpr second_reference get_second() {
                return *this;
            }
            
            constexpr second_const_reference get_second() const {
                return *this;
            }

            constexpr void swap(compressed_pair<Ty1, Ty2> &pair) {
                std::swap(first, pair.first);
            }

            first_type first;
        };

        template <typename Ty1, typename Ty2>
        class compressed_pair_impl<Ty1, Ty2, 3> : protected foundation::type_traits::internals::remove_cv_t<Ty1>,
                                                  protected foundation::type_traits::internals::remove_cv_t<Ty2> {
        public:
            using first_type = Ty1;
            using second_type = Ty2;
            using first_param_type = typename foundation::type_traits::internals::_call_traits<first_type>::param_type;
            using second_param_type = typename foundation::type_traits::internals::_call_traits<second_type>::param_type;
            using first_reference = typename foundation::type_traits::internals::_call_traits<first_type>::reference;
            using second_reference = typename foundation::type_traits::internals::_call_traits<second_type>::reference;
            using first_const_reference = typename foundation::type_traits::internals::_call_traits<first_type>::const_reference;
            using second_const_reference = typename foundation::type_traits::internals::_call_traits<second_type>::const_reference;

            compressed_pair_impl() = default;

            compressed_pair_impl(first_param_type first, second_param_type second) : first_type(first), second_type(second) {
            }

            explicit compressed_pair_impl(first_param_type first) : first_type(first) {
            }

            explicit compressed_pair_impl(second_param_type second) : second_type(second) {
            }

            first_reference get_first() {
                return *this;
            }
            first_const_reference get_first() const {
                return *this;
            }

            second_reference get_second() {
                return *this;
            }
            second_const_reference get_second() const {
                return *this;
            }

            void swap(compressed_pair<Ty1, Ty2> &) {
            }
        };

        template <typename Ty1, typename Ty2>
        class compressed_pair_impl<Ty1, Ty2, 4> : protected foundation::type_traits::internals::remove_cv_t<Ty1> {
        public:
            using first_type = Ty1;
            using second_type = Ty2;
            using first_param_type = typename foundation::type_traits::internals::_call_traits<first_type>::param_type;
            using second_param_type = typename foundation::type_traits::internals::_call_traits<second_type>::param_type;
            using first_reference = typename foundation::type_traits::internals::_call_traits<first_type>::reference;
            using second_reference = typename foundation::type_traits::internals::_call_traits<second_type>::reference;
            using first_const_reference = typename foundation::type_traits::internals::_call_traits<first_type>::const_reference;
            using second_const_reference = typename foundation::type_traits::internals::_call_traits<second_type>::const_reference;

            compressed_pair_impl() = default;

            compressed_pair_impl(first_param_type x, second_param_type y) : first_type(x), second(y) {
            }

            explicit compressed_pair_impl(first_param_type x) : first_type(x), second(x) {
            }

            constexpr first_reference get_first() {
                return *this;
            }
            
            constexpr first_const_reference get_first() const {
                return *this;
            }

            constexpr second_reference get_second() {
                return second;
            }
            
            constexpr second_const_reference get_second() const {
                return second;
            }

            constexpr void swap(compressed_pair<Ty1, Ty2> &pair) {
                std::swap(second, pair.second);
            }

            Ty2 second;
        };

        template <typename Ty1, typename Ty2>
        class compressed_pair_impl<Ty1, Ty2, 5> {
        public:
            using first_type = Ty1;
            using second_type = Ty2;
            using first_param_type = typename foundation::type_traits::internals::_call_traits<first_type>::param_type;
            using second_param_type = typename foundation::type_traits::internals::_call_traits<second_type>::param_type;
            using first_reference = typename foundation::type_traits::internals::_call_traits<first_type>::reference;
            using second_reference = typename foundation::type_traits::internals::_call_traits<second_type>::reference;
            using first_const_reference = typename foundation::type_traits::internals::_call_traits<first_type>::const_reference;
            using second_const_reference = typename foundation::type_traits::internals::_call_traits<second_type>::const_reference;

            compressed_pair_impl() = default;

            compressed_pair_impl(first_param_type first, second_param_type second) : first(first), second(second) {
            }

            explicit compressed_pair_impl(first_param_type first) : first(first), second(first) {
            }

            constexpr first_reference get_first() {
                return first;
            }
            
            constexpr first_const_reference get_first() const {
                return first;
            }

            constexpr second_reference get_second() {
                return second;
            }
            
            constexpr second_const_reference get_second() const {
                return second;
            }

            void swap(compressed_pair<Ty1, Ty2> &y) {
                std::swap(first, y.first);
                std::swap(second, y.second);
            }

            Ty1 first;
            Ty2 second;
        };
    }

    template <typename Ty1, typename Ty2>
    class compressed_pair
        : public internals::compressed_pair_impl<
              Ty1, Ty2,
              internals::compressed_pair_switch<
                  Ty1, Ty2,
                                    foundation::type_traits::internals::_is_same_v<foundation::type_traits::cv_modify::remove_cv_t<Ty1>,
                                                                                   foundation::type_traits::cv_modify::remove_cv_t<Ty2>>,
                  internals::compressed_pair_empty<Ty1>::value, internals::compressed_pair_empty<Ty2>::value>::value> {
    public:
        using base = internals::compressed_pair_impl<
            Ty1, Ty2,
            internals::compressed_pair_switch<
                Ty1, Ty2,
                foundation::type_traits::internals::_is_same_v<foundation::type_traits::cv_modify::remove_cv_t<Ty1>,
                                                               foundation::type_traits::cv_modify::remove_cv_t<Ty2>>,
                internals::compressed_pair_empty<Ty1>::value, internals::compressed_pair_empty<Ty2>::value>::value>;
        using base::base;
    };

    template <typename Ty>
    class compressed_pair<Ty, Ty>
        : public internals::compressed_pair_impl<
              Ty, Ty,
              internals::compressed_pair_switch<
                  Ty, Ty,
                  foundation::type_traits::internals::_is_same_v<foundation::type_traits::cv_modify::remove_cv_t<Ty>,
                                                                 foundation::type_traits::cv_modify::remove_cv_t<Ty>>,
                  internals::compressed_pair_empty<Ty>::value, internals::compressed_pair_empty<Ty>::value>::value> {
    public:
        using base = internals::compressed_pair_impl<
            Ty, Ty,
            internals::compressed_pair_switch<
                Ty, Ty,
                foundation::type_traits::internals::_is_same_v<foundation::type_traits::cv_modify::remove_cv_t<Ty>,
                                                               foundation::type_traits::cv_modify::remove_cv_t<Ty>>,
                internals::compressed_pair_empty<Ty>::value, internals::compressed_pair_empty<Ty>::value>::value>;
        using base::base;
    };
}

namespace rainy::utility {
    template <typename Iter, typename container, std::enable_if_t<std::is_pointer_v<Iter>,int> = 0>
    class iterator {
    public:
        using iterator_type = Iter;
        using iterator_traits = std::iterator_traits<iterator_type>;
        using iterator_category = typename iterator_traits::iterator_category;
        using value_type = typename iterator_traits::value_type;
        using difference_type = typename iterator_traits::difference_type;
        using reference = typename iterator_traits::reference;
        using const_reference = const value_type &;
        using pointer = typename iterator_traits::pointer;
        using const_pointer = const value_type *;

        RAINY_CONSTEXPR20 iterator() noexcept = default;
        RAINY_CONSTEXPR20 iterator &operator=(iterator &&) noexcept = default;
        RAINY_CONSTEXPR20 iterator &operator=(const iterator &) noexcept = default;

        explicit RAINY_CONSTEXPR20 iterator(iterator_type current) noexcept : ptr(current) {
        }

        RAINY_CONSTEXPR20 iterator(const iterator &right) : ptr(right.ptr) {
        }

        RAINY_CONSTEXPR20 iterator(iterator &&right) noexcept : ptr(std::exchange(right.ptr, nullptr)) {
        }

        template <typename Iter_, std::enable_if_t<std::is_convertible_v<Iter_, Iter>, int> = 0>
        RAINY_CONSTEXPR20 explicit iterator(const iterator<Iter_, container> &right) : ptr(right.ptr) {
        }

        template <typename Iter_, std::enable_if_t<std::is_convertible_v<Iter_, Iter>, int> = 0>
        RAINY_CONSTEXPR20 explicit iterator(iterator<Iter_, container> &&right) : ptr(right.ptr) {
        }

        RAINY_CONSTEXPR20 ~iterator() = default;

        RAINY_CONSTEXPR20 pointer operator->() noexcept {
            return ptr;
        }

        RAINY_CONSTEXPR20 const_pointer operator->() const noexcept {
            return ptr;
        }

        RAINY_CONSTEXPR20 reference operator*() noexcept {
            return *ptr;
        }

        RAINY_CONSTEXPR20 const_reference operator*() const noexcept {
            return *ptr;
        }

        RAINY_CONSTEXPR20 iterator operator++() noexcept {
            return iterator{++ptr};
        }

        RAINY_CONSTEXPR20 iterator operator++(int) noexcept {
            iterator temp = *this;
            ++(*this);
            return temp;
        }

        RAINY_CONSTEXPR20 iterator operator--() noexcept {
            return iterator{--ptr};
        }

        RAINY_CONSTEXPR20 iterator operator--(int) noexcept {
            iterator temp = *this;
            --(*this);
            return temp;
        }

        RAINY_CONSTEXPR20 iterator &operator+=(difference_type n) noexcept {
            ptr += n;
            return *this;
        }

        RAINY_CONSTEXPR20 iterator &operator-=(difference_type n) noexcept {
            ptr -= n;
            return *this;
        }

        RAINY_CONSTEXPR20 void swap(iterator &right) noexcept {
            std::swap(this->ptr, right.ptr);
        }

        RAINY_CONSTEXPR20 friend bool operator==(const iterator &left, const iterator &right) {
            return left.ptr == right.ptr;
        }

        RAINY_CONSTEXPR20 friend bool operator!=(const iterator &left, const iterator &right) {
            return left.ptr != right.ptr;
        }

        RAINY_CONSTEXPR20 reference operator[](difference_type idx) noexcept {
            return ptr[idx];
        }

        RAINY_CONSTEXPR20 const_reference operator[](difference_type idx) const noexcept {
            return ptr[idx];
        }

        RAINY_NODISCARD_CONSTEXPR20 bool empty() const noexcept {
            return this->ptr == nullptr;
        }

        RAINY_CONSTEXPR20 explicit operator bool() const noexcept {
            return !empty();
        }

        RAINY_CONSTEXPR20 friend iterator operator+(const iterator &_iterator, difference_type n) {
            return iterator{_iterator.ptr + n};
        }

        RAINY_CONSTEXPR20 friend iterator operator+(const iterator &left, const iterator &right) {
            return iterator{left.ptr + right.ptr};
        }

        RAINY_CONSTEXPR20 friend difference_type operator-(const iterator &left, const iterator &right) {
            return left.ptr - right.ptr;
        }

        RAINY_CONSTEXPR20 friend bool operator<(const iterator &left, const iterator &right) {
            return left.ptr < right.ptr;
        }

        RAINY_CONSTEXPR20 friend bool operator<=(const iterator &left, const iterator &right) {
            return left.ptr <= right.ptr;
        }

        RAINY_CONSTEXPR20 friend bool operator>(const iterator &left, const iterator &right) {
            return left.ptr > right.ptr;
        }

        RAINY_CONSTEXPR20 friend bool operator>=(const iterator &left, const iterator &right) {
            return left.ptr >= right.ptr;
        }

    private:
        pointer ptr;
    };
}

namespace rainy::foundation::containers {
    template <typename Ty, std::size_t N>
    class array final {
    public:
        using value_type = Ty;
        using size_type = std::size_t;
        using reference = Ty &;
        using const_reference = const Ty &;
        using pointer = value_type *;
        using const_pointer = const value_type *;
        using difference_type = std::ptrdiff_t;
        /* 普通迭代器 */
        using iterator = utility::iterator<pointer, array>;
        using const_iterator = utility::iterator<const_pointer, array>;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<iterator>;

        RAINY_CONSTEXPR20 ~array() = default;

        /**
         * @brief 获取指定下标的元素
         * @attention 此函数进行范围检查
         * @param off 偏移量
         * @return 返回对应下标的元素引用
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE reference at(difference_type off) {
            range_check(off);
            return elems_[off];
        }

        /**
         * @brief 获取指定下标的元素
         * @attention 此函数进行范围检查
         * @param off 偏移量
         * @return 返回对应下标的元素引用
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE  const_reference at(difference_type off) const {
            check_zero_length_array();
            range_check(off);
            return elems_[off];
        }

        /**
         * @brief 获取当前数组第一个元素
         * @return 返回第一个元素的引用
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE reference front() {
            check_zero_length_array();
            return elems_[0];
        }

        /**
         * @brief 获取当前数组第一个元素
         * @return 返回第一个元素的引用
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE constexpr const_reference front() const {
            check_zero_length_array();
            return elems_[0];
        }

        /**
         * @brief 获取当前数组最后一个元素
         * @return 返回最后一个元素的引用
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE reference back() {
            check_zero_length_array();
            return elems_[N - 1];
        }

        /**
         * @brief 获取当前数组最后一个元素
         * @return 返回最后一个元素的引用
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE constexpr const_reference back() const {
            check_zero_length_array();
            return elems_[N - 1];
        }

        /**
         * @brief 获取指定下标的元素
         * @attention 此函数不进行范围检查
         * @param idx 索引下标
         * @return 返回对应下标的元素引用
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE reference operator[](difference_type idx) noexcept {
            return elems_[idx]; // NOLINT
        }

        /**
         * @brief 获取指定下标的元素
         * @attention 此函数不进行范围检查
         * @param idx 索引下标
         * @return 返回对应下标的元素引用
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE constexpr const_reference operator[](difference_type idx) const noexcept {
            return elems_[idx]; // NOLINT
        }

        /**
         * @brief 将一个对象的数据与当前对象的数据进行交换
         * @param right 待交换的数组对象
         */
        RAINY_ALWAYS_INLINE void swap(array &right) noexcept(std::is_nothrow_swappable_v<Ty>) {
            check_zero_length_array();
            std::swap_ranges(elems_, right.elems_);
        }

        /**
         * @brief 将一个值填充到数组中
         * @param value 要填充的值
         */
        RAINY_CONSTEXPR20 void fill(const Ty &value) {
            check_zero_length_array();
            std::fill_n(begin(), size(), value);
        }

        /**
         * @brief 获取当前数组大小
         * @return 返回当前数组大小
         */
        RAINY_ALWAYS_INLINE constexpr static size_type size() noexcept {
            return N;
        }

        /**
         * @brief 检查当前数组容器是否为空
         * @return 容器状态
         */
        RAINY_ALWAYS_INLINE constexpr static bool empty() noexcept {
            return size() == 0;
        }

        /**
         * @brief 获取当前数组地址
         * @return 返回数组的地址，以指针形式
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE constexpr pointer data() noexcept {
            return elems_;
        }

        /**
         * @brief 获取当前数组地址
         * @return 返回数组的地址，以指针形式
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE constexpr const_pointer data() const noexcept {
            return elems_;
        }

        /**
         * @brief 获取指向当前数组起始位置的迭代器
         * @return 返回指向起始的迭代器
         */
        RAINY_ALWAYS_INLINE iterator begin() {
            check_zero_length_array();
            return iterator(elems_);
        }

        /**
         * @brief 获取指向当前数组起始位置的迭代器
         * @return 返回指向起始的迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD const_iterator begin() const {
            check_zero_length_array();
            return const_iterator(elems_);
        }

        /**
         * @brief 获取指向当前数组起始位置的迭代器常量
         * @return 返回指向起始的迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD const_iterator cbegin() const {
            check_zero_length_array();
            return const_iterator(elems_);
        }

        /**
         * @brief 获取指向当前数组末尾位置的反向迭代器
         * @return 返回指向起始的反向迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD reverse_iterator rbegin() {
            check_zero_length_array();
            return reverse_iterator(end());
        }

        /**
         * @brief 获取指向当前数组末尾位置的反向迭代器
         * @return 返回指向起始的反向迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD const_reverse_iterator rbegin() const {
            check_zero_length_array();
            return const_reverse_iterator(end());
        }

        /**
         * @brief 获取指向当前数组末尾位置的反向迭代器常量
         * @return 返回指向起始的反向迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD const_reverse_iterator crbegin() const {
            check_zero_length_array();
            return const_reverse_iterator(end());
        }

        /**
         * @brief 获取指向当前数组末尾位置的迭代器
         * @return 返回指向末尾的迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD iterator end() {
            check_zero_length_array();
            return iterator(elems_ + N);
        }

        /**
         * @brief 获取指向当前数组末尾位置的迭代器
         * @return 返回指向末尾的迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD const_iterator end() const {
            check_zero_length_array();
            return const_iterator(elems_ + N);
        }

        /**
         * @brief 获取指向当前数组末尾位置的迭代器常量
         * @return 返回指向末尾的迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD const_iterator cend() const {
            check_zero_length_array();
            return const_iterator(elems_ + N);
        }

        /**
         * @brief 获取指向当前数组末尾位置的反向迭代器
         * @return 返回指向末尾的反向迭代器
         */
        RAINY_ALWASY_INLINE_NODISCARD reverse_iterator rend() {
            check_zero_length_array();
            return reverse_iterator(end());
        }

        /**
         * @brief 获取指向当前数组末尾位置的反向迭代器
         * @return 返回指向末尾的反向迭代器
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE const_reverse_iterator rend() const {
            check_zero_length_array();
            return const_reverse_iterator(end());
        }

        /**
         * @brief 获取指向当前数组末尾位置的反向迭代器常量
         * @return 返回指向末尾的反向迭代器
         */
        RAINY_NODISCARD RAINY_ALWAYS_INLINE const_reverse_iterator crend() const {
            check_zero_length_array();
            return const_reverse_iterator(end());
        }

        Ty elems_[N == 0 ? 1 : N];

    private:
        RAINY_ALWAYS_INLINE static void range_check(const difference_type offset) {
            if (offset > N || offset == 0) {
                system::exceptions::logic::throw_out_of_range("Invalid array subscript");
            }
        }

        RAINY_ALWAYS_INLINE static void check_zero_length_array() {
            if (size() == 0) {
                std::terminate();
            }
        }
    };
}

namespace rainy::foundation::containers {
    template <typename Ty>
    class array_view {
    public:
        using value_type = Ty;
        using size_type = std::size_t;
        using reference = Ty &;
        using const_reference = const Ty &;
        using pointer = value_type *;
        using const_pointer = const value_type *;
        using difference_type = std::ptrdiff_t;
        using iterator = utility::iterator<pointer, array_view>;
        using const_iterator = const utility::iterator<const_pointer, array_view>;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = const std::reverse_iterator<iterator>;

        constexpr array_view() = default;

        RAINY_CONSTEXPR20 array_view(array_view &&) = default;
        RAINY_CONSTEXPR20 array_view(const array_view &) = default;
        array_view &operator=(array_view &&) = default;
        array_view &operator=(const array_view &) = default;

        RAINY_CONSTEXPR20 ~array_view() = default;

        template <size_type N>
        RAINY_CONSTEXPR20 array_view(value_type (&reference_array)[N]) : data_(reference_array), size_(N) {
        }

        RAINY_CONSTEXPR20 array_view(std::vector<Ty> &vector) : data_(vector.data()), size_(vector.size()) {
        }

        RAINY_CONSTEXPR20 array_view(const std::vector<Ty> &vector) : data_(vector.data()), size_(vector.size()) {
        }

        RAINY_CONSTEXPR20 array_view(pointer first, pointer last) : data_(first), size_(std::distance(first, last)) {
        }

        RAINY_CONSTEXPR20 array_view(const_pointer first, const_pointer last) : data_(first), size_(std::distance(first, last)) {
        }

        RAINY_CONSTEXPR20 array_view(std::initializer_list<Ty> initializer_list) :
            data_(initializer_list.begin()), size_(initializer_list.size()) {
        }

        template <size_type N>
        RAINY_CONSTEXPR20 array_view(std::array<Ty, N> &array) : data_(array.data()), size_(array.size()) {
        }

        template <size_type N>
        RAINY_CONSTEXPR20 array_view(const std::array<Ty, N> &array) : data_(array.data()), size_(array.size()) {
        }

        template <size_type N>
        RAINY_CONSTEXPR20 array_view(array<Ty, N> &array) : data_(array.data()), size_(array.size()) {
        }

        template <size_type N>
        RAINY_CONSTEXPR20 array_view(const array<Ty, N> &array) : data_(array.data()), size_(array.size()) {
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 pointer data() noexcept {
            return data_;
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 const_pointer data() const noexcept {
            return data_;
        }

        RAINY_NODISCARD constexpr size_type size() const {
            return size_;
        }

        RAINY_NODISCARD constexpr bool empty() const {
            return size_ == 0;
        }

        RAINY_CONSTEXPR20 reference at(const difference_type idx) {
            rangecheck(size(), idx);
            return data_[idx];
        }

        RAINY_NODISCARD constexpr const_reference at(const difference_type idx) const {
            rangecheck(size(), idx);
            return data_[idx];
        }

        constexpr const_reference operator[](const difference_type idx) const {
            return data_[idx];
        }

        RAINY_NODISCARD constexpr bool check_index(const size_type idx) const noexcept {
            return idx < size_;
        }

    private:
        static void rangecheck(const size_type size, const difference_type idx) {
            if (size <= idx) {
                utility::throw_exception(std::out_of_range("Invalid array subscript"));
            }
        }

        pointer data_;
        size_type size_{};
    };

    template <typename Ty>
    RAINY_CONSTEXPR20 array_view<Ty> make_array_view(Ty *first, Ty *last) {
        return array_view<Ty>(first, last);
    }

    template <typename Ty>
    RAINY_CONSTEXPR20 array_view<Ty> make_array_view(const Ty *first, const Ty *last) {
        return array_view<Ty>(first, last);
    }

    template <typename Ty, std::size_t N>
    RAINY_CONSTEXPR20 array_view<Ty> make_array_view(Ty (&array)[N]) {
        return array_view<Ty>(array);
    }

    template <typename Ty>
    RAINY_CONSTEXPR20 array_view<Ty> make_array_view(std::vector<Ty> &vector) {
        return array_view<Ty>(vector);
    }

    template <typename Ty>
    RAINY_CONSTEXPR20 array_view<Ty> make_array_view(const std::vector<Ty> &vector) {
        return array_view<Ty>(vector);
    }

    template <typename Ty, std::size_t N>
    RAINY_CONSTEXPR20 array_view<Ty> make_array_view(std::array<Ty, N> &array) {
        return array_view<Ty>(array);
    }

    template <typename Ty, std::size_t N>
    RAINY_CONSTEXPR20 array_view<Ty> make_array_view(const std::array<Ty, N> &array) {
        return array_view<Ty>(array);
    }

    template <typename Ty, std::size_t N>
    RAINY_CONSTEXPR20 array_view<Ty> make_array_view(array<Ty, N> &array) {
        return array_view<Ty>(array);
    }

    template <typename Ty, std::size_t N>
    RAINY_CONSTEXPR20 array_view<Ty> make_array_view(const array<Ty, N> &array) {
        return array_view<Ty>(array);
    }
}

/*
仿函数实现
*/
namespace rainy::foundation::functional {
    namespace operators {
        template <typename Ty = void>
        struct plus {
            constexpr Ty operator()(const Ty &left, const Ty &right) const {
                return left + right;
            }
        };

        template <typename Ty = void>
        struct minus {
            constexpr Ty operator()(const Ty &left, const Ty &right) const {
                return left - right;
            }
        };

        template <typename Ty = void>
        struct multiplies {
            constexpr Ty operator()(const Ty &left, const Ty &right) const {
                return left * right;
            }
        };

        template <typename Ty = void>
        struct divides {
            constexpr Ty operator()(const Ty &left, const Ty &right) const {
                return left / right;
            }
        };

        template <typename Ty = void>
        struct modulus {
            constexpr Ty operator()(const Ty &left, const Ty &right) const {
                return left % right;
            }
        };

        template <typename Ty = void>
        struct negate {
            constexpr Ty operator()(const Ty &object) const {
                return -object;
            }
        };

        template <>
        struct plus<void> {
            template <typename Ty, typename U>
            constexpr auto operator()(const Ty &left, const Ty &right) const
                noexcept(noexcept(std::forward<Ty>(left) + std::forward<U>(right)))
                    -> decltype(std::forward<Ty>(left) + std::forward<U>(right)) {
                return left + right;
            }
        };

        template <>
        struct minus<void> {
            template <typename Ty, typename U>
            constexpr auto operator()(const Ty &left, const Ty &right) const
                noexcept(noexcept(std::forward<Ty>(left) - std::forward<U>(right)))
                    -> decltype(std::forward<Ty>(left) - std::forward<U>(right)) {
                return left - right;
            }
        };

        template <>
        struct multiplies<void> {
            template <typename Ty, typename U>
            constexpr auto operator()(const Ty &left, const U &right) const
                noexcept(noexcept(std::forward<Ty>(left) * std::forward<U>(right)))
                    -> decltype(std::forward<Ty>(left) * std::forward<U>(right)) {
                return left * right;
            }
        };

        template <>
        struct divides<void> {
            template <typename Ty, typename U>
            constexpr auto operator()(const Ty &left, const U &right) const
                noexcept(noexcept(std::forward<Ty>(left) / std::forward<U>(right)))
                    -> decltype(std::forward<Ty>(left) / std::forward<U>(right)) {
                return left / right;
            }
        };

        template <>
        struct modulus<void> {
            template <typename Ty, typename U>
            constexpr auto operator()(const Ty &left, const U &right) const
                noexcept(noexcept(std::forward<Ty>(left) % std::forward<U>(right)))
                    -> decltype(std::forward<Ty>(left) % std::forward<U>(right)) {
                return left % right;
            }
        };

        template <>
        struct negate<void> {
            template <typename Ty>
            constexpr auto operator()(const Ty &object) const noexcept(noexcept(-std::forward<Ty>(object)))
                -> decltype(-std::forward<Ty>(object)) {
                return -object;
            }
        };
    }

    namespace predicate {
        template <typename Ty = void>
        struct equal {
            constexpr bool operator()(const Ty &left, const Ty &right) const {
                return left == right;
            }
        };

        template <>
        struct equal<void> {
            template <typename Ty, typename U>
            constexpr auto operator()(const Ty &left, const Ty &right) const
                noexcept(noexcept(std::forward<Ty>(left) == std::forward<U>(right)))
                    -> decltype(std::forward<Ty>(left) == std::forward<U>(right)) {
                return left == right;
            }
        };
    }
}

namespace rainy::foundation::system::memory {
    template <typename Ty>
    struct default_deleter {
        constexpr default_deleter() noexcept = default;

        constexpr default_deleter(utility::placeholder) noexcept {}

        template <typename U, typename = std::enable_if_t<std::is_convertible_v<U *, Ty *>>>
        constexpr explicit default_deleter(const default_deleter &) noexcept {
        }

        RAINY_CONSTEXPR20 void operator()(const Ty *resource) const noexcept {
            static_assert(0 < sizeof(Ty), "can't delete an incomplete type"); // NOLINT
            delete resource;
        }
    };

    template <typename Ty>
    struct default_deleter<Ty[]> {
        constexpr default_deleter() = default;

        constexpr default_deleter(utility::placeholder) noexcept {
        }

        template <typename U, std::enable_if_t<std::is_convertible_v<U (*)[], Ty (*)[]>, int> = 0>
        constexpr explicit default_deleter(const default_deleter &) noexcept {
        }

        template <typename U, std::enable_if_t<std::is_convertible_v<U (*)[], Ty (*)[]>, int> = 0>
        RAINY_CONSTEXPR20 void operator()(const U *resource) const noexcept {
            static_assert(!std::is_abstract_v<U>, "can't delete an incomplete type");
            delete[] resource;
        }
    };

    template <typename Ty>
    struct no_delete {
        constexpr no_delete() noexcept = default;

        constexpr no_delete(utility::placeholder) noexcept {
        }

        template <typename U, typename = std::enable_if_t<std::is_convertible_v<U *, Ty *>>>
        constexpr explicit no_delete(const no_delete &) noexcept {
        }

        RAINY_CONSTEXPR20 void operator()(const Ty *resource) const noexcept {
            static_assert(0 < sizeof(Ty), "can't delete an incomplete type"); // NOLINT
        }
    };

    template <typename Ty>
    struct no_delete<Ty[]> {
        constexpr no_delete() = default;

        constexpr no_delete(utility::placeholder) noexcept {
        }

        template <typename U, std::enable_if_t<std::is_convertible_v<U (*)[], Ty (*)[]>, int> = 0>
        constexpr explicit no_delete(const no_delete &) noexcept {
        }

        template <typename U, std::enable_if_t<std::is_convertible_v<U (*)[], Ty (*)[]>, int> = 0>
        RAINY_CONSTEXPR20 void operator()(const U *resource) const noexcept {
            static_assert(!std::is_abstract_v<U>, "can't delete an incomplete type");
        }
    };
}


namespace rainy::utility {
    /**
     * @brief 用于将结果和资源进行绑定以便快捷返回.
     * @tparam _error 函数返回的结果，此处应当尽可能使用枚举或整型数据
     * @tparam _type 用于提供实际返回资源
     * @note 该模板结构体根据 _type 是否具有默认构造函数提供不同的实现。
     */
    template <typename Result, typename Type, bool = std::is_default_constructible_v<Type>>
    struct result_collection {
        Result result;
        Type data;
    };

    /**
     * @brief 特化版本，用于 _type 不具有默认构造函数的情况。
     * @tparam _error 函数返回的结果，应当使用枚举或整型数据。
     * @tparam _type 用于提供实际返回资源。
     */
    template <typename Result, typename Type>
    struct result_collection<Result, Type, false> {
        template <typename... Args>
        explicit result_collection(Result result,
                                   Args &&...construct_args) noexcept(std::conjunction_v<std::is_nothrow_default_constructible<Type>>) :
            result(result), data(utility::forward<Args>(construct_args)...) {
        }

        Result result;
        Type data;
    };
}

namespace rainy::utility {
    template <typename Ty>
    struct not_null {
    public:
        using pointer = Ty;

        /// static_assert(foundation::type_traits::primary_types::is_pointer_v<Ty>, "Ty must be a pointer!");

        not_null() = delete;

        constexpr not_null(std::nullptr_t) = delete;

        constexpr not_null(pointer resource) noexcept : resource(resource) {
            ensures(resource != nullptr, "resource cannot be a null pointer!");
        }

        constexpr decltype(auto) operator->() const {
            return resource;
        }

        constexpr decltype(auto) operator*() const {
            return *resource;
        }

    private:
        pointer resource;
    };
}

namespace rainy::utility::internals {
    template <typename TY>
    constexpr void verify_range(const TY *const first, const TY *const last) noexcept {
        // special case range verification for pointers
        expects(first <= last, "transposed pointer range");
    }

    template <typename iter, typename = void>
    constexpr bool allow_inheriting_unwrap_v = true;

    template <typename iter>
    constexpr bool allow_inheriting_unwrap_v<iter, foundation::type_traits::internals::_void_t<typename iter::prevent_inheriting_unwrap>> =
        foundation::type_traits::internals::_is_same_v<iter, typename iter::prevent_inheriting_unwrap>;

    template <typename iter, typename sentinel = iter, typename = void>
    constexpr bool range_verifiable_v = false;

    template <typename iter, typename sentinel>
    constexpr bool range_verifiable_v<
        iter, sentinel, foundation::type_traits::internals::_void_t<decltype(verify_range(declval<const iter &>(), declval<const sentinel &>()))>> =
        allow_inheriting_unwrap_v<iter>;

    template <typename iter, typename sentinel>
    constexpr void adl_verify_range(const iter &first, const sentinel &last) {
        // check that [first, last) forms an iterator range
        if constexpr (foundation::type_traits::internals::_is_pointer_v<iter> &&
                      foundation::type_traits::internals::_is_pointer_v<sentinel>) {
            expects(first <= last, "transposed pointer range");
        } else if constexpr (range_verifiable_v<iter, sentinel>) {
            verify_range(first, last);
        }
    }
}

namespace rainy::utility {
    template <typename Iter>
    RAINY_NODISCARD constexpr std::ptrdiff_t distance(Iter first, Iter last) {
        if constexpr (std::is_same_v<typename std::iterator_traits<Iter>::iterator_category, std::random_access_iterator_tag>) {
            return last - first; // assume the iterator will do debug checking
        } else {
            internals::adl_verify_range(first, last);
            auto ufirst = addressof(*first);
            const auto ulast = addressof(*(last - 1)) + 1;
            std::ptrdiff_t off = 0;
            for (; ufirst != ulast; ++ufirst) {
                ++off;
            }
            return off;
        }
    }
}

namespace rainy::text {
    template <typename Elem>
    struct common_char_traits {
        using char_type = Elem;
        using int_type = int;
        using off_type = std::streamoff;
        using pos_type = std::streampos;
        using state_type = std::mbstate_t;
        using size_type = std::size_t;
#if RAINY_HAS_CXX20
        using comparison_category = std::strong_ordering;
#endif

        static constexpr void assign(char_type &char_to, const char_type &char_from) noexcept {
            char_to = char_from;
        }

        static RAINY_CONSTEXPR20 void assign(char_type *char_to, const size_type num, const char_type &char_from) {
            for (int i = 0; i < num; ++i) {
                char_to[i] = char_from;
            }
        }

        static RAINY_CONSTEXPR20 bool eq(const char_type left, const char_type right) noexcept {
            return left == right;
        }

        static RAINY_CONSTEXPR20 bool eq_int_type(const int_type &left, const int_type &right) noexcept {
            return left == right;
        }

        static RAINY_CONSTEXPR20 int compare(const char_type *string1, const char_type *string2, const size_type count) {
            using information::internals::compare_string;
            using information::internals::compare_string_compile_time;
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                return compare_string_compile_time(string1, string2, count);
            }
#endif
            return compare_string(string1, string2, count);
        }

        static RAINY_CONSTEXPR20 size_type length(const char_type *string) {
            using information::internals::string_length;
            using information::internals::string_length_compile_time;
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                return string_length_compile_time(string);
            }
#endif
            return string_length(string);
        }

        RAINY_NODISCARD static RAINY_CONSTEXPR20 const char_type *find(const char_type *string, std::size_t count,
                                                                             const char_type &target) {
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                for (; 0 < count; --count, ++string) {
                    if (*string == target) {
                        return string;
                    }
                }
            } else
#endif
            {
#if RAINY_USING_AVX2
                auto *bytes = reinterpret_cast<const unsigned char *>(string);
                const __m128i target_vector = _mm_set1_epi8(target);
                for (std::size_t i = 0; i + 16 <= count; i += 16) {
                    const __m128i chunk = _mm_loadu_si128(reinterpret_cast<const __m128i *>(bytes + i));
                    const __m128i cmp = _mm_cmpeq_epi8(chunk, target_vector);
                    if (const int mask = _mm_movemask_epi8(cmp); mask != 0) {
                        return string + i + information::internals::ctz_avx2(mask);
                    }
                }
                for (std::size_t i = count & ~0xF; i < count; ++i) {
                    if (bytes[i] == target) {
                        return string + i;
                    }
                }
#else
                for (; 0 < num; --num, ++string) {
                    if (*string == target) {
                        return string;
                    }
                }
#endif
            }
            return nullptr;
        }

        static RAINY_CONSTEXPR20 bool lt(const char_type left, const char_type right) {
            return left < right;
        }

        static RAINY_CONSTEXPR20 char_type *move(char_type *to, const char_type *from, size_type count) {
#if RAINY_HAS_CXX20
            if (std::is_constant_evaluated()) {
                bool loop_forward = true;
                for (const Elem *source = from; source != from + count; ++source) {
                    if (to == source) {
                        loop_forward = false;
                        break;
                    }
                }
                if (loop_forward) {
                    for (size_t idx = 0; idx != count; ++idx) {
                        to[idx] = from[idx];
                    }
                } else {
                    for (size_t idx = count; idx != 0; --idx) {
                        to[idx - 1] = from[idx - 1];
                    }
                }
                return to;
            }
#endif
#if RAINY_USING_MSVC
            errno_t error = memmove_s(to, count, from, count);
            if (error != 0) {
                std::abort();
            }
#elif RAINY_USING_CLANG
            __builtin_memmove(to, from, count);
#elif RAINY_USING_GCC
            memmove(to, from, count);
#endif
            return to;
        }

        template <size_type N>
        static RAINY_CONSTEXPR20 char_type *move(std::array<char_type, N> &to, const char_type *from, const size_type count) {
            if (N < count) {
                return nullptr;
            }
            return move(to.data(), from, count);
        }

        template <size_type N>
        static RAINY_CONSTEXPR20 char_type *move(foundation::containers::array<char_type, N> &to, const char_type *from,
                                                    const size_type count) {
            if (N < count) {
                return nullptr;
            }
            return move(to.data(), from, count);
        }

        static RAINY_CONSTEXPR20 char_type *move(foundation::containers::array_view<char_type> &to, const char_type *from,
                                                    const size_type count) {
            if (to.size() < count || to.empty()) {
                return nullptr;
            }
            return move(to.data(), from, count);
        }

        static RAINY_CONSTEXPR20 char_type *move(std::vector<char_type> &to, const char_type *from, const size_type count) {
            if (to.size() < count || to.empty()) {
                return nullptr;
            }
            return move(to.data(), from, count);
        }

        template <size_type N>
        static RAINY_CONSTEXPR20 char_type *move_s(Elem (&to)[N], const char_type *from, const size_type count) {
            if (N < count) {
                return nullptr;
            }
            return move(to, from, N);
        }

        static RAINY_CONSTEXPR20 char_type *move_s(char_type *dest, const size_type dest_size, const char_type *from,
                                                      const size_type count) {
            if (dest_size < count) {
                return nullptr;
            }
            return move(dest, from, count);
        }

        static constexpr int_type to_int_type(const int_type &ch) {
            return ch;
        }

        static constexpr char_type to_char_type(const int_type &ch) {
            return ch;
        }

        static constexpr int_type eof() {
            return EOF;
        }

        static constexpr int_type not_eof(const int_type &ch) {
            return ch != eof() ? ch : false;
        }
    };

    template <typename>
    struct char_traits;

    template <>
    struct char_traits<char> : common_char_traits<char> {
        using char_type = char;
        using int_type = int;
        using off_type = std::streamoff;
        using pos_type = std::streampos;
        using state_type = std::mbstate_t;
        using size_type = std::size_t;
#if RAINY_HAS_CXX20
        using comparison_category = std::strong_ordering;
#endif

        static constexpr void assign(char_type &char_to, const char_type &char_from) noexcept {
            common_char_traits<char_type>::assign(char_to, char_from);
        }

        static RAINY_CONSTEXPR20 void assign(char_type *char_to, const size_type num, const char_type &char_from) {
            common_char_traits<char_type>::assign(char_to, num, char_from);
        }

        static RAINY_CONSTEXPR20 bool eq(const char_type left, const char_type right) noexcept {
            return left == right;
        }

        static RAINY_CONSTEXPR20 bool eq_int_type(const int_type &left, const int_type &right) noexcept {
            return left == right;
        }

        static RAINY_CONSTEXPR20 int compare(const char_type *string1, const char_type *string2, const size_type count) {
            return common_char_traits<char_type>::compare(string1, string2, count);
        }

        static RAINY_CONSTEXPR20 size_type length(const char_type *string) {
            return common_char_traits<char_type>::length(string);
        }

        RAINY_NODISCARD static RAINY_CONSTEXPR20 const char_type *find(const char_type *string, std::size_t count,
                                                                             const char_type &target) {
            return common_char_traits<char_type>::find(string, count, target);
        }

        static RAINY_CONSTEXPR20 bool lt(const char_type left, const char_type right) {
            return left < right;
        }

        static RAINY_CONSTEXPR20 char_type *move(char_type *to, const char_type *from, size_type count) {
            return common_char_traits<char_type>::move(to, from, count);
        }

        template <size_type N>
        static RAINY_CONSTEXPR20 char_type *move(std::array<char_type, N> &to, const char_type *from, const size_type count) {
            return common_char_traits<char_type>::move(to, from, count);
        }

        template <size_type N>
        static RAINY_CONSTEXPR20 char_type *move(foundation::containers::array<char_type, N> &to, const char_type *from,
                                                    const size_type count) {
            return common_char_traits<char_type>::move(to, from, count);
        }

        static RAINY_CONSTEXPR20 char_type *move(foundation::containers::array_view<char_type> &to, const char_type *from,
                                                    const size_type count) {
            return common_char_traits<char_type>::move(to, from, count);
        }

        static RAINY_CONSTEXPR20 char_type *move(std::vector<char_type> &to, const char_type *from, const size_type count) {
            return common_char_traits<char_type>::move(to, from, count);
        }

        template <size_type N>
        static RAINY_CONSTEXPR20 char_type *move_s(char_type (&to)[N], const char_type *from, const size_type count) {
            return common_char_traits<char_type>::move_s(to, from, count);
        }

        static RAINY_CONSTEXPR20 char_type *move_s(char_type *dest, const size_type dest_size, const char_type *from,
                                                      const size_type count) {
            if (dest_size < count) {
                return nullptr;
            }
            return move(dest, from, count);
        }

        static constexpr int_type to_int_type(const int_type &ch) {
            return ch;
        }

        static constexpr char_type to_char_type(const int_type &ch) {
            return static_cast<char_type>(ch);
        }

        static constexpr int_type eof() {
            return EOF;
        }

        static constexpr int_type not_eof(const int_type &ch) {
            return ch != eof() ? ch : false;
        }
    };

    template <>
    struct char_traits<wchar_t> : common_char_traits<wchar_t> {
        using char_type = wchar_t;
        using int_type = std::wint_t;
        using off_type = std::streamoff;
        using pos_type = std::streampos;
        using state_type = std::mbstate_t;
        using size_type = std::size_t;
#if RAINY_HAS_CXX20
        using comparison_category = std::strong_ordering;
#endif

        static constexpr void assign(char_type &char_to, const char_type &char_from) noexcept {
            common_char_traits<char_type>::assign(char_to, char_from);
        }

        static RAINY_CONSTEXPR20 void assign(char_type *char_to, const size_type num, const char_type &char_from) {
            common_char_traits<char_type>::assign(char_to, num, char_from);
        }

        static RAINY_CONSTEXPR20 bool eq(const char_type left, const char_type right) noexcept {
            return left == right;
        }

        static RAINY_CONSTEXPR20 bool eq_int_type(const int_type &left, const int_type &right) noexcept {
            return left == right;
        }

        static RAINY_CONSTEXPR20 int compare(const char_type *string1, const char_type *string2, const size_type count) {
            return common_char_traits<char_type>::compare(string1, string2, count);
        }

        static RAINY_CONSTEXPR20 size_type length(const char_type *string) {
            return common_char_traits<char_type>::length(string);
        }

        RAINY_NODISCARD static RAINY_CONSTEXPR20 const char_type *find(const char_type *string, std::size_t count,
                                                                             const char_type &target) {
            return common_char_traits<char_type>::find(string, count, target);
        }

        static RAINY_CONSTEXPR20 bool lt(const char_type left, const char_type right) {
            return left < right;
        }

        static RAINY_CONSTEXPR20 char_type *move(char_type *to, const char_type *from, size_type count) {
            return common_char_traits<char_type>::move(to, from, count);
        }

        template <size_type N>
        static RAINY_CONSTEXPR20 char_type *move(std::array<char_type, N> &to, const char_type *from, const size_type count) {
            return common_char_traits<char_type>::move(to, from, count);
        }

        template <size_type N>
        static RAINY_CONSTEXPR20 char_type *move(foundation::containers::array<char_type, N> &to, const char_type *from,
                                                    const size_type count) {
            return common_char_traits<char_type>::move(to, from, count);
        }

        static RAINY_CONSTEXPR20 char_type *move(foundation::containers::array_view<char_type> &to, const char_type *from,
                                                    const size_type count) {
            return common_char_traits<char_type>::move(to, from, count);
        }

        static RAINY_CONSTEXPR20 char_type *move(std::vector<char_type> &to, const char_type *from, const size_type count) {
            return common_char_traits<char_type>::move(to, from, count);
        }

        template <size_type N>
        static RAINY_CONSTEXPR20 char_type *move_s(char_type (&to)[N], const char_type *from, const size_type count) {
            return common_char_traits<char_type>::move_s(to, from, count);
        }

        static RAINY_CONSTEXPR20 char_type *move_s(char_type *dest, const size_type dest_size, const char_type *from,
                                                      const size_type count) {
            if (dest_size < count) {
                return nullptr;
            }
            return move(dest, from, count);
        }

        static constexpr int_type to_int_type(const int_type &ch) {
            return ch;
        }

        static constexpr char_type to_char_type(const int_type &ch) {
            return ch;
        }

        static constexpr int_type eof() {
            return EOF;
        }

        static constexpr int_type not_eof(const int_type &ch) {
            return ch != eof() ? ch : false;
        }
    };

    template <typename Elem, typename Traits = char_traits<Elem>>
    class basic_string_view {
    public:
        using value_type = Elem;
        using pointer = const value_type *;
        using const_pointer = const value_type *;
        using size_type = std::size_t;
        using iterator = utility::iterator<pointer, basic_string_view<Elem>>;
        using reference = const value_type &;
        using const_reference = const value_type &;
        using traits_type = Traits;

        static inline constexpr size_type npos = static_cast<size_type>(-1);

        constexpr basic_string_view() noexcept : view_data(nullptr){};

        RAINY_CONSTEXPR20 basic_string_view(const value_type *string) noexcept :
            view_data(string), view_size(traits_type::length(string)) {
        }

        RAINY_CONSTEXPR20 basic_string_view(const std::basic_string<Elem> &stdstring) noexcept :
            view_data(stdstring.c_str()), view_size(stdstring.size()) {
        }

        constexpr basic_string_view(const value_type *string, const size_type size) noexcept : view_data(string), view_size(size) {
        }

        template <std::size_t N>
        constexpr basic_string_view(const value_type (&string)[N]) noexcept {
        }

        RAINY_NODISCARD constexpr size_type size() const noexcept {
            return view_size;
        }

        RAINY_NODISCARD constexpr size_type length() const noexcept {
            return view_size;
        }

        RAINY_NODISCARD constexpr bool empty() const noexcept {
            return view_size == 0;
        }

        RAINY_NODISCARD constexpr pointer data() const noexcept {
            return view_data;
        }

        RAINY_NODISCARD constexpr basic_string_view substr(const size_type offset = 0, size_type count = npos) const {
            range_check(offset);
            count = clamp_suffix_size(offset, count);
            return basic_string_view(view_data + offset, count);
        }

        RAINY_NODISCARD constexpr const_reference operator[](const std::size_t idx) const {
            if (empty()) {
                std::terminate();
            }
            range_check(idx);
            return view_data[idx];
        }

        RAINY_NODISCARD constexpr const_reference at(const std::size_t idx) const {
            if (empty()) {
                std::terminate();
            }
            range_check(idx);
            return view_data[idx];
        }

        template <typename... Args>
        std::basic_string<value_type> make_format(Args... fmt_args) const {
            std::basic_string<value_type> buffer;
#if RAINY_HAS_CXX20
            utility::basic_format(buffer, std::basic_string_view{view_data, view_size}, fmt_args...);
#else
            utility::cstyle_format(buffer, view_data, fmt_args...);
#endif
            return buffer;
        }

        template <typename Elem_, typename Uty>
        friend std::basic_ostream<Elem_> &operator<<(std::basic_ostream<Elem_> &ostream, const basic_string_view<Uty> &right) {
            ostream.write(right.view_data, right.view_size);
            return ostream;
        }

    private:
        RAINY_NODISCARD constexpr size_type clamp_suffix_size(const size_type offset, const size_type count) const noexcept {
            return (std::min)(count, view_size - offset);
        }


        constexpr void range_check(const std::size_t idx) const {
            if (view_size <= idx) {
                foundation::system::exceptions::logic::throw_out_of_range("Invalid subscript");
            }
        }

        pointer view_data{nullptr};
        size_type view_size{0};
    };

    using string_view = basic_string_view<char>;
    using wstring_view = basic_string_view<wchar_t>;
}

namespace rainy::foundation::system::output {
    namespace utils {
        RAINY_INLINE std::shared_mutex &get_mtx() {
            static std::shared_mutex lock;
            return lock;
        }
    }

    template <typename... Args>
    std::string make_sstream_string(Args... args) {
        std::stringstream stream;
        ((stream << args), ...);
        return stream.str();
    }

    template <typename... Args>
    void stdout_print(Args... args) {
        std::string output_string = make_sstream_string(args...);
        (void)std::fwrite(output_string.c_str(), sizeof(char), output_string.size(), stdout);
    }

    template <typename... Args>
    void stderr_print(Args... args) {
        std::string output_string = make_sstream_string(args...);
        (void)std::fwrite(output_string.c_str(), sizeof(char), output_string.size(), stderr);
    }

    template <typename... Args>
    void stdout_println(Args... args) {
        std::string output_string = make_sstream_string(args...) + "\n";
        (void)std::fwrite(output_string.c_str(), sizeof(char), output_string.size(), stdout);
    }

    template <typename... Args>
    void stderr_println(Args... args) {
        std::string output_string = make_sstream_string(args...) + "\n";
        (void)std::fwrite(output_string.c_str(), sizeof(char), output_string.size(), stderr);
    }

    template <typename... Args>
    void ts_stdout_print(Args... args) {
        std::lock_guard<std::shared_mutex> lock(utils::get_mtx());
        stdout_print(args...);
    }

    template <typename... Args>
    void ts_stderr_print(Args... args) {
        std::lock_guard<std::shared_mutex> lock(utils::get_mtx());
        stderr_print(args...);
    }

    template <typename... Args>
    void ts_stdout_println(Args... args) {
        std::lock_guard<std::shared_mutex> lock(utils::get_mtx());
        stdout_println(args...);
    }

    template <typename... Args>
    void ts_stderr_println(Args... args) {
        std::lock_guard<std::shared_mutex> lock(utils::get_mtx());
        stderr_println(args...);
    }
}

#if RAINY_USING_WINDOWS
namespace rainy::winapi::error_process {
    template <typename CharType>
    RAINY_NODISCARD std::basic_string<CharType> last_error_message(DWORD error_code = 0) {
        if (error_code == 0) {
            error_code = GetLastError();
        }
        static CharType static_buffer[2048];
#if RAINY_USING_MSVC
#pragma warning(push)
#pragma warning(disable : 6387)
        if constexpr (foundation::type_traits::internals::_is_same_v<CharType, wchar_t>) {
            DWORD buffer_len =
                FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, error_code,
                               MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), static_cast<LPWSTR>(static_buffer), 2048, nullptr);
            if (buffer_len == 0) {
                return L"error in format_message";
            }
        } else {
            DWORD buffer_len =
                FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, error_code,
                               MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), static_cast<LPSTR>(static_buffer), 2048, nullptr);
            if (buffer_len == 0) {
                return "error in format_message";
            }
        }
#pragma warning(pop)
#endif
        std::basic_string<CharType> message = static_buffer;
        return message;
    }
}

namespace rainy::winapi::dynamic_libray {
    template <typename CharType = char, foundation::type_traits::other_transformations::enable_if_t<
                                            foundation::type_traits::type_relations::is_any_of_v<CharType, char, wchar_t>, int> = 0>
    HMODULE get_module_handle(const CharType *module_name = nullptr) {
        if constexpr (foundation::type_traits::helper::is_wchar_t<CharType>) {
            return GetModuleHandleW(module_name);
        } else {
            return GetModuleHandleA(module_name);
        }
    }
}

namespace rainy::utility {
    template <typename Ty>
    class reference_wrapper {
    public:
        static_assert(foundation::type_traits::internals::_is_object_v<Ty> || foundation::type_traits::internals::_is_function_v<Ty>,
                      "reference_wrapper<T> requires T to be an object type or a function type.");

        using type = Ty;

        template <typename Uty>
        RAINY_CONSTEXPR20 reference_wrapper(Uty &&val) noexcept {
            Ty &ref = static_cast<Uty &&>(val);
            reference_data = rainy::utility::addressof(ref);
        }

        reference_wrapper(const reference_wrapper &) = delete;
        reference_wrapper(reference_wrapper &&) = delete;

        RAINY_CONSTEXPR20 operator Ty &() const noexcept {
            return *reference_data;
        }

        RAINY_NODISCARD RAINY_CONSTEXPR20 Ty &get() const noexcept {
            return *reference_data;
        }

        template <typename Elem, typename Uty>
        friend std::basic_ostream<Elem> &operator<<(std::basic_ostream<Elem> &ostream, const reference_wrapper<Uty> &ref_wrap) {
            ostream << ref_wrap.get();
            return ostream;
        }

    private:
        Ty *reference_data{};
    };
}
#endif

#if RAINY_HAS_CXX20
template <typename Ty>
class std::formatter<rainy::utility::reference_wrapper<Ty>,char> // NOLINT
{
public:
    explicit formatter() noexcept = default;

    auto parse(format_parse_context &ctx) const noexcept {
        return ctx.begin();
    }

    auto format(const rainy::utility::reference_wrapper<Ty> &value,std::format_context fc) const noexcept {
        return std::format_to(fc.out(), "{}", value.get());
    }
};
#endif

namespace rainy::utility {
    template <typename Fx>
    class mem_fn_impl;

    template <typename Rx, typename Class, typename... Args>
    class mem_fn_impl<Rx (Class::*)(Args...)> {
    public:
        using return_type = Rx;
        using class_type = Class;
        using pointer = return_type (class_type::*)(Args...);

        mem_fn_impl() = delete;

        mem_fn_impl(Rx (Class::*mem_fn)(Args...)) noexcept : fn(mem_fn) {
        }

        return_type invoke(class_type &object, Args &&...args) const {
            return (object.*fn)(std::forward<Args>(args)...);
        }

        return_type operator()(class_type &object, Args &&...args) const {
            return (object.*fn)(std::forward<Args>(args)...);
        }

        RAINY_NODISCARD std::type_info &type() const noexcept {
            return get_typeid<pointer>();
        }

        RAINY_NODISCARD std::size_t arity() const noexcept {
            return sizeof...(Args);
        }

    private:
        union {
            pointer fn;
            std::max_align_t dummy;
        };
    };

    template <typename Rx, typename Class, typename... Args>
    class mem_fn_impl<Rx (Class::*)(Args...) const> {
    public:
        using return_type = Rx;
        using class_type = Class;
        using pointer = return_type (class_type::*)(Args...) const;

        mem_fn_impl() = delete;

        mem_fn_impl(Rx (Class::*mem_fn)(Args...) const) noexcept : fn(mem_fn) {
        }

        return_type invoke(const class_type &object, Args &&...args) const {
            return (object.*fn)(std::forward<Args>(args)...);
        }

        return_type operator()(class_type &object, Args &&...args) const {
            return (object.*fn)(std::forward<Args>(args)...);
        }

        RAINY_NODISCARD std::type_info &type() const noexcept {
            return get_typeid<pointer>();
        }

        RAINY_NODISCARD std::size_t arity() const noexcept {
            return sizeof...(Args);
        }

    private:
        union {
            pointer fn;
            std::max_align_t dummy;
        };
    };

    template <typename Rx, typename Class, typename... Args>
    auto mem_fn(Rx (Class::*memptr)(Args...)) -> mem_fn_impl<Rx (Class::*)(Args...)> {
        return {memptr};
    }

    template <typename Rx, typename Class, typename... Args>
    auto mem_fn(Rx (Class::*memptr)(Args...) const) -> mem_fn_impl<Rx (Class::*)(Args...) const> {
        return {memptr};
    }
}

namespace rainy::algorithm::execution {
    enum policy {
        par,
        seq,
        max_par
    };
}

namespace rainy::algorithm::internals {
    RAINY_INLINE std::size_t get_paralells(const std::size_t count) {
        std::size_t paralells = 1;
        foundation::containers::array<std::pair<std::size_t, std::size_t>, 4> thresholds = {
            {{75, 100}, {100, 250}, {300, 648}, {800, 1500}}};
        foundation::containers::array<std::size_t, 4> increments = {1, 2, 3, 4};
        for (std::size_t i = 0; i < thresholds.size(); ++i) {
            if (count > thresholds[static_cast<ptrdiff_t>(i)].first && count <= thresholds[static_cast<ptrdiff_t>(i)].second) {
                paralells += increments[static_cast<ptrdiff_t>(i)];
                break;
            }
        }
        if (count > 2500) {
            paralells += 2;
        }
        if (information::max_threads < 16 && count > 2000) {
            paralells += 1;
        } else if (information::max_threads > 16 && count > 1250) {
            if (count > 3500) {
                paralells += 2;
            } else if (count > 2000) {
                paralells += 1;
            }
        }
        paralells = (std::min)(information::max_threads, paralells);
        return paralells;
    }
}

namespace rainy::algorithm::container_operater {
    template <typename InputIter, typename OutIter>
    RAINY_CONSTEXPR20 OutIter copy(InputIter begin, InputIter end, OutIter dest) noexcept(
        std::is_nothrow_copy_constructible_v<foundation::type_traits::other_transformations::conditional_t<
            foundation::type_traits::internals::_is_pointer_v<InputIter>,
            foundation::type_traits::pointer_modify::remove_pointer_t<InputIter>, typename InputIter::value_type>>) {
        using value_type = typename InputIter::value_type;
        if (begin == end || (end - 1) == begin) {
            return dest; // 不进行复制
        }
#if RAINY_HAS_CXX20
        if (std::is_constant_evaluated()) {
            auto input_begin = utility::addressof(*begin);
            auto input_end = utility::addressof(*(end - 1)) + 1; // 防止MSVC编译器的DEBUG功能导致此处无法运作
            auto out_dest = utility::addressof(*dest);
            for (auto i = input_begin; i != input_end; ++i) {
                *out_dest = *i;
            }
            return out_dest;
        }
#endif
        if constexpr (std::is_standard_layout_v<value_type> && std::is_trivial_v<value_type>) {
            const auto input_begin = utility::addressof(*begin);
            const auto input_end = utility::addressof(*(end - 1)) + 1; // 防止MSVC编译器的DEBUG功能导致此处无法运作
            auto out_dest = utility::addressof(*dest);
            std::memcpy(out_dest, input_begin, sizeof(value_type) * utility::distance(input_begin, input_end));
        } else {
            for (InputIter i = begin; begin != end; ++i, ++dest) {
                *dest = *i;
            }
        }
        return dest;
    }

    template <typename InputIter, typename OutIter>
    RAINY_CONSTEXPR20 OutIter copy(execution::policy policy, InputIter begin, InputIter end, OutIter dest) {
        using value_type = typename InputIter::value_type;
        if (policy == execution::seq) {
            return copy(begin, end, dest);
        }
#if RAINY_HAS_CXX20
        if (std::is_constant_evaluated()) {
            return copy(begin, end, dest);
        }
#endif

        auto input_begin = utility::addressof(*begin);
        const std::size_t count = utility::distance(begin, end);
        std::size_t threads = internals::get_paralells(count);
        if (policy == execution::max_par) {
            threads = information::max_threads;
        } else {
            if (threads == 1) {
                return copy(begin, end, dest);
            }
        }
        std::size_t chunk_size = count / threads;
        std::size_t remainder = count % threads;
        std::vector<std::future<void>> tasks(threads);
        InputIter chunk_start = begin;
        OutIter chunk_dest = dest;
        for (std::size_t i = 0; i < threads; ++i) {
            std::size_t current_chunk_size = chunk_size + (i < remainder ? 1 : 0);
            tasks.emplace_back(std::async(std::launch::async, [chunk_start, chunk_dest, current_chunk_size]() {
                copy_n(chunk_start, current_chunk_size, chunk_dest);
            }));
            std::advance(chunk_start, current_chunk_size);
            std::advance(chunk_dest, current_chunk_size);
        }
        for (auto &task: tasks) {
            task.get();
        }
        return chunk_dest;
    }

    template <typename InputIter, typename OutIter>
    RAINY_CONSTEXPR20 OutIter copy_n(InputIter begin, const std::size_t count, OutIter dest) noexcept(
        std::is_nothrow_copy_constructible_v<foundation::type_traits::other_transformations::conditional_t<
            foundation::type_traits::internals::_is_pointer_v<InputIter>,
            foundation::type_traits::pointer_modify::remove_pointer_t<InputIter>, typename InputIter::value_type>>) {

        using value_type = foundation::type_traits::other_transformations::conditional_t<
            foundation::type_traits::internals::_is_pointer_v<InputIter>,
            foundation::type_traits::pointer_modify::remove_pointer_t<InputIter>, typename InputIter::value_type>;

        if (count == 0) {
            return dest; // 不进行复制
        }
#if RAINY_HAS_CXX20
        if (std::is_constant_evaluated()) {
            // 在C++20中，如果我们在常量环境求值，我们不需要考虑过多运行时优化
            auto input_begin = utility::addressof(*begin);
            auto out_dest = utility::addressof(*dest);
            for (std::size_t i = 0; i < count; ++i, ++input_begin, ++dest) {
                *out_dest = *input_begin;
            }
        } else
#endif
        {
            if constexpr (std::is_standard_layout_v<value_type> && std::is_trivial_v<value_type>) {
                const auto input_begin = utility::addressof(*begin);
                auto out_dest = utility::addressof(*dest);
                std::memcpy(out_dest, input_begin, sizeof(value_type) * count);
            } else {
                for (std::size_t i = 0; i < count; ++i, ++begin, ++dest) {
                    *dest = *begin;
                }
            }
        }
        return dest;
    }

    template <typename InputIter, typename OutIter>
    RAINY_CONSTEXPR20 OutIter copy_n(execution::policy policy, InputIter begin, const std::size_t count, OutIter dest) {
        using value_type = typename InputIter::value_type;
        if (policy == execution::seq) {
            return copy_n(begin, count, dest);
        }
#if RAINY_HAS_CXX20
        if (std::is_constant_evaluated()) {
            return copy_n(begin, count, dest);
        }
#endif
        auto input_begin = utility::addressof(*begin);
        std::size_t threads = internals::get_paralells(count); // 获取可能的规模并计算出可能适合的线程数量（考虑最大线程数）
        if (policy == execution::max_par) {
            threads = information::max_threads;
        } else {
            if (threads == 1) {
                // 线程数若为1，则只调用单线程版本
                return copy_n(begin, count, dest);
            }
        }
        std::size_t chunk_size = count / threads;
        std::size_t remainder = count % threads;
        std::vector<std::future<void>> tasks(threads); // 创建task表
        InputIter chunk_start = begin;
        OutIter chunk_dest = dest;
        for (std::size_t i = 0; i < threads; ++i) {
            std::size_t current_chunk_size = chunk_size + (i < remainder ? 1 : 0);
            tasks.emplace_back(std::async(std::launch::async, [chunk_start, chunk_dest, current_chunk_size]() {
                copy_n(chunk_start, current_chunk_size, chunk_dest);
            }));
            // 让std::async为我们分配工作线程（不考虑返回值）
            std::advance(chunk_start, current_chunk_size);
            std::advance(chunk_dest, current_chunk_size);
        }
        for (auto &task: tasks) {
            task.get();
        }
        return chunk_dest;
    }


    template <typename InputIter, typename OutIter, typename Fx>
    constexpr OutIter transform(InputIter begin, InputIter end, OutIter dest, Fx func) noexcept(
        std::is_nothrow_copy_assignable_v<foundation::type_traits::other_transformations::conditional_t<
            foundation::type_traits::internals::_is_pointer_v<InputIter>,
            foundation::type_traits::pointer_modify::remove_pointer_t<InputIter>, typename InputIter::value_type>>) {
        if (begin == end || (end - 1) == begin) {
            return dest;
        }
        for (InputIter iter = begin; iter != end; ++iter, ++dest) {
            *dest = func(*iter);
        }
        return dest;
    }

    template <typename InputIter, typename OutIter, typename Fx>
    constexpr OutIter transform(InputIter begin1, InputIter end1, InputIter begin2, OutIter dest, Fx func) noexcept(
        std::is_nothrow_copy_assignable_v<foundation::type_traits::other_transformations::conditional_t<
            foundation::type_traits::internals::_is_pointer_v<InputIter>,
            foundation::type_traits::pointer_modify::remove_pointer_t<InputIter>, typename InputIter::value_type>>) {
        if (begin1 == end1 || (end1 - 1) == begin1) {
            return dest;
        }
        for (InputIter iter = begin1; iter != end1; ++iter, ++dest, ++begin2) {
            *dest = func(*iter, *begin2);
        }
        return dest;
    }
}

namespace rainy::algorithm::ranges::container_operater {
    template <typename InputContainer, typename OutContainer>
    RAINY_CONSTEXPR20 auto copy(InputContainer &container,
                                   OutContainer dest) noexcept(std::is_nothrow_copy_constructible_v<typename InputContainer::value_type>) {
        return algorithm::container_operater::copy(container.begin(), container.end(), dest.begin());
    }

    template <typename InputContainer, typename OutContainer>
    RAINY_CONSTEXPR20 auto copy(execution::policy policy, InputContainer &container, OutContainer dest) {
        return algorithm::container_operater::copy(policy, container.begin(), container.end(), dest.begin());
    }

    template <typename InputContainer, typename OutContainer>
    RAINY_CONSTEXPR20 auto copy_n(InputContainer &container, const std::size_t count, OutContainer dest) noexcept(
        std::is_nothrow_copy_constructible_v<typename InputContainer::value_type>) {
        return algorithm::container_operater::copy_n(container.begin(), count, dest.begin());
    }

    template <typename InputContainer, typename OutContainer>
    RAINY_CONSTEXPR20 auto copy_n(execution::policy policy, InputContainer &container, const std::size_t count, OutContainer dest) {
        return algorithm::container_operater::copy_n(policy, container.begin(), count, dest.begin());
    }
}

/* 为rainy::utility::type_index进行一个hash特化 */
template <>
struct std::hash<rainy::utility::type_index> {
    using argument_type RAINY_CXX17_DEPRECATED_TYPEDEF = rainy::utility::type_index;
    using result_type RAINY_CXX17_DEPRECATED_TYPEDEF = std::size_t;

    RAINY_NODISCARD size_t operator()(const rainy::utility::type_index &val) const noexcept {
        return val.hash_code();
    }
};

namespace rainy::component::sync_event {
    class event {
    public:
        virtual ~event() = default;

        virtual const std::type_info &type_info() {
            return typeid(*this);
        }
    };

    template <typename Derived>
    class event_handler {
    public:
        event_handler() {
            static_assert(std::is_base_of_v<event, Derived>);
        }

        virtual ~event_handler() = default;

        virtual void on(event &user_event) = 0;

        RAINY_NODISCARD std::string_view handler_name() const noexcept {
            return typeid(*this).name();
        }

        RAINY_NODISCARD std::size_t handler_hash_code() const noexcept {
            return typeid(*this).hash_code();
        }

        void dispatch(event &user_event) {
            on(dynamic_cast<Derived &>(user_event));
        }
    };

    class handler_registration {
    public:
        virtual ~handler_registration() = default;

        virtual void remove_handler() = 0;
        RAINY_NODISCARD virtual std::string_view name() const noexcept = 0;
        RAINY_NODISCARD virtual std::size_t hash_code() const noexcept = 0;
        virtual void invoke_dispatch(event &) const = 0;
    };

    class dispatcher {
    public:
        static dispatcher *instance() {
            static dispatcher instance;
            return &instance;
        }

        ~dispatcher() {
            clear();
        }

        void clear() {
            handlers.clear();
        }

        template <typename EventType, typename ListenerType, typename... Args>
        RAINY_NODISCARD std::shared_ptr<handler_registration> subscribe(Args &&...args) {
            return subscribe<EventType>(std::make_shared<ListenerType>(utility::forward<Args>(args)...));
        }

        template <typename EventType>
        RAINY_NODISCARD std::shared_ptr<handler_registration> subscribe(std::shared_ptr<event_handler<EventType>> handler) {
            auto &registrations_instance = handlers[typeid(EventType)];
            if (!registrations_instance) {
                registrations_instance = std::make_shared<registrations>();
            }
            auto registration_instance = std::make_shared<registration>(registrations_instance);
            registration_instance->set<EventType>(handler);
            registrations_instance->emplace_back(registration_instance);
            return registration_instance;
        }

        template <typename EventType, typename Fx,
                  foundation::type_traits::other_transformations::enable_if_t<
                      foundation::type_traits::type_properties::is_invocable_r_v<void, Fx, event &>, int> = 0>
        RAINY_NODISCARD std::shared_ptr<handler_registration> subscribe(Fx &&func) {
            /* 我们实际创建了一个virtual_listener实例，表示虚拟的监听器 */
            struct virtual_listener : public event_handler<EventType>, Fx {
                virtual_listener(Fx &&func) : Fx(std::forward<Fx>(func)) {
                }

                void on(event &user_event) override {
                    (*this)(user_event);
                }
            };
            return subscribe<EventType, virtual_listener>(std::forward<Fx>(func));
        }

        template <typename EventType>
        RAINY_NODISCARD std::shared_ptr<handler_registration> subscribe(
            foundation::functional::function_pointer<void(event &)> fptr) {
            struct virtual_listener : public event_handler<EventType> {
                virtual_listener(foundation::functional::function_pointer<void(event &)> fptr) : fptr(std::move(fptr)) {
                }

                void on(event &user_event) override {
                    fptr(user_event);
                    anti_jesus(); // 防止静态工具的const建议
                }

                void anti_jesus() {
                    jesus = 666;
                }

                foundation::functional::function_pointer<void(event &)> fptr;
                int jesus{33};
            };
            return subscribe<EventType, virtual_listener>(fptr);
        }

        template <typename EventType>
        void for_each_in_handlers(foundation::functional::function_pointer<void(event_handler<EventType> *)> pred) const {
            const auto it = handlers.find(typeid(EventType));
            if (it == handlers.end()) {
                return;
            }
            const auto &registrations_instance = it->second;
            if (!registrations_instance || registrations_instance->empty()) {
                return;
            }
            for (const auto &handler: *registrations_instance) {
                pred(static_cast<event_handler<EventType> *>(handler->get_handler()));
            }
        }

        template <typename EventType>
        void publish(EventType &event) const {
            const auto it = handlers.find(typeid(EventType));
            if (it == handlers.end()) {
                return;
            }
            const auto &registrations_instance = it->second;
            if (!registrations_instance || registrations_instance->empty()) {
                return;
            }
            for (const auto &handler: *registrations_instance) {
                try {
                    static_cast<event_handler<EventType> *>(handler->get_handler())->dispatch(event);
                } catch (foundation::system::exceptions::exception &RAINY_exception) {
                    foundation::system::output::stderr_println(
                        "found a error in :" RAINY_STRINGIZE(rainy::component::sync_event::dispatcher::publish),
                        "with addtional information from RAINY_exception: ", RAINY_exception.what());
                    utility::throw_exception(RAINY_exception);
                } catch (std::exception &e) {
                    foundation::system::output::stderr_println(
                        "found a error in :" RAINY_STRINGIZE(rainy::component::sync_event::dispatcher::publish),
                        "with addtional information from std::exception: ", e.what());
                    utility::throw_exception(utility::stdexcept_to_jexcept(e));
                }
            }
        }

        template <typename EventType>
        void publish_noexcept(EventType &event) const noexcept {
            const auto it = handlers.find(typeid(EventType));
            if (it == handlers.end()) {
                return;
            }
            const auto &registrations_instance = it->second;
            if (!registrations_instance || registrations_instance->empty()) {
                return;
            }
            for (const auto &handler: *registrations_instance) {
                handler->invoke_dispatch(event);
            }
        }

    private:
        class registration;

        using registrations = std::list<std::shared_ptr<registration>>;
        using type_mapping = std::unordered_map<utility::type_index, std::shared_ptr<registrations>>;

        class registration : public handler_registration, public std::enable_shared_from_this<registration> {
        public:
            registration(std::shared_ptr<registrations> registrations) :
                registrations(utility::move(registrations)) {
            }

            void remove_handler() override {
                if (registered) {
                    auto self = shared_from_this();
                    registrations->remove_if([self](const std::shared_ptr<registration> &reg) { return reg == self; });
                    registered = false;
                }
            }

            RAINY_NODISCARD std::string_view name() const noexcept override {
                return instance->name();
            }

            RAINY_NODISCARD std::size_t hash_code() const noexcept override {
                return instance->hash_code();
            }

            void invoke_dispatch(event &user_event) const noexcept override {
                return instance->invoke(user_event);
            }

            void *get_handler() {
                return instance->get_handler();
            }

            template <typename EventType>
            void set(std::shared_ptr<event_handler<EventType>> handler) {
                instance = std::make_unique<impl<EventType>>(handler);
            }

        private:
            struct resource {
                virtual ~resource() = default;
                virtual void *get_handler() = 0;
                RAINY_NODISCARD virtual std::string_view name() const noexcept = 0;
                RAINY_NODISCARD virtual std::size_t hash_code() const noexcept = 0;
                virtual void invoke(event &) const = 0;
            };

            template <typename EventType>
            struct impl : resource {
                impl(std::shared_ptr<event_handler<EventType>> handler) : handler(std::move(handler)) {
                }

                void *get_handler() override {
                    return handler.get();
                }

                RAINY_NODISCARD std::string_view name() const noexcept override {
                    return handler->handler_name();
                }

                RAINY_NODISCARD std::size_t hash_code() const noexcept override {
                    return handler->handler_hash_code();
                }

                void invoke(event &user_event) const override {
                    handler->dispatch(user_event);
                }

                std::shared_ptr<event_handler<EventType>> handler;
            };

            std::shared_ptr<registrations> registrations;
            bool registered{true};
            std::unique_ptr<resource> instance;
        };

        dispatcher() {
            handlers.reserve(16);
        }

        type_mapping handlers;
    };
}

/*
comint是仿制Microsoft中组件对象模型（COM）的一种接口规范
采用shared_ptr避免直接管理引用计数，用于增强内存安全性，同时定义一套适用于现代C++的规范
*/
namespace rainy::foundation::comint {
    template <typename Interface>
    class comint_ptr;

    struct the_unknown {
        using the_unknown_ptr = std::shared_ptr<the_unknown>;

        virtual ~the_unknown() = default;

        RAINY_NODISCARD std::string_view name() const noexcept {
            return typeid(*this).name();
        }

        RAINY_NODISCARD std::size_t hash_code() const noexcept {
            return typeid(*this).hash_code();
        }

        /* 我们通过此接口，为某个实例复制或新实例化一个实例 */
        virtual void copy(the_unknown_ptr &ptr) = 0;
        /* 负责实现移动语义（不针对智能指针，而是对象，行为上是make_shared附上move后的参数） */
        virtual void move(the_unknown_ptr &ptr) noexcept = 0;
    };

    template  <typename Interface>
    class comint_ptr {
    public:
        comint_ptr() {
            static_assert(std::is_base_of_v<the_unknown, Interface>, "Interface must be a derived interface from the_unknown struct!");
        }

        comint_ptr(comint_ptr &&right) noexcept : pointer() {
            static_assert(std::is_base_of_v<the_unknown, Interface>, "Interface must be a derived interface from the_unknown struct!");
        }

        the_unknown &as_unknown() {
            if (!pointer) {
                system::exceptions::cast::throw_bad_cast("the internal pointer is null!");
            }
            rainy_let cast_ptr = dynamic_cast<the_unknown*>(pointer.get());
            if (!cast_ptr) {
                system::exceptions::cast::throw_bad_cast("failed to cast pointer to the_unknown");
            }
            return *cast_ptr;
        }

        the_unknown * as_unknown_ptr() {
            return &as_unknown();
        }

    private:
        std::shared_ptr<Interface> pointer;
    };

    class interface_table {
    public:
        static interface_table *instance() {
            static interface_table instance;
            return &instance;
        }

        template <typename InterfaceClass>
        void add_interface() {
            utility::type_index type_idx = typeid(InterfaceClass);
            const auto find = table.find(type_idx);
            if (find == table.end()) {
                table.insert({type_idx, &InterfaceClass::make_instance});
            }
        }

        template <typename InterfaceClass>
        void remove_interface() {
            utility::type_index type_idx = typeid(InterfaceClass);
            const auto find = table.find(type_idx);
            if (find != table.end()) {
                table.erase(find);
            }
        }

        template <typename InterfaceClass>
        RAINY_NODISCARD std::shared_ptr<comint::the_unknown> create_instance() {
            utility::type_index type_idx = typeid(InterfaceClass);
            const auto find = table.find(type_idx);
            if (find == table.end()) {
                return nullptr;
            }
            return find->second();
        }

    private:
        std::unordered_map<utility::type_index, foundation::functional::function_pointer<std::shared_ptr<comint::the_unknown>()>>
            table;
    };

    template <typename InterfaceClass>
    void add_interface() {
        interface_table::instance()->add_interface<InterfaceClass>();
    }

    template <typename InterfaceClass>
    RAINY_NODISCARD std::shared_ptr<InterfaceClass> create_instance() {
        return std::dynamic_pointer_cast<InterfaceClass>(interface_table::instance()->create_instance<InterfaceClass>());
    }

    template <typename InterfaceClass>
    void remove_interface() {
        interface_table::instance()->remove_interface<InterfaceClass>();
    }
}

#endif // RAINY_BASE_HPP

#ifndef RAINY_FOUNDATION_COMINT_MODEL
#define RAINY_FOUNDATION_COMINT_MODEL

#include <memory>
#include <optional>
#include <rainy/core/core.hpp>
#include <rainy/foundation/functional/function_pointer.hpp>
#include <rainy/foundation/typeinfo.hpp>
#include <unordered_map>

/*
comint是仿制Microsoft中组件对象模型（COM）的一种接口规范
采用shared_ptr避免直接管理引用计数，用于增强内存安全性，同时定义一套适用于现代C++的规范
*/
namespace rainy::foundation::comint {
    template <typename Interface>
    class comint_ptr;

    struct the_unknown {
        enum class constructor_call_type {
            default_ctor,
            move_ctor,
            copy_ctor,
            muti_args_ctor
        };

        using the_unknown_ptr = std::shared_ptr<the_unknown>;

        virtual ~the_unknown() = default;

        RAINY_NODISCARD std::string_view name() const noexcept {
            return typeid(*this).name();
        }

        RAINY_NODISCARD std::size_t hash_code() const noexcept {
            return typeid(*this).hash_code();
        }

        template <typename... Args>
        void construct(constructor_call_type call_type, Args&&... args) {
            auto tuple = std::make_tuple<Args...>(utility::forward<Args>(args)...);
            construct_impl(call_type, static_cast<void *>(&tuple));
        }

        /* 我们通过此接口，为某个实例复制或新实例化一个实例 */
        virtual void copy(the_unknown_ptr &ptr) = 0;
        /* 负责实现移动语义（不针对智能指针，而是对象，行为上是make_shared附上move后的参数） */
        virtual void move(the_unknown_ptr &ptr) noexcept = 0;
        /* 如果要实现构造，请重写该方法 */
        virtual void construct_impl(constructor_call_type, void*) {
        }
    };

    template <typename Interface>
    class comint_ptr {
    public:
        comint_ptr() {
            static_assert(type_traits::type_relations::is_base_of_v<the_unknown, Interface>, "Interface must be a derived interface from the_unknown struct!");
        }

        comint_ptr(comint_ptr &&right) noexcept : pointer(utility::exchange(right.pointer, nullptr)) {
            static_assert(type_traits::type_relations::is_base_of_v<the_unknown, Interface>,
                          "Interface must be a derived interface from the_unknown struct!");
        }

        std::optional<the_unknown*> as_unknown() {
            if (!pointer) {
                return std::nullopt;
            }
            return static_cast<the_unknown *>(pointer.get()); // 由静态安全保证
        }

        the_unknown *as_unknown_ptr() {
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
            rtti::typeinfo type_idx = rainy_typeid(InterfaceClass);
            const auto find = table.find(type_idx);
            if (find == table.end()) {
                table.insert({type_idx, &InterfaceClass::make_instance});
            }
        }

        template <typename InterfaceClass>
        void remove_interface() {
            rtti::typeinfo type_idx = rainy_typeid(InterfaceClass);
            const auto find = table.find(type_idx);
            if (find != table.end()) {
                table.erase(find);
            }
        }

        template <typename InterfaceClass>
        RAINY_NODISCARD std::shared_ptr<comint::the_unknown> create_instance() {
            rtti::typeinfo type_idx = rainy_typeid(InterfaceClass);
            const auto find = table.find(type_idx);
            if (find == table.end()) {
                return nullptr;
            }
            return find->second();
        }

    private:
        std::unordered_map<rtti::typeinfo, foundation::functional::function_pointer<std::shared_ptr<comint::the_unknown>()>> table;
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

#endif
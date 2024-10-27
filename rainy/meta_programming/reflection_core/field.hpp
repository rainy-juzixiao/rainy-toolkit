#ifndef rainy_refLECTION_CORE_FIELD_HPP
#define rainy_refLECTION_CORE_FIELD_HPP
/*
此文件提供了对字段的抽象。配合动态反射系统使用
*/
#include <rainy/base.hpp>
#include <rainy/containers/any.hpp>
#include <variant>

namespace rainy::foundation::reflection {
    class enum_field final {
    public:
        constexpr enum_field() = default;

        ~enum_field() = default;

        template <typename Enum>
        enum_field(const std::string_view name, const std::string_view type, Enum value) : _name(name), _type(type), _value(value) {
        }

        enum_field(const enum_field &) = default;

        enum_field(enum_field &&) = default;

        enum_field &operator=(const enum_field &) = default;
        enum_field &operator=(enum_field &&) = default;

        RAINY_NODISCARD std::string_view name() const noexcept {
            return _name;
        }

        RAINY_NODISCARD std::string_view type() const noexcept {
            return _type;
        }

        auto &value() noexcept {
            return _value;
        }

        const auto &value() const noexcept {
            return _value;
        }

    private:
        std::string_view _name{};
        std::string_view _type{};
        std::variant<int, unsigned int, long, unsigned long, long long, unsigned long long, float, double, char, unsigned char, long double,
                     short, unsigned short>
            _value{};
    };

    class attributes final {
    public:
        static const std::size_t not_found = static_cast<std::size_t>(-1);

        rainy_constEXPR20 attributes() noexcept = default;

        attributes(std::initializer_list<std::string_view> ilist) : meta_data(ilist) {
        }

        bool empty() const noexcept {
            return meta_data.size();
        }

        template <typename Fx, std::enable_if_t<std::is_invocable_r_v<void, Fx, const std::string_view &>>>
        void for_each(Fx &&callable_with_onearg) const noexcept {
            for (const std::string_view &meta: meta_data) {
                rainy::utility::invoke(callable_with_onearg, meta);
            }
        }

        std::string_view at(const std::size_t idx) const {
            return meta_data.at(idx);
        }

        std::string_view operator[](const std::size_t idx) const {
            return meta_data.at(idx);
        }

        std::size_t size() const noexcept {
            return meta_data.size();
        }

        std::size_t find(const std::string_view meta_info) const noexcept {
            const auto iter = std::find(meta_data.begin(), meta_data.end(), meta_info);
            if (iter == meta_data.end()) {
                return not_found;
            }
            return std::distance(meta_data.begin(), iter);
        }

        void add(const std::string_view meta_info) {
            if (find(meta_info) == not_found) {
                meta_data.emplace_back(meta_info);
            }
        }

        void remove(const std::string_view meta_info) {
            const auto iter = std::find(meta_data.begin(), meta_data.end(), meta_info);
            if (iter != meta_data.end()) {
                meta_data.erase(iter);
            }
        }

    private:
        std::vector<std::string_view> meta_data; // 保存多个元数据
    };

    class const_field {
    public:
        friend class field;

        using any = foundation::containers::any;

        const_field() = default;

        ~const_field() = default;

        template <typename Type, typename Class>
        const_field(const std::string_view name, const std::string_view type, Type Class::*memptr, const attributes &attrs) :
            _name(name), _type(type), _accessor(std::make_unique<accessor_impl<Type, Class, Type Class::*>>(memptr)), _attrs(attrs) {
        }

        template <typename Type, typename Class>
        const_field(const std::string_view name, const std::string_view type, Type *static_ptr, const attributes &attrs) :
            _name(name), _type(type), _accessor(std::make_unique<accessor_impl<Type, void, Type *>>(static_ptr)), _attrs(attrs) {
        }

        RAINY_NODISCARD any get(void *object) const noexcept {
            if (!object) {
                return {};
            }
            return _accessor->getter(object);
        }

        RAINY_NODISCARD std::string_view name() const noexcept {
            return _name;
        }

        RAINY_NODISCARD std::string_view type() const noexcept {
            return _type;
        }

        RAINY_NODISCARD bool is_const() const noexcept {
            return _accessor->is_const();
        }

        RAINY_NODISCARD bool is_static() const noexcept {
            return _accessor->is_static();
        }

        const attributes &attrs() const noexcept {
            return this->_attrs;
        }

    private:
        void anti_jesus() noexcept {
            // 此操作实际无意义，但是必须存在
            std::string_view *unused = &_name;
            unused->swap(_name);
        }
        
        struct accessor {
            virtual rainy::foundation::containers::any getter(const void *) = 0;
            virtual void setter(void *, const rainy::foundation::containers::any &) = 0;
            virtual bool is_const() const noexcept = 0;
            virtual bool is_static() const noexcept = 0;  
        };

        template <typename Ty, typename Class, typename RealPointer>
        struct accessor_impl : public accessor {
            accessor_impl(RealPointer pointer) : pointer(pointer) {
            }

            virtual rainy::foundation::containers::any getter(const void *obj) override {
                if constexpr (std::is_same_v<Class, void>) {
                    return rainy::foundation::containers::any(*pointer);
                } else {
                    return rainy::foundation::containers::any(static_cast<const Class *>(obj)->*pointer);
                }
            }

            virtual void setter(void *obj, const rainy::foundation::containers::any &value) override {
                if (typeid(Ty) != value.type() || !value.has_value()) {
                    // 没有值或类型不匹配的都会被视为无意义的操作
                    return;
                }
                if constexpr (!std::is_const_v<Ty>) {
                    if constexpr (std::is_void_v<Class>) {
                        // 我们默认把静态所属的类的归纳为void类型，以方便我们辨认
                        *pointer = rainy::utility::any_cast<Ty>(value);
                    } else {
                        static_cast<Class *>(obj)->*pointer = rainy::utility::any_cast<Ty>(value);
                    }
                }
            }

            bool is_const() const noexcept override {
                return std::is_const_v<Ty>;
            }

            bool is_static() const noexcept override {
                return !std::is_member_object_pointer_v<RealPointer>;
            }

            RealPointer pointer;
        };

        std::string_view _name;
        std::string_view _type;
        std::shared_ptr<accessor> _accessor{nullptr};
        attributes _attrs;
    };

    class field final : public const_field {
    public:
        friend class reflection;

        field() = default;

        ~field() = default;

        field(const field &) = default;

        template <typename Type, typename Class>
        field(const std::string_view name, const std::string_view type, Type Class::*memptr, const attributes &attr) :
            const_field(name, type, memptr, attr) {
        }

        template <typename Type, typename Class>
        field(const std::string_view name, const std::string_view type, Type *static_ptr, const attributes &attr) :
            const_field(name, type, type, static_ptr, attr) {
        }

        void set(void *object, const any &val) noexcept {
            if (!object) {
                return;
            }
            /*
            此处做无意义的操作。此处让静态分析工具认为无法作为const函数。
            为什么这么做？
            1. 防止静态分析的建议优化:
                静态分析工具有时会对代码进行优化，尤其是当它检测到某些代码看起来没有实际效果时。而在编译器层面，即使没有
                anti_jesus也不会造成多大影响。因为编译器尽可能保持较大的自由空间。
                此处通过调用此方法，我们可以确保这一函数永远不能被作为const函数，即使它对实际程序行为没有直接影响。
            2. 确保静态分析工具正确处理:
                静态分析工具虽然用于检查代码中的潜在问题，但是有时这些工具可能会产生误判。
                而anti_jesus()可以确保它们不会误判函数的行为或副作用，从而避免静态分析中的误报。
            3. 保持一致的函数语义:
                尽管anti_jesus()本身不对对象的状态产生实际变化（仅进行一个内部字段的自我交换，不会产生UB）
                但它的存在是确保函数内部的操作符合预期的逻辑要求。这有助于维持函数行为的逻辑一致性，哪怕编译器和工具在处理过程中可能会有额外的分析需求。
            */
            this->anti_jesus();
            _accessor->setter(object, val);
        }

        attributes &attrs() noexcept {
            return this->_attrs;
        }

        void rename(const std::string_view new_name) noexcept {
            // 重命名字段可能会导致系统并不一定能找到此字段。因为重命名后，需要使用新名字才能查找到此字段。
            // 此外，命名冲突还有可能导致意想不到的后果
            if (new_name != this->_name) {
                this->_name = new_name;
            }
        }
    };

    template <typename Class, typename Type>
    field make_field(std::string_view name, Type Class::*member, const attributes &attr = {}) {
        return field(name, typeid(Type).name(), member, attr);
    }

    template <typename, typename Type>
    field make_field(std::string_view name, Type *static_member, const attributes &attr = {}) {
        return field(name, typeid(Type).name(), static_member, attr);
    }
}
#endif
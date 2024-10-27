#ifndef rainy_refLECTION_CORE_FIELD_HPP
#define rainy_refLECTION_CORE_FIELD_HPP
/*
���ļ��ṩ�˶��ֶεĳ�����϶�̬����ϵͳʹ��
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
        std::vector<std::string_view> meta_data; // ������Ԫ����
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
            // �˲���ʵ�������壬���Ǳ������
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
                    // û��ֵ�����Ͳ�ƥ��Ķ��ᱻ��Ϊ������Ĳ���
                    return;
                }
                if constexpr (!std::is_const_v<Ty>) {
                    if constexpr (std::is_void_v<Class>) {
                        // ����Ĭ�ϰѾ�̬��������Ĺ���Ϊvoid���ͣ��Է������Ǳ���
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
            �˴���������Ĳ������˴��þ�̬����������Ϊ�޷���Ϊconst������
            Ϊʲô��ô����
            1. ��ֹ��̬�����Ľ����Ż�:
                ��̬����������ʱ��Դ�������Ż��������ǵ�����⵽ĳЩ���뿴����û��ʵ��Ч��ʱ�����ڱ��������棬��ʹû��
                anti_jesusҲ������ɶ��Ӱ�졣��Ϊ�����������ܱ��ֽϴ�����ɿռ䡣
                �˴�ͨ�����ô˷��������ǿ���ȷ����һ������Զ���ܱ���Ϊconst��������ʹ����ʵ�ʳ�����Ϊû��ֱ��Ӱ�졣
            2. ȷ����̬����������ȷ����:
                ��̬����������Ȼ���ڼ������е�Ǳ�����⣬������ʱ��Щ���߿��ܻ�������С�
                ��anti_jesus()����ȷ�����ǲ������к�������Ϊ�����ã��Ӷ����⾲̬�����е��󱨡�
            3. ����һ�µĺ�������:
                ����anti_jesus()�����Զ����״̬����ʵ�ʱ仯��������һ���ڲ��ֶε����ҽ������������UB��
                �����Ĵ�����ȷ�������ڲ��Ĳ�������Ԥ�ڵ��߼�Ҫ����������ά�ֺ�����Ϊ���߼�һ���ԣ����±������͹����ڴ�������п��ܻ��ж���ķ�������
            */
            this->anti_jesus();
            _accessor->setter(object, val);
        }

        attributes &attrs() noexcept {
            return this->_attrs;
        }

        void rename(const std::string_view new_name) noexcept {
            // �������ֶο��ܻᵼ��ϵͳ����һ�����ҵ����ֶΡ���Ϊ����������Ҫʹ�������ֲ��ܲ��ҵ����ֶΡ�
            // ���⣬������ͻ���п��ܵ������벻���ĺ��
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
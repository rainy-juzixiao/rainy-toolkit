#include <rainy/meta/reflection/type.hpp>

namespace rainy::meta::reflection {
    type type::get_by_name(const std::string_view name) noexcept {
        type instance{};
        instance.accessor = implements::register_table::get_accessor_by_name(name);
        return instance;
    }

    RAINY_NODISCARD std::string_view type::get_name() const noexcept {
        if (!accessor) {
            return {};
        }
        return accessor->name();
    }

    RAINY_NODISCARD type::type_id type::get_id() const noexcept {
        if (!accessor) {
            return 0;
        }
        return accessor->typeinfo().hash_code();
    }

    RAINY_NODISCARD std::size_t type::get_sizeof() const noexcept {
        if (!accessor) {
            return 0;
        }
        return accessor->typeinfo().get_sizeof();
    }

    RAINY_NODISCARD collections::views::array_view<foundation::ctti::typeinfo> type::get_template_arguments() const noexcept {
        if (!accessor) {
            return {};
        }
        return accessor->typeinfo().template_arguemnts();
    }

    RAINY_NODISCARD const method &type::get_method(const std::string_view name) const noexcept {
        static const method empty;
        if (!accessor) {
            return empty;
        }
        const auto &cont = accessor->methods();
        const auto [fst, snd] = cont.equal_range(name);
        if (fst != snd) {
            return fst->second;
        }
        for (const auto &bases: accessor->bases()) {
            // 我们将检查父类是否存有目标的注册代码，然后进行深度优先的递归查找，直到找到目标
            if (const method &meth = bases.second.get_method(name); !meth.empty()) {
                return meth;
            }
        }
        return empty;
    }

    RAINY_NODISCARD const method &type::get_method(
        const std::string_view name, const collections::views::array_view<foundation::ctti::typeinfo> overload_version_paramlist,
        const method_flags filter_item) const noexcept {
        static const method empty;
        if (!accessor) {
            return empty;
        }
        static const auto match_method_type = [](method_flags candidate, method_flags filter) -> bool {
            if (filter == method_flags::none) {
                return true;
            }
            candidate &= ~(method_flags::noexcept_specified);
            filter &= ~(method_flags::noexcept_specified);
            return candidate == filter;
        };
        const auto [fst, snd] = accessor->methods().equal_range(name);
        if (fst == snd) {
            errno = EACCES;
            return empty;
        }
        if (utility::distance(fst, snd) == 1) {
            return fst->second;
        }
        if (filter_item != method_flags::none) {
            for (auto iter = fst; iter != snd; ++iter) {
                if (const auto &method = iter->second;
                    method.is_invocable(overload_version_paramlist) && match_method_type(method.type(), filter_item)) {
                    return method;
                }
            }
        } else {
            for (auto iter = fst; iter != snd; ++iter) {
                if (const auto &method = iter->second; method.is_invocable(overload_version_paramlist)) {
                    return method;
                }
            }
        }
        for (const auto &bases: accessor->bases()) {
            if (const method &meth = bases.second.get_method(name, overload_version_paramlist, filter_item); !meth.empty()) {
                return meth;
            }
        }
        errno = EACCES;
        return empty;
    }

    RAINY_NODISCARD type::methods_view_t type::get_methods() const {
        static implements::method_storage_t empty;
        if (!accessor) {
            return utility::mapped_range(empty);
        }
        return utility::mapped_range(accessor->methods());
    }

    RAINY_NODISCARD const property &type::get_property(const std::string_view name) const noexcept {
        static const property empty;
        if (!accessor) {
            return empty;
        }
        const auto &cont = accessor->properties();
        const auto iter = cont.find(name);
        return iter != cont.end() ? iter->second : empty;
    }

    RAINY_NODISCARD type::property_view_t type::get_properties() const noexcept {
        static implements::property_storage_t empty;
        if (!accessor) {
            return utility::mapped_range(empty);
        }
        return utility::mapped_range(accessor->properties());
    }

    RAINY_NODISCARD type::constcutor_view_t type::get_construtors() const noexcept {
        static implements::ctor_storage_t empty;
        if (!accessor) {
            return empty;
        }
        return accessor->ctors();
    }

    RAINY_NODISCARD const constructor &type::get_construtor(
        const collections::views::array_view<foundation::ctti::typeinfo> overload_version_paramlist) const noexcept {
        static const constructor empty;
        if (!accessor) {
            return empty;
        }
        for (auto &item: accessor->ctors()) {
            if (const auto &method = item; method.is_invocable(overload_version_paramlist)) {
                return method;
            }
        }
        errno = EACCES;
        return empty;
    }

    type::base_classes_view_t type::get_base_classes() const noexcept {
        if (!accessor) {
            static std::unordered_map<std::string_view, type> empty;
            return utility::mapped_range(empty);
        }
        return utility::mapped_range(accessor->bases());
    }

    type::derived_classes_view_t type::get_derived_classes() const noexcept {
        if (!accessor) {
            static std::unordered_map<std::string_view, type> empty;
            return utility::mapped_range(empty);
        }
        return utility::mapped_range(accessor->bases());
    }

    bool type::is_base_of(annotations::lifetime::in<foundation::ctti::typeinfo> typeinfo) const noexcept {
        if (!accessor) {
            return false;
        }
        if (typeinfo.hash_code() == this->get_id()) {
            return true;
        }
        for (auto &item: accessor->bases()) {
            if (item.second.is_base_of(typeinfo)) {
                return true;
            }
        }
        return false;
    }

    bool type::is_base_of(annotations::lifetime::in<type> type) const noexcept {
        return is_base_of(type.accessor->typeinfo());
    }

    bool type::is_derived_from(annotations::lifetime::in<foundation::ctti::typeinfo> typeinfo) const noexcept {
        if (!accessor) {
            return false;
        }
        if (typeinfo.hash_code() == this->get_id()) {
            return true;
        }
        for (auto &item: accessor->deriveds()) {
            if (item.second.is_derived_from(typeinfo)) {
                return true;
            }
        }
        return false;
    }

    bool type::is_derived_from(annotations::lifetime::in<type> type) const noexcept {
        return is_derived_from(type.accessor->typeinfo());
    }

    RAINY_NODISCARD const foundation::ctti::typeinfo &type::get_typeinfo() noexcept {
        if (!accessor) {
            return rainy_typeid(void);
        }
        return accessor->typeinfo();
    }

    RAINY_NODISCARD const metadata &type::get_metadata(const utility::any &key) const noexcept {
        static const metadata empty;
        if (!accessor) {
            return empty;
        }
        const auto &metadatas = accessor->metadatas();
        const auto it =
            core::algorithm::find_if(metadatas.begin(), metadatas.end(), [&key](const metadata &meta) { return meta.key() == key; });
        return it != metadatas.end() ? *it : empty;
    }

    bool type::has_method(std::string_view name) const noexcept {
        if (!accessor) {
            return false;
        }
        const auto &cont = accessor->methods();
        const auto iter = cont.find(name);
        if (iter != cont.end()) {
            return true;
        }
        for (const auto &bases: accessor->bases()) {
            if (const method &meth = bases.second.get_method(name); !meth.empty()) {
                return true;
            }
        }
        return false;
    }

    bool type::has_property(std::string_view name) const noexcept {
        if (!accessor) {
            return false;
        }
        const auto &cont = accessor->properties();
        const auto iter = cont.find(name);
        if (iter != cont.end()) {
            return true;
        }
        for (const auto &bases: accessor->bases()) {
            if (bases.second.has_property(name)) {
                return true;
            }
        }
        return false;
    }

    RAINY_NODISCARD bool type::is_valid() const noexcept {
        return static_cast<bool>(accessor);
    }
}

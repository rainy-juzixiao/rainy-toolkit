#include <rainy/meta/reflection/registration.hpp>

namespace rainy::meta::reflection::implements {
    register_table &register_table::instance() {
        static register_table instance;
        return instance;
    }

    type_accessor *register_table::get_accessor_by_name(const std::string_view name) {
        auto *this_ = &instance();
        if (const auto idx_iter = this_->index.find(name); idx_iter != this_->index.end()) {
            if (const auto iter = this_->data.find(idx_iter->second); iter != this_->data.end()) {
                return iter->second;
            }
        }
        return nullptr;
    }

    void register_table::unregister(std::string_view name, foundation::ctti::typeinfo ctti) {
        auto *this_ = &instance();
        this_->data.erase(ctti);
        this_->index.erase(name);
    }
}

namespace rainy::meta::reflection {
    type type::get_by_name(const std::string_view name) noexcept {
        type instance{};
        instance.accessor = implements::register_table::get_accessor_by_name(name);
        return instance;
    }

    RAINY_NODISCARD const method &type::get_method(const std::string_view name) const noexcept {
        static const method empty;
        if (!accessor) {
            return empty;
        }
        const auto &cont = accessor->methods();
        const auto iter = cont.find(name);
        return iter != cont.end() ? iter->second : empty;
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
        if (std::distance(fst, snd) == 1) {
            return fst->second;
        }
        for (auto iter = fst; iter != snd; ++iter) {
            if (const auto &method = iter->second; method.has(filter_item)) {
                if (method.is_invocable(overload_version_paramlist)) {
                    return method;
                }
            }
        }
        for (auto iter = fst; iter != snd; ++iter) {
            if (const auto &method = iter->second;
                method.is_invocable(overload_version_paramlist) && match_method_type(method.type(), filter_item)) {
                return method;
            }
        }
        if (filter_item != method_flags::none) {
            for (auto iter = fst; iter != snd; ++iter) {
                if (const auto &method = iter->second; method.is_invocable(overload_version_paramlist)) {
                    return method;
                }
            }
        }
        errno = EACCES;
        return empty;
    }
}

namespace rainy::meta::reflection::implements {
    bool check_method_field(type_accessor *type, std::string_view name, method &meth) {
#if RAINY_HAS_CXX20
        if (type->methods().contains(name))
#else
        if (type->methods().find(name) != type->methods().end())
#endif
        {
            auto [fst, snd] = type->methods().equal_range(name);
            for (auto &it = fst; it != snd; ++it) {
                const auto &existing_params = it->second.paramlists();
                const auto &wiat_for_emplace_params = meth.paramlists();
                if (existing_params.size() != wiat_for_emplace_params.size()) {
                    continue;
                }
                const bool same = core::algorithm::all_of(wiat_for_emplace_params.begin(), wiat_for_emplace_params.end(),
                                                          [&, i = std::size_t{0}](const auto &param) mutable {
                                                              return param == existing_params[static_cast<std::ptrdiff_t>(i++)];
                                                          });
                if (same && it->second.function_signature() == meth.function_signature()) {
                    return false;
                }
            }
        }
        return true;
    }

    void register_method_helper(type_accessor *type, std::string_view name, method &&meth) {
        if (check_method_field(type, name, meth)) {
            type->methods().emplace(name, utility::move(meth));
        }
    }
}

template <typename CharType>
struct rainy::meta::reflection::moon::reflect<std::basic_string<CharType>>
    : rainy::meta::reflection::moon::reflect_base<std::basic_string<CharType>> {
    using base = rainy::meta::reflection::moon::reflect_base<std::basic_string<CharType>>;
    using type = typename base::reflect_type;

    static inline ctor_list ctors = ctor_list{
        expose_ctor([](const std::basic_string_view<CharType> &str) { return type{str.data(), str.size()}; }),
        expose_ctor<type(const CharType *)>(),
        expose_ctor<type(const CharType *, std::size_t)>(bind_meta(
            attach("param1",
                   bind_meta(attach("param1", bind_meta(attach("name", "str"),
                                                        attach("type", foundation::ctti::typeinfo::create<const CharType *>()))))),
            attach("param2", foundation::ctti::typeinfo::create<std::size_t>()))),
        expose_ctor<type(const type &)>(bind_meta(attach("is_copy_constructible", true))),
        expose_ctor<type(type &&)>(
            bind_meta(attach("is_move_constructible", true), attach("param", foundation::ctti::typeinfo::create<type &&>()))),
        expose_ctor<type()>(bind_meta(attach("is_default_constructible", true)))};

    static constexpr method_list methods =
        method_list{expose(rainy::utility::cpp_methods::method_append,
                           utility::get_overloaded_func<type, type &(const CharType *)>(&type::append)),
                    expose(rainy::utility::cpp_methods::method_append,
                           utility::get_overloaded_func<type, type &(const type &)>(&type::append)),
                    expose(rainy::utility::cpp_methods::method_size, &type::size),
                    expose(rainy::utility::cpp_methods::method_operator_assign,
                           utility::get_overloaded_func<type, type &(CharType)>(&type::operator=)),
                    expose(rainy::utility::cpp_methods::method_operator_assign,
                           utility::get_overloaded_func<type, type &(const CharType *)>(&type::operator=)),
                    expose(rainy::utility::cpp_methods::method_clear, &type::clear)};
};

RAINY_REFLECTION_REGISTRATION {
    using std::string;
    using namespace rainy;
    using namespace rainy::meta;
    using namespace rainy::utility::cpp_methods;
    using reflection::registration;
    registration::class_<string>("std::string")
        .method(method_operator_assign, utility::get_overloaded_func<string, string &(char)>(&string::operator=))
        .method(method_operator_assign, utility::get_overloaded_func<string, string &(const char *)>(&string::operator=))
        .method(method_operator_assign, utility::get_overloaded_func<string, string &(char)>(&string::operator=))
        .method(method_at, utility::get_overloaded_func<string, const char &(std::size_t) const>(&string::at))
        .method(method_at, utility::get_overloaded_func<string, char &(std::size_t)>(&string::at))
        .method(method_operator_index, utility::get_overloaded_func<string, char &(std::size_t)>(&string::operator[]))
        .method(method_swap, utility::get_overloaded_func<string, void(string &)>(&string::swap))
        .method(method_clear, &string::clear)
        .method(method_begin, utility::get_overloaded_func<string, string::iterator()>(&string::begin))
        .method(method_end, utility::get_overloaded_func<string, string::iterator()>(&string::end))
        .method(method_begin, utility::get_overloaded_func<string, string::const_iterator() const>(&string::begin))
        .method(method_end, utility::get_overloaded_func<string, string::const_iterator() const>(&string::end))
        .method(method_rbegin, utility::get_overloaded_func<string, string::reverse_iterator()>(&string::rbegin))
        .method(method_rend, utility::get_overloaded_func<string, string::reverse_iterator()>(&string::rend))
        .method(method_rbegin, utility::get_overloaded_func<string, string::const_reverse_iterator() const>(&string::rbegin))
        .method(method_rend, utility::get_overloaded_func<string, string::const_reverse_iterator() const>(&string::rend))
        .method(method_push_back, &string::push_back)
        .method(method_pop_back, &string::pop_back)
        .method(method_length, &string::length)
        .method(method_erase, utility::get_overloaded_func<string, string &(string::size_type, string::size_type)>(&string::erase))
        .property("npos", &string::npos);
}

#include <rainy/meta/reflection/refl_impl/type_register.hpp>
#include <rainy/meta/reflection/registration.hpp>

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

    bool check_ctor_field(type_accessor *type, constructor &ctor) {
        auto &ctors = type->ctors();
        auto iter = core::algorithm::find_if(ctors.begin(), ctors.end(),
                                             [&ctor](const constructor &item) { return item.is_invocable(ctor.paramlists()); });
        return iter == ctors.end(); // 如果找不到，意味着可以插入
    }

    void register_method_helper(type_accessor *type, std::string_view name, method &&meth) {
        if (check_method_field(type, name, meth)) {
            type->methods().emplace(name, utility::move(meth));
        }
    }

    type_accessor* do_inject(std::once_flag &consume, std::string_view name, type_accessor *accessor) {
        std::call_once(consume, [&]() { injector::register_type(name, accessor); });
        return register_table::get_accessor(accessor->typeinfo());
    }

    type_accessor *global_type_accessor() {
        return new_type_accessor_instance<utility::invalid_type>("global");
    }
}

// clang-format off

template <typename CharType>
struct rainy::meta::reflection::moon::reflect<std::basic_string<CharType>>
    : rainy::meta::reflection::moon::reflect_base<std::basic_string<CharType>> {
    using base = rainy::meta::reflection::moon::reflect_base<std::basic_string<CharType>>;
    using type = typename base::reflect_type;

    static inline ctor_list ctors = ctor_list{
        expose_ctor(
            [](const std::basic_string_view<CharType> &str) { 
                return type{str.data(), str.size()}; 
            }),
        expose_ctor<type(const CharType *)>(),
        expose_ctor<type(const CharType *, std::size_t)>(
            bind_meta(
                attach("param1",
                bind_meta(
                    attach("param1", 
                        bind_meta(attach("name", "str"),
                            attach("type", 
                                foundation::ctti::typeinfo::create<const CharType *>())
                        )
                    )
                )
                ),
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

// clang-format on

RAINY_REFLECTION_REGISTRATION {
    using std::string;
    using namespace rainy;
    using namespace rainy::meta;
    using namespace rainy::utility::cpp_methods;
    using reflection::registration;
    /*registration::class_<string>("std::string")
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
        .property("npos", &string::npos);*/
}

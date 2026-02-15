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

    type_accessor *global_type_accessor() {
        return new_type_accessor_instance<utility::invalid_type>("global");
    }
}

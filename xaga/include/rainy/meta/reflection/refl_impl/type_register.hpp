/*
 * Copyright 2026 rainy-juzixiao
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef RAINY_META_REFLECTION_REFL_IMPL_HPP
#define RAINY_META_REFLECTION_REFL_IMPL_HPP // NOLINT
#include <mutex>
#include <rainy/collections/dense_set.hpp>
#include <rainy/collections/unordered_map.hpp>
#include <rainy/foundation/typeinfo.hpp>
#include <rainy/meta/reflection/function.hpp>
#include <rainy/meta/reflection/property.hpp>

namespace rainy::meta::reflection {
    class type;
    class method; // 方法类
    class property; // 属性类
    class enumeration; // 枚举类
    class constructor; // 构造器
    class metadata; // 元数据
    class fundmental;
}

namespace rainy::meta::reflection::implements {
    template <typename Type, typename... Args>
    constexpr auto make_ctor_name() {
        using namespace foundation::ctti;
        constexpr auto type_str = type_name<Type>();
        if constexpr (sizeof...(Args) == 0) {
            constexpr std::size_t total_len = type_str.size() + 2 + 1; // "Type()" + '\0'
            type_traits::helper::constexpr_string<total_len> result{};
            std::size_t pos = 0;
            for (char c: type_str) {
                result[pos++] = c;
            }
            result[pos++] = '(';
            result[pos++] = ')';
            result[pos++] = '\0';
            return result;
        } else {
            constexpr collections::array<std::string_view, sizeof...(Args)> arg_names = {type_name<Args>()...};
            constexpr std::size_t args_len = [&arg_names] {
                std::size_t len = 0;
                for (auto &arg: arg_names) {
                    len += arg.size();
                }
                len += 2 * (sizeof...(Args) - 1); // ", "
                return len;
            }();
            constexpr std::size_t total_len = type_str.size() + 1 + args_len + 1 + 1; // '(' + args + ')' + '\0'
            type_traits::helper::constexpr_string<total_len> result{};
            std::size_t pos = 0;
            auto append = [&](const std::string_view &s) {
                for (char c: s) {
                    result[pos++] = c;
                }
            };
            append(type_str);
            result[pos++] = '(';
            for (std::size_t i = 0; i < arg_names.size(); ++i) {
                append(arg_names[i]);
                if (i < arg_names.size() - 1) {
                    result[pos++] = ',';
                    result[pos++] = ' ';
                }
            }
            result[pos++] = ')';
            result[pos++] = '\0';
            return result;
        }
    }
}

namespace rainy::meta::reflection::implements {
    struct type_accessor;
    struct enumeration_accessor;
    struct fundmental_type_accessor;

    template <typename Enum>
    struct enumeration_accessor_impl;

    template <typename Type>
    struct fundmental_type_accessor_impl;

    template <typename Type>
    rain_fn new_fundmental_accessor_instance(type_accessor *type) noexcept -> fundmental_type_accessor * {
        static fundmental_type_accessor_impl<Type> instance{type};
        return &instance;
    }

    template <typename EnumType>
    rain_fn new_enum_type_storage_instance(type_accessor *type) noexcept -> enumeration_accessor_impl<EnumType> * {
        static implements::enumeration_accessor_impl<EnumType> enum_type_storage{type};
        return &enum_type_storage;
    }
}

namespace rainy::meta::reflection::implements {
    using method_storage_t = collections::unordered_multimap<std::string_view, method>;
    using property_storage_t = collections::unordered_map<std::string_view, property>;
    using enumeration_storage_t = collections::unordered_map<std::string_view, enumeration>;
    using ctor_storage_t = std::vector<constructor>;
}

namespace rainy::meta::reflection::implements {
    struct type_accessor {
        virtual ~type_accessor() = default;
        /* 类型名称标记 */
        RAINY_NODISCARD virtual std::string_view name() const noexcept = 0;
        /* 类型信息 */
        virtual foundation::ctti::typeinfo &typeinfo() noexcept = 0;
        /* 方法、静态方法、重载运算符 */
        virtual method_storage_t &methods() noexcept = 0;
        /* 构造函数集合 */
        virtual ctor_storage_t &ctors() noexcept = 0;
        /* 属性、静态属性 */
        virtual property_storage_t &properties() noexcept = 0;
        /* 基类集合 */
        virtual collections::unordered_map<foundation::ctti::typeinfo, type> &bases() noexcept = 0;
        /* 派生类集合，用于未来设计dynamic_cast */
        virtual collections::unordered_map<foundation::ctti::typeinfo, type> &deriveds() noexcept = 0;
        // 基类/派生类返回的是一张表，其中的指针指向位于反射系统内部的实例（其实是静态示例）
        // const在此部分非一等公民，由面向用户的接口进行const属性添加
        // 不考虑过多性能
        virtual std::vector<metadata> &metadatas() noexcept = 0;
        /* 如果是枚举信息，则此方法有效 */
        virtual enumeration &as_enumeration() noexcept = 0;
        /* 如果是fundmental类型，则此方法有效 */
        virtual fundmental &as_fundmental() noexcept = 0;
    };
}

namespace rainy::meta::reflection::implements {
    template <typename Type, typename EnumImpl = enumeration, typename FundImpl = fundmental, typename CtorImpl = constructor,
              typename TypeImpl = type>
    class type_accessor_impl_class final : public type_accessor {
    public:
        using enumeration = EnumImpl;
        using fundmental = FundImpl;
        using constructor = CtorImpl;
        using type = TypeImpl;

        explicit type_accessor_impl_class(const std::string_view name) noexcept :
            name_(name), typeinfo_(foundation::ctti::typeinfo::create<Type>()) {
        }

        RAINY_NODISCARD std::string_view name() const noexcept override {
            return name_;
        }

        foundation::ctti::typeinfo &typeinfo() noexcept override {
            return typeinfo_;
        }

        method_storage_t &methods() noexcept override {
            return methods_;
        }

        ctor_storage_t &ctors() noexcept override {
            return ctors_;
        }

        property_storage_t &properties() noexcept override {
            return properties_;
        }

        collections::unordered_map<foundation::ctti::typeinfo, type> &bases() noexcept override {
            return bases_;
        }

        collections::unordered_map<foundation::ctti::typeinfo, type> &deriveds() noexcept override {
            return deriveds_;
        }

        std::vector<metadata> &metadatas() noexcept override {
            return metadatas_;
        }

        enumeration &as_enumeration() noexcept override {
            static enumeration empty;
            return empty;
        }

        fundmental &as_fundmental() noexcept override {
            static fundmental empty;
            return empty;
        }

    private:
        std::string_view name_;
        foundation::ctti::typeinfo typeinfo_;
        method_storage_t methods_;
        ctor_storage_t ctors_;
        property_storage_t properties_;
        collections::unordered_map<foundation::ctti::typeinfo, type> bases_;
        collections::unordered_map<foundation::ctti::typeinfo, type> deriveds_;
        std::vector<metadata> metadatas_;
    };

    template <typename Type, typename EnumType = enumeration, typename FundImpl = fundmental, typename Constrcutor = constructor,
              typename CtorStorageT = ctor_storage_t, typename Metadata = metadata, typename TypeImpl = type>
    class type_accessor_impl_enumeration final : public type_accessor {
    public:
        using enumeration_impl = EnumType;
        using constructor_impl = Constrcutor;
        using fundmental = FundImpl;
        using type = TypeImpl;

        explicit type_accessor_impl_enumeration(const std::string_view name) noexcept :
            name_(name), typeinfo_(foundation::ctti::typeinfo::create<Type>()),
            enumeration_{core::internal_construct_tag, this, new_enum_type_storage_instance<Type>(this)} {
        }

        RAINY_NODISCARD std::string_view name() const noexcept override {
            return name_;
        }

        foundation::ctti::typeinfo &typeinfo() noexcept override {
            return typeinfo_;
        }

        method_storage_t &methods() noexcept override {
            static method_storage_t empty;
            return empty;
        }

        CtorStorageT &ctors() noexcept override {
            static CtorStorageT ctors_;
            static std::once_flag flag;
            std::call_once(flag, []() {
                static collections::array<Metadata, 0> empty{};
                static std::tuple<> a;
                static constexpr auto underlying_ctor_name = make_ctor_name<Type, type_traits::other_trans::underlying_type_t<Type>>();
                static constexpr auto ctor_name = make_ctor_name<Type, Type>();
                static constexpr auto default_ctor_name = make_ctor_name<Type>();
                ctors_.emplace_back(constructor_impl::make(
                    {underlying_ctor_name.data(), underlying_ctor_name.size()},
                    [](type_traits::other_trans::underlying_type_t<Type> value) { return Type{value}; }, a, empty));
                ctors_.emplace_back(
                    constructor_impl::make({ctor_name.data(), ctor_name.size()}, [](Type value) { return Type{value}; }, a, empty));
                ctors_.emplace_back(
                    constructor_impl::make({default_ctor_name.data(), default_ctor_name.size()}, []() { return Type{}; }, a, empty));
            });
            return ctors_;
        }

        property_storage_t &properties() noexcept override {
            static property_storage_t empty;
            return empty;
        }

        collections::unordered_map<foundation::ctti::typeinfo, type> &bases() noexcept override { // NOLINT
            static collections::unordered_map<foundation::ctti::typeinfo, type> empty;
            return empty;
        }

        collections::unordered_map<foundation::ctti::typeinfo, type> &deriveds() noexcept override { // NOLINT
            static collections::unordered_map<foundation::ctti::typeinfo, type> empty;
            return empty;
        }

        std::vector<metadata> &metadatas() noexcept override {
            static std::vector<Metadata> empty;
            return empty;
        }

        enumeration &as_enumeration() noexcept override {
            return enumeration_;
        }

        fundmental &as_fundmental() noexcept override {
            static fundmental empty;
            return empty;
        }

    private:
        std::string_view name_;
        foundation::ctti::typeinfo typeinfo_;
        enumeration_impl enumeration_;
    };

    template <typename Type, typename EnumType = enumeration, typename FundImpl = fundmental, typename Constrcutor = constructor,
              typename CtorStorageT = ctor_storage_t, typename Metadata = metadata, typename TypeImpl = type>
    class type_accessor_impl_fundmental_type final : public type_accessor {
    public:
        using enumeration_impl = EnumType;
        using constructor_impl = Constrcutor;
        using fundmental_impl = FundImpl;
        using type = TypeImpl;

        explicit type_accessor_impl_fundmental_type(const std::string_view name) noexcept :
            name_(name), typeinfo_(foundation::ctti::typeinfo::create<Type>()),
            fundmental_type(core::internal_construct_tag, this, new_fundmental_accessor_instance<Type>(this)) {
        }

        RAINY_NODISCARD std::string_view name() const noexcept override {
            return name_;
        }

        foundation::ctti::typeinfo &typeinfo() noexcept override {
            return typeinfo_;
        }

        method_storage_t &methods() noexcept override {
            static method_storage_t empty;
            return empty;
        }

        CtorStorageT &ctors() noexcept override {
            static CtorStorageT ctors_;
            static std::once_flag flag;
            std::call_once(flag, []() {
                static collections::array<Metadata, 0> empty{};
                static std::tuple<> a;
                if constexpr (!type_traits::primary_types::is_void_v<Type>) {
                    static constexpr auto ctor_name = make_ctor_name<Type, Type>();
                    static constexpr auto default_ctor_name = make_ctor_name<Type>();
                    ctors_.emplace_back(constructor_impl::make(
                        {ctor_name.data(), ctor_name.size()}, [](Type value) { return Type{value}; }, a, empty));
                    ctors_.emplace_back(constructor_impl::make(
                        {default_ctor_name.data(), default_ctor_name.size()}, []() { return Type{}; }, a, empty));
                }
            });
            return ctors_;
        }

        property_storage_t &properties() noexcept override {
            static property_storage_t empty;
            return empty;
        }

        collections::unordered_map<foundation::ctti::typeinfo, type> &bases() noexcept override {
            static collections::unordered_map<foundation::ctti::typeinfo, type> empty;
            return empty;
        }

        collections::unordered_map<foundation::ctti::typeinfo, type> &deriveds() noexcept override {
            static collections::unordered_map<foundation::ctti::typeinfo, type> empty;
            return empty;
        }

        std::vector<metadata> &metadatas() noexcept override {
            return metadatas_;
        }

        enumeration &as_enumeration() noexcept override {
            static enumeration_impl empty;
            return empty;
        }

        fundmental &as_fundmental() noexcept override {
            return fundmental_type;
        }

    private:
        std::string_view name_;
        foundation::ctti::typeinfo typeinfo_;
        fundmental_impl fundmental_type;
        std::vector<metadata> metadatas_;
    };
}

namespace rainy::meta::reflection::implements {
    template <typename Cont>
    rain_fn check_function_overload_cond(const Cont &container, std::string_view name, const method &meth) -> bool {
        if (const std::string_view key{name.data(), name.size()}; container.contains(key)) {
            auto [fst, snd] = container.equal_range(key);
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
}

namespace rainy::meta::reflection {
    class RAINY_TOOLKIT_LOCAL_API registration_manager {
    public:
        using type_accessor = implements::type_accessor;

        using global_function_iter_t = typename collections::unordered_multimap<std::string_view, method>::iterator; // NOLINT

        type_accessor *get_accessor(const foundation::ctti::typeinfo &type) {
            std::scoped_lock lck(lock);
            rainy_const it = ctti_to_accessor.find(type);
            return it != ctti_to_accessor.end() ? it->second : nullptr;
        }

        type_accessor *get_accessor_by_name(std::string_view name) {
            std::scoped_lock guard(lock);
            const std::string_view type_name{name.data(), name.size()};
            const auto idx_it = name_to_ctti.find(type_name);
            if (idx_it == name_to_ctti.end()) {
                return nullptr;
            }
            const auto it = ctti_to_accessor.find(idx_it->second);
            return it != ctti_to_accessor.end() ? it->second : nullptr;
        }

        void register_type(const std::string_view name, type_accessor *accessor) {
            std::scoped_lock lck(lock);
            const auto &ctti = accessor->typeinfo();
            if (ctti_to_accessor.contains(ctti)) {
                return;
            }
            ctti_to_accessor.emplace(ctti, accessor);
            name_to_ctti.emplace(std::string_view{name.data(), name.size()}, ctti);
        }

        void unregister_type(const foundation::ctti::typeinfo &ctti) {
            std::scoped_lock lck(lock);
            rainy_const it = ctti_to_accessor.find(ctti);
            if (it == ctti_to_accessor.end()) {
                return;
            }
            // 同步清理 name_to_ctti
            for (auto nit = name_to_ctti.begin(); nit != name_to_ctti.end();) {
                if (nit->second == ctti) {
                    nit = name_to_ctti.erase(nit);
                } else {
                    ++nit;
                }
            }
            ctti_to_accessor.erase(it);
        }

        rain_fn has_register(const foundation::ctti::typeinfo &type) const noexcept -> bool {
            std::scoped_lock guard(lock);
            return ctti_to_accessor.contains(type);
        }

        rain_fn unregister_global_function(const global_function_iter_t it) noexcept {
            std::scoped_lock guard(lock);
            if (it != global_functions.end()) {
                global_functions.erase(it);
            }
        }

        rain_fn register_global_function(const method &meth) -> utility::pair<global_function_iter_t, bool> {
            if (implements::check_function_overload_cond(global_functions, meth.get_name(), meth)) {
                return {global_functions.emplace(meth.get_name(), meth).first, true};
            }
            return {global_functions.end(), false};
        }

        rain_fn global_funs() noexcept -> const auto & {
            return global_functions;
        }

        template <typename Fx>
        rain_fn collect(Fx &&handler) const -> void {
            std::scoped_lock guard(lock);
            for (const auto &item: ctti_to_accessor) {
                utility::invoke(utility::forward<Fx>(handler), item.second);
            }
        }

        template <typename Fx>
        rain_fn collect_global_functions(Fx &&handler) const -> void {
            std::scoped_lock guard(lock);
            for (const auto &item: global_functions) {
                utility::invoke(utility::forward<Fx>(handler), item.second);
            }
        }

    private:
        mutable std::mutex lock;
        collections::unordered_map<std::string_view, foundation::ctti::typeinfo> name_to_ctti;
        collections::unordered_map<foundation::ctti::typeinfo, type_accessor *> ctti_to_accessor;
        collections::unordered_multimap<std::string_view, method> global_functions;
    };

    RAINY_TOOLKIT_API registration_manager &get_registration_manager() noexcept;
}

namespace rainy::meta::reflection::implements {
    class RAINY_TOOLKIT_LOCAL_API module_injector {
    public:
        using global_function_iter_t = typename collections::unordered_multimap<std::string_view, method>::iterator; // NOLINT

        void record(const foundation::ctti::typeinfo &ctti, type_accessor *accessor) {
            for (const auto &[type, _]: registered_types) {
                if (type == ctti) {
                    return;
                }
                (void) _;
            }
            registered_types.emplace_back(ctti, accessor); // NOLINT
        }

        void record_function(const global_function_iter_t &func) {
            registered_functions.emplace_back(func);
        }

        void unregister_all() {
            auto &manager = get_registration_manager();
            for (auto &[type, _]: registered_types) {
                manager.unregister_type(type);
                (void) _;
            }
            for (const auto it: registered_functions) {
                manager.unregister_global_function(it);
            }
            registered_functions.clear();
            registered_types.clear();
        }

        ~module_injector() {
            unregister_all();
        }

        RAINY_TOOLKIT_LOCAL_API static module_injector &instance() noexcept {
            static module_injector obj;
            return obj;
        }

    private:
        module_injector() = default;

        struct record_t {
            foundation::ctti::typeinfo type;
            type_accessor *accessor;
        };

        std::vector<global_function_iter_t> registered_functions;
        std::vector<record_t> registered_types;
    };
}

namespace rainy::meta::reflection::implements {
    /**
     * @brief 用于记录模块加载前后注册类型差集的事务状态类
     * @details 典型用法是在 dlopen 前调用 begin_transaction，dlopen 后调用 end_transaction，
     *          然后通过 get_registered_types 获取本次模块新增的类型列表
     */
    class RAINY_TOOLKIT_LOCAL_API registration_state {
    public:
        registration_state() = default;

        ~registration_state() noexcept {
            clear();
        }

        /**
         * @brief 开始事务，记录当前 registration_manager 中已有的类型快照
         */
        rain_fn begin_transaction() -> void { // NOLINT
            before.clear();
            get_registration_manager().collect([&](type_accessor *accessor) { before.insert(accessor->typeinfo()); });
            get_registration_manager().collect_global_functions(
                [&](const method &meth) { before_register_globalfun.emplace(meth.get_name(), meth); });
        }

        /**
         * @brief 结束事务，计算差集并存入内部哈希表
         */
        rain_fn end_transaction() -> void { // NOLINT
            registered.clear();
            get_registration_manager().collect([&](type_accessor *accessor) {
                if (const auto &ctti = accessor->typeinfo(); !before.contains(ctti)) {
                    registered.emplace(ctti, accessor);
                }
            });
            get_registration_manager().collect_global_functions([&](const method &meth) {
                if (check_function_overload_cond(before_register_globalfun, meth.get_name(), meth)) {
                    registered_functions.emplace(meth.get_name(), meth);
                }
            });
            before.clear();
        }

        /**
         * @brief 获取本次事务中新增注册的类型
         * @return 从 ctti 到 type_accessor* 的哈希表
         */
        RAINY_NODISCARD rain_fn get_registered_types() const noexcept
            -> const collections::unordered_map<foundation::ctti::typeinfo, type_accessor *> & {
            return registered;
        }

        /**
         * @brief 获取本次事务中新增注册的全局函数
         * @return 从函数名到 method 的多重哈希表
         */
        RAINY_NODISCARD const collections::unordered_multimap<std::string_view, method> &get_registered_functions() const noexcept {
            return registered_functions;
        }

        /**
         * @brief 清除state
         */
        rain_fn clear() noexcept -> void {
            registered.clear();
            before.clear();
            before_register_globalfun.clear();
            registered_functions.clear();
        }

    private:
        collections::unordered_map<foundation::ctti::typeinfo, type_accessor *> registered{};
        collections::unordered_multimap<std::string_view, method> registered_functions{};
        collections::unordered_multimap<std::string_view, method> before_register_globalfun{};
        collections::dense_set<foundation::ctti::typeinfo> before{};
    };
}

#endif

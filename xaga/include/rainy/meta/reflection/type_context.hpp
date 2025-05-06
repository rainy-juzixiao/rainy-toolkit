/*
 * Copyright 2025 rainy-juzixiao
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
#ifndef RAINY_META_TYPE_CONTEXT_HPP
#define RAINY_META_TYPE_CONTEXT_HPP
/**
* @file type_context.hpp
* @brief 这是一个类型上下文的模块，它描述了rainy's reflection的实例如何被动态方法读取，以及描述一个动态类的数据结构，用于为后面建立完整的动态类对象实现
* @attention 首先明确，不支持动态方法调用，dynamic_instance它仅表示对象自己，不具备方法，方法是由reflection模块提供的dynamic_object提供的
* @attention 因此，不受考虑
* @attention 另外，线程安全不受本模块支持，线程安全应当由使用者负责。明确自己的职责。
*/
#include <optional>
#include <unordered_map>
#include <rainy/core/core.hpp>
#include <rainy/collections/views/array_view.hpp>
#include <rainy/utility/any.hpp>
#include <rainy/foundation/functional/function_pointer.hpp>
#include <rainy/foundation/diagnostics/contract.hpp>

namespace rainy::meta::reflection {
    enum class acc_level {
        public_access,
        protected_access,
        private_access,
        inaccessible
    };

    enum class access_context {
        public_context,
        protected_context,
        private_context
    };

    class rtti_type_register {
    public:
        struct type {
            const foundation::rtti::typeinfo *rtti;
            foundation::functional::function_pointer<utility::any(*)()> creater;
        };

        using value = utility::any;
        using creater = foundation::functional::function_pointer<value(*)()>;

        static rtti_type_register &instance() noexcept {
            static std::once_flag flag;
            static rtti_type_register instance;
            std::call_once(flag, [&] { instance.preheat(); });
            return instance;
        };

        template <typename Type, type_traits::other_trans::enable_if_t<type_traits::type_properties::is_default_constructible_v<Type>,int> = 0>
        rtti_type_register& create_type_mapping(std::string_view name) {
            if (mapping.find(name) == mapping.end()) {
                mapping.insert({name, {&rainy_typeid(Type), &create_helper<Type>}});
            }
            return *this;
        }

        type get_type(std::string_view name) noexcept {
            static const type empty{&rainy_typeid(void), nullptr};
            if (const auto it = mapping.find(name); it != mapping.end()) {
                return it->second;
            }
            return empty;
        }

        std::optional<utility::any> generate_default(std::string_view name) const {
            if (name.empty()) {
                return std::nullopt;
            }
            const auto find = mapping.find(name);
            if (find == mapping.end()) {
                return std::nullopt;
            }
            if (!find->second.creater.empty()) {
                return std::nullopt;
            }
            return find->second.creater();
        }

        void remove_type_mapping(std::string_view name) {
            if (const auto it = mapping.find(name); it != mapping.end()) {
                mapping.erase(it);
            }
        }

        bool has_type_mapping(std::string_view name) const noexcept {
            return mapping.find(name) != mapping.end();
        }

    private:
#if RAINY_USING_32_BIT_PLATFORM
        static constexpr std::size_t preheat_reserve = 64;
#else
        static constexpr std::size_t preheat_reserve = 80;
#endif

        rtti_type_register() noexcept = default;

        void preheat() {
            mapping.reserve(preheat_reserve);
        }

        template <typename Type>
        static value create_helper() {
            return {Type{}};
        }

        std::unordered_map<std::string_view, type> mapping;
    };

    class dynamic_instance {
    public:
        class member {
        public:
            member(acc_level access_level, utility::any data) : access_level(access_level), data(data), dummy{} {
            }

            const foundation::rtti::typeinfo &rtti() const noexcept {
                return data.type();
            }

            template <typename Type>
            const Type &getter(access_context context = access_context::public_context) const {
                utility::expects(can_access(context), "access violation: insufficient access rights");
                return utility::any_cast<Type>(data);
            }

            template <typename Type>
            Type &getter(access_context context = access_context::public_context) {
                utility::expects(can_access(context), "access violation: insufficient access rights");
                return *utility::any_cast<Type>(&data);
            }

            template <typename Type>
            void setter(Type &&value, access_context context = access_context::public_context) {
                utility::expects(can_access(context), "access violation: insufficient access rights");
                utility::expects(data.type().is_compatible(rainy_typeid(Type)), "Invlaid Type");
                data = utility::forward<Type>(value);
            }

            bool can_access(access_context context) const noexcept {
                /* 使用if语句会破坏可读性 */
                switch (access_level) {
                    case acc_level::public_access:
                        // 直接通过
                        return true;
                    case acc_level::protected_access:
                        // 对于protected_access的ctx，我们要检查protected或是否是private
                        return context == access_context::protected_context || context == access_context::private_context;
                    case acc_level::private_access:
                        // 只有private_context允许访问
                        return context == access_context::private_context;
                    case acc_level::inaccessible:
                        return false;
                }
                return false;
            }
            
            const acc_level access_level{acc_level::private_access};

        private:
            utility::any data;
            std::max_align_t dummy{};
        };

        using value = utility::any;
        using mapped_type = std::unordered_map<std::string_view, member>;

        dynamic_instance(std::string_view blueprint_name, mapped_type &&members_init) :
            blueprint_name(blueprint_name), members_(utility::move(members_init)) {
        }

        std::string_view get_blueprint_name() const noexcept {
            return blueprint_name;
        }

        const foundation::rtti::typeinfo &typeinfo(std::string_view member_name) const {
            const auto it = members_.find(member_name);
            utility::expects(it != members_.end(), "Could not find the member");
            return it->second.rtti();
        }

        std::optional<acc_level> get_access_level(std::string_view member_name) const noexcept {
            auto it = members_.find(member_name);
            if (it != members_.end()) {
                return it->second.access_level;
            }
            return std::nullopt;
        }

        const member *get_member(std::string_view member_name,
                                 access_context context = access_context::public_context) const noexcept {
            auto it = members_.find(member_name);
            if (it != members_.end() && it->second.can_access(context)) {
                return &(it->second);
            }
            return nullptr;
        }

        member *get_member(std::string_view member_name, access_context context = access_context::public_context) noexcept {
            auto it = members_.find(member_name);
            if (it != members_.end() && it->second.can_access(context)) {
                return &(it->second);
            }
            return nullptr;
        }

        std::size_t member_count() const noexcept {
            return members_.size();
        }

        template <typename Type>
        void set_member_data(std::string_view member_name, Type &&init, access_context context = access_context::public_context) {
            member *member = get_member(member_name, context);
            utility::expects(member != nullptr, "Could not find the member");
            member->setter(init, context);
        }

        bool has_member(std::string_view member_name) const noexcept {
            return members_.find(member_name) != members_.end();
        }

        bool can_access_member(std::string_view member_name, access_context context = access_context::public_context) const noexcept {
            auto it = members_.find(member_name);
            return it != members_.end() && it->second.can_access(context);
        }

        auto begin() const noexcept {
            return members_.begin();
        }
        auto end() const noexcept {
            return members_.end();
        }

        std::vector<std::string_view> get_member_names(access_context context = access_context::public_context) const {
            std::vector<std::string_view> names;
            names.reserve(members_.size());
            for (const auto &[name, member]: members_) {
                if (member.can_access(context)) {
                    names.push_back(name);
                }
            }
            return names;
        }

        bool add_member(std::string_view name, acc_level access, utility::any &&data,
                        access_context context = access_context::private_context) {
            if (context != access_context::private_context) {
                return false;
            }
            return members_.emplace(name, member(access, utility::move(data))).second;
        }

        bool remove_member(std::string_view name, access_context context = access_context::private_context) {
            if (context != access_context::private_context) {
                return false;
            }
            return members_.erase(name) > 0;
        }

    private:
        std::string_view blueprint_name;
        mapped_type members_;
    };
}

namespace rainy::meta::reflection {
    class instance_context {
    public:
        class blueprint {
        public:
            using value = utility::any;

            struct blueprint_member {
                acc_level access_level;
                foundation::functional::function_pointer<value (*)()> creater;
                const foundation::rtti::typeinfo* type;
            };

            blueprint() noexcept {
                preheat();
            };

            blueprint(std::string_view name) : name_(name) {
                preheat();
            }

            template <typename Type,
                      type_traits::other_trans::enable_if_t<type_traits::type_properties::is_default_constructible_v<Type> && !type_traits::type_relations::is_void_v<Type>, int> = 0>
            blueprint &add_member(std::string_view name, acc_level level = acc_level::private_access) {
                if (name.empty()) {
                    return *this;
                }
                if (members.find(name) == members.end()) {
                    const foundation::rtti::typeinfo &rtti = rainy_typeid(Type);
                    members.insert({name, {level, &create_helper<Type>, &rtti}});
                    if (!rtti_type_register::instance().has_type_mapping(rtti.name())) {
                        rtti_type_register::instance().create_type_mapping<Type>(rtti.name());
                    }
                }
                return *this;
            }

            blueprint &add_member_by_typename(std::string_view type_name, std::string_view name,
                                              acc_level level = acc_level::private_access) {
                if (type_name.empty() || name.empty()) {
                    return *this;
                }
                if (members.find(name) == members.end()) {
                    auto type = rtti_type_register::instance().get_type(type_name);
                    if (!type.rtti->is_same(rainy_typeid(void))) {
                        members.insert({name, {level, type.creater, type.rtti}});
                    }
                }
                return *this;
            }

            blueprint &add_member_by_rtti(const foundation::rtti::typeinfo &rtti, std::string_view name,
                                  acc_level level = acc_level::private_access) {
                return add_member_by_typename(rtti.name(), name, level);
            }

            void remove_member(std::string_view name) {
                if (members.find(name) != members.end()) {
                    members.erase(name);
                }
            }

            dynamic_instance create_dynamic_object() const {
                using mapped_type = dynamic_instance::mapped_type;
                mapped_type instance_members;
                for (auto &i: members) {
                    instance_members.insert({i.first, {i.second.access_level, i.second.creater()}});
                }
                dynamic_instance instance(name_, utility::move(instance_members));
                return instance;
            }

            dynamic_instance create_dynamic_object(
                std::unordered_map<std::string_view /*member_name*/, utility::any /* data */> member_init) const {
                dynamic_instance::mapped_type instance_members;
                for (auto &[name, member]: members) {
                    auto find = member_init.find(name);
                    if (find != member_init.end()) {
                        utility::expects(member.type->is_same(find->second.type()), "invalid type!");
                        const auto pair = instance_members.insert({name, {member.access_level, find->second}});
                        utility::ensures(pair.second, "Cannot add data to object!");
                    } else {
                        instance_members.insert({name, {member.access_level, member.creater()}});
                    }
                }
                return dynamic_instance(name_, utility::move(instance_members));
            }

        private:
#if RAINY_USING_32_BIT_PLATFORM
            static constexpr std::size_t preheat_reserve = 4;
#else
            static constexpr std::size_t preheat_reserve = 6;
#endif

            void preheat() {
                members.reserve(preheat_reserve);
            }

            template <typename Type>
            static value create_helper() {
                return {Type{}};
            }

            std::string_view name_;
            std::unordered_map<std::string_view, blueprint_member> members;
        };

        static instance_context &instance() noexcept {
            static std::once_flag flag;
            static instance_context instance;
            std::call_once(flag, [&]() { instance.context.reserve(5); }); // 预先准备5份蓝图
            return instance;
        };

        blueprint &create_blueprint(std::string_view name) {
            if (auto it = context.find(name); it == context.end()) {
                context.insert({name, {name}});
                return context[name];
            } else {
                return it->second;
            }
        }

        void remove_blueprint(std::string_view name) {
            if (auto it = context.find(name); it != context.end()) {
                context.erase(it);
            }
        }

        blueprint &get_blueprint(std::string_view name) noexcept {
            const auto find = context.find(name);
            utility::expects(find != context.end(), "Could not find the blueprint!");
            return find->second;
        }

        bool has_blueprint(std::string_view name) const noexcept {
            return context.find(name) != context.end();
        }

    private:
        std::unordered_map<std::string_view, blueprint> context;
    };
}

#define RAINY_TOOLKIT_REGISTRATION_FOR_TYPECONTEXT(TYPE)\
        namespace {                                                                                                                       \
        struct RAINY_CAT(rainy_toolkit_registrarion_for_type_context_, __LINE__) {                                                    \
            RAINY_CAT(rainy_toolkit_registrarion_for_type_context_, __LINE__)() {                                                     \
                ::rainy::meta::reflection::rtti_type_register::instance().create_type_mapping<TYPE>(RAINY_STRINGIZE(TYPE));           \
            }                                                                                                                         \
        };                                                                                                                            \
        const static RAINY_CAT(rainy_toolkit_registrarion_for_type_context_,__LINE__) RAINY_CAT(rainy_toolkit_type_context_auto_register_, __LINE__);                                                  \
    }

namespace {
    RAINY_TOOLKIT_REGISTRATION_FOR_TYPECONTEXT(bool);
    RAINY_TOOLKIT_REGISTRATION_FOR_TYPECONTEXT(char);
    RAINY_TOOLKIT_REGISTRATION_FOR_TYPECONTEXT(int);
    RAINY_TOOLKIT_REGISTRATION_FOR_TYPECONTEXT(unsigned int);
    RAINY_TOOLKIT_REGISTRATION_FOR_TYPECONTEXT(short);
    RAINY_TOOLKIT_REGISTRATION_FOR_TYPECONTEXT(short int);
    RAINY_TOOLKIT_REGISTRATION_FOR_TYPECONTEXT(unsigned short);
    RAINY_TOOLKIT_REGISTRATION_FOR_TYPECONTEXT(long);
    RAINY_TOOLKIT_REGISTRATION_FOR_TYPECONTEXT(unsigned long);
    RAINY_TOOLKIT_REGISTRATION_FOR_TYPECONTEXT(long long);
    RAINY_TOOLKIT_REGISTRATION_FOR_TYPECONTEXT(unsigned long long);
    RAINY_TOOLKIT_REGISTRATION_FOR_TYPECONTEXT(wchar_t);
    RAINY_TOOLKIT_REGISTRATION_FOR_TYPECONTEXT(float);
    RAINY_TOOLKIT_REGISTRATION_FOR_TYPECONTEXT(double);
    RAINY_TOOLKIT_REGISTRATION_FOR_TYPECONTEXT(long double);
    RAINY_TOOLKIT_REGISTRATION_FOR_TYPECONTEXT(char16_t);
    RAINY_TOOLKIT_REGISTRATION_FOR_TYPECONTEXT(char32_t);
    RAINY_TOOLKIT_REGISTRATION_FOR_TYPECONTEXT(std::size_t);
    RAINY_TOOLKIT_REGISTRATION_FOR_TYPECONTEXT(std::int32_t);
    RAINY_TOOLKIT_REGISTRATION_FOR_TYPECONTEXT(std::int64_t);
    RAINY_TOOLKIT_REGISTRATION_FOR_TYPECONTEXT(std::uintptr_t);
#if RAINY_HAS_CXX20
    RAINY_TOOLKIT_REGISTRATION_FOR_TYPECONTEXT(char8_t);
#endif
    RAINY_TOOLKIT_REGISTRATION_FOR_TYPECONTEXT(std::string);
    RAINY_TOOLKIT_REGISTRATION_FOR_TYPECONTEXT(std::wstring)
    RAINY_TOOLKIT_REGISTRATION_FOR_TYPECONTEXT(std::string_view);
    RAINY_TOOLKIT_REGISTRATION_FOR_TYPECONTEXT(std::vector<bool>);
    RAINY_TOOLKIT_REGISTRATION_FOR_TYPECONTEXT(std::vector<int>);
    RAINY_TOOLKIT_REGISTRATION_FOR_TYPECONTEXT(std::vector<char>);
    RAINY_TOOLKIT_REGISTRATION_FOR_TYPECONTEXT(std::vector<float>);
    RAINY_TOOLKIT_REGISTRATION_FOR_TYPECONTEXT(std::vector<double>);
    RAINY_TOOLKIT_REGISTRATION_FOR_TYPECONTEXT(rainy::utility::any);
    RAINY_TOOLKIT_REGISTRATION_FOR_TYPECONTEXT(rainy::collections::views::array_view<bool>);
    RAINY_TOOLKIT_REGISTRATION_FOR_TYPECONTEXT(rainy::collections::views::array_view<int>);
    RAINY_TOOLKIT_REGISTRATION_FOR_TYPECONTEXT(rainy::collections::views::array_view<char>);
    RAINY_TOOLKIT_REGISTRATION_FOR_TYPECONTEXT(rainy::collections::views::array_view<float>);
    RAINY_TOOLKIT_REGISTRATION_FOR_TYPECONTEXT(rainy::collections::views::array_view<double>);
}

#endif

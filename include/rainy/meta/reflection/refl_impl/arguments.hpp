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
#ifndef RAINY_META_RELF_IMPL_ARGUMENTS_HPP
#define RAINY_META_RELF_IMPL_ARGUMENTS_HPP
#include <rainy/core/core.hpp>
#include <rainy/foundation/system/memory/nebula_ptr.hpp>
#include <rainy/foundation/typeinfo.hpp>
#include <rainy/meta/reflection/type_context.hpp>
#include <tuple>
#include <vector>

namespace rainy::meta::reflection::internals {
    template <typename... Args>
    static constexpr std::size_t eval_hash_code() noexcept {
        return (0 + ... + ::rainy::foundation::rtti::typeinfo::get_type_hash<Args>());
    }
}

namespace rainy::meta::reflection {
    class arguments_visitor {
    public:
        virtual std::size_t arguments_count() const noexcept = 0;
        virtual bool is_view() const noexcept = 0;
        virtual const std::vector<foundation::rtti::typeinfo>& typeinfo() const noexcept = 0;
        virtual containers::array_view<object_view> paramlist_view(core::byte_t *temporary_buffer) noexcept = 0;
        virtual std::size_t hash_code() const noexcept = 0;
        virtual ~arguments_visitor() = default;
    };

    template <typename... Args>
    class arguments final : public arguments_visitor {
    public:
        arguments(Args &&...args) : tuples(utility::forward<Args>(args)...) {
        }

        ~arguments() final = default;

        bool is_view() const noexcept override {
            return false;
        }

        RAINY_NODISCARD std::size_t arguments_count() const noexcept override {
            return sizeof...(Args);
        }

        RAINY_NODISCARD const std::tuple<Args...> &get_args() const {
            return tuples;
        }

        template <std::size_t Idx, typename Ty,
                  type_traits::other_trans::enable_if_t<type_traits::type_relations::is_convertible_v<Ty, decltype(std::get<Idx>())>,
                                                        int> = 0>
        void change_argument(Ty &&arugment) noexcept(std::is_nothrow_assignable_v<Ty, decltype(std::get<Idx>())>) {
            std::get<Idx>(*tuples) = arugment;
        }

        template <std::size_t Idx, typename = decltype(std::get<Idx>())>
        RAINY_NODISCARD decltype(auto) get_arg() const {
            return std::get<Idx>(*tuples);
        }

        const std::vector<foundation::rtti::typeinfo> &typeinfo() const noexcept override {
            static std::vector<foundation::rtti::typeinfo> res = {
                foundation::rtti::typeinfo::create<type_traits::other_trans::decay_t<decltype(utility::declval<Args>())>>()...};
            return res;
        }

        containers::array_view<object_view> paramlist_view(core::byte_t *temporary_buffer) noexcept override {
            auto res = reinterpret_cast<containers::array<object_view, sizeof...(Args)>*>(temporary_buffer);
            if (res != nullptr) {
                paramlist_construct_helper(res, type_traits::helper::index_sequence_for<Args...>{});
                return *res;
            }
            return {};
        }

        std::size_t hash_code() const noexcept override {
            if constexpr (sizeof...(Args) == 0) {
                return rainy_typehash(void);
            } else {
                return hash_code_;
            }
        }
        
    private:
        template <std::size_t... Indices>
        void paramlist_construct_helper(containers::array<object_view, sizeof...(Args)> *buffer,
                                        type_traits::helper::index_sequence<Indices...>) {
            ((buffer->at(Indices) = std::get<Indices>(tuples)), ...);
        }

        static constexpr std::size_t hash_code_ = internals::eval_hash_code<Args...>();
        std::tuple<Args...> tuples;
    };

    template <typename... Args>
    arguments<Args...> make_arguments(Args&&... args) {
        return arguments<Args...>(utility::forward<Args>(args)...);
    }

    template <typename... Args>
    class arguments_view final : public arguments_visitor {
    public:
        arguments_view(std::tuple<Args...> &ref_tuple) : tuple_view(ref_tuple) {
        }

        arguments_view(const std::tuple<Args...> &ref_tuple) : tuple_view(ref_tuple) {
        }

        ~arguments_view() final = default;

        bool is_view() const noexcept override {
            return true;
        }

        RAINY_NODISCARD std::size_t arguments_count() const noexcept override {
            return sizeof...(Args);
        }

        RAINY_NODISCARD const std::tuple<Args...> &get_args() const {
            return tuple_view.get();
        }

        template <std::size_t Idx, typename Ty,
                  type_traits::other_trans::enable_if_t<type_traits::type_relations::is_convertible_v<Ty, decltype(std::get<Idx>())>,
                                                        int> = 0>
        void change_argument(Ty arugment) noexcept(std::is_nothrow_assignable_v<Ty, decltype(std::get<Idx>())>) {
            std::get<Idx>(tuple_view.get()) = arugment;
        }

        template <std::size_t Idx, typename = decltype(std::get<Idx>())>
        RAINY_NODISCARD decltype(auto) get_arg() const {
            return std::get<Idx>(tuple_view.get());
        }

        const std::vector<foundation::rtti::typeinfo> &typeinfo() const noexcept override {
            static std::vector<foundation::rtti::typeinfo> res = {
                foundation::rtti::typeinfo::create<type_traits::other_trans::decay_t<decltype(utility::declval<Args>())>>()...};
            return res;
        }

        containers::array_view<object_view> paramlist_view(core::byte_t *temporary_buffer) noexcept override {
            auto res = reinterpret_cast<containers::array<object_view, sizeof...(Args)> *>(temporary_buffer);
            if (res != nullptr) {
                paramlist_construct_helper(res, type_traits::helper::index_sequence_for<Args...>{});
                return *res;
            }
            return {};
        }

        std::size_t hash_code() const noexcept override {
            if constexpr (sizeof...(Args) == 0) {
                return rainy_typehash(void);
            } else {
                return hash_code_;
            }
        }

    private:
        template <std::size_t... Indices>
        void paramlist_construct_helper(containers::array<object_view, sizeof...(Args)> *buffer,
                                        type_traits::helper::index_sequence<Indices...>) {
            ((buffer->at(Indices) = std::get<Indices>(tuple_view.get())), ...);
        }

        static constexpr std::size_t hash_code_ = internals::eval_hash_code<Args...>();
        utility::reference_wrapper<std::tuple<Args...>> tuple_view;
    };

    template <typename... Args>
    arguments_view<Args...> make_arguments_view(std::tuple<Args...> &ref_tuple) {
        return arguments_view(ref_tuple);
    }

    template <typename... Args>
    const arguments_view<Args...> make_arguments_view(const std::tuple<Args...> &ref_tuple) {
        return arguments_view(ref_tuple);
    }
}

namespace rainy::meta::reflection {
    static arguments<> empty_param{};
}

#endif

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
#ifndef RAINY_FOUNDATION_POLY_HPP
#define RAINY_FOUNDATION_POLY_HPP
#include <rainy/core/core.hpp>

#define RAINY_DEF_MEM_DISPATCH(Name, member)                                                                                          \
    struct Name {                                                                                                                     \
        template <typename Ty>                                                                                                        \
        static constexpr auto get() noexcept {                                                                                        \
            return &Ty::member;                                                                                                       \
        }                                                                                                                             \
                                                                                                                                      \
        template <std::size_t I, typename, typename Base>                                                                             \
        struct mixin : Base {                                                                                                         \
            template <typename... Args>                                                                                               \
            decltype(auto) member(Args &&...args) {                                                                                   \
                return rainy::core::poly_call<I>(*this, std::forward<Args>(args)...);                                                 \
            }                                                                                                                         \
            template <typename... Args>                                                                                               \
            decltype(auto) member(Args &&...args) const {                                                                             \
                return rainy::core::poly_call<I>(*this, std::forward<Args>(args)...);                                                 \
            }                                                                                                                         \
        };                                                                                                                            \
    }

namespace rainy::foundation::poly {
    template <typename Sig>
    struct sig_to_fn_ptr;
    template <typename R, typename... Args>
    struct sig_to_fn_ptr<R(Args...)> {
        using type = R (*)(void *, Args...);
        static constexpr type placeholder = nullptr;
    };
    template <typename R, typename... Args>
    struct sig_to_fn_ptr<R(Args...) const> {
        using type = R (*)(const void *, Args...);
        static constexpr type placeholder = nullptr;
    };

    template <typename Accessor, typename Sig>
    struct convention {
        using accessor_type = Accessor;
        using signature = Sig;
    };

    namespace detail {
        template <typename... Cs>
        struct convention_list {};

        template <typename List, typename C>
        struct append;
        template <typename... Cs, typename C>
        struct append<convention_list<Cs...>, C> {
            using type = convention_list<Cs..., C>;
        };
        template <typename List, typename C>
        using append_t = typename append<List, C>::type;

        template <typename ConvList>
        struct conv_count;
        template <typename... Cs>
        struct conv_count<convention_list<Cs...>> : std::integral_constant<std::size_t, sizeof...(Cs)> {};

        template <std::size_t I, typename C>
        struct indexed_convention {
            static constexpr std::size_t index = I;
            using accessor_type = typename C::accessor_type;
            using signature = typename C::signature;
        };

        template <typename IndexSeq, typename ConvList>
        struct index_conventions_impl;
        template <std::size_t... Is, typename... Cs>
        struct index_conventions_impl<std::index_sequence<Is...>, convention_list<Cs...>> {
            using type = convention_list<indexed_convention<Is, Cs>...>;
        };

        template <typename ConvList>
        using index_conventions_t =
            typename index_conventions_impl<std::make_index_sequence<conv_count<ConvList>::value>, ConvList>::type;

        template <typename IndexedConvList, typename Ty>
        struct build_value_list;

        template <typename Ty, typename... ICs>
        struct build_value_list<convention_list<ICs...>, Ty> {
            template <typename IC>
            static constexpr auto slot() noexcept {
                if constexpr (std::is_base_of_v<rainy::core::implements::poly_inspector, Ty>) {
                    return rainy::foundation::poly::sig_to_fn_ptr<typename IC::signature>::placeholder;
                } else {
                    return IC::accessor_type::template get<Ty>();
                }
            }
            using type = rainy::type_traits::other_trans::value_list<slot<ICs>()...>;
        };

        template <typename FacadeType, typename IndexedConvList, typename Base>
        struct build_type {
            using type = Base;
        };

        template <typename FacadeType, typename IC, typename... Rest, typename Base>
        struct build_type<FacadeType, convention_list<IC, Rest...>, Base> {
            using inner = typename build_type<FacadeType, convention_list<Rest...>, Base>::type;
            using type = typename IC::accessor_type::template mixin<IC::index, FacadeType, inner>;
        };

        template <typename ConvList, bool SupportClone>
        struct facade_concept_impl {
            using indexed = index_conventions_t<ConvList>;

            template <typename Base>
            using type = typename build_type<rainy::core::basic_poly<facade_concept_impl>, indexed, Base>::type;

            template <typename Ty>
            using impl = typename build_value_list<indexed, Ty>::type;

            static constexpr bool support_clone = SupportClone;
        };

        template <typename ConvList, bool SupportClone>
        struct builder_state {
            template <typename Accessor, typename Sig>
            using add_convention = builder_state<append_t<ConvList, convention<Accessor, Sig>>, SupportClone>;

            using support_clone = builder_state<ConvList, true>;
            using build = facade_concept_impl<ConvList, SupportClone>;
        };

        struct lifecycle_ops {
            void (*destroy)(void *s, bool is_soo) noexcept;
            void (*move_to)(void *dst, void *src, bool is_soo) noexcept;
            void *(*clone)(const void *s, bool is_soo);
        };

        template <typename Ty, bool InSoo>
        constexpr lifecycle_ops make_lifecycle_ops() noexcept {
            return {[](void *s, bool) noexcept {
                        if constexpr (InSoo) {
                            static_cast<Ty *>(s)->~Ty();
                        } else {
                            delete *static_cast<Ty **>(s);
                        }
                    },
                    [](void *dst, void *src, bool) noexcept {
                        if constexpr (InSoo) {
                            ::new (dst) Ty(std::move(*static_cast<Ty *>(src)));
                            static_cast<Ty *>(src)->~Ty();
                        } else {
                            *static_cast<Ty **>(dst) = *static_cast<Ty **>(src);
                            *static_cast<Ty **>(src) = nullptr;
                        }
                    },
                    []() -> void *(*) (const void *, bool) {
                        if constexpr (std::is_copy_constructible_v<Ty>) {
                            return [](const void *s, bool) -> void * {
                                if constexpr (InSoo) {
                                    return new Ty(*static_cast<const Ty *>(s));
                                } else {
                                    return new Ty(**static_cast<const Ty *const *>(s));
                                }
                            };
                        } else {
                            return nullptr;
                        }
                    }()};
        }

        template <typename Ty, bool InSoo>
        inline constexpr lifecycle_ops lifecycle_for = make_lifecycle_ops<Ty, InSoo>();

        template <typename F, typename = void>
        struct support_clone_trait : std::false_type {};
        template <typename F>
        struct support_clone_trait<F, std::void_t<decltype(F::support_clone)>> : std::bool_constant<F::support_clone> {};

    }

    using facade_builder = detail::builder_state<detail::convention_list<>, false>;

    template <typename F>
    class viewbox : public core::basic_poly<F> {
    public:
        using base = core::basic_poly<F>;

        using base::base;
        using base::operator=;
        using base::operator->;
        using base::operator bool;
        using base::empty;
        using base::reset;
        using base::target_as_void_ptr;

        template <typename Ty>
        viewbox &operator=(Ty *ptr) noexcept {
            base::reset(ptr);
            return *this;
        }

        rain_fn reset_ptr(void *ptr) noexcept -> void {
            base::reset_ptr(ptr);
        }
    };

    template <typename F, std::size_t SooSize = 64>
    class box {
    public:
        using poly_t = viewbox<F>;
        static constexpr bool cloneable = detail::support_clone_trait<F>::value;

        box() noexcept = default;

        template <typename Ty, typename D = std::decay_t<Ty>, typename = std::enable_if_t<!std::is_same_v<D, box>>>
        explicit box(Ty &&val) {
            emplace<D>(std::forward<Ty>(val));
        }

        box(box &&o) noexcept {
            move_from(o);
        }
        box &operator=(box &&o) noexcept {
            if (this != &o) {
                reset();
                move_from(o);
            }
            return *this;
        }

        box(const box &) = delete;
        box &operator=(const box &) = delete;

        ~box() {
            reset();
        }

        template <typename Ty, typename... Args>
        Ty &emplace(Args &&...args) {
            reset();
            constexpr bool soo = sizeof(Ty) <= SooSize && alignof(Ty) <= alignof(std::max_align_t);
            if constexpr (soo) {
                Ty *p = ::new (buf()) Ty(std::forward<Args>(args)...);
                poly_.reset(p);
                ops_ = &detail::lifecycle_for<Ty, true>;
                is_soo_ = true;
                return *p;
            } else {
                Ty *p = new Ty(std::forward<Args>(args)...);
                std::memcpy(buf(), &p, sizeof(Ty *));
                poly_.reset(p);
                ops_ = &detail::lifecycle_for<Ty, false>;
                is_soo_ = false;
                return *p;
            }
        }

        void reset() noexcept {
            if (!ops_)
                return;
            ops_->destroy(buf(), is_soo_);
            poly_.reset();
            ops_ = nullptr;
        }

        template <bool C = cloneable, std::enable_if_t<C, int> = 0>
        box clone() const {
            box result;
            if (!ops_ || !ops_->clone)
                return result;
            void *raw = ops_->clone(buf(), is_soo_);
            std::memcpy(result.buf(), &raw, sizeof(void *));
            result.poly_ = poly_;
            result.poly_.reset_ptr(raw);
            result.ops_ = ops_;
            result.is_soo_ = false;
            return result;
        }

        RAINY_NODISCARD explicit operator bool() const noexcept {
            return static_cast<bool>(poly_);
        }

        RAINY_NODISCARD auto operator->() noexcept {
            return poly_.operator->();
        }

        RAINY_NODISCARD auto operator->() const noexcept {
            return poly_.operator->();
        }

        RAINY_NODISCARD bool empty() const noexcept {
            return poly_.empty();
        }

        const void *target_as_void_ptr() const noexcept {
            return poly_.target_as_void_ptr();
        }

    private:
        void *buf() noexcept {
            return storage_.data();
        }

        const void *buf() const noexcept {
            return storage_.data();
        }

        void move_from(box &o) noexcept {
            if (!o.ops_) {
                return;
            }
            ops_ = o.ops_;
            is_soo_ = o.is_soo_;
            o.ops_->move_to(buf(), o.buf(), o.is_soo_);
            poly_ = std::move(o.poly_);
            if (is_soo_) {
                poly_.reset_ptr(buf());
            } else {
                void *ptr;
                std::memcpy(&ptr, buf(), sizeof(void *));
                poly_.reset_ptr(ptr);
            }
            o.ops_ = nullptr;
        }

        alignas(std::max_align_t) std::array<char, SooSize> storage_{};
        poly_t poly_{};
        const detail::lifecycle_ops *ops_{nullptr};
        bool is_soo_{false};
    };
}
#endif

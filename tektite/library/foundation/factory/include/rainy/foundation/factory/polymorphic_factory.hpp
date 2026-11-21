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
#ifndef RAINY_FOUNDATION_FACTORY_POLYMORPHIC_FACTORY_HPP
#define RAINY_FOUNDATION_FACTORY_POLYMORPHIC_FACTORY_HPP
#include <rainy/foundation/factory/fact.hpp>

namespace rainy::foundation::fact {
    /**
     * \lang english
     * @brief A convenience wrapper around unsynchronized_factory for "interface base class -> many implementations".
     *
     * Products are constructed with uniform arguments (usually nothing) and returned through
     * a smart-pointer projection holder (nebula_ptr by default). It adds conveniences that a
     * polymorphic creation scenario typically needs: listing all registered keys, creating
     * every registered implementation, and registering a type by its name without writing a
     * lambda manually.
     *
     * @tparam Base The interface (base) type; creators must return types convertible to Base *.
     * @tparam Map The associative container template used for storage.
     * @tparam ProdKey The key type, defaults to core::text::hashed_string.
     * @tparam ProjHolder The smart pointer template used to project created products.
     * @tparam Alloc The allocator template for storage.
     *
     * \lang simp-chinese
     * @brief 面向「接口基类 -> 多实现」场景的 unsynchronized_factory 便捷封装。
     *
     * 产品以统一参数（通常为空）构造，并通过智能指针投影持有者（默认 nebula_ptr）返回。
     * 它补充了多态创建场景常用的便捷能力：列出全部注册键、遍历创建所有实现，以及直接按
     * 类型注册而无需手写 lambda。
     *
     * @tparam Base 接口（基类）类型；创建器须返回可转换为 Base * 的类型。
     * @tparam Map 用于存储的关联容器模板。
     * @tparam ProdKey 键类型，默认为 core::text::hashed_string。
     * @tparam ProjHolder 用于投影创建产品的智能指针模板。
     * @tparam Alloc 存储使用的分配器模板。
     */
    template <typename Base, template <typename Key, typename Mapped, typename...> typename Map,
              typename ProdKey = core::text::hashed_string,
              template <typename Product, typename...> typename ProjHolder = core::memory::nebula_ptr,
              template <typename FactoryStorageType> typename Alloc = core::memory::allocator>
    class polymorphic_factory {
        using impl_t =
            unsynchronized_factory<Base, Map, ProdKey, ProjHolder, Base *(), functional::delegate, Alloc>;

    public:
        using base_type = Base;
        using product = typename impl_t::product;
        using creater = typename impl_t::creater;
        using product_key = typename impl_t::product_key;
        using projection_holder = typename impl_t::projection_holder;
        using iterator = typename impl_t::iterator;
        using const_iterator = typename impl_t::const_iterator;
        using size_type = typename impl_t::size_type;

        polymorphic_factory() = default;

        polymorphic_factory(std::initializer_list<typename impl_t::map::value_type> ilist) : impl_{ilist} {
        }

        polymorphic_factory(const polymorphic_factory &) = default;
        polymorphic_factory(polymorphic_factory &&) noexcept = default;
        polymorphic_factory &operator=(const polymorphic_factory &) = default;
        polymorphic_factory &operator=(polymorphic_factory &&) noexcept = default;

        /**
         * \lang english
         * @brief Creates a product of the implementation registered under the key.
         *
         * \lang simp-chinese
         * @brief 创建指定键所注册实现的产品实例。
         */
        projection_holder create(const product_key &id) const {
            return impl_.make_product(id);
        }

        /**
         * \lang english
         * @brief Registers a concrete implementation type under the given key.
         *
         * The type must be default-constructible and convertible to Base *.
         *
         * \lang simp-chinese
         * @brief 以给定键注册一个具体实现类型。
         *
         * 该类型必须可默认构造且可转换为 Base *。
         */
        template <typename Impl>
        polymorphic_factory &register_type(const product_key &id) {
            static_assert(type_traits::type_relations::is_base_of_v<Base, Impl>, "Impl must derive from Base");
            static_assert(type_traits::properties::is_default_constructible_v<Impl>, "Impl must be default constructible");
            impl_.emplace(id, []() -> Base * { return new Impl(); });
            return *this;
        }

        template <typename Producer>
        polymorphic_factory &emplace(const product_key &id, Producer &&producer) {
            impl_.emplace(id, utility::forward<Producer>(producer));
            return *this;
        }

        template <typename Producer>
        polymorphic_factory &try_emplace(const product_key &id, Producer &&producer) {
            impl_.try_emplace(id, utility::forward<Producer>(producer));
            return *this;
        }

        polymorphic_factory &erase(const product_key &id) noexcept {
            impl_.erase(id);
            return *this;
        }

        bool contains(const product_key &id) const noexcept {
            return impl_.contains(id);
        }

        /**
         * \lang english
         * @brief Collects all registered keys.
         *
         * \lang simp-chinese
         * @brief 收集全部已注册的键。
         */
        core::collections::vector<product_key> keys() const {
            core::collections::vector<product_key> result;
            result.reserve(impl_.size());
            for (const_iterator it = impl_.begin(); it != impl_.end(); ++it) {
                result.push_back(it->first);
            }
            return result;
        }

        /**
         * \lang english
         * @brief Creates one product for every registered implementation.
         *
         * \lang simp-chinese
         * @brief 为每个已注册实现各创建一个产品实例。
         */
        core::collections::vector<projection_holder> create_all() const {
            core::collections::vector<projection_holder> result;
            result.reserve(impl_.size());
            for (const_iterator it = impl_.begin(); it != impl_.end(); ++it) {
                result.push_back(it->second());
            }
            return result;
        }

        size_type size() const noexcept {
            return impl_.size();
        }

        iterator begin() noexcept {
            return impl_.begin();
        }

        const_iterator begin() const noexcept {
            return impl_.begin();
        }

        const_iterator cbegin() const noexcept {
            return impl_.cbegin();
        }

        iterator end() noexcept {
            return impl_.end();
        }

        const_iterator end() const noexcept {
            return impl_.end();
        }

        const_iterator cend() const noexcept {
            return impl_.cend();
        }

        const impl_t &get_impl() const noexcept {
            return impl_;
        }

    private:
        impl_t impl_;
    };
}

#endif

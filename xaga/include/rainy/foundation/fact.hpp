#ifndef RAINY_FOUNDATION_FACT_HPP
#define RAINY_FOUNDATION_FACT_HPP
#include <rainy/core/core.hpp>
#include <rainy/foundation/functional/functional.hpp>
#include <rainy/text/hashed_string.hpp>
#include <rainy/collections/dense_map.hpp>
#include <rainy/foundation/system/memory/nebula_ptr.hpp>

namespace rainy::foundation::fact {
    template <typename Prod, typename ProdKey = text::hashed_string,
              template <typename Prod, typename...> typename ProjHolder = system::memory::nebula_ptr, typename Fx = Prod *(),
              template <typename Fx, typename...> typename Creater = functional::delegate,
              template <typename Key, typename Mapped, typename...> typename Map = collections::dense_map,
              template <typename FactoryStorageType> typename Alloc = system::memory::allocator>
    class unsynchronized_factory {
    public:
        using product = Prod;
        using creater = Creater<Fx>;
        using product_key = ProdKey;
        using value_type = typename Map<product_key, creater>::value_type;
        using allocator_type = Alloc<value_type>;
        using map = typename type_traits::extras::templates::replace_last_correct<Map<product_key, creater>, allocator_type>::type;
        using iterator = typename map::iterator;
        using const_iterator = typename map::const_iterator;
        using size_type = typename map::size_type;
        using projection_holder = ProjHolder<Prod>;

        unsynchronized_factory() noexcept = default;

        unsynchronized_factory(std::initializer_list<typename map::value_type> ilist) : storage{ilist} {};

        unsynchronized_factory(const unsynchronized_factory &right) : storage{right.storage} {
        }

        unsynchronized_factory(unsynchronized_factory &&right) noexcept : storage{utility::move(right.storage)} {
        }

        unsynchronized_factory(std::size_t reserved) : storage{} {
            storage.reserve(reserved);
        }

        unsynchronized_factory(const allocator_type &allocator, std::size_t reserved) : storage{allocator} {
            storage.reserve(reserved);
        }

        unsynchronized_factory(const allocator_type &allocator) : storage{allocator} {
        }

        unsynchronized_factory &operator=(const unsynchronized_factory &) noexcept = default;
        unsynchronized_factory &operator=(unsynchronized_factory &&) noexcept = default;

        iterator begin() noexcept {
            return storage.begin();
        }

        const_iterator begin() const noexcept {
            return storage.begin();
        }

        const_iterator cbegin() const noexcept {
            return storage.cbegin();
        }

        iterator end() noexcept {
            return storage.end();
        }

        const_iterator end() const noexcept {
            return storage.end();
        }

        const_iterator cend() const noexcept {
            return storage.end();
        }

        template <typename... Args, type_traits::other_trans::enable_if_t<
                                        type_traits::type_properties::is_invocable_r_v<projection_holder, Fx, Args...>, int> = 0>
        projection_holder make_product(const product_key &id, Args &&...args) const {
            if (const auto res = find(id); res != end()) {
                return projection_holder{res->second(utility::forward<Args>(args)...)};
            }
            return projection_holder{nullptr};
        }

        template <typename Producer>
        unsynchronized_factory &emplace(const product_key &id, Producer &&producer) {
            storage.emplace(id, utility::forward<Producer>(producer));
            return *this;
        }

        template <typename Producer, bool Cond>
        unsynchronized_factory &emplace_if(const product_key &id, Producer &&producer) {
            if constexpr (Cond) {
                storage.emplace(id, utility::forward<Producer>(producer));
            }
            return *this;
        }

        template <typename Producer>
        unsynchronized_factory &try_emplace(const product_key &id, Producer &&producer) {
            storage.try_emplace(id, utility::forward<Producer>(producer));
            return *this;
        }

        template <typename Producer, bool Cond>
        unsynchronized_factory &try_emplace_if(const product_key &id, Producer &&producer) {
            if constexpr (Cond) {
                storage.try_emplace(id, utility::forward<Producer>(producer));
            }
            return *this;
        }

        const creater &operator[](const product_key &id) const noexcept {
            return storage.at(id);
        }

        iterator find(const product_key &id) noexcept {
            return storage.find(id);
        }

        const_iterator find(const product_key &id) const noexcept {
            return storage.find(id);
        }

        const creater &get_creater(const product_key &id) const noexcept {
            return storage.at(id);
        }

        bool contains(const product_key &id) const noexcept {
            return storage.contains(id);
        }

        template <typename Producer>
        unsynchronized_factory &rebind_creater(const product_key &id, Producer &&producer) {
            if (auto it = find(id); it != storage.end()) {
                it->second = utility::forward<Producer>(producer);
            } else {
                emplace(id, utility::forward<Producer>(producer));
            }
            return *this;
        }

        size_type size() const noexcept {
            return storage.size();
        }

        size_type max_size() const noexcept {
            return storage.max_size();
        }

        unsynchronized_factory &erase(const product_key &id) noexcept {
            storage.erase(id);
            return *this;
        }

        void clear() {
            storage.clear();
        }

        void swap(unsynchronized_factory &right) noexcept {
            storage.swap(right.storage);
        }

        allocator_type get_allocator() const noexcept {
            return storage.get_allocator();
        }

    private:
        map storage;
    };

    template <typename Prod, typename ProdKey = text::hashed_string,
              template <typename Prod, typename...> typename ProjHolder = system::memory::nebula_ptr, typename Fx = Prod *(),
              template <typename Fx, typename...> typename Creater = functional::delegate,
              template <typename Key, typename Mapped, typename...> typename Map = collections::dense_map,
              template <typename FactoryStorageType> typename Alloc = system::memory::allocator>
    class synchronized_factory {
    public:
        using impl_t = unsynchronized_factory<Prod, ProdKey, ProjHolder, Fx, Creater, Map, Alloc>;

        using product = typename impl_t::product;
        using creater = typename impl_t::creater;
        using product_key = typename impl_t::product_key;
        using value_type = typename impl_t::value_type;
        using allocator_type = typename impl_t::allocator_type;
        using map = typename impl_t::map;
        using iterator = typename impl_t::iterator;
        using const_iterator = typename impl_t::const_iterator;
        using size_type = typename impl_t::size_type;
        using projection_holder = typename impl_t::projection_holder;

        synchronized_factory() noexcept = default;

        synchronized_factory(std::initializer_list<typename map::value_type> ilist) : impl_{ilist} {};

        synchronized_factory(const synchronized_factory &right) : impl_{right.impl_} {
        }

        synchronized_factory(synchronized_factory &&right) noexcept : impl_{utility::move(right.impl_)} {
        }

        synchronized_factory(size_type reserved) : impl_{reserved} {
        }

        synchronized_factory(const allocator_type &allocator, size_type reserved) : impl_{allocator, reserved} {
        }

        synchronized_factory(const allocator_type &allocator) : impl_{allocator} {
        }

        synchronized_factory &operator=(const synchronized_factory &) noexcept = default;
        synchronized_factory &operator=(synchronized_factory &&) noexcept = default;

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
            return impl_.end();
        }

        template <typename... Args, type_traits::other_trans::enable_if_t<
                                        type_traits::type_properties::is_invocable_r_v<projection_holder, Fx, Args...>, int> = 0>
        projection_holder make_product(const product_key &id, Args &&...args) const {
            return impl_.make_product(id, utility::forward<Args>(args)...);
        }

        template <typename Producer>
        synchronized_factory &emplace(const product_key &id, Producer &&producer) {
            std::lock_guard lock{mtx_};
            impl_.emplace(id, utility::forward<Producer>(producer));
            return *this;
        }

        template <typename Producer, bool Cond>
        synchronized_factory &emplace_if(const product_key &id, Producer &&producer) {
            if constexpr (Cond) {
                std::lock_guard lock{mtx_};
                impl_.emplace(id, utility::forward<Producer>(producer));
            }
            return *this;
        }

        template <typename Producer>
        synchronized_factory &try_emplace(const product_key &id, Producer &&producer) {
            std::lock_guard lock{mtx_};
            impl_.try_emplace(id, utility::forward<Producer>(producer));
            return *this;
        }

        template <typename Producer, bool Cond>
        synchronized_factory &try_emplace_if(const product_key &id, Producer &&producer) {
            if constexpr (Cond) {
                std::lock_guard lock{mtx_};
                impl_.try_emplace(id, utility::forward<Producer>(producer));
            }
            return *this;
        }

        const creater &operator[](const product_key &id) const noexcept {
            return impl_.operator[](id);
        }

        iterator find(const product_key &id) noexcept {
            return impl_.find(id);
        }

        const_iterator find(const product_key &id) const noexcept {
            return impl_.find(id);
        }

        const creater &get_creater(const product_key &id) const noexcept {
            return impl_.get_creater(id);
        }

        bool contains(const product_key &id) const noexcept {
            return impl_.contains(id);
        }

        template <typename Producer>
        synchronized_factory &rebind_creater(const product_key &id, Producer &&producer) {
            std::lock_guard lock{mtx_};
            impl_.rebind_creater(id, utility::forward<Producer>(producer));
            return *this;
        }

        size_type size() const noexcept {
            return impl_.size();
        }

        size_type max_size() const noexcept {
            return impl_.max_size();
        }

        synchronized_factory &erase(const product_key &id) noexcept {
            std::lock_guard lock{mtx_};
            impl_.erase(id);
            return *this;
        }

        void clear() {
            std::lock_guard lock{mtx_};
            impl_.clear();
        }

        void swap(synchronized_factory &right) noexcept {
            std::lock_guard lock{mtx_};
            impl_.swap(right.impl_);
        }

        allocator_type get_allocator() const noexcept {
            return impl_.get_allocator();
        }

        const impl_t &get_impl() const noexcept {
            return impl_;
        }

    private:
        impl_t impl_;
        std::mutex mtx_;
    };
}

namespace rainy::foundation::fact::implements {
    template <typename Factory>
    struct factory_traits_impl {};

    template <
        typename Prod, typename ProdKey, template <typename Prod, typename...> typename ProjHolder, typename Fx,
        template <typename Fx, typename...> typename Creater, template <typename Key, typename Mapped, typename...> typename Map,
        template <typename FactoryStorageType> typename Alloc,
        template <typename, typename, template <typename, typename...> typename, typename, template <typename, typename...> typename,
                  template <typename, typename, typename...> typename, template <typename> typename> typename FactoryTemplate>
    struct factory_traits_impl<FactoryTemplate<Prod, ProdKey, ProjHolder, Fx, Creater, Map, Alloc>> {
        using factory = FactoryTemplate<Prod, ProdKey, ProjHolder, Fx, Creater, Map, Alloc>;
        using product = Prod;
        using creater = typename factory::creater;
        using product_key = typename factory::product_key;
        using value_type = typename factory::value_type;
        using allocator_type = typename factory::allocator_type;
        using map = typename factory::map;
        using iterator = typename factory::iterator;
        using const_iterator = typename factory::const_iterator;
        using size_type = typename factory::size_type;
        using projection_holder = typename factory::projection_holder;

        template <typename... Args>
        using is_constructible_from_factory =
            type_traits::helper::bool_constant<type_traits::type_properties::is_invocable_r_v<projection_holder, creater, Args...>>;

        using is_default_constructible_from_factory =
            type_traits::helper::bool_constant<type_traits::type_properties::is_invocable_r_v<projection_holder, creater>>;

        template <typename... Args>
        static projection_holder make_product(const factory &factory, const product_key &id, Args &&...args) {
            return factory.make_product(id, utility::forward<Args>(args)...);
        }

        template <typename Producer>
        static factory &emplace(factory &factory, const product_key &id, Producer &&producer) {
            return factory.emplace(id, utility::forward<Producer>(producer));
        }
    };
}

namespace rainy::foundation::fact {
    template <typename Factory>
    struct factory_traits : type_traits::helper::false_type {};

    template <typename Prod, typename ProdKey, template <typename Prod, typename...> typename ProjHolder, typename Fx,
              template <typename Fx, typename...> typename Creater, template <typename Key, typename Mapped, typename...> typename Map,
              template <typename FactoryStorageType> typename Alloc>
    struct factory_traits<unsynchronized_factory<Prod, ProdKey, ProjHolder, Fx, Creater, Map, Alloc>>
        : type_traits::helper::true_type,
          implements::factory_traits_impl<unsynchronized_factory<Prod, ProdKey, ProjHolder, Fx, Creater, Map, Alloc>> {};

    template <typename Prod, typename ProdKey, template <typename Prod, typename...> typename ProjHolder, typename Fx,
              template <typename Fx, typename...> typename Creater, template <typename Key, typename Mapped, typename...> typename Map,
              template <typename FactoryStorageType> typename Alloc>
    struct factory_traits<synchronized_factory<Prod, ProdKey, ProjHolder, Fx, Creater, Map, Alloc>>
        : type_traits::helper::true_type,
          implements::factory_traits_impl<synchronized_factory<Prod, ProdKey, ProjHolder, Fx, Creater, Map, Alloc>> {};

    template <typename Factory>
    struct is_factory : type_traits::helper::bool_constant<factory_traits<Factory>::value> {};

    template <typename Factory>
    RAINY_CONSTEXPR_BOOL is_factory_v = is_factory<Factory>::value;

    template <typename Factory>
    struct product {
        using type = factory_traits<Factory>::product;
    };

    template <typename Factory>
    using product_t = typename product<Factory>::type;

    template <typename Factory>
    struct creater {
        using type = factory_traits<Factory>::creater;
    };

    template <typename Factory>
    using creater_t = typename creater<Factory>::type;

    template <typename Factory>
    struct product_key {
        using type = factory_traits<Factory>::product_key;
    };

    template <typename Factory>
    using product_key_t = typename product_key<Factory>::type;

    template <typename Factory>
    struct projection_holder {
        using type = factory_traits<Factory>::projection_holder;
    };

    template <typename Factory>
    using projection_holder_t = typename projection_holder<Factory>::type;

    template <typename Factory, typename... Args>
    using is_constructible_from_factory = typename factory_traits<Factory>::template is_constructible_from_factory<Args...>;

    template <typename Factory, typename... Args>
    RAINY_CONSTEXPR_BOOL is_constructible_from_factory_v = is_constructible_from_factory<Factory, Args...>::value;

    template <typename Factory>
    using is_default_constructible_from_factory = typename factory_traits<Factory>::is_default_constructible_from_factory;

    template <typename Factory>
    RAINY_CONSTEXPR_BOOL is_default_constructible_from_factory_v = is_default_constructible_from_factory<Factory>::value;

    template <
        typename Factory, typename... Args,
        type_traits::other_trans::enable_if_t<is_factory_v<Factory> && is_constructible_from_factory_v<Factory, Args...>, int> = 0>
    auto make_product_from_factory(const Factory &factory, product_key_t<Factory> id, Args &&...args) -> projection_holder_t<Factory> {
        return factory_traits<Factory>::make_product(factory, id, utility::forward<Args>(args)...);
    }

    template <typename Factory, typename Producer>
    auto emplace_product_to_factory(Factory &factory, product_key_t<Factory> id, Producer &&producer) -> Factory & {
        return factory_traits<Factory>::emplace(factory, id, utility::forward<Producer>(producer));
    }
}

#endif
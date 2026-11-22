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
#ifndef RAINY_FOUNDATION_FACTORY_AUTO_REGISTER_HPP
#define RAINY_FOUNDATION_FACTORY_AUTO_REGISTER_HPP
#include <rainy/foundation/factory/fact.hpp>

namespace rainy::foundation::fact {
    /**
     * \lang english
     * @brief Holds a process-wide singleton instance of a factory.
     *
     * The instance is a function-local static, so it is thread-safe to initialize
     * and its destruction order is reverse of first use.
     *
     * @tparam Factory A factory type recognized by factory_traits.
     *
     * \lang simp-chinese
     * @brief 持有某个工厂的进程级单例实例。
     *
     * 实例是函数局部静态对象，因此初始化是线程安全的，其析构顺序为逆首次使用顺序。
     *
     * @tparam Factory 可被 factory_traits 识别的工厂类型。
     */
    template <typename Factory>
    class auto_registrar {
    public:
        using factory_type = Factory;

        static Factory &instance() {
            static Factory factory;
            return factory;
        }
    };

    /**
     * \lang english
     * @brief A token that registers a producer into a factory during static initialization.
     *
     * Constructing a token at namespace scope registers the producer before main() runs.
     * The token itself holds no state beyond the registration side effect.
     *
     * @tparam Factory A factory type recognized by factory_traits.
     *
     * \lang simp-chinese
     * @brief 在静态初始化阶段向工厂注册生产者的令牌。
     *
     * 在命名空间作用域构造令牌即可在 main() 之前完成注册。令牌本身除注册副作用外不持有状态。
     *
     * @tparam Factory 可被 factory_traits 识别的工厂类型。
     */
    template <typename Factory>
    class registration_token {
    public:
        using factory_type = Factory;

        template <typename Key, typename Producer>
        explicit registration_token(const Key &id, Producer &&producer,
                                    Factory &factory = auto_registrar<Factory>::instance()) {
            factory.emplace(id, utility::forward<Producer>(producer));
        }

        registration_token(const registration_token &) = delete;
        registration_token &operator=(const registration_token &) = delete;
    };
}

#define RAINY_FACTORY_CONCAT_IMPL(left, right) left##right
#define RAINY_FACTORY_CONCAT(left, right) RAINY_FACTORY_CONCAT_IMPL(left, right)

/**
 * \lang english
 * @brief Registers a producer into a factory's auto_registrar singleton at static initialization time.
 *
 * Must be used at namespace scope. Each expansion creates a uniquely named token via __COUNTER__.
 *
 * @param factory_type The factory type (e.g. a using alias of unsynchronized_factory<...>).
 * @param id The product key, typically a string literal.
 * @param producer A callable or functor convertible to the factory's creater.
 *
 * \lang simp-chinese
 * @brief 在静态初始化阶段向工厂的 auto_registrar 单例注册生产者。
 *
 * 必须在命名空间作用域使用。每次展开通过 __COUNTER__ 生成唯一的令牌名。
 *
 * @param factory_type 工厂类型（如 unsynchronized_factory<...> 的 using 别名）。
 * @param id 产品键，通常为字符串字面量。
 * @param producer 可转换为工厂 creater 的可调用对象或仿函数。
 */
#define RAINY_FACTORY_REGISTER(factory_type, id, producer)                                                                             \
    static ::rainy::foundation::fact::registration_token<factory_type> RAINY_FACTORY_CONCAT(rainy_factory_token_, __COUNTER__){id,    \
                                                                                                                            producer}

#endif

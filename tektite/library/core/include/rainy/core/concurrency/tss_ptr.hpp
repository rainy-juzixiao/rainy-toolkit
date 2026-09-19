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
#ifndef RAINY_CORE_CONCURRENCY_TSS_PTR_HPP
#define RAINY_CORE_CONCURRENCY_TSS_PTR_HPP
#include <rainy/core/layer.hpp>

namespace rainy::core::concurrency {
    /**
     * \lang english
     * @brief Thread-specific storage pointer.
     *
     *  Stores a pointer with thread-local storage semantics; each thread observes its own value.
     *
     * @tparam Ty The type of the stored pointer
     *
     * \lang simp-chinese
     * @brief 线程特定存储指针。
     *
     *  以线程本地存储语义保存指针，每个线程看到各自的值。
     *
     * @tparam Ty 存储指针的类型
     */
    template <typename Ty>
    class tss_ptr {
    public:
        /**
         * \lang english
         * @brief Creates a thread-specific storage key.
         *
         * \lang simp-chinese
         * @brief 创建一个线程特定存储键。
         */
        tss_ptr() : tss_key{layer::tss_create()} {
        }

        /**
         * \lang english
         * @brief Destroys the thread-specific storage key.
         *
         * \lang simp-chinese
         * @brief 销毁线程特定存储键。
         */
        ~tss_ptr() {
            layer::tss_delete(tss_key);
        }

        /**
         * \lang english
         * @brief Returns the stored pointer of the calling thread.
         *
         * @return The pointer stored for the calling thread.
         *
         * \lang simp-chinese
         * @brief 返回当前线程存储的指针。
         *
         * @return 当前线程存储的指针。
         */
        operator Ty *() { // NOLINT
            return static_cast<Ty*>(layer::tss_get(tss_key));
        }

        /**
         * \lang english
         * @brief Sets the stored pointer of the calling thread.
         *
         * @param value The pointer to store.
         *
         * \lang simp-chinese
         * @brief 设置当前线程存储的指针。
         *
         * @param value 要存储的指针。
         */
        void operator=(Ty *value) {
            layer::tss_set(tss_key, value);
        }

    private:
        core::handle tss_key;
    };
}

#endif
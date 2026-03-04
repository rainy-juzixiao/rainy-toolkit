#ifndef RAINY_META_REFLECTION_LIBRARY_HPP
#define RAINY_META_REFLECTION_LIBRARY_HPP
#include <rainy/core/core.hpp>
#include <rainy/foundation/pal/module_context.hpp>
#include <rainy/meta/reflection/registration.hpp>
#include <string_view>

namespace rainy::meta::reflection {
    /**
     * @brief 反射库的模块加载器，负责加载动态库并追踪其注册的反射类型
     */
    class library {
    public:
        /**
         * @brief 默认构造，不加载任何模块
         */
        library() = default;

        ~library() {
            unload();
        }

        /**
         * @brief 尝试加载指定路径的模块，并记录其注册的反射类型
         * @param module_path 模块路径，包括模块名
         * @return 加载成功返回 true，否则 false
         */
        bool load(const std::string_view module_path) noexcept {
            // 我们在加载前，开启事务以截取快照
            state.begin_transaction();
            if (!context_.load(module_path)) {
                return false;
            }
            state.end_transaction();
            is_active = true;
            return true;
        }

        /**
         * @brief 释放当前模块，并清空已记录的类型
         */
        void unload() noexcept {
            if (is_active) {
                state.clear();
                context_.release();
                is_active = false;
            }
        }

        /**
         * @brief 检查模块是否已加载
         * @return 已加载返回 true，否则 false
         */
        RAINY_NODISCARD bool is_loaded() const noexcept {
            return context_.is_loaded();
        }

        /**
         * @brief 获取本模块注册的所有反射类型
         * @return 从 ctti 到 type_accessor* 的哈希表
         */
        RAINY_NODISCARD const collections::unordered_map<foundation::ctti::typeinfo, implements::type_accessor *> &get_types() const noexcept {
            return state.get_registered_types();
        }

        /**
         * @brief 获取本模块注册的所有反射类型
         * @return 从 ctti 到 type_accessor* 的哈希表
         */
        RAINY_NODISCARD const collections::unordered_multimap<std::string_view, method>& get_global_functions() const noexcept {
            return state.get_registered_functions();
        }

    private:
        foundation::pal::module_context::context context_;
        implements::registration_state state;
        bool is_active = false;
    };
}

#endif
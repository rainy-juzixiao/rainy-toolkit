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
#include <rainy/meta/reflection/function.hpp>

namespace rainy::meta::reflection {
    function::function() noexcept : invoker_storage{0} {
    }

    function::function(function &&right) noexcept {
        move_from_other(utility::move(right));
    }

    function::function(const function &right) noexcept {
        copy_from_other(right);
    }

    function::function(std::nullptr_t) noexcept : invoker_storage{} {
    }

    utility::any function::invoke_variadic(object_view instance, collections::views::array_view<utility::any> any_args) const {
        utility::expects(!empty());
        rainy_let invoker = reinterpret_cast<const implements::remote_invoker *>(invoker_storage);
        std::size_t arg_count = any_args.size();
        if (arg_count == 0) {
            return invoker->invoke(instance);
        }
        switch (arg_count) {
            case 1:
                return invoker->invoke(instance, any_args[0]);
            case 2:
                return invoker->invoke(instance, any_args[0], any_args[1]);
            case 3:
                return invoker->invoke(instance, any_args[0], any_args[1], any_args[2]);
            case 4:
                return invoker->invoke(instance, any_args[0], any_args[1], any_args[2], any_args[3]);
            case 5:
                return invoker->invoke(instance, any_args[0], any_args[1], any_args[2], any_args[3], any_args[4]);
            case 6:
                return invoker->invoke(instance, any_args[0], any_args[1], any_args[2], any_args[3], any_args[4], any_args[5]);
            case 7:
                return invoker->invoke(instance, any_args[0], any_args[1], any_args[2], any_args[3], any_args[4], any_args[5],
                                       any_args[6]);
            default:
                return invoker->invoke_variadic(instance, any_args);
        }
    }

    utility::any function::invoke(object_view object) const {
        utility::expects(!empty());
        return reinterpret_cast<const implements::remote_invoker *>(invoker_storage)->invoke(object);
    }

    utility::any function::invoke(object_view object, utility::any ax1) const {
        utility::expects(!empty());
        return reinterpret_cast<const implements::remote_invoker *>(invoker_storage)->invoke(object, ax1);
    }

    utility::any function::invoke(object_view object, utility::any ax1, utility::any ax2) const {
        utility::expects(!empty());
        return reinterpret_cast<const implements::remote_invoker *>(invoker_storage)->invoke(object, ax1, ax2);
    }

    utility::any function::invoke(object_view object, utility::any ax1, utility::any ax2, utility::any ax3) const {
        utility::expects(!empty());
        return reinterpret_cast<const implements::remote_invoker *>(invoker_storage)->invoke(object, ax1, ax2, ax3);
    }

    utility::any function::invoke(object_view object, utility::any ax1, utility::any ax2, utility::any ax3, utility::any ax4) const {
        utility::expects(!empty());
        return reinterpret_cast<const implements::remote_invoker *>(invoker_storage)->invoke(object, ax1, ax2, ax3, ax4);
    }

    utility::any function::invoke(object_view object, utility::any ax1, utility::any ax2, utility::any ax3, utility::any ax4,
                                  utility::any ax5) const {
        utility::expects(!empty());
        return reinterpret_cast<const implements::remote_invoker *>(invoker_storage)->invoke(object, ax1, ax2, ax3, ax4, ax5);
    }

    utility::any function::invoke(object_view object, utility::any ax1, utility::any ax2, utility::any ax3, utility::any ax4,
                                  utility::any ax5, utility::any ax6) const {
        utility::expects(!empty());
        return reinterpret_cast<const implements::remote_invoker *>(invoker_storage)->invoke(object, ax1, ax2, ax3, ax4, ax5, ax6);
    }

    utility::any function::invoke(object_view object, utility::any ax1, utility::any ax2, utility::any ax3, utility::any ax4,
                                  utility::any ax5, utility::any ax6, utility::any ax7) const {
        utility::expects(!empty());
        return reinterpret_cast<const implements::remote_invoker *>(invoker_storage)
            ->invoke(object, ax1, ax2, ax3, ax4, ax5, ax6, ax7);
    }

    const foundation::rtti::typeinfo &function::return_type() const noexcept {
        utility::expects(!empty());
        return reinterpret_cast<const implements::remote_invoker *>(invoker_storage)->return_type();
    }

    rainy::collections::views::array_view<foundation::rtti::typeinfo> function::param_lists() const noexcept {
        utility::expects(!empty());
        return reinterpret_cast<const implements::remote_invoker *>(invoker_storage)->param_types();
    }

    bool function::empty() const noexcept {
        struct canary {
            std::int64_t header;
        };
        static_assert(sizeof(canary) <= soo_buffer_size);
        return reinterpret_cast<const canary *>(invoker_storage)->header == 0;
    }

    void function::copy_from_other(const function &right) noexcept {
        if (this != utility::addressof(right) && !right.empty()) {
            reinterpret_cast<const implements::remote_invoker *>(right.invoker_storage)->construct_from_this(this->invoker_storage);
        }
    }

    void function::move_from_other(function &&right) noexcept {
        if (this != utility::addressof(right) && !right.empty()) {
            reinterpret_cast<const implements::remote_invoker *>(right.invoker_storage)->construct_from_this(this->invoker_storage);
            std::memset(right.invoker_storage, 0, sizeof(right.invoker_storage));
        }
    }

    void function::swap(function &right) noexcept {
        alignas(std::max_align_t) char temporary_memory[soo_buffer_size]{};
        core::builtin::copy_memory(&temporary_memory, &right.invoker_storage, soo_buffer_size);
        core::builtin::copy_memory(&right.invoker_storage, &this->invoker_storage, soo_buffer_size);
        core::builtin::copy_memory(&this->invoker_storage, &temporary_memory, soo_buffer_size);
    }

    function &function::operator=(const function &right) noexcept {
        if (this == utility::addressof(right)) {
            return *this;
        }
        copy_from_other(right);
        return *this;
    }

    function &function::operator=(function &&right) noexcept {
        move_from_other(utility::move(right));
        return *this;
    }

    function &function::operator=(std::nullptr_t) noexcept {
        clear();
        return *this;
    }

    const foundation::rtti::typeinfo &function::function_signature() const noexcept {
        utility::expects(!empty());
        return reinterpret_cast<const implements::remote_invoker *>(invoker_storage)->function_signature();
    }

    method_type function::type() const noexcept {
        utility::expects(!empty());
        return reinterpret_cast<const implements::remote_invoker *>(invoker_storage)->type();
    }

    function::operator bool() const noexcept {
        return !empty();
    }

    bool function::equal_with(const function &right) const noexcept {
        if (this->empty() && right.empty()) {
            return true;
        }
        if (this->empty() || right.empty()) {
            return false;
        }
        return reinterpret_cast<const implements::remote_invoker *>(invoker_storage)
            ->equal_with(reinterpret_cast<const implements::remote_invoker *>(right.invoker_storage));
    }

    bool function::not_equal_with(const function &right) const noexcept {
        return !equal_with(right);
    }

    void function::clear() noexcept {
        invoker_storage[0] = '\0';
        std::memset(invoker_storage, 0, soo_buffer_size);
    }

    void function::rebind(function &&function) noexcept {
        move_from_other(utility::move(function));
    }

    void function::rebind(const function &function) noexcept {
        copy_from_other(function);
    }

    void function::rebind(std::nullptr_t) noexcept {
        clear();
    }

    const foundation::rtti::typeinfo &function::which_belongs() const noexcept {
        utility::expects(!empty());
        return reinterpret_cast<const implements::remote_invoker *>(invoker_storage)->which_belongs();
    }

    bool function::is_static() const noexcept {
        if (empty()) {
            return false;
        }
        return type() == method_type::static_method || type() == method_type::static_method_noexcept;
    }

    bool function::is_memfn() const noexcept {
        if (empty()) {
            return false;
        }
        return !is_static();
    }

    bool function::is_volatile() const noexcept {
        if (empty()) {
            return false;
        }
        method_type method_type = type();
        if (method_type >= method_type::volatile_method && method_type <= method_type::volatile_method_right_noexcept) {
            return true;
        }
        return method_type >= method_type::const_volatile_method && method_type <= method_type::const_volatile_method_right_noexcept;
    }

    bool function::is_const() const noexcept {
        if (empty()) {
            return false;
        }
        method_type method_type = type();
        return method_type >= method_type::const_method && method_type <= method_type::const_volatile_method_right_noexcept;
    }

    bool function::is_noexcept() const noexcept {
        if (empty()) {
            return false;
        }
        method_type method_type = type();
        switch (method_type) {
            case method_type::static_method_noexcept:
            case method_type::normal_method_noexcept:
            case method_type::normal_method_left_noexcept:
            case method_type::normal_method_right_noexcept:
            case method_type::volatile_method_noexcept:
            case method_type::volatile_method_left_noexcept:
            case method_type::volatile_method_right_noexcept:
            case method_type::const_method_noexcept:
            case method_type::const_method_left_noexcept:
            case method_type::const_method_right_noexcept:
            case method_type::const_volatile_method_noexcept:
            case method_type::const_volatile_method_left_noexcept:
            case method_type::const_volatile_method_right_noexcept:
                return true;
            default:
                break;
        }
        return false;
    }

    bool function::is_invoke_for_lvalue() const noexcept {
        if (empty()) {
            return false;
        }
        method_type method_type = type();
        switch (method_type) {
            case method_type::normal_method_left:
            case method_type::normal_method_left_noexcept:
            case method_type::volatile_method_left:
            case method_type::volatile_method_left_noexcept:
            case method_type::const_method_left:
            case method_type::const_method_left_noexcept:
            case method_type::const_volatile_method_left:
            case method_type::const_volatile_method_left_noexcept:
                return true;
            default:
                break;
        }
        return false;
    }

    bool function::is_invoke_for_rvalue() const noexcept {
        if (empty()) {
            return false;
        }
        method_type method_type = type();
        switch (method_type) {
            case method_type::normal_method_right:
            case method_type::normal_method_right_noexcept:
            case method_type::volatile_method_right:
            case method_type::volatile_method_right_noexcept:
            case method_type::const_method_right:
            case method_type::const_method_right_noexcept:
            case method_type::const_volatile_method_right:
            case method_type::const_volatile_method_right_noexcept:
                return true;
            default:
                break;
        }
        return false;
    }

    bool function::is_invocable(collections::views::array_view<foundation::rtti::typeinfo> paramlist) const noexcept {
        return reinterpret_cast<const implements::remote_invoker *>(invoker_storage)->is_invocable(paramlist);
    }
}

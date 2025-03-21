#ifndef RAINY_META_REFLECTION_FUNCTION_CXX
#define RAINY_META_REFLECTION_FUNCTION_CXX
#include <rainy/meta/reflection/function.hpp>

namespace rainy::meta::reflection {
    function::function() noexcept : invoker_storage{}, invoker_{nullptr} {
    }

    containers::any function::invoke_variadic(instance instance, std::vector<containers::any> &any_args) const {
        utility::expects(!empty(), "You're trying to invoke a empty object!");
        return invoker_->invoke_variadic(instance, any_args);
    }

    containers::any function::invoke_variadic(instance instance, const std::vector<containers::any> &any_args) const {
        utility::expects(!empty(), "You're trying to invoke a empty object!");
        return invoker_->invoke_variadic(instance, const_cast<std::vector<containers::any> &>(any_args));
    }

    containers::any function::invoke_static(arguments_visitor &args) const {
        return invoke(invoke_strategy::as_static, non_exists_instance, args);
    }

    containers::any function::invoke_static(const arguments_visitor &args) const {
        return invoke(invoke_strategy::as_static, non_exists_instance, args);
    }

    containers::any function::invoke(instance object, arguments_visitor &args) const {
        return invoke(invoke_strategy::as_memfn, object, args);
    }
    
    containers::any function::invoke(instance object, const arguments_visitor &args) const {
        return invoke(invoke_strategy::as_memfn, object, args);
    }

    containers::any function::invoke(invoke_strategy strategy, instance object, arguments_visitor &args) const {
        utility::expects(!empty(), "You're trying to invoke a empty object!");
        switch (strategy) {
            case invoke_strategy::as_static:
                utility::expects(is_static(), "You cannot invoke a non-static function");
                return invoker_->invoke(non_exists_instance, args);
            case invoke_strategy::as_memfn:
                utility::expects(object.valid(), "You cannot pass a non-valid object");
                return invoker_->invoke(object, args);
            case invoke_strategy::as_operator:
                utility::expects(is_static(), "You cannot invoke a operator");
                return invoker_->invoke(non_exists_instance, args);
        }
        return {};
    }

    containers::any function::invoke(invoke_strategy strategy, instance object, const arguments_visitor &args) const {
        return invoke(strategy, object, const_cast<arguments_visitor &>(args));
    }

    const foundation::rtti::typeinfo &function::return_type() const noexcept {
        return invoker_->return_type();
    }

    const containers::array_view<foundation::rtti::typeinfo> function::param_lists() const noexcept {
        return invoker_->param_types();
    }

    bool function::empty() const noexcept {
        return !static_cast<bool>(invoker_);
    }

    void function::copy_from_other(const function &right) noexcept {
        if (right.empty()) {
            if (empty()) {
                return;
            }
            this->invoker_ = nullptr; // 我们不需要释放它，因为它始终指向invoker_storage，我们只需要让它不再指向就可以了
        }
        if (this != utility::addressof(right)) {
            this->invoker_ = right.invoker_->construct_from_this(this->invoker_storage);
        }
    }

    void function::move_from_other(function &&right) noexcept {
        this->invoker_ = right.invoker_->construct_from_this(this->invoker_storage);
        right.invoker_ = nullptr;
    }

    void function::swap(function &right) noexcept {
        alignas(std::max_align_t) char temporary_memory[soo_buffer_size]{};
        core::builtin::copy_memory(temporary_memory, right.invoker_storage, soo_buffer_size);
        right.invoker_ =
            static_cast<remote_invoker *>(core::builtin::copy_memory(right.invoker_storage, this->invoker_storage, soo_buffer_size));
        invoker_ = static_cast<remote_invoker *>(core::builtin::copy_memory(this->invoker_storage, temporary_memory, soo_buffer_size));
    }

    function &function::operator=(const function &right) noexcept {
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
        return invoker_->function_signature();
    }

    method_type function::type() const noexcept {
        return invoker_->type();
    }

    function::operator bool() const noexcept {
        return static_cast<bool>(invoker_);
    }

    bool function::equal_with(const function &right) const noexcept {
        if (this->empty() && right.empty()) {
            return true;
        }
        if (this->empty() || right.empty()) {
            return false;
        }
        return invoker_->equal_with(right.invoker_);
    }

    bool function::not_equal_with(const function &right) const noexcept {
        return !equal_with(right);
    }

    void function::clear() noexcept {
        // 我们只需要把指针置空即可，因为invoke_impl内部仅仅只有指针
        this->invoker_ = nullptr;
    }

    void function::rebind(function &&function) noexcept {
        move_from_other(utility::move(function));
    }

    void function::rebind(const function &function) noexcept {
        copy_from_other(function);
    }

    const foundation::rtti::typeinfo &function::which_belongs() const noexcept {
        return invoker_->which_belongs();
    }

    bool function::is_static() const noexcept {
        if (empty()) {
            return false;
        }
        return type() == method_type::static_method || type() == method_type::static_method_noexcept;
    }

    bool function::is_volatile() const noexcept {
        if (empty()) {
            return false;
        }
        method_type method_type = type();
        if (method_type >= method_type::volatile_method && method_type <= method_type::volatile_method_right_noexcept) {
            return true;
        }
        if (method_type >= method_type::const_volatile_method && method_type <= method_type::const_volatile_method_right_noexcept) {
            return true;
        }
        return false;
    }

    bool function::is_const() const noexcept {
        if (empty()) {
            return false;
        }
        method_type method_type = type();
        return (method_type >= method_type::const_method && method_type <= method_type::const_volatile_method_right_noexcept) ? true
                                                                                                                              : false;
    }
}

#endif
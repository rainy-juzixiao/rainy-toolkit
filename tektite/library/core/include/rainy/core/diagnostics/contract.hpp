/*
 * Copyright 2026 rainy-juzixiao
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain left copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef RAINY_CORE_DEBUGGING_CONTRACT_HPP
#define RAINY_CORE_DEBUGGING_CONTRACT_HPP

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <rainy/core/diagnostics/source_location.hpp>
#include <rainy/core/platform.hpp>

namespace rainy::core::diagnostics::contracts {
    class contract_violation;

    /**
     * \lang english
     * @brief Specifies the kind of contract that was violated.
     *
     * \lang simp-chinese
     * @brief 指明被违反的契约类型。
     */
    enum class assertion_kind {
        /**
         * \lang english
         * @brief A precondition contract, checked by expects.
         *
         * \lang simp-chinese
         * @brief 前置条件契约，由expects检查。
         */
        for_pre = 1,
        /**
         * \lang english
         * @brief A postcondition contract, checked by ensures.
         *
         * \lang simp-chinese
         * @brief 后置条件契约，由ensures检查。
         */
        for_post = 2,
        /**
         * \lang english
         * @brief A general assertion contract, checked by contract_assertion.
         *
         * \lang simp-chinese
         * @brief 一般断言契约，由contract_assertion检查。
         */
        for_assert = 3
    };

    /**
     * \lang english
     * @brief Specifies the semantic behavior used to handle contract violations.
     *
     * \lang simp-chinese
     * @brief 指定用于处理契约违约的语义行为。
     */
    enum class evaluation_semantic {
        /**
         * \lang english
         * @brief Ignore the violation and continue execution.
         *
         * \lang simp-chinese
         * @brief 忽略违约并继续执行。
         */
        ignore = 1,
        /**
         * \lang english
         * @brief Report the violation without terminating execution.
         *
         * \lang simp-chinese
         * @brief 报告违约但不终止执行。
         */
        observe = 2,
        /**
         * \lang english
         * @brief Report the violation and terminate the program.
         *
         * \lang simp-chinese
         * @brief 报告违约并终止程序。
         */
        enforce = 3,
        /**
         * \lang english
         * @brief Terminate immediately without invoking the violation handler.
         *
         * \lang simp-chinese
         * @brief 立即终止，不调用违约处理程序。
         */
        quick_enforce = 4
    };

    /**
     * \lang english
     * @brief Specifies how a contract violation was detected.
     *
     * \lang simp-chinese
     * @brief 指明契约违约的检测方式。
     */
    enum class detection_mode {
        /**
         * \lang english
         * @brief The contract predicate evaluated to false.
         *
         * \lang simp-chinese
         * @brief 契约谓词求值为false。
         */
        predicate_false = 1,
        /**
         * \lang english
         * @brief An exception was thrown while evaluating the contract.
         *
         * \lang simp-chinese
         * @brief 在求值契约时抛出了异常。
         */
        evaluation_exception = 2
    };
}

namespace rainy::core::diagnostics::contracts::implements {
    RAINY_INLINE void handle_contract_violation(const contract_violation &v);
}

namespace rainy::core::diagnostics::contracts {
    /**
     * \lang english
     * @brief Default contract violation handler.
     *
     * Forwards the violation to `implements::handle_contract_violation`, which
     * performs the implementation's default response (e.g. logging and/or
     * terminating). Used when no user-supplied contract violation handler has
     * been installed.
     *
     * @param violation The contract violation to handle
     *
     * \lang simp-chinese
     * @brief 默认的契约违背处理器。
     *
     * 将违背信息转发给 `implements::handle_contract_violation`，由后者执行实现的
     * 默认响应（例如记录日志和/或终止程序）。在未安装用户自定义契约违背处理器时使用。
     *
     * @param violation 要处理的契约违背信息
     */
    RAINY_INLINE void invoke_default_contract_violation_handler(const contract_violation &violation) {
        implements::handle_contract_violation(violation);
    }

    /**
     * \lang english
     * @brief Describes a contract violation that was detected at runtime.
     *
     * A contract_violation object carries the kind of the violated contract, the
     * evaluation semantic that triggered the report, how the violation was detected,
     * an optional comment and the source location where the contract was checked.
     *
     * \lang simp-chinese
     * @brief 描述在运行时检测到的契约违约。
     *
     * contract_violation 对象携带被违反契约的类型、触发报告的求值语义、
     * 违约的检测方式、可选的注释以及检查契约处的源码位置。
     */
    class contract_violation {
    public:
        /**
         * \lang english
         * @brief Deleted default constructor. Violations are created only by `make_contract_violation`.
         *
         * \lang simp-chinese
         * @brief 被删除的默认构造函数。违约对象只能由 `make_contract_violation` 创建。
         */
        contract_violation() = delete;

        /**
         * \lang english
         * @brief Deleted copy constructor. A violation object is not copyable.
         *
         * \lang simp-chinese
         * @brief 被删除的拷贝构造函数。违约对象不可拷贝。
         */
        contract_violation(const contract_violation &) = delete;

        /**
         * \lang english
         * @brief Deleted copy assignment operator. A violation object is not assignable.
         *
         * \lang simp-chinese
         * @brief 被删除的拷贝赋值运算符。违约对象不可赋值。
         */
        contract_violation &operator=(const contract_violation &) = delete;

        /**
         * \lang english
         * @brief Destructor (defaulted).
         *
         * \lang simp-chinese
         * @brief 析构函数（默认）。
         */
        ~contract_violation() = default;

        /**
         * \lang english
         * @brief Gets the optional comment attached to the violation.
         * @return A NUL-terminated string, or an empty string when no comment was provided
         *
         * \lang simp-chinese
         * @brief 获取附加在违约上的可选注释。
         * @return 以 NUL 结尾的字符串；未提供注释时为空字符串
         */
        const char *comment() const noexcept {
            return comment_;
        }

        /**
         * \lang english
         * @brief Gets how the contract violation was detected.
         * @return The detection mode of the violation
         *
         * \lang simp-chinese
         * @brief 获取契约违约的检测方式。
         * @return 违约的检测模式
         */
        contracts::detection_mode detection_mode() const noexcept {
            return detection_mode_;
        }

        /**
         * \lang english
         * @brief Checks whether the violation terminates the program.
         * @return true if the semantic is enforce or quick_enforce, otherwise false
         *
         * \lang simp-chinese
         * @brief 检查该违约是否会终止程序。
         * @return 若语义为 enforce 或 quick_enforce 则返回 true，否则返回 false
         */
        bool is_terminating() const noexcept {
            return semantic_ == evaluation_semantic::enforce || semantic_ == evaluation_semantic::quick_enforce;
        }

        /**
         * \lang english
         * @brief Gets the kind of the violated contract.
         * @return The assertion kind of the violation
         *
         * \lang simp-chinese
         * @brief 获取被违反契约的类型。
         * @return 违约的断言类型
         */
        assertion_kind kind() const noexcept {
            return kind_;
        }

        /**
         * \lang english
         * @brief Gets the source location where the contract was checked.
         * @return The source location of the contract check
         *
         * \lang simp-chinese
         * @brief 获取检查契约处的源码位置。
         * @return 契约检查处的源码位置
         */
        source_location location() const noexcept {
            return loc_;
        }

        /**
         * \lang english
         * @brief Gets the evaluation semantic that triggered the report.
         * @return The evaluation semantic of the violation
         *
         * \lang simp-chinese
         * @brief 获取触发报告的求值语义。
         * @return 违约的求值语义
         */
        evaluation_semantic semantic() const noexcept {
            return semantic_;
        }

        /**
         * \lang english
         * @brief Grants `make_contract_violation` access to the private constructor.
         *
         * \lang simp-chinese
         * @brief 授予 `make_contract_violation` 访问私有构造函数的权限。
         */
        friend contract_violation make_contract_violation(assertion_kind, evaluation_semantic, contracts::detection_mode, const char *,
                                                          source_location);
    private:
        contract_violation(assertion_kind kind, evaluation_semantic semantic, contracts::detection_mode mode, const char *comment,
                           source_location loc) noexcept : kind_(kind), semantic_(semantic), detection_mode_(mode), loc_(loc) {
            if (comment) {
                std::strncpy(comment_, comment, sizeof(comment_) - 1);
                comment_[sizeof(comment_) - 1] = '\0';
            } else {
                comment_[0] = '\0';
            }
        }

        assertion_kind kind_{};
        evaluation_semantic semantic_{};
        contracts::detection_mode detection_mode_{};
        source_location loc_;
        char comment_[256]{};
    };

    RAINY_INLINE contract_violation make_contract_violation(assertion_kind kind, evaluation_semantic semantic, detection_mode mode,
                                                            const char *comment, source_location loc = source_location::current()) {
        return contract_violation(kind, semantic, mode, comment, loc);
    }

    /**
     * \lang english
     * @brief Returns the evaluation semantic used by default for contract checks.
     *
     * Resolved at compile time:
     * - If `RAINY_CONTRACT_SEMANTIC` is defined, that value is returned;
     * - otherwise, in debug builds (`RAINY_ENABLE_DEBUG`) `enforce` is returned;
     * - otherwise (release builds) `observe` is returned.
     *
     * @return The default evaluation semantic
     *
     * \lang simp-chinese
     * @brief 返回契约检查默认使用的求值语义。
     *
     * 在编译期确定：
     * - 若定义了 `RAINY_CONTRACT_SEMANTIC`，则返回该值；
     * - 否则在调试构建（`RAINY_ENABLE_DEBUG`）下返回 `enforce`；
     * - 否则（发布构建）返回 `observe`。
     *
     * @return 默认的求值语义
     */
    RAINY_INLINE evaluation_semantic default_semantic() noexcept {
#if defined(RAINY_CONTRACT_SEMANTIC)
        return RAINY_CONTRACT_SEMANTIC;
#elif RAINY_ENABLE_DEBUG
        return evaluation_semantic::enforce;
#else
        return evaluation_semantic::observe;
#endif
    }

    /**
     * \lang english
     * @brief Reports a contract violation if the checked condition did not pass.
     *
     * Does nothing when `passed` is true. Otherwise the default evaluation
     * semantic (see `default_semantic`) decides the outcome:
     * - `ignore`: the violation is silently ignored;
     * - `quick_enforce`: a minimal message is printed to `stderr` and the
     *   program terminates immediately, without building a violation object;
     * - any other semantic: a `contract_violation` is created with
     *   `detection_mode::predicate_false` and passed to
     *   `implements::handle_contract_violation`, after which the program
     *   terminates if the semantic is `enforce`.
     *
     * @param kind The kind of the violated contract
     * @param text The comment attached to the violation, may be null
     * @param passed Whether the checked condition held
     * @param loc The source location where the contract was checked
     *
     * \lang simp-chinese
     * @brief 在检查的条件未通过时报告契约违约。
     *
     * 当 `passed` 为 true 时不做任何事。否则由默认求值语义（见 `default_semantic`）
     * 决定后续行为：
     * - `ignore`：静默忽略该违约；
     * - `quick_enforce`：向 `stderr` 输出一条最简消息并立即终止程序，
     *   不构造违约对象；
     * - 其他语义：以 `detection_mode::predicate_false` 创建 `contract_violation`
     *   并交给 `implements::handle_contract_violation`，若语义为 `enforce`
     *   随后终止程序。
     *
     * @param kind 被违反契约的类型
     * @param text 附加在违约上的注释，可为空指针
     * @param passed 所检查的条件是否成立
     * @param loc 检查契约处的源码位置
     */
    RAINY_INLINE void check_contract(assertion_kind kind, const char *text, bool passed, const source_location &loc) {
        if (passed) {
            return;
        }
        const auto semantic = default_semantic();
        if (semantic == evaluation_semantic::ignore) {
            return;
        }
        if (semantic == evaluation_semantic::quick_enforce) {
            std::fprintf(stderr, "%s:%u: contract violated\n", loc.file_name() ? loc.file_name() : "?", loc.line());
            std::fflush(stderr);
            std::terminate();
        }
        auto violation = make_contract_violation(kind, semantic, detection_mode::predicate_false, text, loc);
        implements::handle_contract_violation(violation);
        if (semantic == evaluation_semantic::enforce) {
            std::terminate();
        }
    }
}

namespace rainy::core::diagnostics::contracts::implements {
    void handle_contract_violation(const contract_violation &v) {
        const auto loc = v.location();
        const char *k = v.kind() == contracts::assertion_kind::for_post ? "post" : "assert";
        std::fprintf(stderr, "%s:%u: %s: %s\n", loc.file_name() ? loc.file_name() : "?", loc.line(), k,
                     v.comment() ? v.comment() : "");
        std::fflush(stderr);
    }
}

namespace rainy::core::diagnostics::contracts {
    /**
     * \lang english
     * @brief Checks a precondition contract.
     *
     * @param cond The condition that must hold on entry.
     * @param msg Optional message describing the precondition.
     * @param loc The source location of the check.
     *
     * \lang simp-chinese
     * @brief 检查前置条件契约。
     *
     * @param cond 进入时必须成立的条件。
     * @param msg 描述前置条件的可选消息。
     * @param loc 检查处的源码位置。
     */
    RAINY_INLINE void expects(bool cond, const char *msg = nullptr, const source_location &loc = source_location::current()) {
        check_contract(assertion_kind::for_pre, msg ? msg : "precondition", cond, loc);
    }

    /**
     * \lang english
     * @brief Checks a postcondition contract.
     *
     * @param cond The condition that must hold on exit.
     * @param msg Optional message describing the postcondition.
     * @param loc The source location of the check.
     *
     * \lang simp-chinese
     * @brief 检查后置条件契约。
     *
     * @param cond 退出时必须成立的条件。
     * @param msg 描述后置条件的可选消息。
     * @param loc 检查处的源码位置。
     */
    RAINY_INLINE void ensures(bool cond, const char *msg = nullptr, const source_location &loc = source_location::current()) {
        check_contract(assertion_kind::for_post, msg ? msg : "postcondition", cond, loc);
    }

    /**
     * \lang english
     * @brief Checks a general assertion contract.
     *
     * @param cond The condition that must hold.
     * @param msg Optional message describing the assertion.
     * @param loc The source location of the check.
     *
     * \lang simp-chinese
     * @brief 检查一般断言契约。
     *
     * @param cond 必须成立的条件。
     * @param msg 描述断言的可选消息。
     * @param loc 检查处的源码位置。
     */
    RAINY_INLINE void contract_assertion(bool cond, const char *msg = nullptr,
                                         const source_location &loc = source_location::current()) {
        check_contract(assertion_kind::for_assert, msg ? msg : "assertion", cond, loc);
    }
}

namespace rainy::utility {
    using rainy::core::diagnostics::contracts::assertion_kind;
    using rainy::core::diagnostics::contracts::contract_assertion;
    using rainy::core::diagnostics::contracts::contract_violation;
    using rainy::core::diagnostics::contracts::detection_mode;
    using rainy::core::diagnostics::contracts::ensures;
    using rainy::core::diagnostics::contracts::evaluation_semantic;
    using rainy::core::diagnostics::contracts::expects;
}

#endif

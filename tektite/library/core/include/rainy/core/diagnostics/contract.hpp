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
        contract_violation() = delete;
        contract_violation(const contract_violation &) = delete;
        contract_violation &operator=(const contract_violation &) = delete;
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

    private:
        friend contract_violation make_contract_violation(assertion_kind, evaluation_semantic, contracts::detection_mode, const char *,
                                                          source_location);

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

    RAINY_INLINE evaluation_semantic default_semantic() noexcept {
#if defined(RAINY_CONTRACT_SEMANTIC)
        return RAINY_CONTRACT_SEMANTIC;
#elif RAINY_ENABLE_DEBUG
        return evaluation_semantic::enforce;
#else
        return evaluation_semantic::observe;
#endif
    }

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

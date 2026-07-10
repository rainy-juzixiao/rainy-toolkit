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

    enum class assertion_kind {
        pre = 1,
        post = 2,
        assert = 3
    };

    enum class evaluation_semantic {
        ignore = 1,
        observe = 2,
        enforce = 3,
        quick_enforce = 4
    };

    enum class detection_mode {
        predicate_false = 1,
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

    class contract_violation {
    public:
        contract_violation() = delete;
        contract_violation(const contract_violation &) = delete;
        contract_violation &operator=(const contract_violation &) = delete;
        ~contract_violation() = default;

        const char *comment() const noexcept {
            return comment_;
        }
        detection_mode detection_mode() const noexcept {
            return detection_mode_;
        }

        bool is_terminating() const noexcept {
            return semantic_ == evaluation_semantic::enforce || semantic_ == evaluation_semantic::quick_enforce;
        }

        assertion_kind kind() const noexcept {
            return kind_;
        }

        source_location location() const noexcept {
            return loc_;
        }

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
        const char *k = v.kind() == contracts::assertion_kind::pre    ? "pre"
                        : v.kind() == contracts::assertion_kind::post ? "post"
                                                                      : "assert";
        std::fprintf(stderr, "%s:%u: %s: %s\n", loc.file_name() ? loc.file_name() : "?", loc.line(), k,
                     v.comment() ? v.comment() : "");
        std::fflush(stderr);
    }
}

namespace rainy::core::diagnostics::contracts {
    RAINY_INLINE void expects(bool cond, const char *msg = nullptr, const source_location &loc = source_location::current()) {
        check_contract(assertion_kind::pre, msg ? msg : "precondition", cond, loc);
    }

    RAINY_INLINE void ensures(bool cond, const char *msg = nullptr, const source_location &loc = source_location::current()) {
        check_contract(assertion_kind::post, msg ? msg : "postcondition", cond, loc);
    }

    RAINY_INLINE void contract_assert(bool cond, const char *msg = nullptr, const source_location &loc = source_location::current()) {
        check_contract(assertion_kind::assert, msg ? msg : "assertion", cond, loc);
    }
}

namespace rainy::utility {
    using rainy::core::diagnostics::contracts::assertion_kind;
    using rainy::core::diagnostics::contracts::contract_assert;
    using rainy::core::diagnostics::contracts::contract_violation;
    using rainy::core::diagnostics::contracts::detection_mode;
    using rainy::core::diagnostics::contracts::ensures;
    using rainy::core::diagnostics::contracts::evaluation_semantic;
    using rainy::core::diagnostics::contracts::expects;
}

#endif

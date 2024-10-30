#ifndef RAINY_CONTRACT_HPP
#define RAINY_CONTRACT_HPP

#include <rainy/core.hpp>
#include <rainy/text/format_wrapper.hpp>

namespace rainy::utility {
    RAINY_INLINE RAINY_NODEBUG_CONSTEXPR void contract(const bool cond, const std::string_view type,
                                                       const utility::source_location &location) {
#if RAINY_HAS_CXX20
        if (std::is_constant_evaluated()) {
            if (!cond) {
                throw;
            }
        } else
#endif
        {
#if RAINY_ENABLE_DEBUG
            std::string buffer{};
#if RAINY_HAS_CXX20
            utility::format(buffer, "contract : {} triggered {}\n", type, location.to_string());
#else
            const std::string temp = location.to_string();
            utility::format(buffer, "contract : %s triggered %s\n", type.data(), temp.c_str());
#endif
            if (!cond) {
                (void) std::fwrite(buffer.c_str(), 1, buffer.size(), stderr);
                information::system_call::debug_break();
            }
#else
            if (!cond) {
                std::terminate();
            }
#endif // RAINY_ENABLE_DEBUG
        }
    }


    RAINY_INLINE RAINY_NODEBUG_CONSTEXPR void contract(const bool cond, const std::string_view type, const std::string_view &message,
                                                       const utility::source_location &location) {
#if RAINY_HAS_CXX20
        if (std::is_constant_evaluated()) {
            if (!cond) {
                throw;
            }
        } else
#endif
        {
#if RAINY_ENABLE_DEBUG
            std::string buffer{};
#if RAINY_HAS_CXX20
            utility::format(buffer, "contract : {} triggered {}\nMessage: {}\n", type, location.to_string(), message);
#else
            const std::string temp = location.to_string();
            utility::cstyle_format(buffer, "contract : %s triggered %s\nMessage: %s\n", type.data(), temp.c_str(), message.data());
#endif
            if (!cond) {
                (void) std::fwrite(buffer.c_str(), 1, buffer.size(), stderr);
                information::system_call::debug_break();
            };
#else
            if (!cond) {
                std::terminate();
            }
#endif
        }
    }

#if RAINY_ENABLE_DEBUG
    RAINY_INLINE RAINY_NODEBUG_CONSTEXPR void expects(
        const bool cond, const utility::source_location &location = utility::source_location::current()) noexcept {
        contract(cond, "Pre-condition", location);
    }

    RAINY_INLINE RAINY_NODEBUG_CONSTEXPR void expects(
        const bool cond, const std::string_view message,
        const utility::source_location &location = utility::source_location::current()) noexcept {
        contract(cond, "Pre-condition", message, location);
    }

    RAINY_INLINE RAINY_NODEBUG_CONSTEXPR void ensures(
        const bool cond, const utility::source_location &location = utility::source_location::current()) noexcept {
        contract(cond, "Post-condition", location);
    }

    RAINY_INLINE RAINY_NODEBUG_CONSTEXPR void ensures(
        const bool cond, const std::string_view message,
        const utility::source_location &location = utility::source_location::current()) noexcept {
        contract(cond, "Post-condition", message, location);
    }
#else
    RAINY_INLINE RAINY_CONSTEXPR20 void expects(
        const bool cond, const utility::source_location &location = utility::source_location::current()) noexcept {
        contract(cond, "Pre-condition", location);
    }

    RAINY_INLINE RAINY_CONSTEXPR20 void expects(
        const bool cond, const std::string_view message,
        const utility::source_location &location = utility::source_location::current()) noexcept {
        contract(cond, "Pre-condition", message, location);
    }

    RAINY_INLINE RAINY_CONSTEXPR20 void ensures(
        const bool cond, const utility::source_location &location = utility::source_location::current()) noexcept {
        contract(cond, "Post-condition", location);
    }

    RAINY_INLINE RAINY_CONSTEXPR20 void ensures(
        const bool cond, const std::string_view message,
        const utility::source_location &location = utility::source_location::current()) noexcept {
        contract(cond, "Post-condition", message, location);
    }
#endif
}

#endif

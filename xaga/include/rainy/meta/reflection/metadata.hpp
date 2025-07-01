#ifndef RAINY_META_REFLECTION_METADATA_HPP
#define RAINY_META_REFLECTION_METADATA_HPP
#include <rainy/core/core.hpp>
#include <rainy/utility/any.hpp>
#include <rainy/utility/arg_extractor.hpp>

namespace rainy::meta::reflection {
    class metadata {
    public:
        metadata() = default;

        metadata(metadata &&right) noexcept : key_{utility::move(right.key_)}, value_{utility::move(right.value_)} {
        }
        metadata(const metadata & right) : key_{right.key_}, value_{right.value_} {
        }

        template <typename Ty>
        metadata(std::string_view key, Ty &&value) : key_(key), value_(utility::forward<Ty>(value)) {
        }

        RAINY_NODISCARD const std::string_view& key() const noexcept {
            return key_;
        }

        RAINY_NODISCARD const utility::any &value() const noexcept {
            return value_;
        }

    private:
        std::string_view key_;
        utility::any value_;
    };
}

namespace rainy::meta::reflection::implements {
    template <typename... Args>
    RAINY_INLINE_CONSTEXPR std::size_t metadata_count =
        type_traits::other_trans::count_type_v<metadata, type_traits::other_trans::type_list<Args...>>;
}

#endif
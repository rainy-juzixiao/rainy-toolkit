#ifndef RAINY_SPAN_HPP
#define RAINY_SPAN_HPP

namespace rainy::foundation::container {
    template <class _Ty, size_t _Extent>
    struct _Span_extent_type {
        using pointer = _Ty *;

        constexpr _Span_extent_type() noexcept = default;

        constexpr explicit _Span_extent_type(const pointer _Data, size_t) noexcept : _Mydata{_Data} {
        }

        pointer _Mydata{nullptr};
        static constexpr size_t _Mysize = _Extent;
    };

    template <class _Ty>
    struct _Span_extent_type<_Ty, dynamic_extent> {
        using pointer = _Ty *;

        constexpr _Span_extent_type() noexcept = default;

        constexpr explicit _Span_extent_type(const pointer _Data, const size_t _Size) noexcept : _Mydata{_Data}, _Mysize{_Size} {
        }

        pointer _Mydata{nullptr};
        size_t _Mysize{0};
    };
}

#endif

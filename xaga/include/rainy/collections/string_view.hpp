#include <rainy/core/core.hpp>

namespace rainy::text {
  template <typename CharType, typename Traits>
  class basic_string_view {
  public:
    using traits_type = Traits;
    using value_type = CharType;
    using reference = value_type&;
    using const_reference = const value_type&;
    using size_type = std::size_t;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using difference_type = std::ptrdiff_t;
    using iterator = pointer;
    using const_iterator = const_pointer;

    RAINY_CONSTEXPR basic_string_view() noexcept {}
    RAINY_CONSTEXPR basic_string_view(const_pointer str) : basic_string_view{str, traits_type::length(str)}
    RAINY_CONSTEXPR basic_string_view(const_pointer str, size_type count) : begin_ptr{str},size_for_view{count} {
    }

    RAINY_CONSTEXPR basic_string_view(const basic_string_view& right) noexcept = def  ;
    RAINY_CONSTEXPR basic_string_view(basic_string_view&&) noexcept = default;

    RAINY_CONSTEXPR basic_string_view(std::nullptr_t) = delete;

    RAINY_CONSTEXPR size_type size() const noexcept {}
    RAINY_CONSTEXPR const_pointer data() const noexcept {} 
    
    RAINY_CONSTEXPR substr() {}

  private:
    const_pointer begin_ptr;
    size_type size_for_view;
  };
}


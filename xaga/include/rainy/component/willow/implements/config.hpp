#ifndef RAINY_COMPONENT_WILLOW_CONFIG_HPP
#define RAINY_COMPONENT_WILLOW_CONFIG_HPP
#include <map>
#include <memory_resource>
#include <rainy/collections/dense_map.hpp>
#include <rainy/collections/string.hpp>
#include <rainy/core/core.hpp>

namespace rainy::component::willow::implements {
    inline constexpr std::uint32_t unicode_surrogate_base = 0x10000;
    inline constexpr std::uint32_t unicode_surrogate_lead_begin = 0xD800;
    inline constexpr std::uint32_t unicode_surrogate_lead_end = 0xDBFF;
    inline constexpr std::uint32_t unicode_surrogate_trail_begin = 0xDC00;
    inline constexpr std::uint32_t unicode_surrogate_trail_end = 0xDFFF;
    inline constexpr std::uint32_t unicode_surrogate_bits = 10;
    inline constexpr std::uint32_t unicode_surrogate_max_sur = 0x3FF;

    enum class token_type {
        uninitialized,
        literal_true,
        literal_false,
        literal_null,
        value_string,
        value_integer,
        value_float,
        begin_array,
        end_array,
        begin_object,
        end_object,
        name_separator,
        value_separator,
        end_of_input
    };

    RAINY_TOOLKIT_API std::uint32_t merge_surrogates(const std::uint32_t lead_surrogate, const std::uint32_t trail_surrogate) noexcept;
}

namespace rainy::component::willow {
    /**
     * @brief 设置当前willow中json使用的内存资源实例
     *
     * @param memres 必须是有效的内存资源实例，否则无法替换
     * @attention 当memres为空时，则不会触发设置
     * @return 返回原本的内存资源指针，在memres为空时也会返回原本的内存资源指针
     */
    RAINY_TOOLKIT_API std::pmr::memory_resource *set_memory_resource(std::pmr::memory_resource *memres) noexcept;
    /**
     * @brief 获取当前willow使用的内存资源指针
     * @return 返回一个始终有效的内存资源指针
     */
    RAINY_TOOLKIT_API std::pmr::memory_resource *get_memory_resource() noexcept;

    enum class json_type {
        number_integer,
        number_float,
        string,
        array,
        object,
        boolean,
        null,
    };

    /**
     * @brief 基本的json定义模板，若需自定义willow使用的容器，请使用别名加模板即可实现
     *
     * @tparam ObjectType 一个键值对映射容器的模板，可以是std::map、std::unordered_map又或者是任意以 container<Key, Mapped>
     * 形式出现的任意键值对存储容器。并且，应当能分配而非固定大小（具体参见详细文档）
     * @tparam ArrayType 一个数组容器的模板，用于存储json数组。应至少满足其具备一个参数以用于
     * @tparam StringType ...
     * 一个字符串容器，需是已实例化的，例如std::string这类，且第一个模板参数应为字符型，以确保basic_json可以提取这个类型来确定具体存储形式
     * @tparam IntegerType 一个整形值类型，可以是std::int32_t、std::int64_t、int这种。但是，无论如何，不应当设置其为无符号类型
     * @tparam FloatingType 一个浮点值类型，可以是double或float
     * @tparam BooleanType 一个布尔值类型，可以是bool，也可以根据情况使用int，但一般不会设置该值，故一般忽略
     * @tparam Alloc 一个分配器模板，它将会作为所有容器使用的分配器来分配资源
     */
    template <template <typename Key, typename Ty, typename... Args> typename ObjectType = collections::dense_map,
              template <typename Key, typename... Args> typename ArrayType = std::vector, typename StringType = text::string,
              typename IntegerType = std::int32_t, typename FloatingType = double, typename BooleanType = bool,
              template <typename Ty> typename Alloc = std::pmr::polymorphic_allocator>
    class basic_json;

    using json = basic_json<>;
    using json64 = basic_json<collections::dense_map, std::vector, text::string, std::int64_t>;
    using wjson = basic_json<collections::dense_map, std::vector, text::wstring>;
    using wjson64 = basic_json<collections::dense_map, std::vector, text::wstring, std::int64_t>;
    using u16json = basic_json<collections::dense_map, std::vector, text::u16string>;
    using u16json64 = basic_json<collections::dense_map, std::vector, text::u16string, std::int64_t>;

    template <typename BasicJson>
    struct serializer_args {
        using char_type = typename BasicJson::char_type;
        using float_type = typename BasicJson::float_type;

        int precision = utility::numeric_limits<float_type>::digits10 + 1;
        unsigned int indent = 0;
        char_type indent_char = ' ';
        bool escape_unicode = false;
    };
}

#endif

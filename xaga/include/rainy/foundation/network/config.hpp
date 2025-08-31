#ifndef RAINY_FOUNDATION_NETWORK_CONFIG_HPP
#define RAINY_FOUNDATION_NETWORK_CONFIG_HPP
#include <rainy/core/core.hpp>
#include <rainy/collections/dense_map.hpp>
#include <rainy/text/hashed_string.hpp>
#include <filesystem>
#include <string_view>
#include <unordered_map>

namespace rainy::foundation::network {
    enum class http_method {
        nil = 0,
        get,
        head,
        post,
        put,
        trace,
        patch,
        connect,
        options,
        del,
    };

    enum class content_encoding {
        gzip,
        deflate,
        br,
        none
    };

    constexpr inline auto get = http_method::get;
    constexpr inline auto post = http_method::post;
    constexpr inline auto del = http_method::del;
    constexpr inline auto head = http_method::head;
    constexpr inline auto put = http_method::put;
    constexpr inline auto connect = http_method::connect;
#ifdef TRACE
    constexpr inline auto trace = http_method::trace;
#endif
    constexpr inline auto options = http_method::options;

    RAINY_INLINE constexpr std::string_view method_name(http_method mthd) noexcept {
        switch (mthd) {
            case http_method::del:
                return "DELETE";
            case http_method::get:
                return "GET";
            case http_method::head:
                return "HEAD";
            case http_method::post:
                return "POST";
            case http_method::put:
                return "PUT";
            case http_method::patch:
                return "PATCH";
            case http_method::connect:
                return "CONNECT";
            case http_method::options:
                return "OPTIONS";
            case http_method::trace:
                return "TRACE";
            default:
                return "NIL";
        }
    }

    constexpr http_method method_type(std::string_view mthd) noexcept {
        constexpr collections::array<int, 20> method_table = {3, 1, 9, 0, 0, 0, 4, 5, 0, 0, 8, 0, 0, 0, 2, 0, 0, 0, 6, 7};
        int index = ((mthd[0] & ~0x20) ^ ((mthd[1] + 1) & ~0x20)) % 20;
        return static_cast<http_method>(method_table[index]);
    }

    enum class transfer_type {
        CHUNKED,
        ACCEPT_RANGES
    };

    enum class content_type {
        string,
        multipart,
        urlencoded,
        chunked,
        octet_stream,
        websocket,
        unknown,
    };

    enum class req_content_type {
        html,
        json,
        text,
        string,
        multipart,
        ranges,
        form_url_encode,
        octet_stream,
        xml,
        none
    };

    constexpr inline auto html = req_content_type::html;
    constexpr inline auto json = req_content_type::json;
    constexpr inline auto text = req_content_type::string;
    constexpr inline auto ranges = req_content_type::ranges;
    constexpr inline auto none = req_content_type::none;

    struct chunked_result {
        std::error_code ec;
        bool eof = false;
        std::string_view data;
    };

    struct part_head_t {
        std::error_code ec;
        std::string name;
        std::string filename;
    };

    enum class resp_content_type {
        css,
        csv,
        htm,
        html,
        js,
        mjs,
        txt,
        vtt,
        apng,
        avif,
        bmp,
        gif,
        png,
        svg,
        webp,
        ico,
        tif,
        tiff,
        jpg,
        jpeg,
        mp4,
        mpeg,
        webm,
        mp3,
        mpga,
        weba,
        wav,
        otf,
        ttf,
        woff,
        woff2,
        x7z,
        atom,
        pdf,
        json,
        rss,
        tar,
        xht,
        xhtml,
        xslt,
        xml,
        gz,
        zip,
        wasm,
        unknown
    };
}

namespace rainy::foundation::network::implements {
    constexpr collections::array<std::string_view, 45> content_type_arr{"Content-Type: text/css\r\n",
                                                                        "Content-Type: text/csv\r\n",
                                                                        "Content-Type: text/html\r\n",
                                                                        "Content-Type: text/html\r\n",
                                                                        "Content-Type: text/javascript\r\n",
                                                                        "Content-Type: text/javascript\r\n",
                                                                        "Content-Type: text/plain\r\n",
                                                                        "Content-Type: text/vtt\r\n",
                                                                        "Content-Type: image/apng\r\n",
                                                                        "Content-Type: image/avif\r\n",
                                                                        "Content-Type: image/bmp\r\n",
                                                                        "Content-Type: image/gif\r\n",
                                                                        "Content-Type: image/png\r\n",
                                                                        "Content-Type: image/svg+xml\r\n",
                                                                        "Content-Type: image/webp\r\n",
                                                                        "Content-Type: image/x-icon\r\n",
                                                                        "Content-Type: image/tiff\r\n",
                                                                        "Content-Type: image/tiff\r\n",
                                                                        "Content-Type: image/jpeg\r\n",
                                                                        "Content-Type: image/jpeg\r\n",
                                                                        "Content-Type: video/mp4\r\n",
                                                                        "Content-Type: video/mpeg\r\n",
                                                                        "Content-Type: video/webm\r\n",
                                                                        "Content-Type: audio/mp3\r\n",
                                                                        "Content-Type: audio/mpeg\r\n",
                                                                        "Content-Type: audio/webm\r\n",
                                                                        "Content-Type: audio/wave\r\n",
                                                                        "Content-Type: font/otf\r\n",
                                                                        "Content-Type: font/ttf\r\n",
                                                                        "Content-Type: font/woff\r\n",
                                                                        "Content-Type: font/woff2\r\n",
                                                                        "Content-Type: application/x-7z-compressed\r\n",
                                                                        "Content-Type: application/atom+xml\r\n",
                                                                        "Content-Type: application/pdf\r\n",
                                                                        "Content-Type: application/json\r\n",
                                                                        "Content-Type: application/rss+xml\r\n",
                                                                        "Content-Type: application/x-tar\r\n",
                                                                        "Content-Type: application/xhtml+xml\r\n",
                                                                        "Content-Type: application/xhtml+xml\r\n",
                                                                        "Content-Type: application/xslt+xml\r\n",
                                                                        "Content-Type: application/xml\r\n",
                                                                        "Content-Type: application/gzip\r\n",
                                                                        "Content-Type: application/zip\r\n",
                                                                        "Content-Type: application/wasm\r\n",
                                                                        "Content-Type: unknown\r\n"};

    collections::dense_map<text::hashed_string, std::string_view> &content_type_map();
}

namespace rainy::foundation::network {
    template <resp_content_type N>
    constexpr std::string_view get_content_type() {
        constexpr std::size_t idx = static_cast<std::size_t>(N);
        if constexpr (idx > 43) {
            return implements::content_type_arr[44];
        } else {
            return implements::content_type_arr[idx];
        }
    }

    struct non_ssl {};
    struct ssl {};

    inline constexpr int64_t MAX_HTTP_BODY_SIZE = 4294967296; // 4GB

    enum class time_format {
        http_format,
        utc_format,
        utc_without_punctuation_format
    };
}

namespace rainy::foundation::network::time {
    enum component_of_time_format {
        day_name,
        day,
        month_name,
        month,
        year,
        hour,
        minute,
        second,
        second_decimal_part,
        SP,
        comma,
        colon,
        hyphen,
        dot,
        GMT,
        T,
        Z,
        ending
    };

    inline constexpr collections::array<int, 17> month_table = {11, 4, -1, 7, -1, -1, -1, 0, 6, 3, 5, 2, 10, 8, -1, 9, 1};

    inline constexpr collections::array<int, 17> week_table = {2, 4, 3, 1, -1, -1, -1, 6, -1, -1, -1, -1, 0, -1, -1, 5, -1};

    inline constexpr collections::array<component_of_time_format, 32> http_time_format{
        component_of_time_format::day_name, component_of_time_format::comma,  component_of_time_format::SP,
        component_of_time_format::day,      component_of_time_format::SP,     component_of_time_format::month_name,
        component_of_time_format::SP,       component_of_time_format::year,   component_of_time_format::SP,
        component_of_time_format::hour,     component_of_time_format::colon,  component_of_time_format::minute,
        component_of_time_format::colon,    component_of_time_format::second, component_of_time_format::SP,
        component_of_time_format::GMT,      component_of_time_format::ending};

    inline constexpr collections::array<component_of_time_format, 32> utc_time_format{component_of_time_format::year,
                                                                                      component_of_time_format::hyphen,
                                                                                      component_of_time_format::month,
                                                                                      component_of_time_format::hyphen,
                                                                                      component_of_time_format::day,
                                                                                      component_of_time_format::T,
                                                                                      component_of_time_format::hour,
                                                                                      component_of_time_format::colon,
                                                                                      component_of_time_format::minute,
                                                                                      component_of_time_format::colon,
                                                                                      component_of_time_format::second,
                                                                                      component_of_time_format::dot,
                                                                                      component_of_time_format::second_decimal_part,
                                                                                      component_of_time_format::Z,
                                                                                      component_of_time_format::ending};

    inline constexpr collections::array<component_of_time_format, 32> utc_time_without_punctuation_format{
        component_of_time_format::year,   component_of_time_format::month,
        component_of_time_format::day,    component_of_time_format::T,
        component_of_time_format::hour,   component_of_time_format::minute,
        component_of_time_format::second, component_of_time_format::second_decimal_part,
        component_of_time_format::Z,      component_of_time_format::ending};

    constexpr inline int len_of_http_time_format = 3 + 1 + 1 + 2 + 1 + 3 + 1 + 4 + 1 + 2 + 1 + 2 + 1 + 2 + 1 + 3;
    constexpr inline int len_of_utc_time_format = 4 + 1 + 2 + 1 + 2 + 1 + 2 + 1 + 2 + 1 + 2 + 1 + 0 + 1;
    constexpr inline int len_of_utc_time_without_punctuation_format = 4 + 2 + 2 + 1 + 2 + 2 + 2 + 0 + 1;
    constexpr inline std::int64_t absolute_zero_year = -292277022399;
    constexpr inline std::int64_t days_per_100_years = 365 * 100 + 24;
    constexpr inline std::int64_t days_per_400_years = 365 * 400 + 97;
    constexpr inline std::int64_t days_per_4_years = 365 * 4 + 1;
    constexpr inline std::int64_t seconds_per_minute = 60;
    constexpr inline std::int64_t seconds_per_hour = 60 * seconds_per_minute;
    constexpr inline std::int64_t seconds_per_day = 24 * seconds_per_hour;
    constexpr inline std::int64_t seconds_per_week = 7 * seconds_per_day;
    constexpr inline std::int64_t internal_year = 1;
    constexpr inline std::int64_t absolute_to_internal =
        (absolute_zero_year - internal_year) * std::int64_t(365.2425 * seconds_per_day);
    constexpr inline std::int64_t unix_to_internal = (1969 * 365 + 1969 / 4 - 1969 / 100 + 1969 / 400) * seconds_per_day;
    constexpr inline std::int64_t internal_to_unix = -unix_to_internal;

    constexpr inline collections::array<std::int32_t, 13> days_before = {
        0,
        31,
        31 + 28,
        31 + 28 + 31,
        31 + 28 + 31 + 30,
        31 + 28 + 31 + 30 + 31,
        31 + 28 + 31 + 30 + 31 + 30,
        31 + 28 + 31 + 30 + 31 + 30 + 31,
        31 + 28 + 31 + 30 + 31 + 30 + 31 + 31,
        31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30,
        31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,
        31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30,
        31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30 + 31,
    };
}

#endif
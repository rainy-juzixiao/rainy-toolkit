#ifndef RAINY_WINAPI_ENVIRONMENT_H
#define RAINY_WINAPI_ENVIRONMENT_H
#include <rainy/core.hpp>
#if RAINY_USING_WINDOWS
#include <string>
#include <memory>
#include <chrono>
#include <future>
#include <stdexcept>
#include <windows.h>
#include <string_view>
#include <rainy/base.hpp>
#include <rainy/meta_programming/type_traits.hpp>

namespace rainy {
    namespace winapi::environment {
        enum class environment_status {
            failed,
            success,
            invalid_args,
            not_found
        };
    }

	namespace utility {
        /* 为winapi::environment::environment_status提供特化实现 */

		template <typename CharType>
        struct result_collection<winapi::environment::environment_status, std::vector<std::basic_string<CharType>>, true> {
            winapi::environment::environment_status status;
            std::vector<std::basic_string<CharType>> vector_list;
        };

		template <typename CharType>
        struct result_collection<winapi::environment::environment_status, std::basic_string<CharType>, true> {
            winapi::environment::environment_status status;
            std::basic_string<CharType> value;
        };
	}
}


namespace rainy::winapi::environment {
    class environment_manager {
    public:
        template <typename CharType>
        using result_list = utility::result_collection<environment_status, std::vector<std::basic_string<CharType>>, true>;

        template <typename CharType>
        using result = utility::result_collection<environment_status, std::basic_string<CharType>, true>;

        using setter_errorcallback_t = foundation::functional::function_pointer<void(bool, DWORD)>;

        environment_manager() = default;

        template <typename CharType>
        result_list<CharType> get_environment_var_by_list(const CharType *name) const {
            if (!name) {
                return {environment_status::invalid_args, {}};
            }
            return get_environment_var_by_list(std::basic_string_view<CharType>{name});
        }

        template <typename CharType>
        result_list<CharType> get_environment_var_by_list(std::basic_string_view<CharType> name) const {
            using namespace foundation::type_traits;
            static_assert(type_relations::is_any_of_v<CharType, char, wchar_t>, "Only support wchat_t and char types");

            std::lock_guard<std::shared_mutex> lock(get_shared_mutex()); 

            using string_type = std::basic_string<CharType>;
            using collection_type = std::vector<string_type>;

            static constexpr CharType split_chars = split_char<CharType>;

            if (name.empty()) {
                return {environment_status::invalid_args, {}};
            }

            string_type path_variable{};
            DWORD require_size = get_environment_impl(name, path_variable, 0);

            if (require_size == 0 && GetLastError() == ERROR_ENVVAR_NOT_FOUND) {
                return {environment_status::not_found, {}};
            }
            path_variable.resize(require_size);
            if (!get_environment_impl(name, path_variable, require_size)) {
                return {environment_status::failed, {}};
            }
            std::size_t start{0};
            if (std::size_t find = path_variable.find(split_chars); find != string_type::npos /* 检查到诸如PATH这样的环境变量 */) {
                collection_type returns;
                size_t start, end;
                for (start = 0, end = path_variable.find(split_chars); end != std::string::npos;
                     start = end + 1, end = path_variable.find(split_chars, start)) {
                    returns.emplace_back(path_variable.substr(start, end - start));
                }
                // 添加最后一个分割的子字符串
                if (start < path_variable.size()) {
                    returns.emplace_back(path_variable.substr(start));
                }
                return {environment_status::success, returns};
            }
            return {environment_status::success, collection_type{path_variable}};
        }

        template <typename CharType>
        result<CharType> get_environment_var(std::basic_string_view<CharType> name) const {
            using namespace foundation::type_traits;
            static_assert(type_relations::is_any_of_v<CharType, char, wchar_t>, "Only support wchat_t and char types");

            using string_type = std::basic_string<CharType>;

            static constexpr CharType split_chars = split_char<CharType>;

            if (name.empty()) {
                return {environment_status::invalid_args, {}};
            }

            string_type path_variable{};
            DWORD require_size = get_environment_impl(name, path_variable, 0);
            if (require_size == 0 && GetLastError() == ERROR_ENVVAR_NOT_FOUND) {
                return {environment_status::not_found, {}};
            }
            path_variable.resize(require_size);
            if (!get_environment_impl(name, path_variable, require_size)) {
                return {environment_status::failed, {}};
            }
            if (std::size_t find = path_variable.find(split_chars); find != string_type::npos) {
                path_variable.resize(find);
            }
            return {environment_status::success, path_variable};
        }

        template <typename CharType>
        result<CharType> get_environment_var(const CharType *name) const {
            if (!name) {
                return {environment_status::invalid_args, {}};
            }
            return get_environment_var(std::basic_string_view<CharType>{name});
        }

        template <typename CharType>
        void set_environment_var(std::basic_string_view<CharType> name, std::basic_string_view<CharType> value,
                                 setter_errorcallback_t error_callback = nullptr) {
            auto error_callback_invoker = [&error_callback](bool api_error, DWORD errro_code) -> void {
                if (error_callback) {
                    error_callback(api_error, errro_code);
                }
            };

            using string_type = std::basic_string<CharType>;
            using collection_type = std::vector<string_type>;
            std::lock_guard<std::mutex> lock(get_mutex());
            string_type path_variable{};
            DWORD require_size = get_environment_impl(name, path_variable, 0);
            if (require_size == 0 && GetLastError() == ERROR_ENVVAR_NOT_FOUND) {
                error_callback_invoker(true, ERROR_ENVVAR_NOT_FOUND);
                return;
            }
            path_variable.resize(require_size);
            if (!get_environment_impl(name, path_variable, require_size)) {
                error_callback_invoker(false, 1);
                return;
            }
            static constexpr CharType split_chars = split_char<CharType>;
            if (path_variable.find(split_chars) != string_type::npos) {
                error_callback_invoker(false, 2);
                return;
            }
            if (!set_environment_impl(name, value)) {
                error_callback_invoker(true, GetLastError());
            }
        }

        template <typename CharType>
        void set_environment_var(const CharType *name, const CharType *value, setter_errorcallback_t error_callback = nullptr) {
            if (!name) {
                return;
            }
            set_environment_var(std::basic_string_view<CharType>{name}, std::basic_string_view<CharType>{value}, error_callback);
        }

    private:
        template <typename CharType>
        static inline constexpr bool is_wchar = foundation::type_traits::helper::is_wchar_t<CharType>;

        template <typename CharType>
        static inline constexpr CharType split_char = foundation::type_traits::other_transformations::conditional_t<
            is_wchar<CharType>, foundation::type_traits::helper::integral_constant<char, ';'>,
            foundation::type_traits::helper::integral_constant<wchar_t, L';'>>::value; // 根据字符类型，选择相应的分隔符

        template <typename CharType>
        static DWORD get_environment_impl(const std::basic_string_view<CharType> &name, std::basic_string<CharType> &buffer, DWORD size) {
            using namespace foundation::type_traits;
            static_assert(type_relations::is_any_of_v<CharType, char, wchar_t>, "Only support wchat_t and char types");
            if constexpr (is_wchar<CharType>) {
                return GetEnvironmentVariableW(name.data(), &buffer[0], size);
            } else {
                return GetEnvironmentVariableA(name.data(), &buffer[0], size);
            }
        }

        template <typename CharType>
        static BOOL set_environment_impl(const std::basic_string_view<CharType> &name, const std::basic_string_view<CharType> &value) {
            using namespace foundation::type_traits;
            static_assert(type_relations::is_any_of_v<CharType, char, wchar_t>, "Only support wchat_t and char types");
            if constexpr (is_wchar<CharType>) {
                return SetEnvironmentVariableW(name, value);
            } else {
                return SetEnvironmentVariableA(name, value);
            }
        }

        static std::mutex& get_mutex() noexcept {
            static std::mutex instance;
            return instance;
        }

        static std::shared_mutex& get_shared_mutex() noexcept {
            static std::shared_mutex instance;
            return instance;
        }
    };
}
#endif

#endif
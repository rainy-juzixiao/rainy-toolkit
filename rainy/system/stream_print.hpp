#ifndef RAINY_STREAM_PRINT_HPP
#define RAINY_STREAM_PRINT_HPP
#include <cstdio>
#include <string>
#include <mutex>
#include <sstream>
#include <shared_mutex>
#include <rainy/core.hpp>

namespace rainy::foundation::system::output {
    namespace utils {
        RAINY_INLINE std::shared_mutex &get_mtx() {
            static std::shared_mutex lock;
            return lock;
        }
    }

    template <typename... Args>
    std::string make_sstream_string(Args... args) {
        std::stringstream stream;
        ((stream << args), ...);
        return stream.str();
    }

    template <typename... Args>
    void stdout_print(Args... args) {
        std::string output_string = make_sstream_string(args...);
        (void) std::fwrite(output_string.c_str(), sizeof(char), output_string.size(), stdout);
    }

    template <typename... Args>
    void stderr_print(Args... args) {
        std::string output_string = make_sstream_string(args...);
        (void) std::fwrite(output_string.c_str(), sizeof(char), output_string.size(), stderr);
    }

    template <typename... Args>
    void stdout_println(Args... args) {
        std::string output_string = make_sstream_string(args...) + "\n";
        (void) std::fwrite(output_string.c_str(), sizeof(char), output_string.size(), stdout);
    }

    template <typename... Args>
    void stderr_println(Args... args) {
        std::string output_string = make_sstream_string(args...) + "\n";
        (void) std::fwrite(output_string.c_str(), sizeof(char), output_string.size(), stderr);
    }

    template <typename... Args>
    void ts_stdout_print(Args... args) {
        std::lock_guard<std::shared_mutex> lock(utils::get_mtx());
        stdout_print(args...);
    }

    template <typename... Args>
    void ts_stderr_print(Args... args) {
        std::lock_guard<std::shared_mutex> lock(utils::get_mtx());
        stderr_print(args...);
    }

    template <typename... Args>
    void ts_stdout_println(Args... args) {
        std::lock_guard<std::shared_mutex> lock(utils::get_mtx());
        stdout_println(args...);
    }

    template <typename... Args>
    void ts_stderr_println(Args... args) {
        std::lock_guard<std::shared_mutex> lock(utils::get_mtx());
        stderr_println(args...);
    }
}

#endif
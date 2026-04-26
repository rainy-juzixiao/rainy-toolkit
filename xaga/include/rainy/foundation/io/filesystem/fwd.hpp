#ifndef RAINY_FOUNDATION_IO_FILESYSTEM_FWD_HPP
#define RAINY_FOUNDATION_IO_FILESYSTEM_FWD_HPP
#include <chrono>
#include <rainy/core/core.hpp>

namespace rainy::foundation::io::filesystem {
    class path;

    class filesystem_error;

    class directory_entry;

    class directory_iterator;

    directory_iterator begin(directory_iterator iter) noexcept;
    directory_iterator end(directory_iterator) noexcept;

    class recursive_directory_iterator;

    recursive_directory_iterator begin(recursive_directory_iterator iter) noexcept;
    recursive_directory_iterator end(recursive_directory_iterator) noexcept;

    using core::pal::file_type;
    using core::pal::perms;
    using core::pal::perm_options;
    using core::pal::copy_options;
    using core::pal::directory_options;
    using core::pal::space_info;

    class file_status {
    public:
        file_status() noexcept : file_status(file_type::none) {
        }
        explicit file_status(file_type ft, perms prms = perms::unknown) noexcept;
        file_status(const file_status &) noexcept = default;
        file_status(file_status &&) noexcept = default;
        ~file_status();

        file_status &operator=(const file_status &) noexcept = default;
        file_status &operator=(file_status &&) noexcept = default;

        void type(file_type ft) noexcept;
        void permissions(perms prms) noexcept;

        file_type type() const noexcept;
        perms permissions() const noexcept;

        friend bool operator==(const file_status &left, const file_status &right) noexcept {
            return left.type() == right.type() && left.permissions() == right.permissions();
        }
    };

    using file_time_type = std::chrono::time_point<std::chrono::file_clock>;
}

namespace rainy::foundation::io::filesystem {
    enum class open_mode : unsigned int {
        none = 0,
        read_only = 1 << 0, // O_RDONLY  / GENERIC_READ
        write_only = 1 << 1, // O_WRONLY  / GENERIC_WRITE
        read_write = read_only | write_only,
        append = 1 << 2, // O_APPEND
        create = 1 << 3, // O_CREAT
        truncate = 1 << 4, // O_TRUNC
        exclusive = 1 << 5, // O_EXCL
        sync = 1 << 6, // O_SYNC / FILE_FLAG_WRITE_THROUGH
        direct = 1 << 7, // O_DIRECT / FILE_FLAG_NO_BUFFERING
    };

    inline constexpr open_mode operator|(open_mode a, open_mode b) noexcept {
        return static_cast<open_mode>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
    }

    inline constexpr open_mode operator&(open_mode a, open_mode b) noexcept {
        return static_cast<open_mode>(static_cast<unsigned>(a) & static_cast<unsigned>(b));
    }

    inline constexpr bool has_flag(open_mode flags, open_mode bit) noexcept {
        return (flags & bit) != open_mode::none;
    }

    enum class seek_basis : int {
        begin = 0, // SEEK_SET
        current = 1, // SEEK_CUR
        end = 2 // SEEK_END
    };
}

#endif

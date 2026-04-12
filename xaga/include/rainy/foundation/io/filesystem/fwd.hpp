#ifndef RAINY_FOUNDATION_IO_FILESYSTEM_FWD_HPP
#define RAINY_FOUNDATION_IO_FILESYSTEM_FWD_HPP
#include <rainy/core/core.hpp>

namespace rainy::foundation::io::filesystem {
    class path {
    public:
#if RAINY_USING_WINDOWS
        using value_type = wchar_t;
#else
        using value_type = char;
#endif

        using string_type = text::basic_string<value_type>;

#if RAINY_USING_WINDOWS
        static constexpr value_type preferred_separator = L'\\';
#else
        static constexpr value_type preferred_separator = '/';
#endif

        enum format {
            native_format,
            generic_format,
            auto_format
        };

        path() noexcept;
        path(const path &path);
        path(path &&path) noexcept;
        path(string_type &&source, format fmt = auto_format);
        template <class Source>
        path(const Source &source, format fmt = auto_format);
        template <class InputIt>
        path(InputIt first, InputIt last, format fmt = auto_format);
        template <class Source>
        path(const Source &source, const std::locale &loc, format fmt = auto_format);
        template <class InputIt>
        path(InputIt first, InputIt last, const std::locale &loc, format fmt = auto_format);
        ~path();

        path &operator=(const path &path);
        path &operator=(path &&path) noexcept;
        path &operator=(string_type &&source);
        path &assign(string_type &&source);
        template <class Source>
        path &operator=(const Source &source);
        template <class Source>
        path &assign(const Source &source);
        template <class InputIt>
        path &assign(InputIt first, InputIt last);

        path &operator/=(const path &path);
        template <class Source>
        path &operator/=(const Source &source);
        template <class Source>
        path &append(const Source &source);
        template <class InputIt>
        path &append(InputIt first, InputIt last);

        // concatenation
        path &operator+=(const path &x);
        path &operator+=(const string_type &x);
        path &operator+=(text::basic_string_view<value_type> x);
        path &operator+=(const value_type *x);
        path &operator+=(value_type x);
        template <class Source>
        path &operator+=(const Source &x);
        template <class ECharT>
        path &operator+=(ECharT x);
        template <class Source>
        path &concat(const Source &x);
        template <class InputIt>
        path &concat(InputIt first, InputIt last);

        // modifiers
        void clear() noexcept;
        path &make_preferred();
        path &remove_filename();
        path &replace_filename(const path &replacement);
        path &replace_extension(const path &replacement = path());
        void swap(path &right) noexcept;

        friend bool operator==(const path &left, const path &right) noexcept;

#if RAINY_HAS_CXX20
        friend std::strong_ordering operator<=>(const path &left, const path &right) noexcept;
#endif
        friend path operator/(const path &left, const path &right);

        // native format observers
        const string_type &native() const noexcept;
        const value_type *c_str() const noexcept;
        operator string_type() const;

        template <class ECharT, class Traits = text::char_traits<ECharT>, class Allocator = std::allocator<ECharT>>
        text::basic_string<ECharT, Traits, Allocator> string(const Allocator &a = Allocator()) const;

        std::string string() const;
        std::wstring wstring() const;
        std::u8string u8string() const;
        std::u16string u16string() const;
        std::u32string u32string() const;

        template <class ECharT, class Traits = text::char_traits<ECharT>, class Allocator = std::allocator<ECharT>>
        text::basic_string<ECharT, Traits, Allocator> generic_string(const Allocator &a = Allocator()) const;

        std::string generic_string() const;
        std::wstring generic_wstring() const;
        std::u8string generic_u8string() const;
        std::u16string generic_u16string() const;
        std::u32string generic_u32string() const;

        int compare(const path &path) const noexcept;
        int compare(const string_type &string) const;
        int compare(text::basic_string_view<value_type> string) const;
        int compare(const value_type *string) const;

        path root_name() const;
        path root_directory() const;
        path root_path() const;
        path relative_path() const;
        path parent_path() const;
        path filename() const;
        path stem() const;
        path extension() const;

        bool empty() const noexcept;
        bool has_root_name() const;
        bool has_root_directory() const;
        bool has_root_path() const;
        bool has_relative_path() const;
        bool has_parent_path() const;
        bool has_filename() const;
        bool has_stem() const;
        bool has_extension() const;
        bool is_absolute() const;
        bool is_relative() const;

        path lexically_normal() const;
        path lexically_relative(const path &base) const;
        path lexically_proximate(const path &base) const;

        class iterator;
        using const_iterator = iterator;

        iterator begin() const;
        iterator end() const;

        template <class CharT, class Traits>
        friend std::basic_ostream<CharT, Traits> &operator<<(std::basic_ostream<CharT, Traits> &ostream, const path &path);
        template <class CharT, class Traits>
        friend std::basic_istream<CharT, Traits> &operator>>(std::basic_istream<CharT, Traits> &istream, path &path);
    };

    class filesystem_error;

    class directory_entry;

    class directory_iterator;

    directory_iterator begin(directory_iterator iter) noexcept;
    directory_iterator end(directory_iterator) noexcept;

    class recursive_directory_iterator;

    recursive_directory_iterator begin(recursive_directory_iterator iter) noexcept;
    recursive_directory_iterator end(recursive_directory_iterator) noexcept;

    class file_status;

    struct space_info {
        std::uintmax_t capacity;
        std::uintmax_t free;
        std::uintmax_t available;

        friend bool operator==(const space_info &, const space_info &) = default;
    };

    enum class file_type {
        none = -1,
        not_found = -1,
        regular = 0,
        directory = 1,
        symlink = 2,
        block = 3,
        character = 4,
        fifo = 5,
        socket = 6,
        unknown = 7
    };

    enum class perms : unsigned {
        none = 0,

        // Owner
        owner_read = 0400,
        owner_write = 0200,
        owner_exec = 0100,
        owner_all = 0700,

        // Group
        group_read = 040,
        group_write = 020,
        group_exec = 010,
        group_all = 070,

        // Others
        others_read = 04,
        others_write = 02,
        others_exec = 01,
        others_all = 07,

        all = 0777,

        // Special bits
        set_uid = 04000,
        set_gid = 02000,
        sticky_bit = 01000,

        mask = 07777,
        unknown = 0xFFFF
    };

    enum class perm_options : unsigned {
        replace = 1, // 替换现有权限
        add = 2, // 追加权限位
        remove = 4, // 移除权限位
        nofollow = 8 // 不跟随符号链接
    };

    enum class copy_options : unsigned {
        none = 0,
        skip_existing = 1, // 跳过已存在目标
        overwrite_existing = 2, // 覆盖已存在目标
        update_existing = 4, // 仅当源更新时覆盖
        recursive = 8, // 递归复制目录
        copy_symlinks = 16, // 复制符号链接本身
        skip_symlinks = 32, // 忽略符号链接
        directories_only = 64, // 仅复制目录结构
        create_symlinks = 128, // 创建符号链接而非复制
        create_hard_links = 256 // 创建硬链接而非复制
    };

    enum class directory_options : unsigned {
        none = 0,
        follow_directory_symlink = 1, // 遍历时跟随目录符号链接
        skip_permission_denied = 2 // 跳过无权限目录而非抛异常
    };

    using file_time_type = std::chrono::time_point<std::chrono::file_clock>;

    path absolute(const path &path);
    path absolute(const path &path, std::error_code &ec);

    path canonical(const path &path);
    path canonical(const path &path, std::error_code &ec);

    void copy(const path &from, const path &to);
    void copy(const path &from, const path &to, std::error_code &ec);
    void copy(const path &from, const path &to, copy_options options);
    void copy(const path &from, const path &to, copy_options options, std::error_code &ec);

    bool copy_file(const path &from, const path &to);
    bool copy_file(const path &from, const path &to, std::error_code &ec);
    bool copy_file(const path &from, const path &to, copy_options option);
    bool copy_file(const path &from, const path &to, copy_options option, std::error_code &ec);

    void copy_symlink(const path &existing_symlink, const path &new_symlink);
    void copy_symlink(const path &existing_symlink, const path &new_symlink, std::error_code &ec) noexcept;

    bool create_directories(const path &path);
    bool create_directories(const path &path, std::error_code &ec);

    bool create_directory(const path &path);
    bool create_directory(const path &path, std::error_code &ec) noexcept;

    bool create_directory(const path &path, const filesystem::path &attributes);
    bool create_directory(const path &path, const filesystem::path &attributes, std::error_code &ec) noexcept;

    void create_directory_symlink(const path &to, const path &new_symlink);
    void create_directory_symlink(const path &to, const path &new_symlink, std::error_code &ec) noexcept;

    void create_hard_link(const path &to, const path &new_hard_link);
    void create_hard_link(const path &to, const path &new_hard_link, std::error_code &ec) noexcept;

    void create_symlink(const path &to, const path &new_symlink);
    void create_symlink(const path &to, const path &new_symlink, std::error_code &ec) noexcept;

    path current_path();
    path current_path(std::error_code &ec);
    void current_path(const path &path);
    void current_path(const path &path, std::error_code &ec) noexcept;

    bool equivalent(const path &path1, const path &path2);
    bool equivalent(const path &path1, const path &path2, std::error_code &ec) noexcept;

    bool exists(file_status string) noexcept;
    bool exists(const path &path);
    bool exists(const path &path, std::error_code &ec) noexcept;

    std::uintmax_t file_size(const path &path);
    std::uintmax_t file_size(const path &path, std::error_code &ec) noexcept;

    std::uintmax_t hard_link_count(const path &path);
    std::uintmax_t hard_link_count(const path &path, std::error_code &ec) noexcept;

    bool is_block_file(file_status string) noexcept;
    bool is_block_file(const path &path);
    bool is_block_file(const path &path, std::error_code &ec) noexcept;

    bool is_character_file(file_status string) noexcept;
    bool is_character_file(const path &path);
    bool is_character_file(const path &path, std::error_code &ec) noexcept;

    bool is_directory(file_status string) noexcept;
    bool is_directory(const path &path);
    bool is_directory(const path &path, std::error_code &ec) noexcept;

    bool is_empty(const path &path);
    bool is_empty(const path &path, std::error_code &ec);

    bool is_fifo(file_status string) noexcept;
    bool is_fifo(const path &path);
    bool is_fifo(const path &path, std::error_code &ec) noexcept;

    bool is_other(file_status string) noexcept;
    bool is_other(const path &path);
    bool is_other(const path &path, std::error_code &ec) noexcept;

    bool is_regular_file(file_status string) noexcept;
    bool is_regular_file(const path &path);
    bool is_regular_file(const path &path, std::error_code &ec) noexcept;

    bool is_socket(file_status string) noexcept;
    bool is_socket(const path &path);
    bool is_socket(const path &path, std::error_code &ec) noexcept;

    bool is_symlink(file_status string) noexcept;
    bool is_symlink(const path &path);
    bool is_symlink(const path &path, std::error_code &ec) noexcept;

    file_time_type last_write_time(const path &path);
    file_time_type last_write_time(const path &path, std::error_code &ec) noexcept;
    void last_write_time(const path &path, file_time_type new_time);
    void last_write_time(const path &path, file_time_type new_time, std::error_code &ec) noexcept;

    void permissions(const path &path, perms prms, perm_options opts = perm_options::replace);
    void permissions(const path &path, perms prms, std::error_code &ec) noexcept;
    void permissions(const path &path, perms prms, perm_options opts, std::error_code &ec);

    path proximate(const path &path, std::error_code &ec);
    path proximate(const path &path, const filesystem::path &base = current_path());
    path proximate(const path &path, const filesystem::path &base, std::error_code &ec);

    path read_symlink(const path &path);
    path read_symlink(const path &path, std::error_code &ec);

    path relative(const path &path, std::error_code &ec);
    path relative(const path &path, const filesystem::path &base = current_path());
    path relative(const path &path, const filesystem::path &base, std::error_code &ec);

    bool remove(const path &path);
    bool remove(const path &path, std::error_code &ec) noexcept;

    std::uintmax_t remove_all(const path &path);
    std::uintmax_t remove_all(const path &path, std::error_code &ec);

    void rename(const path &from, const path &to);
    void rename(const path &from, const path &to, std::error_code &ec) noexcept;

    void resize_file(const path &path, std::uintmax_t size);
    void resize_file(const path &path, std::uintmax_t size, std::error_code &ec) noexcept;

    space_info space(const path &path);
    space_info space(const path &path, std::error_code &ec) noexcept;

    file_status status(const path &path);
    file_status status(const path &path, std::error_code &ec) noexcept;

    bool status_known(file_status string) noexcept;

    file_status symlink_status(const path &path);
    file_status symlink_status(const path &path, std::error_code &ec) noexcept;

    path temp_directory_path();
    path temp_directory_path(std::error_code &ec);

    path weakly_canonical(const path &path);
    path weakly_canonical(const path &path, std::error_code &ec);
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

    // 文件 seek 基准
    enum class seek_basis : int {
        begin = 0, // SEEK_SET
        current = 1, // SEEK_CUR
        end = 2 // SEEK_END
    };
}

#endif

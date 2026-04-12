#ifndef RAINY_FOUNDATION_IO_FILESYSTEM_PATH_HPP
#define RAINY_FOUNDATION_IO_FILESYSTEM_PATH_HPP
#include <rainy/core/core.hpp>

namespace rainy::foundation::io::filesystem {
    class path;
}

namespace rainy::foundation::io::filesystem::implements {
#if RAINY_USING_WINDOWS
    using native_char = wchar_t;
#else
    using native_char = char;
#endif
    using native_string_t = text::basic_string<native_char>;

    template <typename CharT>
    RAINY_INLINE bool is_separator(CharT c) noexcept {
        return c == static_cast<CharT>('/') || c == static_cast<CharT>('\\');
    }

    template <typename CharT>
    std::size_t root_name_length(const text::basic_string<CharT> &p) noexcept {
#if RAINY_USING_WINDOWS
        if (p.size() >= 2 && p[1] == static_cast<CharT>(':') &&
            ((p[0] >= static_cast<CharT>('A') && p[0] <= static_cast<CharT>('Z')) ||
             (p[0] >= static_cast<CharT>('a') && p[0] <= static_cast<CharT>('z')))) {
            return 2;
        }
        if (p.size() >= 2 && is_separator(p[0]) && is_separator(p[1])) {
            if (p.size() == 2) {
                return 2;
            }
            auto pos = p.find_first_of(static_cast<CharT>('/'));
            auto pos2 = p.find_first_of(static_cast<CharT>('\\'));
            // 取更小的非 npos 值
            std::size_t sep = text::basic_string<CharT>::npos;
            if (pos != text::basic_string<CharT>::npos) {
                sep = pos;
            }
            if (pos2 != text::basic_string<CharT>::npos && pos2 < sep) {
                sep = pos2;
            }
            return (sep == text::basic_string<CharT>::npos) ? p.size() : sep;
        }
#else
        (void) p;
#endif
        return 0;
    }

#if RAINY_USING_WINDOWS
    RAINY_INLINE native_string_t to_native(text::basic_string_view<wchar_t> sv) {
        return {sv.data(), sv.size()};
    }

    RAINY_INLINE native_string_t to_native(text::basic_string_view<char> sv) {
        using conv_t = foundation::text::wstring_convert<foundation::text::codecvt_utf8<wchar_t>, text::basic_string, wchar_t>;
        conv_t conv;
        return conv.from_bytes(sv.data(), sv.data() + sv.size());
    }

#if RAINY_HAS_CXX20
    RAINY_INLINE native_string_t to_native(text::basic_string_view<char8_t> sv) {
        return to_native(text::basic_string_view<char>(reinterpret_cast<const char *>(sv.data()), sv.size()));
    }
#endif

    RAINY_INLINE native_string_t to_native(text::basic_string_view<char16_t> sv) {
        using u16_conv = foundation::text::wstring_convert<foundation::text::codecvt_utf8<char16_t>, text::basic_string, char16_t>;
        using w_conv = foundation::text::wstring_convert<foundation::text::codecvt_utf8<wchar_t>, text::basic_string, wchar_t>;
        u16_conv c1;
        w_conv c2;
        auto utf8 = c1.to_bytes(sv.data(), sv.data() + sv.size());
        return c2.from_bytes(utf8.data(), utf8.data() + utf8.size());
    }

    RAINY_INLINE native_string_t to_native(text::basic_string_view<char32_t> sv) {
        using u32_conv = foundation::text::wstring_convert<foundation::text::codecvt_utf8<char32_t>, text::basic_string, char32_t>;
        using w_conv = foundation::text::wstring_convert<foundation::text::codecvt_utf8<wchar_t>, text::basic_string, wchar_t>;
        u32_conv c1;
        w_conv c2;
        auto utf8 = c1.to_bytes(sv.data(), sv.data() + sv.size());
        return c2.from_bytes(utf8.data(), utf8.data() + utf8.size());
    }

#else
    RAINY_INLINE native_string_t to_native(text::basic_string_view<char> sv) {
        return {sv.data(), sv.size()};
    }

    RAINY_INLINE native_string_t to_native(text::basic_string_view<wchar_t> sv) {
        using conv_t = foundation::text::wstring_convert<foundation::text::codecvt_utf8<wchar_t>, text::basic_string, wchar_t>;
        conv_t conv;
        return conv.to_bytes(sv.data(), sv.data() + sv.size());
    }

#if RAINY_HAS_CXX20
    RAINY_INLINE native_string_t to_native(text::basic_string_view<char8_t> sv) {
        return text::basic_string<char>(reinterpret_cast<const char *>(sv.data()), sv.size());
    }
#endif

    RAINY_INLINE native_string_t to_native(text::basic_string_view<char16_t> sv) {
        using conv_t = foundation::text::wstring_convert<foundation::text::codecvt_utf8<char16_t>, text::basic_string, char16_t>;
        conv_t conv;
        return conv.to_bytes(sv.data(), sv.data() + sv.size());
    }

    RAINY_INLINE native_string_t to_native(text::basic_string_view<char32_t> sv) {
        using conv_t = foundation::text::wstring_convert<foundation::text::codecvt_utf8<char32_t>, text::basic_string, char32_t>;
        conv_t conv;
        return conv.to_bytes(sv.data(), sv.data() + sv.size());
    }

#endif
    template <typename CharT>
    native_string_t source_to_native(const CharT *s) {
        return to_native(text::basic_string_view<CharT>(s));
    }

    template <typename CharT, typename Traits, typename Alloc>
    native_string_t source_to_native(const text::basic_string<CharT, Traits, Alloc> &s) {
        return to_native(text::basic_string_view<CharT>(s.data(), s.size()));
    }

    template <typename CharT, typename Traits, typename Alloc>
    native_string_t source_to_native(const std::basic_string<CharT, Traits, Alloc> &s) {
        return to_native(text::basic_string_view<CharT>(s.data(), s.size()));
    }

    template <typename CharT, typename Traits>
    native_string_t source_to_native(text::basic_string_view<CharT, Traits> sv) {
        return to_native(text::basic_string_view<CharT>(sv.data(), sv.size()));
    }

    template <typename CharT, typename Traits>
    native_string_t source_to_native(std::basic_string_view<CharT, Traits> sv) {
        return to_native(text::basic_string_view<CharT>(sv.data(), sv.size()));
    }

    template <typename InputIt>
    native_string_t range_to_native(InputIt first, InputIt last) {
        using char_type = typename utility::iterator_traits<InputIt>::value_type;
        text::basic_string<char_type> tmp(first, last);
        return to_native(text::basic_string_view<char_type>(tmp.data(), tmp.size()));
    }
}

namespace rainy::foundation::io::filesystem::implements {
    template <typename Path>
    class path_iterator {
    public:
        friend class filesystem::path;

        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = Path;
        using difference_type = std::ptrdiff_t;
        using pointer = const Path *;
        using reference = const Path &;

        path_iterator() noexcept : owner_(nullptr), pos_(0), end_(0), at_end_(true) {
        }

        reference operator*() const noexcept {
            return element_;
        }
        pointer operator->() const noexcept {
            return &element_;
        }

        path_iterator &operator++() {
            advance();
            return *this;
        }

        path_iterator operator++(int) {
            path_iterator tmp = *this;
            advance();
            return tmp;
        }

        path_iterator &operator--() {
            retreat();
            return *this;
        }

        path_iterator operator--(int) {
            path_iterator tmp = *this;
            retreat();
            return tmp;
        }

        bool operator==(const path_iterator &o) const noexcept {
            if (at_end_ && o.at_end_) {
                return true;
            }
            if (at_end_ != o.at_end_) {
                return false;
            }
            return owner_ == o.owner_ && pos_ == o.pos_;
        }

        bool operator!=(const path_iterator &o) const noexcept {
            return !(*this == o);
        }

    private:
        static path_iterator make_begin(const Path &p) {
            path_iterator it;
            it.owner_ = &p;
            it.at_end_ = false;
            it.pos_ = 0;
            it.end_ = 0;
            if (p.pathstr_.empty()) {
                it.at_end_ = true;
            } else {
                it.load_element();
            }
            return it;
        }

        static path_iterator make_end(const Path &p) {
            path_iterator it;
            it.owner_ = &p;
            it.at_end_ = true;
            it.pos_ = p.pathstr_.size();
            it.end_ = p.pathstr_.size();
            return it;
        }

        void load_element() {
            const auto &s = owner_->pathstr_;
            const std::size_t n = s.size();
            if (pos_ >= n) {
                at_end_ = true;
                return;
            }
            // 根名（如 "C:" 或 "//host"）
            std::size_t rn = implements::root_name_length(s);
            if (pos_ < rn) {
                end_ = rn;
                element_ = Path(typename Path::string_type(s.data(), rn));
                return;
            }
            // 根目录分隔符
            if (pos_ == rn && implements::is_separator(s[pos_])) {
                end_ = pos_ + 1;
                element_ = Path(typename Path::string_type(1, Path::preferred_separator));
                return;
            }
            // 普通分量：跳过分隔符，读取直到下一个分隔符
            std::size_t start = pos_;
            while (start < n && implements::is_separator(s[start])) {
                ++start;
            }
            if (start >= n) {
                // 路径以分隔符结尾时产生一个空文件名分量（符合标准行为）
                end_ = n;
                element_ = Path{};
                return;
            }
            std::size_t finish = start;
            while (finish < n && !implements::is_separator(s[finish])) {
                ++finish;
            }
            pos_ = start;
            end_ = finish;
            element_ = Path(typename Path::string_type(s.data() + start, finish - start));
        }

        void advance() {
            if (at_end_) {
                return;
            }
            pos_ = end_;
            const auto &s = owner_->pathstr_;
            while (pos_ < s.size() && implements::is_separator(s[pos_])) {
                ++pos_;
            }
            if (pos_ >= s.size()) {
                at_end_ = true;
                return;
            }
            load_element();
        }

        void retreat() {
            const auto &s = owner_->pathstr_;
            std::size_t p = at_end_ ? s.size() : pos_;
            at_end_ = false;
            // 跳过末尾分隔符
            while (p > 0 && implements::is_separator(s[p - 1])) {
                --p;
            }
            if (p == 0) {
                pos_ = 0;
                end_ = 0;
                load_element();
                return;
            }
            std::size_t finish = p;
            while (p > 0 && !implements::is_separator(s[p - 1])) {
                --p;
            }
            pos_ = p;
            end_ = finish;
            element_ = Path(typename Path::string_type(s.data() + p, finish - p));
        }

        const Path *owner_;
        Path element_;
        std::size_t pos_; // 当前分量在 pathstr_ 中的起始下标
        std::size_t end_; // 当前分量结束下标（exclusive）
        bool at_end_;
    };
}

namespace rainy::foundation::io::filesystem {
    class path {
    public:
        using value_type = implements::native_char;
        using string_type = implements::native_string_t;

        using iterator = implements::path_iterator<path>;
        using const_iterator = iterator;


#if RAINY_USING_WINDOWS
        static constexpr value_type preferred_separator = L'\\';
#else
        static constexpr value_type preferred_separator = '/';
#endif

        template <typename>
        friend class implements::path_iterator;

        enum format {
            native_format,
            generic_format,
            auto_format
        };

        path() noexcept : pathstr_() {
        }

        path(const path &other) : pathstr_(other.pathstr_) {
        }

        path(path &&other) noexcept : pathstr_(utility::move(other.pathstr_)) {
        }

        path(string_type &&source, format fmt = auto_format) : pathstr_(utility::move(source)) {
        }

        template <typename Source>
        path(const Source &source, format fmt = auto_format) : pathstr_(implements::source_to_native(source)) {
        }

        template <typename InputIt>
        path(InputIt first, InputIt last, format fmt = auto_format) : pathstr_(implements::range_to_native(first, last)) {
        }

        template <typename Source>
        path(const Source &source, const std::locale &loc, format fmt = auto_format) : path(source, fmt) {
        }

        template <typename InputIt>
        path(InputIt first, InputIt last, const std::locale &loc, format fmt = auto_format) : path(first, last, fmt) {
        }

        ~path() {
        }

        path &operator=(const path &other) {
            if (this != &other) {
                pathstr_ = other.pathstr_;
            }
            return *this;
        }

        path &operator=(path &&other) noexcept {
            if (this != &other)
                pathstr_ = utility::move(other.pathstr_);
            return *this;
        }

        path &operator=(string_type &&source) {
            pathstr_ = utility::move(source);
            return *this;
        }

        path &assign(string_type &&source) {
            pathstr_ = utility::move(source);
            return *this;
        }

        template <typename Source>
        path &operator=(const Source &source) {
            pathstr_ = implements::source_to_native(source);
            return *this;
        }

        template <typename Source>
        path &assign(const Source &source) {
            pathstr_ = implements::source_to_native(source);
            return *this;
        }

        template <typename InputIt>
        path &assign(InputIt first, InputIt last) {
            pathstr_ = implements::range_to_native(first, last);
            return *this;
        }

        path &operator/=(const path &other) {
            if (other.is_absolute()) {
                pathstr_ = other.pathstr_;
                return *this;
            }
            // other 有根名但不是绝对路径的情况（仅 Windows，如 "C:foo"）
            if (!pathstr_.empty() && !implements::is_separator(pathstr_.back()) && !other.pathstr_.empty()) {
                pathstr_ += preferred_separator;
            }
            pathstr_ += other.pathstr_;
            return *this;
        }

        template <typename Source>
        path &operator/=(const Source &source) {
            return operator/=(path(source));
        }

        template <typename Source>
        path &append(const Source &source) {
            return operator/=(path(source));
        }

        template <typename InputIt>
        path &append(InputIt first, InputIt last) {
            return operator/=(path(first, last));
        }

        path &operator+=(const path &x) {
            pathstr_ += x.pathstr_;
            return *this;
        }

        path &operator+=(const string_type &x) {
            pathstr_ += x;
            return *this;
        }

        path &operator+=(text::basic_string_view<value_type> x) {
            pathstr_.append(x.data(), x.size());
            return *this;
        }

        path &operator+=(const value_type *x) {
            pathstr_ += x;
            return *this;
        }
        path &operator+=(value_type x) {
            pathstr_ += x;
            return *this;
        }

        template <typename Source>
        path &operator+=(const Source &x) {
            pathstr_ += implements::source_to_native(x);
            return *this;
        }

        template <typename ECharT>
        path &operator+=(ECharT x) {
            ECharT buf[2] = {x, ECharT(0)};
            pathstr_ += implements::source_to_native(buf);
            return *this;
        }

        template <typename Source>
        path &concat(const Source &x) {
            return operator+=(x);
        }

        template <typename InputIt>
        path &concat(InputIt first, InputIt last) {
            pathstr_ += implements::range_to_native(first, last);
            return *this;
        }

        void clear() noexcept {
            pathstr_.clear();
        }

        path &make_preferred() {
#if RAINY_USING_WINDOWS
            for (auto &c: pathstr_) {
                if (c == L'/') {
                    c = L'\\';
                }
            }
#endif
            return *this;
        }

        path &remove_filename() {
            pathstr_ = parent_path().pathstr_;
            return *this;
        }

        path &replace_filename(const path &replacement) {
            remove_filename();
            return operator/=(replacement);
        }

        path &replace_extension(const path &replacement) {
            // 去掉现有扩展名
            if (has_extension()) {
                auto s = stem();
                auto p = parent_path();
                if (!p.empty()) {
                    pathstr_ = utility::move(p.pathstr_);
                    pathstr_ += preferred_separator;
                    pathstr_ += s.pathstr_;
                } else {
                    pathstr_ = utility::move(s.pathstr_);
                }
            }
            if (!replacement.empty()) {
                if (replacement.pathstr_[0] != static_cast<value_type>('.')) {
                    pathstr_ += static_cast<value_type>('.');
                }
                pathstr_ += replacement.pathstr_;
            }
            return *this;
        }

        void swap(path &other) noexcept {
            pathstr_.swap(other.pathstr_);
        }


        friend path operator/(const path &left, const path &right) {
            path result(left);
            result /= right;
            return result;
        }

        friend bool operator==(const path &left, const path &right) noexcept {
            return left.pathstr_ == right.pathstr_;
        }

#if RAINY_HAS_CXX20
        friend std::strong_ordering operator<=>(const path &left, const path &right) noexcept {
            if (left.pathstr_ < right.pathstr_) {
                return std::strong_ordering::less;
            }
            if (left.pathstr_ > right.pathstr_) {
                return std::strong_ordering::greater;
            }
            return std::strong_ordering::equal;
        }
#endif

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

        bool empty() const noexcept {
            return pathstr_.empty();
        }

        bool has_root_name() const {
            return !root_name().empty();
        }

        bool has_root_directory() const {
            return !root_directory().empty();
        }

        bool has_root_path() const {
            return !root_path().empty();
        }

        bool has_relative_path() const {
            return !relative_path().empty();
        }

        bool has_parent_path() const {
            return !parent_path().empty();
        }

        bool has_filename() const {
            return !filename().empty();
        }

        bool has_stem() const {
            return !stem().empty();
        }

        bool has_extension() const {
            return !extension().empty();
        }

        bool is_absolute() const {
#if RAINY_USING_WINDOWS
            // Windows：必须同时有根名和根目录（如 "C:\\"）
            return has_root_name() && has_root_directory();
#else
            return has_root_directory();
#endif
        }

        bool is_relative() const {
            return !is_absolute();
        }

        path lexically_normal() const {
            // 若路径为空直接返回
            // 把所有分量收集起来，处理 "." 和 ".."
            // 重建路径
            if (empty())
                return {};

            // 收集所有分量
            text::basic_string<value_type> dot(1, static_cast<value_type>('.'));
            text::basic_string<value_type> dotdot(2, static_cast<value_type>('.'));

            // 用 vector 模拟：存储每段分量字符串
            // 由于 rainy 可能没有 std::vector，这里用 std::vector 作为内部工具
            std::vector<string_type> parts;
            bool has_root_dir = has_root_directory();
            path rn = root_name();

            for (auto it = begin(); it != end(); ++it) {
                const string_type &seg = it->pathstr_;
                if (seg == dot) {
                    // "." 跳过
                    continue;
                } else if (seg == dotdot) {
                    if (!parts.empty() && parts.back() != dotdot) {
                        parts.pop_back();
                    } else if (!has_root_dir) {
                        // 相对路径中 ".." 保留
                        parts.push_back(seg);
                    }
                    // 绝对路径中根目录前的 ".." 直接忽略
                } else if (!seg.empty() && !(seg.size() == 1 && implements::is_separator(seg[0]))) {
                    // 普通分量（排除根目录分隔符本身）
                    parts.push_back(seg);
                }
            }

            // 重建
            path result;
            if (!rn.empty())
                result.pathstr_ = rn.pathstr_;
            if (has_root_dir)
                result.pathstr_ += preferred_separator;

            for (std::size_t i = 0; i < parts.size(); ++i) {
                if (i > 0)
                    result.pathstr_ += preferred_separator;
                result.pathstr_ += parts[i];
            }

            // 原始路径以分隔符结尾且结果非空非纯根路径时，加一个 "."
            if (!pathstr_.empty() && implements::is_separator(pathstr_.back()) && !parts.empty()) {
                result.pathstr_ += preferred_separator;
                result.pathstr_ += dot;
            }

            if (result.empty())
                result.pathstr_ = dot;
            return result;
        }

        path lexically_relative(const path &base) const {
            // 按标准：先确认根路径相同，然后找公共前缀，再用 ".." 补足
            if (root_path() != base.root_path()) {
                return {};
            }
            auto a = begin(), a_end = end();
            auto b = base.begin(), b_end = base.end();

            // 跳过公共前缀
            while (a != a_end && b != b_end && *a == *b) {
                ++a;
                ++b;
            }

            // base 剩余的非 ".." 分量数即为需要退后的层数
            text::basic_string<value_type> dotdot(2, static_cast<value_type>('.'));
            dotdot[1] = static_cast<value_type>('.');

            int up = 0;
            for (auto it = b; it != b_end; ++it) {
                const string_type &seg = it->pathstr_;
                if (!seg.empty() && !(seg.size() == 1 && implements::is_separator(seg[0]))) {
                    if (seg == dotdot) {
                        --up;
                    } else {
                        ++up;
                    }
                }
            }
            if (up < 0) {
                return {};
            }

            path result;
            text::basic_string<value_type> dotdot_seg(2, static_cast<value_type>('.'));
            dotdot_seg[0] = static_cast<value_type>('.');
            dotdot_seg[1] = static_cast<value_type>('.');

            for (int i = 0; i < up; ++i) {
                result /= path(dotdot_seg);
            }
            for (auto it = a; it != a_end; ++it) {
                result /= *it;
            }
            if (result.empty()) {
                result = path(text::basic_string<value_type>(1, static_cast<value_type>('.')));
            }
            return result;
        }

        path lexically_proximate(const path &base) const {
            path rel = lexically_relative(base);
            if (rel.empty())
                return *this;
            return rel;
        }


        path::iterator begin() const {
            return iterator::make_begin(*this);
        }

        path::iterator end() const {
            return iterator::make_end(*this);
        }

        template <class CharT, class Traits>
        friend std::basic_ostream<CharT, Traits> &operator<<(std::basic_ostream<CharT, Traits> &ostream, const path &path);
        template <class CharT, class Traits>
        friend std::basic_istream<CharT, Traits> &operator>>(std::basic_istream<CharT, Traits> &istream, path &path);

    private:
        string_type pathstr_;
    };

    void swap(path &left, path &right) noexcept;
    std::size_t hash_value(const path &path) noexcept;
}


namespace rainy::foundation::io::filesystem {
    const path::string_type &path::native() const noexcept {
        return pathstr_;
    }

    const path::value_type *path::c_str() const noexcept {
        return pathstr_.c_str();
    }

    path::operator path::string_type() const {
        return pathstr_;
    }

    static std::string path_to_utf8(const path &p) {
#if RAINY_USING_WINDOWS
        using conv_t = foundation::text::wstring_convert<foundation::text::codecvt_utf8<wchar_t>, text::basic_string, wchar_t>;
        conv_t conv;
        auto r = conv.to_bytes(p.c_str(), p.c_str() + p.native().size());
        return std::string(r.data(), r.size());
#else
        return std::string(p.c_str(), p.native().size());
#endif
    }

    std::string path::string() const {
        return path_to_utf8(*this);
    }

    std::wstring path::wstring() const {
#if RAINY_USING_WINDOWS
        return std::wstring(pathstr_.data(), pathstr_.size());
#else
        using conv_t = foundation::text::wstring_convert<foundation::text::codecvt_utf8<wchar_t>, text::basic_string, wchar_t>;
        conv_t conv;
        auto r = conv.from_bytes(pathstr_.data(), pathstr_.data() + pathstr_.size());
        return std::wstring(r.data(), r.size());
#endif
    }

    std::u8string path::u8string() const {
        auto s = path_to_utf8(*this);
        return std::u8string(reinterpret_cast<const char8_t *>(s.data()), s.size());
    }

    std::u16string path::u16string() const {
        auto s = path_to_utf8(*this);
        using conv_t = foundation::text::wstring_convert<foundation::text::codecvt_utf8<char16_t>, text::basic_string, char16_t>;
        conv_t conv;
        auto r = conv.from_bytes(s.data(), s.data() + s.size());
        return std::u16string(r.data(), r.size());
    }

    std::u32string path::u32string() const {
        auto s = path_to_utf8(*this);
        using conv_t = foundation::text::wstring_convert<foundation::text::codecvt_utf8<char32_t>, text::basic_string, char32_t>;
        conv_t conv;
        auto r = conv.from_bytes(s.data(), s.data() + s.size());
        return std::u32string(r.data(), r.size());
    }

    // ---- generic_* 版本：仅将 '\\' 替换为 '/' ---------------------------------

    std::string path::generic_string() const {
        auto s = string();
#if RAINY_USING_WINDOWS
        for (auto &c: s)
            if (c == '\\')
                c = '/';
#endif
        return s;
    }

    std::wstring path::generic_wstring() const {
        auto s = wstring();
#if RAINY_USING_WINDOWS
        for (auto &c: s)
            if (c == L'\\')
                c = L'/';
#endif
        return s;
    }

    std::u8string path::generic_u8string() const {
        auto s = u8string();
#if RAINY_USING_WINDOWS
        for (auto &c: s)
            if (c == u8'\\')
                c = u8'/';
#endif
        return s;
    }

    std::u16string path::generic_u16string() const {
        auto s = u16string();
#if RAINY_USING_WINDOWS
        for (auto &c: s)
            if (c == u'\\')
                c = u'/';
#endif
        return s;
    }

    std::u32string path::generic_u32string() const {
        auto s = u32string();
#if RAINY_USING_WINDOWS
        for (auto &c: s)
            if (c == U'\\')
                c = U'/';
#endif
        return s;
    }

    // =========================================================================
    // 比较
    // =========================================================================

    int path::compare(const path &other) const noexcept {
        return pathstr_.compare(other.pathstr_);
    }

    int path::compare(const string_type &s) const {
        return pathstr_.compare(s);
    }

    int path::compare(text::basic_string_view<value_type> s) const {
        return pathstr_.compare(string_type(s.data(), s.size()));
    }

    int path::compare(const value_type *s) const {
        return pathstr_.compare(s);
    }

    path path::root_name() const {
        std::size_t n = implements::root_name_length(pathstr_);
        if (n == 0) {
            return {};
        }
        return path(string_type(pathstr_.data(), n));
    }

    path path::root_directory() const {
        std::size_t rn = implements::root_name_length(pathstr_);
        if (rn < pathstr_.size() && implements::is_separator(pathstr_[rn])) {
            return path(string_type(1, preferred_separator));
        }
        return {};
    }

    path path::root_path() const {
        std::size_t rn = implements::root_name_length(pathstr_);
        std::size_t end = rn;
        if (end < pathstr_.size() && implements::is_separator(pathstr_[end]))
            ++end;
        return path(string_type(pathstr_.data(), end));
    }

    path path::relative_path() const {
        std::size_t rn = implements::root_name_length(pathstr_);
        if (rn < pathstr_.size() && implements::is_separator(pathstr_[rn]))
            ++rn;
        while (rn < pathstr_.size() && implements::is_separator(pathstr_[rn]))
            ++rn;
        return path(string_type(pathstr_.data() + rn, pathstr_.size() - rn));
    }

    path path::parent_path() const {
        if (empty())
            return {};
        // 从末尾往前找最后一个分隔符
        std::size_t rn = implements::root_name_length(pathstr_);
        // 跳过根目录后的内容起点
        std::size_t start = rn;
        if (start < pathstr_.size() && implements::is_separator(pathstr_[start]))
            ++start;

        // 从末尾向前找最后一个非分隔符字符的位置，然后继续找分隔符
        std::size_t end = pathstr_.size();
        // 跳过末尾分隔符
        while (end > start && implements::is_separator(pathstr_[end - 1]))
            --end;
        // 跳过最后一个分量
        while (end > start && !implements::is_separator(pathstr_[end - 1]))
            --end;
        // 再跳过该分量之前的分隔符（保留一个）
        if (end > start) {
            // 保留一个分隔符（即 end 已经指向分隔符后）
            // 但不能把根目录分隔符也吃掉
            while (end > rn + 1 && implements::is_separator(pathstr_[end - 1]))
                --end;
        }

        if (end <= rn) {
            // 没有父路径（只有根名或完全没有）
            if (rn > 0)
                return path(string_type(pathstr_.data(), rn));
            return {};
        }
        return path(string_type(pathstr_.data(), end));
    }

    path path::filename() const {
        if (empty())
            return {};
        auto it = end();
        --it;
        return *it;
    }

    path path::stem() const {
        string_type fn = filename().pathstr_;
        // "." 和 ".." 原样返回
        if (fn == string_type(1, static_cast<value_type>('.')) || fn == string_type(2, static_cast<value_type>('.'))) {
            return path(fn);
        }
        auto dot = fn.rfind(static_cast<value_type>('.'));
        // 没有点，或点在最开头（隐藏文件）则无扩展名
        if (dot == string_type::npos || dot == 0)
            return path(fn);
        return path(string_type(fn.data(), dot));
    }

    path path::extension() const {
        string_type fn = filename().pathstr_;
        if (fn == string_type(1, static_cast<value_type>('.')) || fn == string_type(2, static_cast<value_type>('.'))) {
            return {};
        }
        auto dot = fn.rfind(static_cast<value_type>('.'));
        if (dot == string_type::npos || dot == 0)
            return {};
        return path(string_type(fn.data() + dot, fn.size() - dot));
    }

    template <class CharT, class Traits>
    std::basic_ostream<CharT, Traits> &operator<<(std::basic_ostream<CharT, Traits> &os, const path &p) {
        // 输出带引号的 generic_string（与标准库行为一致）
        os << std::basic_string<CharT, Traits>(1, static_cast<CharT>('"'));
        if constexpr (std::is_same_v<CharT, wchar_t>) {
            auto s = p.generic_wstring();
            os << s.c_str();
        } else {
            auto s = p.generic_string();
            for (auto c: s)
                os.put(static_cast<CharT>(c));
        }
        os << std::basic_string<CharT, Traits>(1, static_cast<CharT>('"'));
        return os;
    }

    template <class CharT, class Traits>
    std::basic_istream<CharT, Traits> &operator>>(std::basic_istream<CharT, Traits> &is, path &p) {
        std::basic_string<CharT, Traits> tmp;
        is >> tmp;
        if (tmp.size() >= 2 && tmp.front() == static_cast<CharT>('"') && tmp.back() == static_cast<CharT>('"')) {
            tmp = tmp.substr(1, tmp.size() - 2);
        }
        p = path(tmp);
        return is;
    }

    void swap(path &left, path &right) noexcept {
        left.swap(right);
    }

    std::size_t hash_value(const path &p) noexcept {
        auto s = p.generic_string();
        std::size_t hash = 14695981039346656037ULL;
        for (unsigned char c: s) {
            hash ^= static_cast<std::size_t>(c);
            hash *= 1099511628211ULL;
        }
        return hash;
    }
}

#endif

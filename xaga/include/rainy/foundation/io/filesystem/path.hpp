#ifndef RAINY_FOUNDATION_IO_FILESYSTEM_PATH_HPP
#define RAINY_FOUNDATION_IO_FILESYSTEM_PATH_HPP
#include <rainy/collections/vector.hpp>
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

    template <typename CharType>
    RAINY_INLINE bool is_separator(CharType c) noexcept {
        return c == static_cast<CharType>('/') || c == static_cast<CharType>('\\');
    }

    template <typename CharType>
    std::size_t root_name_length(const text::basic_string<CharType> &p) noexcept {
#if RAINY_USING_WINDOWS
        if (p.size() >= 2 && p[1] == static_cast<CharType>(':') &&
            ((p[0] >= static_cast<CharType>('A') && p[0] <= static_cast<CharType>('Z')) ||
             (p[0] >= static_cast<CharType>('a') && p[0] <= static_cast<CharType>('z')))) {
            return 2;
        }
        if (p.size() >= 2 && is_separator(p[0]) && is_separator(p[1])) {
            if (p.size() == 2) {
                return 2;
            }
            auto pos = p.find_first_of(static_cast<CharType>('/'));
            auto pos2 = p.find_first_of(static_cast<CharType>('\\'));
            // 取更小的非 npos 值
            std::size_t sep = text::basic_string<CharType>::npos;
            if (pos != text::basic_string<CharType>::npos) {
                sep = pos;
            }
            if (pos2 != text::basic_string<CharType>::npos && pos2 < sep) {
                sep = pos2;
            }
            return (sep == text::basic_string<CharType>::npos) ? p.size() : sep;
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
    RAINY_INLINE native_string_t to_native(const text::basic_string_view<char> sv) {
        return {sv.data(), sv.size()};
    }

    RAINY_INLINE native_string_t to_native(const text::basic_string_view<wchar_t> sv) {
        using conv_t = foundation::text::wstring_convert<foundation::text::codecvt_utf8<wchar_t>, text::basic_string, wchar_t>;
        conv_t conv;
        return conv.to_bytes(sv.data(), sv.data() + sv.size());
    }

#if RAINY_HAS_CXX20
    RAINY_INLINE native_string_t to_native(const text::basic_string_view<char8_t> sv) {
        return {reinterpret_cast<const char *>(sv.data()), sv.size()};
    }
#endif

    RAINY_INLINE native_string_t to_native(const text::basic_string_view<char16_t> sv) {
        using conv_t = foundation::text::wstring_convert<foundation::text::codecvt_utf8<char16_t>, text::basic_string, char16_t>;
        conv_t conv;
        return conv.to_bytes(sv.data(), sv.data() + sv.size());
    }

    RAINY_INLINE native_string_t to_native(const text::basic_string_view<char32_t> sv) {
        using conv_t = foundation::text::wstring_convert<foundation::text::codecvt_utf8<char32_t>, text::basic_string, char32_t>;
        conv_t conv;
        return conv.to_bytes(sv.data(), sv.data() + sv.size());
    }

#endif
    template <typename CharType>
    native_string_t source_to_native(const CharType *s) {
        return to_native(text::basic_string_view<CharType>(s));
    }

    template <typename CharType, typename Traits, typename Alloc>
    native_string_t source_to_native(const text::basic_string<CharType, Traits, Alloc> &s) {
        return to_native(text::basic_string_view<CharType>(s.data(), s.size()));
    }

    template <typename CharType, typename Traits, typename Alloc>
    native_string_t source_to_native(const std::basic_string<CharType, Traits, Alloc> &s) {
        return to_native(text::basic_string_view<CharType>(s.data(), s.size()));
    }

    template <typename CharType, typename Traits>
    native_string_t source_to_native(text::basic_string_view<CharType, Traits> sv) {
        return to_native(text::basic_string_view<CharType>(sv.data(), sv.size()));
    }

    template <typename CharType, typename Traits>
    native_string_t source_to_native(std::basic_string_view<CharType, Traits> sv) {
        return to_native(text::basic_string_view<CharType>(sv.data(), sv.size()));
    }

    template <typename InputIt>
    native_string_t range_to_native(InputIt first, InputIt last) {
        using char_type = utility::iterator_traits<InputIt>::value_type;
        text::basic_string<char_type> tmp(first, last);
        return to_native(text::basic_string_view<char_type>(tmp.data(), tmp.size()));
    }

    template <typename Path>
    static text::string path_to_utf8(const Path &p) {
#if RAINY_USING_WINDOWS
        using conv_t = foundation::text::wstring_convert<foundation::text::codecvt_utf8<wchar_t>, text::basic_string, wchar_t>;
        conv_t conv;
        auto r = conv.to_bytes(p.c_str(), p.c_str() + p.native().size());
        return text::string(r.data(), r.size());
#else
        return text::string(p.c_str(), p.native().size());
#endif
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

        path_iterator() noexcept : owner_(nullptr), at_end_(true) {
            core::czstring();
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
            const std::size_t finish = p;
            while (p > 0 && !implements::is_separator(s[p - 1])) {
                --p;
            }
            pos_ = p;
            end_ = finish;
            element_ = Path(typename Path::string_type(s.data() + p, finish - p));
        }

        const Path *owner_;
        Path element_;
        bool at_end_;
        std::size_t pos_{0}; // 当前分量在 pathstr_ 中的起始下标
        std::size_t end_{0}; // 当前分量结束下标（exclusive）
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

        enum format { // NOLINT
            native_format,
            generic_format,
            auto_format
        };

        path() noexcept : pathstr_() {
        }

        path(const path &other) : pathstr_(other.pathstr_) { // NOLINT
        }

        path(path &&other) noexcept : pathstr_(utility::move(other.pathstr_)) {
        }

        path(string_type &&source, format fmt = auto_format) : pathstr_(utility::move(source)) { // NOLINT
        }

        template <typename Source>
        path(const Source &source, format fmt = auto_format) : pathstr_(implements::source_to_native(source)) { // NOLINT
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

        ~path() = default;

        path &operator=(const path &other) {
            if (this != &other) {
                pathstr_ = other.pathstr_;
            }
            return *this;
        }

        path &operator=(path &&other) noexcept {
            if (this != &other) {
                pathstr_ = utility::move(other.pathstr_);
            }
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

        path &operator+=(const path &right) {
            pathstr_ += right.pathstr_;
            return *this;
        }

        path &operator+=(const string_type &right) {
            pathstr_ += right;
            return *this;
        }

        path &operator+=(const text::basic_string_view<value_type> right) {
            pathstr_.append(right.data(), right.size());
            return *this;
        }

        path &operator+=(const value_type *right) {
            pathstr_ += right;
            return *this;
        }
        path &operator+=(value_type right) {
            pathstr_ += right;
            return *this;
        }

        template <typename Source>
        path &operator+=(const Source &right) {
            pathstr_ += implements::source_to_native(right);
            return *this;
        }

        template <typename ECharT>
        path &operator+=(ECharT right) {
            ECharT buf[2] = {right, ECharT(0)};
            pathstr_ += implements::source_to_native(buf);
            return *this;
        }

        template <typename Source>
        path &concat(const Source &right) {
            return operator+=(right);
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
            if (has_extension()) {
                auto s = stem();
                if (auto p = parent_path(); !p.empty()) {
                    // NOLINTBEGIN
                    pathstr_ = utility::move(p.pathstr_);
                    pathstr_ += preferred_separator;
                    pathstr_ += s.pathstr_;
                    // NOLINTEND
                } else {
                    pathstr_ = utility::move(s.pathstr_); // NOLINT
                }
            }
            if (!replacement.empty()) {
                if (replacement.pathstr_[0] != static_cast<value_type>('.')) { // NOLINT
                    pathstr_ += static_cast<value_type>('.'); // NOLINT
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

        RAINY_NODISCARD const string_type &native() const noexcept {
            return pathstr_;
        }

        RAINY_NODISCARD const value_type *c_str() const noexcept {
            return pathstr_.c_str();
        }

        RAINY_NODISCARD operator string_type() const { // NOLINT
            return pathstr_;
        }

        template <class ECharT, class Traits = text::char_traits<ECharT>, class Allocator = std::allocator<ECharT>>
        text::basic_string<ECharT, Traits, Allocator> string(const Allocator &a = Allocator()) const {
            auto s = implements::path_to_utf8(*this);
            return {utility::move(s), a};
        }

        RAINY_NODISCARD text::string string() const {
            return implements::path_to_utf8(*this);
        }

        RAINY_NODISCARD text::wstring wstring() const {
#if RAINY_USING_WINDOWS
            return text::wstring(pathstr_.data(), pathstr_.size());
#else
            using conv_t = foundation::text::wstring_convert<foundation::text::codecvt_utf8<wchar_t>, text::basic_string, wchar_t>;
            conv_t conv;
            auto r = conv.from_bytes(pathstr_.data(), pathstr_.data() + pathstr_.size());
            return {r.data(), r.size()};
#endif
        }

#if RAINY_HAS_CXX20
        RAINY_NODISCARD text::u8string u8string() const {
            auto s = implements::path_to_utf8(*this);
            return {reinterpret_cast<const char8_t *>(s.data()), s.size()};
        }
#endif

        RAINY_NODISCARD text::u16string u16string() const {
            auto s = implements::path_to_utf8(*this);
            using conv_t = foundation::text::wstring_convert<foundation::text::codecvt_utf8<char16_t>, text::basic_string, char16_t>;
            conv_t conv;
            auto r = conv.from_bytes(s.data(), s.data() + s.size());
            return {r.data(), r.size()};
        }

        RAINY_NODISCARD text::u32string u32string() const {
            auto s = implements::path_to_utf8(*this);
            using conv_t = foundation::text::wstring_convert<foundation::text::codecvt_utf8<char32_t>, text::basic_string, char32_t>;
            conv_t conv;
            auto r = conv.from_bytes(s.data(), s.data() + s.size());
            return {r.data(), r.size()};
        }

        template <typename ECharT, typename Traits = text::char_traits<ECharT>, typename Allocator = std::allocator<ECharT>>
        RAINY_NODISCARD text::basic_string<ECharT, Traits, Allocator> generic_string(const Allocator &allocator = Allocator()) const {
            if constexpr (type_traits::type_relations::is_same_v<ECharT, char>) {
                auto s = implements::path_to_utf8(*this);
#if RAINY_USING_WINDOWS
                for (auto &c: s) {
                    if (c == '\\') {
                        c = '/';
                    }
                }
#endif
                return {utility::move(s), allocator};
            } else if constexpr (type_traits::type_relations::is_same_v<ECharT, wchar_t>) {
                auto s = wstring();
#if RAINY_USING_WINDOWS
                for (auto &c: s) {
                    if (c == '\\') {
                        c = L'/';
                    }
                }
#endif
                return {utility::move(s), allocator};
            } else if constexpr (type_traits::type_relations::is_same_v<ECharT, char16_t>) {
                auto s = u16string();
#if RAINY_USING_WINDOWS
                for (auto &c: s) {
                    if (c == u'\\') {
                        c = u'/';
                    }
                }
#endif
                return {utility::move(s), allocator};
            } else if constexpr (type_traits::type_relations::is_same_v<ECharT, char32_t>) {
                auto s = u32string();
#if RAINY_USING_WINDOWS
                for (auto &c: s) {
                    if (c == U'\\') {
                        c = U'/';
                    }
                }
#endif
                return {utility::move(s), allocator};
            } else {
#if RAINY_HAS_CXX20
                static_assert(type_traits::type_relations::is_same_v<ECharT, char8_t>);
                auto s = u8string();
#if RAINY_USING_WINDOWS
                for (auto &c: s) {
                    if (c == u8'\\') {
                        c = u8'/';
                    }
                }
#endif
                return {utility::move(s), allocator};
#else
                static_assert(false);
#endif
            }
        }

#if RAINY_HAS_CXX20
        RAINY_NODISCARD text::u8string generic_u8string() const {
            return generic_string<char8_t>();
        }
#endif

        RAINY_NODISCARD text::string generic_string() const {
            return generic_string<char>();
        }

        RAINY_NODISCARD text::wstring generic_wstring() const {
            return generic_string<wchar_t>();
        }

        RAINY_NODISCARD text::u16string generic_u16string() const {
            return generic_string<char16_t>();
        }

        RAINY_NODISCARD text::u32string generic_u32string() const {
            return generic_string<char32_t>();
        }

        RAINY_NODISCARD int compare(const path &other) const noexcept {
            return pathstr_.compare(other.pathstr_);
        }

        RAINY_NODISCARD int compare(const string_type &s) const {
            return pathstr_.compare(s);
        }

        RAINY_NODISCARD int compare(text::basic_string_view<value_type> s) const {
            return pathstr_.compare(string_type(s.data(), s.size()));
        }

        RAINY_NODISCARD int compare(const value_type *s) const {
            return pathstr_.compare(s);
        }

        RAINY_NODISCARD path root_name() const {
            const std::size_t n = implements::root_name_length(pathstr_);
            if (n == 0) { // NOLINT
                return {};
            }
            return {string_type(pathstr_.data(), n)}; // NOLINT
        }

        RAINY_NODISCARD path root_directory() const {
            const std::size_t rn = implements::root_name_length(pathstr_);
            if (rn < pathstr_.size() && implements::is_separator(pathstr_[rn])) {
                return string_type(1, preferred_separator);
            }
            return {};
        }

        RAINY_NODISCARD path root_path() const {
            std::size_t rn = implements::root_name_length(pathstr_);
            std::size_t end = rn;
            if (end < pathstr_.size() && implements::is_separator(pathstr_[end])) {
                ++end;
            }
            return {string_type(pathstr_.data(), end)};
        }

        RAINY_NODISCARD path relative_path() const {
            std::size_t rn = implements::root_name_length(pathstr_);
            if (rn < pathstr_.size() && implements::is_separator(pathstr_[rn])) {
                ++rn;
            }
            while (rn < pathstr_.size() && implements::is_separator(pathstr_[rn])) {
                ++rn;
            }
            return path(string_type(pathstr_.data() + rn, pathstr_.size() - rn));
        }

        RAINY_NODISCARD path parent_path() const {
            if (empty()) {
                return {};
            }
            const std::size_t rn = implements::root_name_length(pathstr_);
            std::size_t start = rn;
            if (start < pathstr_.size() && implements::is_separator(pathstr_[start])) {
                ++start;
            }
            std::size_t end = pathstr_.size();
            while (end > start && implements::is_separator(pathstr_[end - 1])) {
                --end;
            }
            while (end > start && !implements::is_separator(pathstr_[end - 1])) {
                --end;
            }
            if (end > start) {
                while (end > rn + 1 && implements::is_separator(pathstr_[end - 1])) {
                    --end;
                }
            }
            if (end <= rn) {
                if (rn > 0) {
                    return {string_type(pathstr_.data(), rn)}; // NOLINT
                }
                return {};
            }
            return path(string_type(pathstr_.data(), end));
        }

        RAINY_NODISCARD path filename() const {
            if (empty()) {
                return {};
            }
            auto it = end();
            --it;
            return *it;
        }

        RAINY_NODISCARD path stem() const {
            string_type fn = filename().pathstr_;
            if (fn == string_type(1, static_cast<value_type>('.')) || fn == string_type(2, static_cast<value_type>('.'))) {
                return {fn};
            }
            auto dot = fn.rfind(static_cast<value_type>('.'));
            if (dot == string_type::npos || dot == 0) {
                return {fn};
            }
            return path(string_type(fn.data(), dot));
        }

        RAINY_NODISCARD path extension() const {
            string_type fn = filename().pathstr_;
            if (fn == string_type(1, static_cast<value_type>('.')) || fn == string_type(2, static_cast<value_type>('.'))) {
                return {};
            }
            auto dot = fn.rfind(static_cast<value_type>('.'));
            if (dot == string_type::npos || dot == 0) {
                return {};
            }
            return path(string_type(fn.data() + dot, fn.size() - dot));
        }

        RAINY_NODISCARD bool empty() const noexcept {
            return pathstr_.empty();
        }

        RAINY_NODISCARD bool has_root_name() const {
            return !root_name().empty();
        }

        RAINY_NODISCARD bool has_root_directory() const {
            return !root_directory().empty();
        }

        RAINY_NODISCARD bool has_root_path() const {
            return !root_path().empty();
        }

        RAINY_NODISCARD bool has_relative_path() const {
            return !relative_path().empty();
        }

        RAINY_NODISCARD bool has_parent_path() const {
            return !parent_path().empty();
        }

        RAINY_NODISCARD bool has_filename() const {
            return !filename().empty();
        }

        RAINY_NODISCARD bool has_stem() const {
            return !stem().empty();
        }

        RAINY_NODISCARD bool has_extension() const {
            return !extension().empty();
        }

        RAINY_NODISCARD bool is_absolute() const {
#if RAINY_USING_WINDOWS
            // Windows：必须同时有根名和根目录（如 "C:\\"）
            return has_root_name() && has_root_directory();
#else
            return has_root_directory();
#endif
        }

        RAINY_NODISCARD bool is_relative() const {
            return !is_absolute();
        }

        RAINY_NODISCARD path lexically_normal() const {
            if (empty()) {
                return {};
            }
            const string_type dot(1, static_cast<value_type>('.')); // NOLINT
            const string_type dotdot(2, static_cast<value_type>('.')); // NOLINT
            collections::vector<string_type> parts;
            const bool has_root_dir = has_root_directory();
            const path rn = root_name();

            for (auto it = begin(); it != end(); ++it) {
                const string_type &seg = it->pathstr_;
                if (seg == dot) {
                    // "." 跳过
                    continue; // NOLINT
                }
                if (seg == dotdot) {
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
            if (!rn.empty()) {
                result.pathstr_ = rn.pathstr_;
            }
            if (has_root_dir) {
                result.pathstr_ += preferred_separator;
            }
            for (std::size_t i = 0; i < parts.size(); ++i) {
                if (i > 0) {
                    result.pathstr_ += preferred_separator;
                }
                result.pathstr_ += parts[i];
            }
            // 原始路径以分隔符结尾且结果非空非纯根路径时，加一个 "."
            if (!pathstr_.empty() && implements::is_separator(pathstr_.back()) && !parts.empty()) {
                result.pathstr_ += preferred_separator;
                result.pathstr_ += dot;
            }
            if (result.empty()) {
                result.pathstr_ = dot;
            }
            return result;
        }

        RAINY_NODISCARD path lexically_relative(const path &base) const {
            // 先确认根路径相同，然后找公共前缀，再用 ".." 补足
            if (root_path() != base.root_path()) {
                return {};
            }
            auto a = begin(), a_end = end();
            auto b = base.begin(), b_end = base.end();
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

        template <typename CharType, typename Traits>
        friend std::basic_ostream<CharType, Traits> &operator<<(std::basic_ostream<CharType, Traits> &os, const path &p) {
            os << std::basic_string<CharType, Traits>(1, static_cast<CharType>('"'));
            if constexpr (std::is_same_v<CharType, wchar_t>) {
                auto s = p.generic_wstring();
                os << s.c_str();
            } else {
                auto s = p.generic_string();
                for (auto c: s) {
                    os.put(static_cast<CharType>(c));
                }
            }
            os << std::basic_string<CharType, Traits>(1, static_cast<CharType>('"'));
            return os;
        }


        template <typename CharType, typename Traits>
        friend std::basic_istream<CharType, Traits> &operator>>(std::basic_istream<CharType, Traits> &is, path &p) {
            std::basic_string<CharType, Traits> tmp;
            is >> tmp;
            if (tmp.size() >= 2 && tmp.front() == static_cast<CharType>('"') && tmp.back() == static_cast<CharType>('"')) {
                tmp = tmp.substr(1, tmp.size() - 2);
            }
            p = path(tmp);
            return is;
        }

    private:
        string_type pathstr_;
    };

    void swap(path &left, path &right) noexcept;
    std::size_t hash_value(const path &path) noexcept;
}


namespace rainy::foundation::io::filesystem {
    RAINY_INLINE void swap(path &left, path &right) noexcept {
        left.swap(right);
    }
}

namespace rainy::utility {
    template <>
    struct hash<::rainy::foundation::io::filesystem::path> {
        static std::size_t hash_this_val(const ::rainy::foundation::io::filesystem::path &right) {
            return hash<::rainy::foundation::io::filesystem::path::string_type>{}(right.native());
        }

        std::size_t operator()(const ::rainy::foundation::io::filesystem::path &right) const {
            return hash_this_val(right);
        }
    };
}

namespace std { // NOLINT
    template <>
    struct hash<::rainy::foundation::io::filesystem::path> {
        std::size_t operator()(const ::rainy::foundation::io::filesystem::path &right) const noexcept {
            return std::hash<::rainy::foundation::io::filesystem::path::string_type>{}(right.native());
        }
    };
}

#endif

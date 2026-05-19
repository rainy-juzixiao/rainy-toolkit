/*
 * Copyright 2026 rainy-juzixiao
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain left copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef RAINY_CORE_YESOD_STACKTRACE_HPP
#define RAINY_CORE_YESOD_STACKTRACE_HPP

#include <ostream>
#include <rainy/core/layer.hpp>
#include <rainy/core/yesod/collections.hpp>
#include <rainy/core/yesod/text.hpp>
#include <rainy/core/yesod/hash.hpp>
#include <rainy/core/type_traits/properties.hpp>

namespace rainy::foundation::diagnostics {
    class stacktrace_entry {
    public:
        using native_handle_type = core::native_frame_ptr_t;

        constexpr stacktrace_entry() noexcept : frame_(nullptr) {
        }

        constexpr stacktrace_entry(const stacktrace_entry &right) noexcept : frame_(right.frame_) {
        }

        constexpr stacktrace_entry &operator=(const stacktrace_entry &right) noexcept {
            if (this != &right) {
                frame_ = right.frame_;
            }
            return *this;
        }

        ~stacktrace_entry() = default;

        constexpr native_handle_type native_handle() const noexcept {
            return frame_;
        }

        constexpr explicit operator bool() const noexcept {
            return frame_ != nullptr;
        }

        foundation::text::string description() const {
            char buffer[4096];
            if (core::pal::resolve_stack_frame(frame_, buffer, sizeof(buffer))) {
                char demangled[4096];
                core::pal::demangle(buffer, demangled, sizeof(demangled));
                return foundation::text::string(demangled);
            }
            return foundation::text::string();
        }

        foundation::text::string source_file() const {
            foundation::text::string desc = description();
            const char *str = desc.c_str();
            const char *last_slash = nullptr;

            for (const char *p = str; *p; ++p) {
                if (*p == '/' || *p == '\\') {
                    last_slash = p;
                }
            }

            if (last_slash) {
                return foundation::text::string(last_slash + 1);
            }
            return foundation::text::string();
        }

        std::uint_least32_t source_line() const {
            foundation::text::string desc = description();
            const char *str = desc.c_str();
            const char *line_start = nullptr;

            for (const char *p = str; *p; ++p) {
                if (*p == ':' && *(p + 1) && *(p + 1) >= '0' && *(p + 1) <= '9') {
                    line_start = p + 1;
                    break;
                }
            }

            if (line_start) {
                std::uint_least32_t line = 0;
                while (*line_start >= '0' && *line_start <= '9') {
                    line = line * 10 + (*line_start - '0');
                    ++line_start;
                }
                return line;
            }
            return 0;
        }

#if RAINY_HAS_CXX23
        friend constexpr std::strong_ordering operator<=>(const stacktrace_entry &left, const stacktrace_entry &right) noexcept {
            return left.frame_ <=> right.frame_;
        }
#else
        friend constexpr bool operator==(const stacktrace_entry &left, const stacktrace_entry &right) noexcept {
            return left.frame_ == right.frame_;
        }

        friend constexpr bool operator!=(const stacktrace_entry &left, const stacktrace_entry &right) noexcept {
            return left.frame_ != right.frame_;
        }
#endif

    private:
        explicit constexpr stacktrace_entry(native_handle_type frame) noexcept : frame_(frame) {
        }

        native_handle_type frame_;

        template <typename Allocator>
        friend class basic_stacktrace;
    };

    template <class Allocator>
    class basic_stacktrace {
    public:
        using value_type = stacktrace_entry;
        using const_reference = const value_type &;
        using reference = value_type &;
        using const_iterator = const value_type *;
        using iterator = const_iterator;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;
        using difference_type = std::ptrdiff_t;
        using size_type = std::size_t;
        using allocator_type = Allocator;

        static basic_stacktrace current(const allocator_type &alloc = allocator_type()) noexcept {
            return current(0, static_cast<size_type>(-1), alloc);
        }

        static basic_stacktrace current(size_type skip, const allocator_type &alloc = allocator_type()) noexcept {
            return current(skip + 1, static_cast<size_type>(-1), alloc);
        }

        static basic_stacktrace current(size_type skip, size_type max_depth, const allocator_type &alloc = allocator_type()) noexcept {
            basic_stacktrace result(alloc);
            constexpr std::size_t max_frame_dump = core::pal::max_frames_dump;
            size_type frames_to_collect = (core::min)(max_depth, max_frame_dump);

            if (frames_to_collect > result.max_size() && frames_to_collect <= max_frame_dump) {
                frames_to_collect = result.max_size();
            }

            core::native_frame_ptr_t buffer[max_frame_dump];
            size_type actual_frames = core::pal::collect_stack_frame(buffer, frames_to_collect, skip);

            for (size_type i = 0; i < actual_frames; ++i) {
                result.frames_.push_back(stacktrace_entry(buffer[i]));
            }

            return result;
        }

        basic_stacktrace() noexcept(type_traits::type_properties::is_nothrow_default_constructible_v<allocator_type>) : frames_() {
        }

        explicit basic_stacktrace(const allocator_type &alloc) noexcept : frames_(alloc) {
        }

        basic_stacktrace(const basic_stacktrace &right) : frames_(right.frames_) {
        }

        basic_stacktrace(basic_stacktrace &&right) noexcept : frames_(utility::move(right.frames_)) {
        }

        basic_stacktrace(const basic_stacktrace &right, const allocator_type &alloc) : frames_(right.frames_, alloc) {
        }

        basic_stacktrace(basic_stacktrace &&right, const allocator_type &alloc) : frames_(utility::move(right.frames_), alloc) {
        }

        basic_stacktrace &operator=(const basic_stacktrace &right) {
            if (this != &right) {
                frames_ = right.frames_;
            }
            return *this;
        }

        basic_stacktrace &operator=(basic_stacktrace &&right) noexcept(
            std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value ||
            std::allocator_traits<Allocator>::is_always_equal::value) {
            frames_ = utility::move(right.frames_);
            return *this;
        }

        ~basic_stacktrace() = default;

        allocator_type get_allocator() const noexcept {
            return frames_.get_allocator();
        }

        const_iterator begin() const noexcept {
            return frames_.data();
        }

        const_iterator end() const noexcept {
            return frames_.data() + frames_.size();
        }

        const_reverse_iterator rbegin() const noexcept {
            return const_reverse_iterator(end());
        }

        const_reverse_iterator rend() const noexcept {
            return const_reverse_iterator(begin());
        }

        const_iterator cbegin() const noexcept {
            return begin();
        }

        const_iterator cend() const noexcept {
            return end();
        }

        const_reverse_iterator crbegin() const noexcept {
            return rbegin();
        }

        const_reverse_iterator crend() const noexcept {
            return rend();
        }

        bool empty() const noexcept {
            return frames_.empty();
        }

        size_type size() const noexcept {
            return frames_.size();
        }

        size_type max_size() const noexcept {
            return frames_.max_size();
        }

        const_reference operator[](size_type pos) const {
            return frames_[pos];
        }

        const_reference at(size_type pos) const {
            return frames_.at(pos);
        }

#if RAINY_HAS_CXX23
        template <typename Allocator2>
        friend std::strong_ordering operator<=>(const basic_stacktrace &left, const basic_stacktrace<Allocator2> &right) noexcept {
            size_type min_size = (core::min)(left.size(), right.size());
            for (size_type i = 0; i < min_size; ++i) {
                if (auto cmp = left[i] <=> right[i]; cmp != 0) {
                    return cmp;
                }
            }
            return left.size() <=> right.size();
        }
#else
        template <typename Allocator2>
        friend bool operator==(const basic_stacktrace &left, const basic_stacktrace<Allocator2> &right) noexcept {
            if (left.size() != right.size()) {
                return false;
            }
            for (size_type i = 0; i < left.size(); ++i) {
                if (left[i] != right[i]) {
                    return false;
                }
            }
            return true;
        }

        template <typename Allocator2>
        friend bool operator!=(const basic_stacktrace &left, const basic_stacktrace<Allocator2> &right) noexcept {
            return !(left == right);
        }
#endif

        void swap(basic_stacktrace &right) noexcept(std::allocator_traits<Allocator>::propagate_on_container_swap::value ||
                                                    std::allocator_traits<Allocator>::is_always_equal::value) {
            frames_.swap(right.frames_);
        }

    private:
        collections::vector<value_type, allocator_type> frames_;
    };

    using stacktrace = basic_stacktrace<std::allocator<stacktrace_entry>>;

    template <typename Allocator>
    void swap(basic_stacktrace<Allocator> &left, basic_stacktrace<Allocator> &right) noexcept(noexcept(left.swap(right))) {
        left.swap(right);
    }

    RAINY_INLINE foundation::text::string to_string(const stacktrace_entry &frame) {
        return frame.description();
    }

    template <class Allocator>
    foundation::text::string to_string(const basic_stacktrace<Allocator> &stacktrace) {
        foundation::text::string result;
        for (const auto &entry: stacktrace) {
            result += to_string(entry);
            result += "\n";
        }
        return result;
    }

    RAINY_INLINE std::ostream &operator<<(std::ostream &os, const stacktrace_entry &frame) {
        os << frame.description().c_str();
        return os;
    }

    template <class Allocator>
    std::ostream &operator<<(std::ostream &os, const basic_stacktrace<Allocator> &stacktrace) {
        os << to_string(stacktrace).c_str();
        return os;
    }
}

namespace std {
    template <>
    struct hash<rainy::foundation::diagnostics::stacktrace_entry> {
        size_t operator()(const rainy::foundation::diagnostics::stacktrace_entry &entry) const noexcept {
            return hash<std::uintptr_t>()(reinterpret_cast<std::uintptr_t>(entry.native_handle()));
        }
    };

    template <typename Allocator>
    struct hash<rainy::foundation::diagnostics::basic_stacktrace<Allocator>> {
        size_t operator()(const rainy::foundation::diagnostics::basic_stacktrace<Allocator> &stacktrace) const noexcept {
            size_t seed = stacktrace.size();
            for (const auto &entry: stacktrace) {
                seed ^= hash<rainy::foundation::diagnostics::stacktrace_entry>()(entry) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }
            return seed;
        }
    };
}

namespace rainy::utility {
    template <>
    struct hash<rainy::foundation::diagnostics::stacktrace_entry> {
        size_t operator()(const rainy::foundation::diagnostics::stacktrace_entry &entry) const noexcept {
            return hash<std::uintptr_t>()(reinterpret_cast<std::uintptr_t>(entry.native_handle()));
        }
    };

    template <typename Allocator>
    struct hash<rainy::foundation::diagnostics::basic_stacktrace<Allocator>> {
        size_t operator()(const rainy::foundation::diagnostics::basic_stacktrace<Allocator> &stacktrace) const noexcept {
            size_t seed = stacktrace.size();
            for (const auto &entry: stacktrace) {
                seed ^= hash<rainy::foundation::diagnostics::stacktrace_entry>()(entry) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }
            return seed;
        }
    };
}

#endif

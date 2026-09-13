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
#ifndef RAINY_CORE_DIAGNOSTICS_STACKTRACE_HPP
#define RAINY_CORE_DIAGNOSTICS_STACKTRACE_HPP

#include <ostream>
#include <rainy/core/layer.hpp>
#include <rainy/core/type_traits/properties.hpp>
#include <rainy/core/collections/vector.hpp>
#include <rainy/core/utility/hash.hpp>
#include <rainy/core/text/string.hpp>

namespace rainy::foundation::diagnostics {
    /**
     * \lang english
     * @brief A single entry (frame) of a captured stack trace.
     *
     *  Wraps a native frame handle and provides access to the resolved
     *  description, source file, and source line of the frame.
     *
     * \lang simp-chinese
     * @brief 已捕获堆栈跟踪中的单个条目（帧）。
     *
     *  包装一个原生帧句柄，并提供对帧的已解析描述、源文件和源行的访问。
     */
    class stacktrace_entry {
    public:
        /**
         * \lang english
         * @brief The native handle type of a stack frame.
         *
         * \lang simp-chinese
         * @brief 堆栈帧的原生句柄类型。
         */
        using native_handle_type = core::native_frame_ptr_t;

        /**
         * \lang english
         * @brief Constructs an empty stacktrace_entry with no frame.
         *
         * \lang simp-chinese
         * @brief 构造一个不含帧的空stacktrace_entry。
         */
        constexpr stacktrace_entry() noexcept : frame_(nullptr) {
        }

        /**
         * \lang english
         * @brief Copies the frame handle from another stacktrace_entry.
         * @param right The entry to copy from.
         *
         * \lang simp-chinese
         * @brief 从另一个stacktrace_entry复制帧句柄。
         * @param right 要复制的条目。
         */
        constexpr stacktrace_entry(const stacktrace_entry &right) noexcept : frame_(right.frame_) {
        }

        /**
         * \lang english
         * @brief Assigns the frame handle from another stacktrace_entry.
         * @param right The entry to copy from.
         * @return A reference to this entry.
         *
         * \lang simp-chinese
         * @brief 从另一个stacktrace_entry赋值帧句柄。
         * @param right 要复制的条目。
         * @return 对此条目的引用。
         */
        constexpr stacktrace_entry &operator=(const stacktrace_entry &right) noexcept {
            if (this != &right) {
                frame_ = right.frame_;
            }
            return *this;
        }

        /**
         * \lang english
         * @brief Destroys the stacktrace_entry.
         *
         * \lang simp-chinese
         * @brief 销毁stacktrace_entry。
         */
        ~stacktrace_entry() = default;

        /**
         * \lang english
         * @brief Returns the native handle of the stack frame.
         * @return The underlying native frame pointer, or nullptr for an empty entry.
         *
         * \lang simp-chinese
         * @brief 返回堆栈帧的原生句柄。
         * @return 底层原生帧指针；空条目为nullptr。
         */
        constexpr native_handle_type native_handle() const noexcept {
            return frame_;
        }

        /**
         * \lang english
         * @brief Checks whether this entry contains a valid frame.
         * @return true if the entry holds a non-null frame handle, false otherwise.
         *
         * \lang simp-chinese
         * @brief 检查此条目是否包含有效帧。
         * @return 如果条目持有非空帧句柄则为true，否则为false。
         */
        constexpr explicit operator bool() const noexcept {
            return frame_ != nullptr;
        }

        /**
         * \lang english
         * @brief Resolves and returns a human-readable description of the stack frame.
         *
         *  The frame is resolved through the platform layer and the symbol
         *  name is demangled before being returned.
         *
         * @return The demangled frame description, or an empty string if resolution fails.
         *
         * \lang simp-chinese
         * @brief 解析并返回堆栈帧的人类可读描述。
         *
         *  通过平台层解析该帧，并在返回前对符号名进行反修饰（demangle）。
         *
         * @return 反修饰后的帧描述；如果解析失败则为空字符串。
         */
        core::text::string description() const {
            char buffer[4096];
            if (core::layer::resolve_stack_frame(frame_, buffer, sizeof(buffer))) {
                char demangled[4096];
                core::layer::demangle(buffer, demangled, sizeof(demangled));
                return core::text::string(demangled);
            }
            return core::text::string();
        }

        /**
         * \lang english
         * @brief Extracts and returns the source file name of the stack frame.
         *
         *  The file name is parsed from the resolved frame description; only the
         *  portion after the last path separator is returned.
         *
         * @return The source file name, or an empty string if unavailable.
         *
         * \lang simp-chinese
         * @brief 提取并返回堆栈帧的源文件名。
         *
         *  从已解析的帧描述中解析出文件名；仅返回最后一个路径分隔符之后的部分。
         *
         * @return 源文件名；如果不可用则为空字符串。
         */
        core::text::string source_file() const {
            core::text::string desc = description();
            const char *str = desc.c_str();
            const char *last_slash = nullptr;

            for (const char *p = str; *p; ++p) {
                if (*p == '/' || *p == '\\') {
                    last_slash = p;
                }
            }

            if (last_slash) {
                return core::text::string(last_slash + 1);
            }
            return core::text::string();
        }

        /**
         * \lang english
         * @brief Returns the source line number of the stack frame.
         * @return The line number parsed from the resolved frame description, or 0 if unavailable.
         *
         * \lang simp-chinese
         * @brief 返回堆栈帧的源行号。
         * @return 从已解析帧描述中解析出的行号；如果不可用则为0。
         */
        std::uint_least32_t source_line() const {
            core::text::string desc = description();
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

        /**
         * \lang english
         * @brief Compares two stack frames by their native handles.
         * @param left The first entry.
         * @param right The second entry.
         * @return The strong ordering of the two frames.
         *
         * \lang simp-chinese
         * @brief 按原生句柄比较两个堆栈帧。
         * @param left 第一个条目。
         * @param right 第二个条目。
         * @return 两个帧的强排序结果。
         */
#if RAINY_HAS_CXX20
        friend constexpr std::strong_ordering operator<=>(const stacktrace_entry &left, const stacktrace_entry &right) noexcept {
            return left.frame_ <=> right.frame_;
        }

        /**
         * \lang english
         * @brief Checks whether two stack frames are not equal.
         * @param left The first entry.
         * @param right The second entry.
         * @return true if the frames differ, false otherwise.
         *
         * \lang simp-chinese
         * @brief 检查两个堆栈帧是否不相等。
         * @param left 第一个条目。
         * @param right 第二个条目。
         * @return 如果帧不同则为true，否则为false。
         */
        friend constexpr bool operator!=(const stacktrace_entry &left, const stacktrace_entry &right) noexcept {
            return left.frame_ != right.frame_;
        }

        /**
         * \lang english
         * @brief Checks whether two stack frames are equal.
         * @param left The first entry.
         * @param right The second entry.
         * @return true if the frames are equal, false otherwise.
         *
         * \lang simp-chinese
         * @brief 检查两个堆栈帧是否相等。
         * @param left 第一个条目。
         * @param right 第二个条目。
         * @return 如果帧相等则为true，否则为false。
         */
        friend constexpr bool operator==(const stacktrace_entry &left, const stacktrace_entry &right) noexcept {
            return left.frame_ == right.frame_;
        }
#else
        /**
         * \lang english
         * @brief Checks whether two stack frames are not equal.
         * @param left The first entry.
         * @param right The second entry.
         * @return true if the frames differ, false otherwise.
         *
         * \lang simp-chinese
         * @brief 检查两个堆栈帧是否不相等。
         * @param left 第一个条目。
         * @param right 第二个条目。
         * @return 如果帧不同则为true，否则为false。
         */
        friend constexpr bool operator!=(const stacktrace_entry &left, const stacktrace_entry &right) noexcept {
            return left.frame_ != right.frame_;
        }

        /**
         * \lang english
         * @brief Checks whether two stack frames are equal.
         * @param left The first entry.
         * @param right The second entry.
         * @return true if the frames are equal, false otherwise.
         *
         * \lang simp-chinese
         * @brief 检查两个堆栈帧是否相等。
         * @param left 第一个条目。
         * @param right 第二个条目。
         * @return 如果帧相等则为true，否则为false。
         */
        friend constexpr bool operator==(const stacktrace_entry &left, const stacktrace_entry &right) noexcept {
            return left.frame_ == right.frame_;
        }

        /**
         * \lang english
         * @brief Checks whether the left frame is ordered after the right frame.
         * @param left The first entry.
         * @param right The second entry.
         * @return true if the left frame is greater than the right frame.
         *
         * \lang simp-chinese
         * @brief 检查左帧是否排在右帧之后。
         * @param left 第一个条目。
         * @param right 第二个条目。
         * @return 如果左帧大于右帧则为true。
         */
        friend constexpr bool operator>(const stacktrace_entry &left, const stacktrace_entry &right) noexcept {
            return left.frame_ > right.frame_;
        }

        /**
         * \lang english
         * @brief Checks whether the left frame is ordered after or equal to the right frame.
         * @param left The first entry.
         * @param right The second entry.
         * @return true if the left frame is greater than or equal to the right frame.
         *
         * \lang simp-chinese
         * @brief 检查左帧是否排在右帧之后或与其相等。
         * @param left 第一个条目。
         * @param right 第二个条目。
         * @return 如果左帧大于或等于右帧则为true。
         */
        friend constexpr bool operator>=(const stacktrace_entry &left, const stacktrace_entry &right) noexcept {
            return left.frame_ >= right.frame_;
        }

        /**
         * \lang english
         * @brief Checks whether the left frame is ordered before the right frame.
         * @param left The first entry.
         * @param right The second entry.
         * @return true if the left frame is less than the right frame.
         *
         * \lang simp-chinese
         * @brief 检查左帧是否排在右帧之前。
         * @param left 第一个条目。
         * @param right 第二个条目。
         * @return 如果左帧小于右帧则为true。
         */
        friend constexpr bool operator<(const stacktrace_entry &left, const stacktrace_entry &right) noexcept {
            return left.frame_ < right.frame_;
        }

        /**
         * \lang english
         * @brief Checks whether the left frame is ordered before or equal to the right frame.
         * @param left The first entry.
         * @param right The second entry.
         * @return true if the left frame is less than or equal to the right frame.
         *
         * \lang simp-chinese
         * @brief 检查左帧是否排在右帧之前或与其相等。
         * @param left 第一个条目。
         * @param right 第二个条目。
         * @return 如果左帧小于或等于右帧则为true。
         */
        friend constexpr bool operator<=(const stacktrace_entry &left, const stacktrace_entry &right) noexcept {
            return left.frame_ < right.frame_;
        }
#endif


    private:
        explicit constexpr stacktrace_entry(native_handle_type frame) noexcept : frame_(frame) {
        }

        native_handle_type frame_;

        template <typename Allocator>
        friend class basic_stacktrace;
    };

    /**
     * \lang english
     * @brief A container of stacktrace_entry objects representing a captured stack trace.
     *
     *  Stores the stack frames collected by a call to current(), and provides
     *  container-style access to iterate over and inspect the captured frames.
     *
     * @tparam Allocator The allocator type used to store the frame entries.
     *
     * \lang simp-chinese
     * @brief 表示已捕获堆栈跟踪的stacktrace_entry对象容器。
     *
     *  存储通过调用current()收集的堆栈帧，并提供容器风格的访问方式，
     *  用于迭代和检查已捕获的帧。
     *
     * @tparam Allocator 用于存储帧条目的分配器类型。
     */
    template <typename Allocator>
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

        /**
         * \lang english
         * @brief Captures the entire current stack trace.
         * @param alloc The allocator used to store the captured frames.
         * @return A basic_stacktrace containing the current stack frames.
         *
         * \lang simp-chinese
         * @brief 捕获完整的当前堆栈跟踪。
         * @param alloc 用于存储所捕获帧的分配器。
         * @return 包含当前堆栈帧的basic_stacktrace。
         */
        static basic_stacktrace current(const allocator_type &alloc = allocator_type()) noexcept {
            return current(0, static_cast<size_type>(-1), alloc);
        }

        /**
         * \lang english
         * @brief Captures the current stack trace, skipping the first skip frames.
         * @param skip The number of leading frames to omit from the capture.
         * @param alloc The allocator used to store the captured frames.
         * @return A basic_stacktrace containing the current stack frames.
         *
         * \lang simp-chinese
         * @brief 捕获当前堆栈跟踪，跳过开头的skip个帧。
         * @param skip 捕获时要省略的前导帧数量。
         * @param alloc 用于存储所捕获帧的分配器。
         * @return 包含当前堆栈帧的basic_stacktrace。
         */
        static basic_stacktrace current(size_type skip, const allocator_type &alloc = allocator_type()) noexcept {
            return current(skip + 1, static_cast<size_type>(-1), alloc);
        }

        /**
         * \lang english
         * @brief Captures the current stack trace, skipping the first skip frames and limiting the depth.
         * @param skip The number of leading frames to omit from the capture.
         * @param max_depth The maximum number of frames to capture.
         * @param alloc The allocator used to store the captured frames.
         * @return A basic_stacktrace containing the current stack frames.
         *
         * \lang simp-chinese
         * @brief 捕获当前堆栈跟踪，跳过开头的skip个帧并限制捕获深度。
         * @param skip 捕获时要省略的前导帧数量。
         * @param max_depth 要捕获的最大帧数量。
         * @param alloc 用于存储所捕获帧的分配器。
         * @return 包含当前堆栈帧的basic_stacktrace。
         */
        static basic_stacktrace current(size_type skip, size_type max_depth, const allocator_type &alloc = allocator_type()) noexcept {
            basic_stacktrace result(alloc);
            constexpr std::size_t max_frame_dump = core::layer::max_frames_dump;
            size_type frames_to_collect = (core::min) (max_depth, max_frame_dump);

            if (frames_to_collect > result.max_size() && frames_to_collect <= max_frame_dump) {
                frames_to_collect = result.max_size();
            }

            core::native_frame_ptr_t buffer[max_frame_dump];
            size_type actual_frames = core::layer::collect_stack_frame(buffer, frames_to_collect, skip);

            for (size_type i = 0; i < actual_frames; ++i) {
                result.frames_.push_back(stacktrace_entry(buffer[i]));
            }

            return result;
        }

        /**
         * \lang english
         * @brief Constructs an empty basic_stacktrace.
         *
         * \lang simp-chinese
         * @brief 构造一个空的basic_stacktrace。
         */
        basic_stacktrace() noexcept(type_traits::properties::is_nothrow_default_constructible_v<allocator_type>) : frames_() {
        }

        /**
         * \lang english
         * @brief Constructs an empty basic_stacktrace with the given allocator.
         * @param alloc The allocator used to store the frame entries.
         *
         * \lang simp-chinese
         * @brief 使用给定的分配器构造一个空的basic_stacktrace。
         * @param alloc 用于存储帧条目的分配器。
         */
        explicit basic_stacktrace(const allocator_type &alloc) noexcept : frames_(alloc) {
        }

        /**
         * \lang english
         * @brief Copy constructor.
         * @param right The basic_stacktrace to copy from.
         *
         * \lang simp-chinese
         * @brief 拷贝构造函数。
         * @param right 要复制的basic_stacktrace。
         */
        basic_stacktrace(const basic_stacktrace &right) : frames_(right.frames_) {
        }

        /**
         * \lang english
         * @brief Move constructor.
         * @param right The basic_stacktrace to move from.
         *
         * \lang simp-chinese
         * @brief 移动构造函数。
         * @param right 要移动的basic_stacktrace。
         */
        basic_stacktrace(basic_stacktrace &&right) noexcept : frames_(utility::move(right.frames_)) {
        }

        /**
         * \lang english
         * @brief Copy constructor with a custom allocator.
         * @param right The basic_stacktrace to copy from.
         * @param alloc The allocator used to store the copied frame entries.
         *
         * \lang simp-chinese
         * @brief 使用自定义分配器的拷贝构造函数。
         * @param right 要复制的basic_stacktrace。
         * @param alloc 用于存储所复制帧条目的分配器。
         */
        basic_stacktrace(const basic_stacktrace &right, const allocator_type &alloc) : frames_(right.frames_, alloc) {
        }

        /**
         * \lang english
         * @brief Move constructor with a custom allocator.
         * @param right The basic_stacktrace to move from.
         * @param alloc The allocator used to store the moved frame entries.
         *
         * \lang simp-chinese
         * @brief 使用自定义分配器的移动构造函数。
         * @param right 要移动的basic_stacktrace。
         * @param alloc 用于存储所移动帧条目的分配器。
         */
        basic_stacktrace(basic_stacktrace &&right, const allocator_type &alloc) : frames_(utility::move(right.frames_), alloc) {
        }

        /**
         * \lang english
         * @brief Copy assignment operator.
         * @param right The basic_stacktrace to copy from.
         * @return A reference to this basic_stacktrace.
         *
         * \lang simp-chinese
         * @brief 拷贝赋值运算符。
         * @param right 要复制的basic_stacktrace。
         * @return 对此basic_stacktrace的引用。
         */
        basic_stacktrace &operator=(const basic_stacktrace &right) {
            if (this != &right) {
                frames_ = right.frames_;
            }
            return *this;
        }

        /**
         * \lang english
         * @brief Move assignment operator.
         * @param right The basic_stacktrace to move from.
         * @return A reference to this basic_stacktrace.
         *
         * \lang simp-chinese
         * @brief 移动赋值运算符。
         * @param right 要移动的basic_stacktrace。
         * @return 对此basic_stacktrace的引用。
         */
        basic_stacktrace &operator=(basic_stacktrace &&right) noexcept(
            core::memory::allocator_traits<Allocator>::propagate_on_container_move_assignment::value ||
            core::memory::allocator_traits<Allocator>::is_always_equal::value) {
            frames_ = utility::move(right.frames_);
            return *this;
        }

        /**
         * \lang english
         * @brief Destroys the basic_stacktrace.
         *
         * \lang simp-chinese
         * @brief 销毁basic_stacktrace。
         */
        ~basic_stacktrace() = default;

        /**
         * \lang english
         * @brief Returns the allocator used to store the frame entries.
         * @return A copy of the allocator.
         *
         * \lang simp-chinese
         * @brief 返回用于存储帧条目的分配器。
         * @return 分配器的一份拷贝。
         */
        allocator_type get_allocator() const noexcept {
            return frames_.get_allocator();
        }

        /**
         * \lang english
         * @brief Returns an iterator to the first frame entry.
         * @return A const iterator to the beginning of the frame sequence.
         *
         * \lang simp-chinese
         * @brief 返回指向第一个帧条目的迭代器。
         * @return 指向帧序列开头的const迭代器。
         */
        const_iterator begin() const noexcept {
            return frames_.data();
        }

        /**
         * \lang english
         * @brief Returns an iterator to the element following the last frame entry.
         * @return A const iterator to the end of the frame sequence.
         *
         * \lang simp-chinese
         * @brief 返回指向最后一个帧条目之后元素的迭代器。
         * @return 指向帧序列末尾的const迭代器。
         */
        const_iterator end() const noexcept {
            return frames_.data() + frames_.size();
        }

        /**
         * \lang english
         * @brief Returns a reverse iterator to the last frame entry.
         * @return A const reverse iterator to the reverse beginning.
         *
         * \lang simp-chinese
         * @brief 返回指向最后一个帧条目的反向迭代器。
         * @return 指向反向序列开头的const反向迭代器。
         */
        const_reverse_iterator rbegin() const noexcept {
            return const_reverse_iterator(end());
        }

        /**
         * \lang english
         * @brief Returns a reverse iterator to the element preceding the first frame entry.
         * @return A const reverse iterator to the reverse end.
         *
         * \lang simp-chinese
         * @brief 返回指向第一个帧条目之前元素的反向迭代器。
         * @return 指向反向序列末尾的const反向迭代器。
         */
        const_reverse_iterator rend() const noexcept {
            return const_reverse_iterator(begin());
        }

        /**
         * \lang english
         * @brief Returns a const iterator to the first frame entry.
         * @return A const iterator to the beginning of the frame sequence.
         *
         * \lang simp-chinese
         * @brief 返回指向第一个帧条目的const迭代器。
         * @return 指向帧序列开头的const迭代器。
         */
        const_iterator cbegin() const noexcept {
            return begin();
        }

        /**
         * \lang english
         * @brief Returns a const iterator to the element following the last frame entry.
         * @return A const iterator to the end of the frame sequence.
         *
         * \lang simp-chinese
         * @brief 返回指向最后一个帧条目之后元素的const迭代器。
         * @return 指向帧序列末尾的const迭代器。
         */
        const_iterator cend() const noexcept {
            return end();
        }

        /**
         * \lang english
         * @brief Returns a const reverse iterator to the last frame entry.
         * @return A const reverse iterator to the reverse beginning.
         *
         * \lang simp-chinese
         * @brief 返回指向最后一个帧条目的const反向迭代器。
         * @return 指向反向序列开头的const反向迭代器。
         */
        const_reverse_iterator crbegin() const noexcept {
            return rbegin();
        }

        /**
         * \lang english
         * @brief Returns a const reverse iterator to the element preceding the first frame entry.
         * @return A const reverse iterator to the reverse end.
         *
         * \lang simp-chinese
         * @brief 返回指向第一个帧条目之前元素的const反向迭代器。
         * @return 指向反向序列末尾的const反向迭代器。
         */
        const_reverse_iterator crend() const noexcept {
            return rend();
        }

        /**
         * \lang english
         * @brief Checks whether this stack trace contains no frame entries.
         * @return true if there are no captured frames, false otherwise.
         *
         * \lang simp-chinese
         * @brief 检查此堆栈跟踪是否不含任何帧条目。
         * @return 如果没有捕获到帧则为true，否则为false。
         */
        bool empty() const noexcept {
            return frames_.empty();
        }

        /**
         * \lang english
         * @brief Returns the number of captured frame entries.
         * @return The number of frames in this stack trace.
         *
         * \lang simp-chinese
         * @brief 返回捕获的帧条目数量。
         * @return 此堆栈跟踪中的帧数量。
         */
        size_type size() const noexcept {
            return frames_.size();
        }

        /**
         * \lang english
         * @brief Returns the maximum number of frame entries this stack trace can hold.
         * @return The maximum possible size.
         *
         * \lang simp-chinese
         * @brief 返回此堆栈跟踪可容纳的最大帧条目数量。
         * @return 最大可能的容量。
         */
        size_type max_size() const noexcept {
            return frames_.max_size();
        }

        /**
         * \lang english
         * @brief Accesses the frame entry at the given position without bounds checking.
         * @param pos The index of the frame entry to access.
         * @return A const reference to the frame entry at pos.
         *
         * \lang simp-chinese
         * @brief 访问指定位置的帧条目，不进行边界检查。
         * @param pos 要访问的帧条目的索引。
         * @return 位于pos处的帧条目的const引用。
         */
        const_reference operator[](size_type pos) const {
            return frames_[pos];
        }

        /**
         * \lang english
         * @brief Accesses the frame entry at the given position with bounds checking.
         * @param pos The index of the frame entry to access.
         * @return A const reference to the frame entry at pos.
         * @throws std::out_of_range if pos is out of bounds.
         *
         * \lang simp-chinese
         * @brief 访问指定位置的帧条目，进行边界检查。
         * @param pos 要访问的帧条目的索引。
         * @return 位于pos处的帧条目的const引用。
         * @throws 如果pos越界则抛出std::out_of_range。
         */
        const_reference at(size_type pos) const {
            return frames_.at(pos);
        }

#if RAINY_HAS_CXX20

        /**
         * \lang english
         * @brief Compares two stack traces lexicographically by their frame entries.
         * @tparam Allocator2 The allocator type of the right-hand stack trace.
         * @param left The first stack trace.
         * @param right The second stack trace.
         * @return The strong ordering of the two stack traces.
         *
         * \lang simp-chinese
         * @brief 按帧条目字典序比较两个堆栈跟踪。
         * @tparam Allocator2 右侧堆栈跟踪的分配器类型。
         * @param left 第一个堆栈跟踪。
         * @param right 第二个堆栈跟踪。
         * @return 两个堆栈跟踪的强排序结果。
         */
        template <typename Allocator2>
        friend std::strong_ordering operator<=>(const basic_stacktrace &left, const basic_stacktrace<Allocator2> &right) noexcept {
            size_type min_size = (core::min) (left.size(), right.size());
            for (size_type i = 0; i < min_size; ++i) {
                if (auto cmp = left[i] <=> right[i]; cmp != 0) {
                    return cmp;
                }
            }
            return left.size() <=> right.size();
        }

        /**
         * \lang english
         * @brief Checks whether two stack traces are not equal.
         * @tparam Allocator2 The allocator type of the right-hand stack trace.
         * @param left The first stack trace.
         * @param right The second stack trace.
         * @return true if the stack traces differ, false otherwise.
         *
         * \lang simp-chinese
         * @brief 检查两个堆栈跟踪是否不相等。
         * @tparam Allocator2 右侧堆栈跟踪的分配器类型。
         * @param left 第一个堆栈跟踪。
         * @param right 第二个堆栈跟踪。
         * @return 如果堆栈跟踪不同则为true，否则为false。
         */
        template <typename Allocator2>
        friend bool operator!=(const basic_stacktrace &left, const basic_stacktrace<Allocator2> &right) noexcept {
            return !(left == right);
        }

        /**
         * \lang english
         * @brief Checks whether two stack traces are equal.
         * @tparam Allocator2 The allocator type of the right-hand stack trace.
         * @param left The first stack trace.
         * @param right The second stack trace.
         * @return true if the stack traces are equal, false otherwise.
         *
         * \lang simp-chinese
         * @brief 检查两个堆栈跟踪是否相等。
         * @tparam Allocator2 右侧堆栈跟踪的分配器类型。
         * @param left 第一个堆栈跟踪。
         * @param right 第二个堆栈跟踪。
         * @return 如果堆栈跟踪相等则为true，否则为false。
         */
        template <typename Allocator2>
        friend bool operator==(const basic_stacktrace &left, const basic_stacktrace<Allocator2> &right) noexcept {
            return left <=> right == 0;
        }

#else

        /**
         * \lang english
         * @brief Checks whether two stack traces are equal.
         * @tparam Allocator2 The allocator type of the right-hand stack trace.
         * @param left The first stack trace.
         * @param right The second stack trace.
         * @return true if the stack traces are equal, false otherwise.
         *
         * \lang simp-chinese
         * @brief 检查两个堆栈跟踪是否相等。
         * @tparam Allocator2 右侧堆栈跟踪的分配器类型。
         * @param left 第一个堆栈跟踪。
         * @param right 第二个堆栈跟踪。
         * @return 如果堆栈跟踪相等则为true，否则为false。
         */
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

        /**
         * \lang english
         * @brief Checks whether two stack traces are not equal.
         * @tparam Allocator2 The allocator type of the right-hand stack trace.
         * @param left The first stack trace.
         * @param right The second stack trace.
         * @return true if the stack traces differ, false otherwise.
         *
         * \lang simp-chinese
         * @brief 检查两个堆栈跟踪是否不相等。
         * @tparam Allocator2 右侧堆栈跟踪的分配器类型。
         * @param left 第一个堆栈跟踪。
         * @param right 第二个堆栈跟踪。
         * @return 如果堆栈跟踪不同则为true，否则为false。
         */
        template <typename Allocator2>
        friend bool operator!=(const basic_stacktrace &left, const basic_stacktrace<Allocator2> &right) noexcept {
            return !(left == right);
        }

        /**
         * \lang english
         * @brief Checks whether the left stack trace is lexicographically ordered before the right one.
         * @tparam Allocator2 The allocator type of the right-hand stack trace.
         * @param left The first stack trace.
         * @param right The second stack trace.
         * @return true if the left stack trace is less than the right one.
         *
         * \lang simp-chinese
         * @brief 检查左侧堆栈跟踪是否在字典序上排在右侧之前。
         * @tparam Allocator2 右侧堆栈跟踪的分配器类型。
         * @param left 第一个堆栈跟踪。
         * @param right 第二个堆栈跟踪。
         * @return 如果左侧堆栈跟踪小于右侧则为true。
         */
        template <typename Allocator2>
        friend bool operator<(const basic_stacktrace &left, const basic_stacktrace<Allocator2> &right) noexcept {
            size_type min_size = (core::min)(left.size(), right.size());
            for (size_type i = 0; i < min_size; ++i) {
                if (left[i] < right[i]) return true;
                if (right[i] < left[i]) return false;
            }
            return left.size() < right.size();
        }

        /**
         * \lang english
         * @brief Checks whether the left stack trace is lexicographically ordered before or equal to the right one.
         * @tparam Allocator2 The allocator type of the right-hand stack trace.
         * @param left The first stack trace.
         * @param right The second stack trace.
         * @return true if the left stack trace is less than or equal to the right one.
         *
         * \lang simp-chinese
         * @brief 检查左侧堆栈跟踪是否在字典序上排在右侧之前或与其相等。
         * @tparam Allocator2 右侧堆栈跟踪的分配器类型。
         * @param left 第一个堆栈跟踪。
         * @param right 第二个堆栈跟踪。
         * @return 如果左侧堆栈跟踪小于或等于右侧则为true。
         */
        template <typename Allocator2>
        friend bool operator<=(const basic_stacktrace &left, const basic_stacktrace<Allocator2> &right) noexcept {
            return !(right < left);
        }

        /**
         * \lang english
         * @brief Checks whether the left stack trace is lexicographically ordered after the right one.
         * @tparam Allocator2 The allocator type of the right-hand stack trace.
         * @param left The first stack trace.
         * @param right The second stack trace.
         * @return true if the left stack trace is greater than the right one.
         *
         * \lang simp-chinese
         * @brief 检查左侧堆栈跟踪是否在字典序上排在右侧之后。
         * @tparam Allocator2 右侧堆栈跟踪的分配器类型。
         * @param left 第一个堆栈跟踪。
         * @param right 第二个堆栈跟踪。
         * @return 如果左侧堆栈跟踪大于右侧则为true。
         */
        template <typename Allocator2>
        friend bool operator>(const basic_stacktrace &left, const basic_stacktrace<Allocator2> &right) noexcept {
            return right < left;
        }

        /**
         * \lang english
         * @brief Checks whether the left stack trace is lexicographically ordered after or equal to the right one.
         * @tparam Allocator2 The allocator type of the right-hand stack trace.
         * @param left The first stack trace.
         * @param right The second stack trace.
         * @return true if the left stack trace is greater than or equal to the right one.
         *
         * \lang simp-chinese
         * @brief 检查左侧堆栈跟踪是否在字典序上排在右侧之后或与其相等。
         * @tparam Allocator2 右侧堆栈跟踪的分配器类型。
         * @param left 第一个堆栈跟踪。
         * @param right 第二个堆栈跟踪。
         * @return 如果左侧堆栈跟踪大于或等于右侧则为true。
         */
        template <typename Allocator2>
        friend bool operator>=(const basic_stacktrace &left, const basic_stacktrace<Allocator2> &right) noexcept {
            return !(left < right);
        }

#endif

        /**
         * \lang english
         * @brief Swaps the contents of this stack trace with another.
         * @param right The stack trace to swap contents with.
         *
         * \lang simp-chinese
         * @brief 将此堆栈跟踪的内容与另一个堆栈跟踪交换。
         * @param right 要交换内容的堆栈跟踪。
         */
        void swap(basic_stacktrace &right) noexcept(core::memory::allocator_traits<Allocator>::propagate_on_container_swap::value ||
                                                    core::memory::allocator_traits<Allocator>::is_always_equal::value) {
            frames_.swap(right.frames_);
        }

    private:
        collections::vector<value_type, allocator_type> frames_;
    };

    /**
     * \lang english
     * @brief The default stack trace type using the default allocator.
     *
     * \lang simp-chinese
     * @brief 使用默认分配器的默认堆栈跟踪类型。
     */
    using stacktrace = basic_stacktrace<core::memory::allocator<stacktrace_entry>>;

    /**
     * \lang english
     * @brief Swaps the contents of two stack traces.
     * @tparam Allocator The allocator type of the stack traces.
     * @param left The first stack trace.
     * @param right The second stack trace.
     *
     * \lang simp-chinese
     * @brief 交换两个堆栈跟踪的内容。
     * @tparam Allocator 堆栈跟踪的分配器类型。
     * @param left 第一个堆栈跟踪。
     * @param right 第二个堆栈跟踪。
     */
    template <typename Allocator>
    void swap(basic_stacktrace<Allocator> &left, basic_stacktrace<Allocator> &right) noexcept(noexcept(left.swap(right))) {
        left.swap(right);
    }

    /**
     * \lang english
     * @brief Converts a stack frame entry to its resolved textual description.
     * @param frame The stack frame entry to convert.
     * @return The description of the frame.
     *
     * \lang simp-chinese
     * @brief 将堆栈帧条目转换为其已解析的文本描述。
     * @param frame 要转换的堆栈帧条目。
     * @return 该帧的描述。
     */
    RAINY_INLINE core::text::string to_string(const stacktrace_entry &frame) {
        return frame.description();
    }

    /**
     * \lang english
     * @brief Converts a stack trace to a multi-line textual representation.
     *
     *  Each captured frame is rendered on its own line, in capture order.
     *
     * @tparam Allocator The allocator type of the stack trace.
     * @param stacktrace The stack trace to convert.
     * @return A string with one frame description per line.
     *
     * \lang simp-chinese
     * @brief 将堆栈跟踪转换为多行文本表示。
     *
     *  每个捕获的帧按捕获顺序单独占用一行。
     *
     * @tparam Allocator 堆栈跟踪的分配器类型。
     * @param stacktrace 要转换的堆栈跟踪。
     * @return 每行包含一个帧描述的字符串。
     */
    template <typename Allocator>
    core::text::string to_string(const basic_stacktrace<Allocator> &stacktrace) {
        core::text::string result;
        for (const auto &entry: stacktrace) {
            result += to_string(entry);
            result += "\n";
        }
        return result;
    }

    /**
     * \lang english
     * @brief Writes the resolved description of a stack frame entry to an output stream.
     * @param os The output stream to write to.
     * @param frame The stack frame entry to output.
     * @return The output stream.
     *
     * \lang simp-chinese
     * @brief 将堆栈帧条目的已解析描述写入输出流。
     * @param os 要写入的输出流。
     * @param frame 要输出的堆栈帧条目。
     * @return 输出流。
     */
    RAINY_INLINE std::ostream &operator<<(std::ostream &os, const stacktrace_entry &frame) {
        os << frame.description().c_str();
        return os;
    }

    /**
     * \lang english
     * @brief Writes the multi-line textual representation of a stack trace to an output stream.
     * @tparam Allocator The allocator type of the stack trace.
     * @param os The output stream to write to.
     * @param stacktrace The stack trace to output.
     * @return The output stream.
     *
     * \lang simp-chinese
     * @brief 将堆栈跟踪的多行文本表示写入输出流。
     * @tparam Allocator 堆栈跟踪的分配器类型。
     * @param os 要写入的输出流。
     * @param stacktrace 要输出的堆栈跟踪。
     * @return 输出流。
     */
    template <typename Allocator>
    std::ostream &operator<<(std::ostream &os, const basic_stacktrace<Allocator> &stacktrace) {
        os << to_string(stacktrace).c_str();
        return os;
    }
}

namespace std {
    /**
     * \lang english
     * @brief Specialization of std::hash for stacktrace_entry.
     *
     *  Hashes the underlying native frame handle of the entry.
     *
     * \lang simp-chinese
     * @brief std::hash对stacktrace_entry的特化。
     *
     *  对该条目的底层原生帧句柄进行哈希。
     */
    template <>
    struct hash<rainy::foundation::diagnostics::stacktrace_entry> {
        /**
         * \lang english
         * @brief Computes a hash value for the given stack frame entry.
         * @param entry The stack frame entry to hash.
         * @return The computed hash value.
         *
         * \lang simp-chinese
         * @brief 计算给定堆栈帧条目的哈希值。
         * @param entry 要哈希的堆栈帧条目。
         * @return 计算得到的哈希值。
         */
        size_t operator()(const rainy::foundation::diagnostics::stacktrace_entry &entry) const noexcept {
            return hash<std::uintptr_t>()(reinterpret_cast<std::uintptr_t>(entry.native_handle()));
        }
    };

    /**
     * \lang english
     * @brief Specialization of std::hash for basic_stacktrace.
     *
     *  Combines the hash values of all captured frame entries.
     *
     * \lang simp-chinese
     * @brief std::hash对basic_stacktrace的特化。
     *
     *  组合所有捕获帧条目的哈希值。
     */
    template <typename Allocator>
    struct hash<rainy::foundation::diagnostics::basic_stacktrace<Allocator>> {
        /**
         * \lang english
         * @brief Computes a hash value for the given stack trace.
         * @param stacktrace The stack trace to hash.
         * @return The computed hash value.
         *
         * \lang simp-chinese
         * @brief 计算给定堆栈跟踪的哈希值。
         * @param stacktrace 要哈希的堆栈跟踪。
         * @return 计算得到的哈希值。
         */
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
    /**
     * \lang english
     * @brief Specialization of rainy::utility::hash for stacktrace_entry.
     *
     *  Hashes the underlying native frame handle of the entry.
     *
     * \lang simp-chinese
     * @brief rainy::utility::hash对stacktrace_entry的特化。
     *
     *  对该条目的底层原生帧句柄进行哈希。
     */
    template <>
    struct hash<rainy::foundation::diagnostics::stacktrace_entry> {
        /**
         * \lang english
         * @brief Computes a hash value for the given stack frame entry.
         * @param entry The stack frame entry to hash.
         * @return The computed hash value.
         *
         * \lang simp-chinese
         * @brief 计算给定堆栈帧条目的哈希值。
         * @param entry 要哈希的堆栈帧条目。
         * @return 计算得到的哈希值。
         */
        size_t operator()(const rainy::foundation::diagnostics::stacktrace_entry &entry) const noexcept {
            return hash<std::uintptr_t>()(reinterpret_cast<std::uintptr_t>(entry.native_handle()));
        }
    };

    /**
     * \lang english
     * @brief Specialization of rainy::utility::hash for basic_stacktrace.
     *
     *  Combines the hash values of all captured frame entries.
     *
     * \lang simp-chinese
     * @brief rainy::utility::hash对basic_stacktrace的特化。
     *
     *  组合所有捕获帧条目的哈希值。
     */
    template <typename Allocator>
    struct hash<rainy::foundation::diagnostics::basic_stacktrace<Allocator>> {
        /**
         * \lang english
         * @brief Computes a hash value for the given stack trace.
         * @param stacktrace The stack trace to hash.
         * @return The computed hash value.
         *
         * \lang simp-chinese
         * @brief 计算给定堆栈跟踪的哈希值。
         * @param stacktrace 要哈希的堆栈跟踪。
         * @return 计算得到的哈希值。
         */
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

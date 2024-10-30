#ifndef RAINY_WINAPI_PROC_THRD_MGR_H
#define RAINY_WINAPI_PROC_THRD_MGR_H
#include <rainy/core.hpp>
#if RAINY_USING_WINDOWS
#include <chrono>
#include <filesystem>
#include <rainy/base.hpp>
#include <rainy/meta/type_traits.hpp>
#include <rainy/system/exceptions.hpp>
#include <memory>
#include <stdexcept>
#include <string>
#include <tlhelp32.h>
#include <windows.h>

namespace rainy::winapi::proc_thrd::utils {
    DWORD get_parent_id(const DWORD pid) noexcept {
        DWORD ppid = 0;
        PROCESSENTRY32W process_entry{};
        process_entry.dwSize = sizeof(PROCESSENTRY32W);
        const HANDLE snap_shot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (Process32FirstW(snap_shot, &process_entry)) {
            do {
                if (process_entry.th32ProcessID == pid) {
                    ppid = process_entry.th32ParentProcessID;
                    break;
                }
            } while (Process32NextW(snap_shot, &process_entry));
        }
        CloseHandle(snap_shot);
        return ppid;
    }

    DWORD get_main_thread_id(const DWORD pid) {
        THREADENTRY32 te32{};
        te32.dwSize = sizeof(THREADENTRY32);
        HANDLE thread_snap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
        if (thread_snap == INVALID_HANDLE_VALUE) {
            return 0;
        }
        if (Thread32First(thread_snap, &te32)) {
            do {
                if (te32.th32OwnerProcessID == pid) {
                    CloseHandle(thread_snap);
                    return te32.th32ThreadID;
                }
            } while (Thread32Next(thread_snap, &te32));
        }
        CloseHandle(thread_snap);
        return 0;
    }

    HANDLE get_main_thread_handle(const DWORD thread_id) {
        return OpenThread(THREAD_ALL_ACCESS, FALSE, thread_id);
    }
}

namespace rainy::winapi::proc_thrd {
    namespace utils {
        template <typename CharType = char>
        utility::result_collection<DWORD, std::basic_string<CharType>> current_directory() noexcept {
            static_assert(type_traits::is_support_charset_v<CharType>, "Only support wchat_t and char types");
            using namespace foundation::type_traits;
            namespace fs = std::filesystem;
            DWORD require_size{0};
            if constexpr (helper::is_wchar_t<CharType>) {
                require_size = GetCurrentDirectoryW(0, nullptr);
            } else {
                require_size = GetCurrentDirectoryA(0, nullptr);
            }
            std::basic_string<CharType> buffer(require_size, helper::char_null_v<CharType>);
            DWORD result;
            if constexpr (helper::is_wchar_t<CharType>) {
                result = GetCurrentDirectoryW(require_size, &buffer[0]);
            } else {
                result = GetCurrentDirectoryA(require_size, &buffer[0]);
            }
            if (result == 0) {
                return {GetLastError(), {}};
            }
            return {0, buffer};
        }

        template <typename CharType = char>
        utility::result_collection<DWORD, std::basic_string<CharType>> parent_directory() noexcept {
            using string_type = std::basic_string<CharType>;

            const DWORD pid = utils::get_parent_id(GetCurrentProcessId());
            if (pid == 0) {
                return {ERROR_NOT_FOUND, {}};
            }
            void* const parent_process = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);

            string_type buffer(MAX_PATH, foundation::type_traits::helper::char_null_v<CharType>);
            DWORD buffer_size{MAX_PATH};
            BOOL result{FALSE};
            if constexpr (foundation::type_traits::helper::is_wchar_t<CharType>) {
                result = QueryFullProcessImageNameW(parent_process, 0, &buffer[0], &buffer_size);
            } else {
                result = QueryFullProcessImageNameA(parent_process, 0, &buffer[0], &buffer_size);
            }
            if (!result) {
                return {GetLastError(), {}};
            }
            return {0, buffer};
        }

        template <typename CharType = char>
        utility::result_collection<DWORD, std::basic_string<CharType>> system_directory() noexcept {
            static_assert(type_traits::is_support_charset_v<CharType>, "Only support wchat_t and char types");
            using namespace foundation::type_traits;
            namespace fs = std::filesystem;
            static std::basic_string<CharType> buffer(MAX_PATH, helper::char_null_v<CharType>);
            if (!buffer[0] == helper::char_null_v<CharType>) {
                /* 因为我们之前已经获取到字符串，所以，此处直接进行返回 */
                return {0, buffer};
            }
            UINT result;
            if constexpr (helper::is_wchar_t<CharType>) {
                result = GetSystemDirectoryW(&buffer[0], MAX_PATH);
            } else {
                result = GetSystemDirectoryA(&buffer[0], MAX_PATH);
            }

            if (!result) {
                return {GetLastError(), {}};
            }
            return {0, buffer};
        }

        template <typename CharType = char>
        utility::result_collection<DWORD, std::basic_string<CharType>> windows_directory() noexcept {
            static_assert(type_traits::is_support_charset_v<CharType>, "Only support wchat_t and char types");
            using namespace foundation::type_traits;
            namespace fs = std::filesystem;
            static std::basic_string<CharType> buffer(MAX_PATH, helper::char_null_v<CharType>);
            if (!buffer[0] == helper::char_null_v<CharType>) {
                /* 因为我们之前已经获取到字符串，所以，此处直接进行返回 */
                return {0, buffer};
            }
            UINT result;
            if constexpr (helper::is_wchar_t<CharType>) {
                result = GetWindowsDirectoryW(&buffer[0], MAX_PATH);
            } else {
                result = GetWindowsDirectoryA(&buffer[0], MAX_PATH);
            }
            if (!result) {
                return {GetLastError(), {}};
            }
            return {0, buffer};
        }
    }

    enum class process_results {
        failed_create,
        not_running
    };

    class process_manager {
    public:
        using native_handle_type = HANDLE; // Windows原生句柄
        // 明确转移所有权语义
        struct make_process_ownership_to_pmgr {
            explicit make_process_ownership_to_pmgr() = default;
        };

        /**
         * @brief 正常拷贝函数.
         */
        process_manager() : info_({}) {
        }

        /**
         * @brief 根据传入的字符串，自动选择对应的模板构造。然后创建一个进程
         * @tparam CharType 由传入字符串类型来推导
         * @param target 目标路径（可执行文件路径，带文件名加后缀，如C:/Windows/System32/notepad.exe)，新创建的进程将打开可执行文件
         * @param command_line 可选，此处是传递给target的命令行参数
         */
        template <typename CharType>
        explicit process_manager(const std::basic_string<CharType> &target, const std::basic_string<CharType> &command_line = {})
            : info_({}) {
            create(target, command_line);
        }

        /**
         * @brief 将进程信息移动到当前对象
         * @param right 要移动的进程信息
         */
        process_manager(make_process_ownership_to_pmgr, PROCESS_INFORMATION &right) : info_({}), running_(true) {
            std::lock_guard<std::mutex> lock(mtx_);
            if (right.hProcess && right.hThread) {
                // 有效的句柄，将所有权转移给当前对象
                info_ = std::exchange(right, {});
            }
        }

        /**
         * @brief 将进程信息拷贝到当前对象
         * @param right 要拷贝的进程信息
         */
        explicit process_manager(const PROCESS_INFORMATION &right) : info_(right), running_(true) {
            // 这里没有make_process_ownership_to_pmgr的占位符，指示只是拷贝而已
        }

        process_manager(const process_manager &) = delete;
        process_manager &operator=(const process_manager &) = delete;

        /**
         * @brief 将右值引用对象移动到当前对象
         * @param right 要移动的右值引用对象
         */
        process_manager(process_manager &&right) noexcept : info_({}), running_(false) {
            *this = utility::move(right);
        }

        ~process_manager() {
            terminate_process();
        }

        /**
         * @brief 根据传入的字符串，自动选择对应的模板构造。然后创建一个进程
         * @tparam CharType 由传入字符串类型来推导
         * @param target 目标路径（可执行文件路径，带文件名加后缀，如C:/Windows/System32/notepad.exe)，新创建的进程将打开可执行文件
         * @param command_line 可选，此处是传递给target的命令行参数
         * @attention 在调用此函数成功创建进程之后，你必须调用terminate_process或者detach方法才能再次调用此方法
         */
        template <typename CharType>
        BOOL create(const std::basic_string_view<CharType> &target, const std::basic_string_view<CharType> &command_line = {}) {
            static_assert(type_traits::is_support_charset_v<CharType>, "Only support wchat_t and char types");
            using namespace foundation::type_traits;
            namespace fs = std::filesystem;
            using startupinfo_type = other_transformations::conditional_t<helper::is_wchar_t<CharType>, STARTUPINFOW, STARTUPINFOA>;
            using string_type = std::basic_string<CharType>;
            using execute_info = other_transformations::conditional_t<helper::is_wchar_t<CharType>, SHELLEXECUTEINFOW, SHELLEXECUTEINFOA>;

            std::lock_guard<std::mutex> lock(mtx_);
            if (running_) {
                // 这个实例仅持有一个进程当它运行的时候。除非这个实例的进程完成，否则不允许为该实例创建第二个进程。
                throw std::runtime_error("This instance holds only one process while it is running. A second process is not allowed to be "
                                         "created for this instance until the process for that instance has completed.");
            }
            string_type procced_command_line{command_line.data(), command_line.size()};
            /*
            用于为CreateProcess提供一个引用，参考此部分:
            https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-createprocessw

            The Unicode version of this function, CreateProcessW, can modify the contents of this string. Therefore, this parameter cannot
            be a pointer to read-only memory (such as a const variable or a literal string). If this parameter is a constant string, the
            function may cause an access violation.
            */
            if (procced_command_line[0] != ' ') {
                // 由于CreateProcess中传递命令行参数，字符串0索引必须是空格才能成功传递，所以此处需要修改
                procced_command_line.insert(procced_command_line.begin(), foundation::type_traits::helper::char_space_v<CharType>);
            }
            startupinfo_type startup_info{};
            std::memset(&startup_info, 0, sizeof(startupinfo_type));
            startup_info.cb = sizeof(startupinfo_type);
            BOOL result{FALSE};
            if constexpr (helper::is_wchar_t<CharType>) {
                result = ::CreateProcessW(target.data(), &procced_command_line[0], nullptr, nullptr, FALSE, CREATE_NEW_CONSOLE, nullptr, nullptr,
                                          &startup_info, &info_);
            } else {
                result = ::CreateProcessA(target.data(), &procced_command_line[0], nullptr, nullptr, FALSE, CREATE_NEW_CONSOLE, nullptr, nullptr,
                                          &startup_info, &info_);
            }
            if (const DWORD last_error = GetLastError();!result) {
                if (last_error == 2) /* 可能要执行的是一个命令，让我们用ShellExecute函数修正一下 */ {
                    execute_info info{};
                    info.cbSize = sizeof(execute_info);
                    info.fMask = SEE_MASK_NOCLOSEPROCESS;
                    info.lpFile = target.data();
                    info.lpParameters = &procced_command_line[0];
                    info.nShow = SW_SHOW;
                    if constexpr (helper::is_wchar_t<CharType>) {
                        result = ShellExecuteExW(&info);
                    } else {
                        result = ShellExecuteExA(&info);
                    }
                    if (result) {
                        /*
                        参考 https://learn.microsoft.com/en-us/windows/win32/api/shellapi/ns-shellapi-shellexecuteinfoa

                        A handle to the newly started application. This member is set on return and is always NULL unless fMask is set to
                        SEE_MASK_NOCLOSEPROCESS. Even if fMask is set to SEE_MASK_NOCLOSEPROCESS, hProcess will be NULL if no process was
                        launched. For example, if a document to be launched is a URL and an instance of Internet Explorer is already
                        running, it will display the document. No new process is launched, and hProcess will be NULL.
                        */
                        if (info_.hProcess = info.hProcess; info_.hProcess) {
                            info_.dwProcessId = GetProcessId(info_.hProcess);
                            info_.dwThreadId = utils::get_main_thread_id(info_.dwProcessId);
                            info_.hThread = utils::get_main_thread_handle(info_.dwThreadId);
                            running_ = true;
                        } else {
                            /* 由于我们不能通过ShellExecute创建出来的进程获取资源。因此，我们必须回退所有资源，保证原子性 */
                            info_ = {};
                            running_ = false;
                        }
                        return TRUE;
                    }
                    return result;
                }
            }
            return running_ = true, result;
        }

        /**
         * @brief 根据传入的字符串，自动选择对应的模板构造。然后创建一个进程
         * @tparam RawStringType 字符类型，由模板自动推导
         * @param target 目标路径（可执行文件路径，带文件名加后缀，如C:/Windows/System32/notepad.exe)，新创建的进程将打开可执行文件
         * @param command_line
         * @return
         */
        template <typename RawStringType>
        BOOL create(const RawStringType *target, const RawStringType *command_line = nullptr) {
            return create(std::basic_string_view<RawStringType>{target},
                          std::basic_string_view<RawStringType>{command_line == nullptr ? "" : command_line});
        }

        template <typename CharType>
        BOOL create(const std::basic_string<CharType> &target, const std::basic_string<CharType> &command_line) {
            return create(std::basic_string_view<CharType>{target}, std::basic_string_view<CharType>{command_line});
        }

        /**
         * @brief 终止当前对象持有的进程
         * @attention 在析构函数里，此函数将被自动调用
         * @remark 在终止后你可以再次调用create创建另一个线程
         * @return 成功的时候返回true，否则是false
         */
        bool terminate_process() {
            std::lock_guard<std::mutex> lock(mtx_);
            if (running_) {
                if (::TerminateProcess(info_.hProcess, 0)) {
                    ::CloseHandle(info_.hProcess);
                    ::CloseHandle(info_.hThread);
                    running_ = false;
                    return true;
                }
            }
            return false;
        }

        /**
         * @brief 获取退出代码
         * @return 退出代码
         */
        DWORD exit_code() const noexcept {
            DWORD exit_code{0};
            return ::GetExitCodeProcess(info_.hProcess, &exit_code) ? exit_code : -1;
        }

        /**
         * @brief 获取进程ID
         * @return 进程ID
         */
        DWORD process_id() const noexcept {
            return info_.dwProcessId;
        }

        /**
         * @brief 获取线程ID
         * @return 线程ID
         */
        DWORD thread_id() const noexcept {
            return info_.dwThreadId;
        }

        /**
         * @brief 检查当前对象是否在运行一个进程
         */
        bool running() const {
            return running_;
        }

        /**
         * @brief 检查当前对象是否在运行一个进程
         */
        explicit operator bool() const noexcept {
            return running_;
        }

        /**
         * @param right 要移动的对象
         * @return 当前对象引用
         */
        process_manager &operator=(process_manager &&right) noexcept {
            std::lock_guard<std::mutex> lock(mtx_);
            if (this != utility::addressof(right)) {
                info_.hProcess = std::exchange(right.info_.hProcess, nullptr);
                info_.hThread = std::exchange(right.info_.hThread, nullptr);
                info_.dwProcessId = std::exchange(right.info_.dwProcessId, 0);
                info_.dwThreadId = std::exchange(right.info_.dwThreadId, 0);
            }
            return *this;
        }

        /**
         * @brief 将当前对象持有句柄分离
         * @attention 可能导致和僵尸线程一样的下场（不推荐）
         */
        void detach() noexcept {
            std::lock_guard lock(mtx_);
            running_ = false;
            info_.hProcess = info_.hThread = nullptr;
            info_.dwProcessId = info_.dwThreadId = 0;
        }

        /**
         * @brief 获取当前对象进程句柄
         * @return 返回当前对象进程句柄
         */
        RAINY_NODISCARD native_handle_type native_process_handle() const noexcept {
            return info_.hProcess;
        }

        /**
         * @brief 获取当前对象线程句柄
         * @return 返回当前对象线程句柄
         */
        RAINY_NODISCARD native_handle_type native_thread_handle() const noexcept {
            return info_.hThread;
        }

        void swap(process_manager &right) noexcept {
            std::swap(info_, right.info_);
            std::swap(running_, right.running_);
        }

    private:
        PROCESS_INFORMATION info_{};
        bool running_{};
        mutable std::mutex mtx_;
    };
}

namespace rainy::winapi::proc_thrd {
    class thread_manager {
    public:
        using native_handle_type = HANDLE;

        thread_manager() : thread_({}) {
        }

        template <typename Fx, typename... Args,foundation::type_traits::other_transformations::enable_if_t<
                                   !foundation::type_traits::type_relations::is_same_v<
                                       foundation::type_traits::cv_modify::remove_cvref_t<Fx>, thread_manager> &&
                                       foundation::type_traits::type_properties::is_invocable_r_v<void, Fx, Args...>,
                                   int> = 0>
        explicit thread_manager(Fx&& handler,Args... args) : thread_({}) {
            create_thread(foundation::utility::forward<Fx>(handler),foundation::utility::forward<Args>(args)...);
        }

        thread_manager(thread_manager &&other) noexcept : thread_(std::exchange(other.thread_, {})) {
        }

        thread_manager(const thread_manager &) = delete;
        thread_manager &operator=(const thread_manager &) = delete;

        ~thread_manager() {
            if (joinable()) {
                join();
            }
        }

        RAINY_NODISCARD bool joinable() const noexcept {
            return thread_.id != 0;
        }

        thread_manager &operator=(thread_manager &&right) noexcept {
            if (joinable()) {
                std::terminate();
            }
            thread_ = std::exchange(right.thread_, {});
            return *this;
        }

        void join() {
            using foundation::system::exceptions::throw_exception;
            using foundation::system::exceptions::runtime::runtime_error;
            if (!joinable()) {
                throw_exception(runtime_error("INVALID_ARGUMENT"));
            }
            if (thread_.id == GetCurrentThreadId()) {
                throw_exception(runtime_error("RESOURCE_DEADLOCK_WOULD_OCCUR"));
            }
            if (WaitForSingleObject(thread_.handle, INFINITE) == WAIT_FAILED) {
                throw_exception(runtime_error("NO_SUCH_PROCESS"));
            }
            ::CloseHandle(thread_.handle);
            thread_ = {};
        }

        void detach() {
            using foundation::system::exceptions::throw_exception;
            using foundation::system::exceptions::runtime::runtime_error;
            if (!joinable()) {
                throw_exception(runtime_error("INVALID_ARGUMENT"));
            }
            // 由于我们没有与detach等效的WindowsAPI，所以我们直接置空
            thread_ = {};
        }

        RAINY_NODISCARD DWORD get_id() const noexcept {
            return thread_.id;
        }

        template <typename Fx, typename... Args,foundation::type_traits::other_transformations::enable_if_t<
                                   !foundation::type_traits::type_relations::is_same_v<
                                       foundation::type_traits::cv_modify::remove_cvref_t<Fx>, thread_manager> &&
                                       foundation::type_traits::type_properties::is_invocable_r_v<void, Fx, Args...>,
                                   int> = 0>
        void start(Fx&& handler,Args &&...args) {
            create_thread(foundation::utility::forward<Fx>(handler),foundation::utility::forward<Args>(args)...);
        }

        void swap(thread_manager &right) noexcept {
            std::swap(thread_.handle, right.thread_.handle);
            std::swap(thread_.id, right.thread_.id);
        }

        RAINY_NODISCARD native_handle_type native_handle() const noexcept {
            return thread_.handle;
        }

        static unsigned int hardware_concurrency() noexcept {
            SYSTEM_INFO sysinfo;
            GetSystemInfo(&sysinfo);
            return sysinfo.dwNumberOfProcessors;
        }

    private:
        template <typename Tuple, std::size_t... Indices>
        static DWORD WINAPI invoke(void *raw_vals) noexcept {
            const std::unique_ptr<Tuple> fnvals(static_cast<Tuple *>(raw_vals));
            Tuple &tup = *fnvals.get();
            foundation::utility::invoke(foundation::utility::move(std::get<Indices>(tup)...));
            return 0;
        }

        template <typename Tuple, size_t... Indices>
        RAINY_NODISCARD static constexpr auto get_invoke(std::index_sequence<Indices...>) noexcept {
            return &invoke<Tuple, Indices...>;
        }

        template <typename Fx, typename... Args>
        void create_thread(Fx &&handler, Args&&... args) {
            using tuple = std::tuple<std::decay_t<Fx>, std::decay_t<Args>...>;
            auto decay_copied = std::make_unique<tuple>(std::forward<Fx>(handler), std::forward<Args...>(args)...);
            constexpr auto invoker_proc = get_invoke<tuple>(std::make_index_sequence<1 + sizeof...(Args)>{});
            thread_.handle = ::CreateThread(nullptr, 0, invoker_proc, decay_copied.get(), 0, &thread_.id);
            if (thread_.handle) {
                (void) decay_copied.release();
            } else {
                using namespace foundation::system::exceptions;
                thread_.id = 0;
                throw_exception(runtime::runtime_error("RESOURCE_UNAVAILABLE_TRY_AGAIN"));
            }
        }

        struct thread_t {
            native_handle_type handle;
            DWORD id;
        } thread_;
    };
}

namespace rainy::foundation::system::this_thread {
    namespace utils {
        template <typename Rep, typename Period>
        RAINY_NODISCARD auto to_absolute_time(const std::chrono::duration<Rep, Period> &rel_time) noexcept {
            constexpr auto zero = std::chrono::duration<Rep, Period>::zero();
            const auto now = std::chrono::steady_clock::now();
            decltype(now + rel_time) abs_time = now;
            if (rel_time > zero) {
                constexpr auto forever = (std::chrono::steady_clock::time_point::max)();
                if (abs_time < forever - rel_time) {
                    abs_time += rel_time;
                } else {
                    abs_time = forever;
                }
            }
            return abs_time;
        }
    }

    template <typename Clock, typename Duration>
    void sleep_until(const std::chrono::time_point<Clock, Duration> &abs_time) {
#if RAINY_HAS_CXX20
        static_assert(std::chrono::is_clock_v<Clock>, "Clock type required");
#endif // RAINY_HAS_CXX20
        for (;;) {
            const auto now = Clock::now();
            if (abs_time <= now) {
                return;
            }
            constexpr std::chrono::milliseconds clamp{std::chrono::hours{24}};
            const auto rel = abs_time - now;
            if (rel >= clamp) {
                Sleep(clamp.count());
            } else {
                const auto rel_ms = std::chrono::ceil<std::chrono::milliseconds>(rel);
                Sleep(static_cast<unsigned long>(rel_ms.count()));
            }
        }
    }

    template <typename Rep, typename Period>
    void sleep_for(const std::chrono::duration<Rep, Period> &rel_time) {
        sleep_until(utils::to_absolute_time(rel_time));
    }

    RAINY_NODISCARD inline HANDLE get_current_thread() noexcept {
        return GetCurrentThread();
    }
}

#endif
#endif
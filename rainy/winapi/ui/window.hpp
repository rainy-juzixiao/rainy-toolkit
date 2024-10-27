#ifndef RAINY_WINAPI_WINDOW_H
#define RAINY_WINAPI_WINDOW_H
#include <rainy/winapi/api_core.h>

/* 对window.hpp使用的WindowsAPI进行重新的包装，并进行模板化 */
namespace rainy::winapi::ui::utils {
    BOOL set_window_position(HWND handle, HWND handle_insert_after, int x, int y, int cx, int cy, UINT flags) {
        return SetWindowPos(handle, handle_insert_after, x, y, cx, cy, flags);
    }

    RAINY_DECLARE_CHARSET_TEMPLATE BOOL get_message(LPMSG msg_ptr, HWND handle, UINT window_msg_filter_min, UINT window_msg_filter_max) {
        if constexpr (foundation::type_traits::helper::is_wchar_t<CharType>) {
            return GetMessageW(msg_ptr, handle, window_msg_filter_min, window_msg_filter_max);
        } else {
            return GetMessageA(msg_ptr, handle, window_msg_filter_min, window_msg_filter_max);
        }
    }

    RAINY_DECLARE_CHARSET_TEMPLATE LRESULT dispatch_message(const MSG *msg_ptr) {
        if constexpr (foundation::type_traits::helper::is_wchar_t<CharType>) {
            return DispatchMessageW(msg_ptr);
        } else {
            return DispatchMessageA(msg_ptr);
        }
    }

    BOOL translate_message(const MSG *msg_ptr) {
        return TranslateMessage(msg_ptr);
    }

    template <typename CharType = char,
              typename WndCPtrType = foundation::type_traits::other_transformations::conditional_t<
                  foundation::type_traits::helper::is_wchar_t<CharType>, LPWNDCLASSEXW, LPWNDCLASSA>,
              rainy::foundation::type_traits::other_transformations::enable_if_t<
                                            rainy::winapi::type_traits::is_support_charset_v<CharType>, int> = 0>
    BOOL get_class_info_ex(HINSTANCE handle_of_instance, const CharType *class_name, WndCPtrType window_class_ptr) {
        if constexpr (foundation::type_traits::helper::is_wchar_t<CharType>) {
            return GetClassInfoExW(handle_of_instance, class_name, window_class_ptr);
        } else {
            return GetClassInfoExA(handle_of_instance, class_name, window_class_ptr);
        }
    }

    template <typename CharType = char, typename WndCPtrType = 
        foundation::type_traits::other_transformations::conditional_t < foundation::type_traits::helper::is_wchar_t<CharType>,WNDCLASSEXW,WNDCLASSEXA>,
              foundation::type_traits::other_transformations::enable_if_t<
                 winapi::type_traits::is_support_charset_v<CharType>, int> = 0>
    ATOM register_window_class(const WndCPtrType *window_class) {
        if constexpr (foundation::type_traits::helper::is_wchar_t<CharType>) {
            return RegisterClassExW(window_class);
        } else {
            return RegisterClassExA(window_class);
        }
    }

    RAINY_DECLARE_CHARSET_TEMPLATE LRESULT invoke_window_callback(WNDPROC callback, HWND handle, UINT msg, WPARAM wparam, LPARAM lparam) {
        if constexpr (foundation::type_traits::helper::is_wchar_t<CharType>) {
            return CallWindowProcW(callback, handle, msg, wparam, lparam);
        } else {
            return CallWindowProcA(callback, handle, msg, wparam, lparam);
        }
    }

    LRESULT send_message(HWND handle, UINT u_msg, WPARAM w_param = 0, LPARAM l_param = 0L) {
        utility::expects(::IsWindow(handle), "Can't Accept A Invalid Handle!");
        return ::SendMessageW(handle, u_msg, w_param, l_param);
    }

    BOOL post_message(HWND handle, UINT u_msg, WPARAM w_param = 0, LPARAM l_param = 0L) {
        utility::expects(::IsWindow(handle), "Can't Accept A Invalid Handle!");
        return ::PostMessageW(handle, u_msg, w_param, l_param);
    }

    RAINY_DECLARE_CHARSET_TEMPLATE LRESULT call_default_window_callback(HWND handle, UINT msg, WPARAM wparam, LPARAM lparam) {
        if constexpr (foundation::type_traits::helper::is_wchar_t<CharType>) {
            return DefWindowProcW(handle, msg, wparam, lparam);
        } else {
            return DefWindowProcA(handle, msg, wparam, lparam);
        }
    }
}

namespace rainy::winapi::ui {
    template <typename CharType = char>
    class window {
    public:
        static_assert(winapi::type_traits::is_support_charset_v<CharType>);

        using handle_type = HWND;

        window() noexcept = default;

        RAINY_NODISCARD handle_type get_handle_type() const {
            return handle;
        }

        operator handle_type() const {
            return get_handle_type();
        }

        int message_loop() {
            MSG msg = {nullptr};
            while (utils::get_message<CharType>(&msg, NULL, 0, 0)) {
                utils::translate_message(&msg);
                utils::dispatch_message(&msg);
            }
            return static_cast<int>(msg.wParam);
        }

        bool register_window_class() {
            WNDCLASSEXA wc = {0};
            wc.cbSize = sizeof(WNDCLASSEXA);
            wc.style = get_class_style();
            wc.cbClsExtra = 0;
            wc.cbWndExtra = 0;
            wc.hIcon = nullptr;
            wc.lpfnWndProc = window::wnd_proc;
            wc.hInstance = dynamic_libray::get_module_handle<CharType>(nullptr);
            wc.hCursor = ::LoadCursorA(nullptr, IDC_ARROW);
            wc.hbrBackground = nullptr;
            wc.lpszMenuName = nullptr;
            wc.lpszClassName = get_window_class_name().data();
            ATOM ret = utils::register_window_class<CharType>(&wc);
#if RAINY_ENABLE_DEBUG
            utility::expects(ret != NULL || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS, "Can't Register Class!");
#endif
            return ret != 0 || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
        }

        bool register_superclass() {
            WNDCLASSEXA wc = {0};
            wc.cbSize = sizeof(WNDCLASSEXA);
            if (!utils::get_class_info_ex<CharType>(NULL, get_super_class_name().data(), &wc)) {
                if (!utils::get_class_info_ex<CharType>(dynamic_libray::get_module_handle<CharType>(nullptr), get_super_class_name().data(), &wc)) {
                    foundation::system::exceptions::runtime::throw_runtime_error("Unable to locate window class");
                }
            }
            old_wnd_proc_ = wc.lpfnWndProc;
            wc.lpfnWndProc = window::control_proc;
            wc.hInstance = dynamic_libray::get_module_handle<CharType>(nullptr);
            wc.lpszClassName = get_window_class_name().data();
            ATOM ret = utils::register_window_class<CharType>(&wc);
            utility::expects(ret != 0 || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS, "Can't Register Class!");
            return ret != 0 || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
        }

        handle_type create(handle_type handle_parent, std::string_view pstr_name, DWORD dw_style, DWORD dw_ex_style, const RECT rc,
                           HMENU hmenu = nullptr) {
            return create(handle_parent, pstr_name, dw_style, dw_ex_style, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hmenu);
        }

        handle_type create(handle_type handle_parent, std::string_view pstr_name, DWORD dw_style, DWORD dw_ex_style, int x = CW_USEDEFAULT,
                           int y = CW_USEDEFAULT, int cx = CW_USEDEFAULT, int cy = CW_USEDEFAULT, HMENU hmenu = nullptr) {
            if (!get_super_class_name().empty() && !register_superclass()) {
                return nullptr;
            }
            if (get_super_class_name().empty() && !register_window_class()) {
                return nullptr;
            }
            handle = ::CreateWindowExA(dw_ex_style, get_window_class_name().data(), pstr_name.data(), dw_style, x, y, cx, cy, handle_parent,
                                       hmenu, dynamic_libray::get_module_handle<CharType>(NULL), this);
            if (!handle) {
                DWORD error = GetLastError();
                foundation::system::output::stderr_println("CreateWindowExW failed with error code: ", error);
            }
            assert(handle);
            return handle;
        }

        handle_type subclass(handle_type sub_handle) {
            utility::expects(::IsWindow(sub_handle), "Can't Accept A Invalid Handle!");
            if (sub_handle) {
                old_wnd_proc_ = reinterpret_cast<WNDPROC>(SetWindowLongPtrA(sub_handle, GWLP_WNDPROC, reinterpret_cast<LPARAM>(wnd_proc)));
                if (!old_wnd_proc_) {
                    return nullptr;
                }
                subclassed_ = true;
                this->handle = sub_handle;
                ::SetWindowLongPtrA(this->sub_handle, GWLP_USERDATA, reinterpret_cast<LPARAM>(this));
            } else {
                assert(!this->handle);
            }
            return this->handle;
        }

        void unsubclass() {
            if (!check_handle()) {
                return;
            }
            if (!subclassed_) {
                return;
            }
            SetWindowLongPtrA((handle), GWLP_WNDPROC, reinterpret_cast<LPARAM>(wnd_proc));
            old_wnd_proc_ = ::DefWindowProcW;
            subclassed_ = false;
        }

        void show_window(bool show = true, bool take_focus = true) {
            utility::expects(::IsWindow(handle), "Can't Accept A Invalid Handle!");
            if (!check_handle()) {
                return;
            }
            ::ShowWindow(handle, show ? (take_focus ? SW_SHOWNORMAL : SW_SHOWNOACTIVATE) : SW_HIDE);
        }

        UINT show_modal() {
            utility::expects(::IsWindow(handle), "Can't Accept A Invalid Handle!");
            UINT ret = 0;
            handle_type wnd_parent = GetWindow(handle, GW_OWNER);
            ::ShowWindow(handle, SW_SHOWNORMAL);
            ::EnableWindow(wnd_parent, FALSE);
            MSG msg{};
            while (::IsWindow(handle) && utils::get_message<CharType>(&msg, NULL, 0, 0)) {
                if (msg.message == WM_CLOSE && msg.hwnd == handle) {
                    ret = static_cast<UINT>(msg.wParam);
                    ::EnableWindow(wnd_parent, TRUE);
                    ::SetFocus(wnd_parent);
                }
                ::TranslateMessage(&msg);
                ::DispatchMessageA(&msg);
                if (msg.message == WM_QUIT) {
                    break;
                }
            }
            ::EnableWindow(wnd_parent, TRUE);
            ::SetFocus(wnd_parent);
            if (msg.message == WM_QUIT) {
                ::PostQuitMessage(static_cast<int>(msg.wParam));
            }
            return ret;
        }

        void close(UINT ret = IDOK) {
            if (!check_handle()) {
                return;
            }
            utils::post_message(handle, WM_CLOSE, (WPARAM) ret, 0L);
        }

        void center_window() {
            if (!check_handle()) {
                return;
            }
            utility::expects(static_cast<DWORD>(GetWindowLongW(handle, GWL_STYLE) & WS_CHILD) == 0);
            RECT rc_dlg = {0};
            ::GetWindowRect(handle, &rc_dlg);
            RECT rc_area = {0};
            RECT rc_center = {0};
            handle_type hwnd_parent = ::GetParent(handle);
            handle_type hwnd_center = ::GetWindow(handle, 4);
            if (hwnd_center != nullptr){
                handle = hwnd_center;
            }
            MONITORINFO monitor_info = {};
            monitor_info.cbSize = sizeof(monitor_info);
            ::GetMonitorInfoW(::MonitorFromWindow(handle, MONITOR_DEFAULTTONEAREST), &monitor_info);
            rc_area = monitor_info.rcWork;
            if (hwnd_center == nullptr || ::IsIconic(hwnd_center)) {
                rc_center = rc_area;
            } else {
                ::GetWindowRect(hwnd_center, &rc_center);
            }
            int dlg_width = rc_dlg.right - rc_dlg.left;
            int dlg_height = rc_dlg.bottom - rc_dlg.top;
            int x_left = (rc_center.left + rc_center.right) / 2 - dlg_width / 2;
            int y_top = (rc_center.top + rc_center.bottom) / 2 - dlg_height / 2;
            if (x_left < rc_area.left) {
                x_left = rc_area.left;
            } else if (x_left + dlg_width > rc_area.right) {
                x_left = rc_area.right - dlg_width;
            }
            if (y_top < rc_area.top) {
                y_top = rc_area.top;
            } else if (y_top + dlg_height > rc_area.bottom) {
                y_top = rc_area.bottom - dlg_height;
            }
            utils::set_window_position(handle, nullptr, x_left, y_top, -1, -1, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
        }

        void set_icon(UINT res_id) {
            rainy_let hicon = static_cast<HICON>(LoadImageA(GetModuleHandleA(nullptr), MAKEINTRESOURCEA(res_id), IMAGE_ICON,
                                                        (::GetSystemMetrics(SM_CXICON) + 15) & ~15,
                                                        (::GetSystemMetrics(SM_CYICON) + 15) & ~15, LR_DEFAULTCOLOR));
            utility::expects(hicon, "Can't Load Image!");
            utils::send_message(handle, WM_SETICON, (WPARAM) TRUE, (LPARAM) hicon);
            hicon = static_cast<HICON>(::LoadImageA(GetModuleHandleA(nullptr), MAKEINTRESOURCEA(res_id), IMAGE_ICON,
                                                    (::GetSystemMetrics(SM_CXSMICON) + 15) & ~15,
                                                    (::GetSystemMetrics(SM_CYSMICON) + 15) & ~15, LR_DEFAULTCOLOR));
            utility::expects(hicon, "Can't Load Image!");
            utils::send_message(handle, WM_SETICON, (WPARAM) FALSE, (LPARAM) hicon);
        }

        static LRESULT wnd_proc(handle_type handle_type, UINT u_msg, WPARAM w_param, LPARAM l_param) {
            rainy_let *self = reinterpret_cast<window *>(::GetWindowLongPtrA(handle_type, GWLP_USERDATA));
            if (self) {
                return self->handle_message(u_msg, w_param, l_param);
            }
            return utils::call_default_window_callback(handle_type, u_msg, w_param, l_param);
        }

        virtual LRESULT handle_message(UINT msg, WPARAM wparam, LPARAM lparam) {
            return utils::call_default_window_callback(handle, msg, wparam, lparam);
        }

        static LRESULT control_proc(handle_type handle_type, UINT msg, WPARAM wparam, LPARAM lparam) {
            rainy_let self = reinterpret_cast<window *>(::GetWindowLongPtrA(handle_type, GWLP_USERDATA));
            if (msg == WM_NCCREATE) {
                rainy_let lpcs = reinterpret_cast<LPCREATESTRUCT>(lparam);
                self = static_cast<window *>(lpcs->lpCreateParams);
                ::SetWindowLongPtrA(handle_type, GWLP_USERDATA, reinterpret_cast<LPARAM>(self));
                self->handle = handle_type;
            }
            if (self) {
                return self->handle_control_message(msg, wparam, lparam);
            }
            return ::DefWindowProcW(handle_type, msg, wparam, lparam);
        }

        LRESULT handle_control_message(UINT msg, WPARAM wparam, LPARAM lparam) {
            return utils::invoke_window_callback<CharType>(old_wnd_proc_, handle, msg, wparam, lparam);
        }

        RAINY_NODISCARD virtual DWORD get_class_style() const {
            return CS_VREDRAW | CS_HREDRAW;
        }

        RAINY_NODISCARD virtual std::string_view get_window_class_name() const {
            return "default_window_classname";
        }

        RAINY_NODISCARD virtual std::string_view get_super_class_name() const {
            return {};
        }

    protected:
        handle_type handle{nullptr};
        WNDPROC old_wnd_proc_{nullptr};
        bool subclassed_{false};

    private:
        RAINY_NODISCARD bool check_handle() const {
#if RAINY_ENABLE_DEBUG
            utility::expects(::IsWindow(handle), "Can't Accept A Invalid Handle!");
            return ::IsWindow(handle);
#else
            return ::IsWindow(handle);
#endif
        }
    };
}

#endif

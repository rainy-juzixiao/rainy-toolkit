#ifndef RAINY_WINAPI_NOTIFACTIONS_HPP
#define RAINY_WINAPI_NOTIFACTIONS_HPP
#include <rainy/winapi/api_core.h>
#include <vector>
#include <map>
#include <memory>
#include <ShObjIdl.h>
#include <strsafe.h>
#include <Psapi.h>
#include <ShlObj.h>
#include <roapi.h>
#include <propvarutil.h>
#include <functiondiscoverykeys.h>
#include <iostream>
#include <optional>
#include <winrt/windows.ui.notifications.h>
#include <winrt/Windows.Data.Xml.Dom.h>
#include <winrt/windows.foundation.collections.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Storage.FileProperties.h>

namespace rainy::winapi::ui::notifcations::internals {
    using abi_toast_dismissal_reason = winrt::Windows::UI::Notifications::ToastDismissalReason;
    using abi_template_type = winrt::Windows::UI::Notifications::ToastTemplateType;
}

namespace rainy::winapi::ui::notifcations {
    class notifcation_handler {
        enum class dismissal_reason {
            user_canceled = internals::abi_toast_dismissal_reason::UserCanceled,
            application_hidden = internals::abi_toast_dismissal_reason::ApplicationHidden,
            timed_out = internals::abi_toast_dismissal_reason::TimedOut
        };

        virtual ~notifcation_handler() = default;
        virtual void activated() const = 0;
        virtual void activated(int action_idx) const = 0;
        virtual void activated(const char *response) const = 0;
        virtual void dismissed(dismissal_reason state) const = 0;
        virtual void failed() const = 0;
    };
}

namespace rainy::winapi::ui::notifcations::form {
    enum class scenario {
        normal,
        alarm,
        incoming_call,
        reminder
    };

    enum class duration {
        system,
        short_duration,
        long_duration
    };

    enum class audio_option {
        default_option,
        slient,
        loop
    };

    enum class textfield {
        first_line,
        second_line,
        third_line
    };

    enum class crop_hint {
        square,
        circle
    };

     enum class audio_system_file {
        default_sound,
        im,
        mail,
        reminder,
        sms,
        alarm,
        alarm_2,
        alarm_3,
        alarm_4,
        alarm_5,
        alarm_6,
        alarm_7,
        alarm_8,
        alarm_9,
        alarm_10,
        call,
        call_1,
        call_2,
        call_3,
        call_4,
        call_5,
        call_6,
        call_7,
        call_8,
        call_9,
        call_10,
    };
}

namespace rainy::winapi::ui::notifcations {
    enum class notifaction_template_type {
        image_and_text01 = internals::abi_template_type::ToastImageAndText01,
        image_and_text02 = internals::abi_template_type::ToastImageAndText02,
        image_and_text03 = internals::abi_template_type::ToastImageAndText03,
        image_and_text04 = internals::abi_template_type::ToastImageAndText04,
        text01 = internals::abi_template_type::ToastText01,
        text02 = internals::abi_template_type::ToastText02,
        text03 = internals::abi_template_type::ToastText03,
        text04 = internals::abi_template_type::ToastText04
    };
}

namespace rainy::winapi::ui::notifcations {
    class notifaction_template {
    public:
        void set_first_line(std::wstring_view text) {
            text_fields_[form::textfield::first_line] = text;
        }

        void set_second_line(std::wstring_view text) {
            text_fields_[form::textfield::second_line] = text;
        }

        void set_third_line(std::wstring_view text) {
            text_fields_[form::textfield::third_line] = text;
        }

        void set_text_field(std::wstring_view txt, form::textfield pos) {
            text_fields_[pos] = text;
        }

        void set_attribution_text(std::wstring_view attribution_text) {
            
        }

        void set_image_path(std::wstring_view img_path, form::crop_hint crop_hint = form::crop_hint::square) {
        
        }

        void set_hero_image_path(std::wstring_view img_path, bool inline_image = false) {
        
        }

        void set_audio_path(form::audio_system_file audio) {
        
        }

        void set_audio_path(std::wstring_view audio_path) {
        
        }
        
        void set_audio_option(form::audio_option audio_option) {
        
        }

        void set_duration(form::duration duration) {
        
        }

        void set_expiration(std::int64_t milliseconds_from_now) {
        
        }

        void set_scenario(form::scenario scenario) {
            scenario_ = scenario_;
        }

        void add_action(std::wstring_view label) {
        
        }

        void add_input() {
        
        }

        /**
         * @brief 获取通知字段中的文本行数
         * @return 通知字段中的文本行数
         */
        RAINY_NODISCARD std::size_t text_fields_count() const;

        /**
         * @brief 获取通知中的操作按钮数
         * @return 通知中的操作按钮数
         */
        RAINY_NODISCARD std::size_t actions_count() const;

        /**
         * @brief 检索通知模板是否包含图片
         * @return 指示通知模板是否包含图片
         */
        RAINY_NODISCARD bool has_image() const;

        /**
         * @brief 检索通知模板是否包含Hero Image
         * @return 指示通知模板是否包含Hero Image
         */
        RAINY_NODISCARD bool has_hero_image() const;

        /**
         * @brief 获取通知模板的文本字段的引用
         * @return 通知模板的文本字段的引用
         */
        RAINY_NODISCARD const std::vector<std::wstring> &text_fields() const;

        /**
         * @brief 获取通知模板的操作按钮的标签的引用，按位置索引
         * @param pos 指定行数的位置
         * @return 对应行数的操作按钮的标签的引用
         */
        RAINY_NODISCARD const std::wstring &text_field(const form::textfield pos) const;

        /**
         * @brief 获取通知模板中的操作按钮的标签的引用，按位置索引
         * @param pos 指定的位置索引
         * @return 对应的位置索引的操作按钮的标签的引用
         */
        RAINY_NODISCARD const std::wstring &action_label(const std::size_t pos) const {
            return actions_.at(pos);
        }

        /**
         * @brief 获取通知模板的图像路径
         * @return 返回图像的路径
         */
        RAINY_NODISCARD const std::wstring &image_path() const {
            return image_path_;
        }

        /**
         * @brief 获取Hero Image的路径
         * @return 返回Hero Image的路径
         */
        RAINY_NODISCARD const std::wstring &hero_image_path() const {
            return hero_image_path_;
        }

        /**
         * @brief 获取通知模板的音频路径
         * @return 返回音频的路径
         */
        RAINY_NODISCARD const std::wstring &audio_path() const {
            return audio_path_;
        }

        /**
         * @brief 获取通知模板的归属信息
         * @return 返回通知模板的归属信息
         */
        RAINY_NODISCARD const std::wstring &attribution_text() const;

        /**
         * @brief 获取通知模板的场景
         * @return 获取通知模板的场景
         */
        RAINY_NODISCARD const std::wstring &scenario() const {
            return scenario_;
        }

        /**
         * @brief 获取通知中不再被视为当前或有效且不应显示的时间
         * @return 以std::int64_t的形式返回，单位为毫秒
         */
        RAINY_NODISCARD std::int64_t expiration() const {
            return expiration_;
        }

        /**
         * @brief 获取通知模板使用的类型
         * @return 返回一个枚举值，表示通知模板的类型
         */
        RAINY_NODISCARD notifcations::notifaction_template_type template_type() const {
            return template_type_;
        }

        /**
         * @brief 检索音频选项
         * @return 返回一个枚举值，表示音频选项
         */
        RAINY_NODISCARD form::audio_option audio_option() const {
            return audio_option_;
        }

        /**
         * @brief 获取通知显示的时间
         * @return 返回一个枚举值进行表示
         */
        RAINY_NODISCARD form::duration duration() const;

        RAINY_NODISCARD bool is_toast_generic() const;
        RAINY_NODISCARD bool is_inline_hero_image() const;
        RAINY_NODISCARD bool is_crop_hint_circle() const;

    private:
        bool has_input;
        bool inline_hero_image{false};
        std::int64_t expiration_{0};
        std::vector<std::wstring> text_fields_{};
        std::vector<std::wstring> actions_{};
        std::wstring image_path_{};
        std::wstring hero_image_path_{};
        std::wstring audio_path_{};
        std::wstring attribution_text_{};
        std::wstring scenario_{L"Default"};
        form::audio_option audio_option_{form::audio_option::default_option};
        notifaction_template_type template_type_{notifaction_template_type::text01};
        form::duration duration_{form::duration::system};
        form::crop_hint crop_hint_{form::crop_hint::square};
    };
}

namespace rainy::winapi::ui::notifcations {
    class notifaction {
    public:


    private:
        RAINY_DECLARE_SIGNLE_INSTANCE(notifaction);

        
    };
}

#endif

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
         * @brief ��ȡ֪ͨ�ֶ��е��ı�����
         * @return ֪ͨ�ֶ��е��ı�����
         */
        RAINY_NODISCARD std::size_t text_fields_count() const;

        /**
         * @brief ��ȡ֪ͨ�еĲ�����ť��
         * @return ֪ͨ�еĲ�����ť��
         */
        RAINY_NODISCARD std::size_t actions_count() const;

        /**
         * @brief ����֪ͨģ���Ƿ����ͼƬ
         * @return ָʾ֪ͨģ���Ƿ����ͼƬ
         */
        RAINY_NODISCARD bool has_image() const;

        /**
         * @brief ����֪ͨģ���Ƿ����Hero Image
         * @return ָʾ֪ͨģ���Ƿ����Hero Image
         */
        RAINY_NODISCARD bool has_hero_image() const;

        /**
         * @brief ��ȡ֪ͨģ����ı��ֶε�����
         * @return ֪ͨģ����ı��ֶε�����
         */
        RAINY_NODISCARD const std::vector<std::wstring> &text_fields() const;

        /**
         * @brief ��ȡ֪ͨģ��Ĳ�����ť�ı�ǩ�����ã���λ������
         * @param pos ָ��������λ��
         * @return ��Ӧ�����Ĳ�����ť�ı�ǩ������
         */
        RAINY_NODISCARD const std::wstring &text_field(const form::textfield pos) const;

        /**
         * @brief ��ȡ֪ͨģ���еĲ�����ť�ı�ǩ�����ã���λ������
         * @param pos ָ����λ������
         * @return ��Ӧ��λ�������Ĳ�����ť�ı�ǩ������
         */
        RAINY_NODISCARD const std::wstring &action_label(const std::size_t pos) const {
            return actions_.at(pos);
        }

        /**
         * @brief ��ȡ֪ͨģ���ͼ��·��
         * @return ����ͼ���·��
         */
        RAINY_NODISCARD const std::wstring &image_path() const {
            return image_path_;
        }

        /**
         * @brief ��ȡHero Image��·��
         * @return ����Hero Image��·��
         */
        RAINY_NODISCARD const std::wstring &hero_image_path() const {
            return hero_image_path_;
        }

        /**
         * @brief ��ȡ֪ͨģ�����Ƶ·��
         * @return ������Ƶ��·��
         */
        RAINY_NODISCARD const std::wstring &audio_path() const {
            return audio_path_;
        }

        /**
         * @brief ��ȡ֪ͨģ��Ĺ�����Ϣ
         * @return ����֪ͨģ��Ĺ�����Ϣ
         */
        RAINY_NODISCARD const std::wstring &attribution_text() const;

        /**
         * @brief ��ȡ֪ͨģ��ĳ���
         * @return ��ȡ֪ͨģ��ĳ���
         */
        RAINY_NODISCARD const std::wstring &scenario() const {
            return scenario_;
        }

        /**
         * @brief ��ȡ֪ͨ�в��ٱ���Ϊ��ǰ����Ч�Ҳ�Ӧ��ʾ��ʱ��
         * @return ��std::int64_t����ʽ���أ���λΪ����
         */
        RAINY_NODISCARD std::int64_t expiration() const {
            return expiration_;
        }

        /**
         * @brief ��ȡ֪ͨģ��ʹ�õ�����
         * @return ����һ��ö��ֵ����ʾ֪ͨģ�������
         */
        RAINY_NODISCARD notifcations::notifaction_template_type template_type() const {
            return template_type_;
        }

        /**
         * @brief ������Ƶѡ��
         * @return ����һ��ö��ֵ����ʾ��Ƶѡ��
         */
        RAINY_NODISCARD form::audio_option audio_option() const {
            return audio_option_;
        }

        /**
         * @brief ��ȡ֪ͨ��ʾ��ʱ��
         * @return ����һ��ö��ֵ���б�ʾ
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

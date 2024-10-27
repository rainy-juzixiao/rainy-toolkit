#ifndef RAINY_EXCEPTION
#define RAINY_EXCEPTION
#include <rainy/base.hpp>

namespace rainy::foundation::system::exceptions {
    namespace runtime /* 运行时异常 */ {
        class overflow_error final : public runtime_error {
        public:
            using base = runtime_error;

            explicit overflow_error(const char *message, const source &location = source::current()) :
                base(message,location)  {
            }

            explicit overflow_error(const std::string &message, const source &location = source::current()) :
                base(message,location)  {
            }
        };

        class underflow_error final : public runtime_error {
        public:
            using base = runtime_error;

            explicit underflow_error(const char *message, const source &location = source::current()) :
                base(message,location)  {
            }

            explicit underflow_error(const std::string &message, const source &location = source::current()) :
                base(message,location)  {
            }
        };

        class range_error final : public runtime_error {
        public:
            using base = runtime_error;

            explicit range_error(const char *message, const source &location = source::current()) :
                base(message,location)  {
            }

            explicit range_error(const std::string &message, const source &location = source::current()) :
                base(message,location)  {
            }
        };
    }

    namespace logic /* 逻辑错误 */ {
        class invalid_argument final : public logic_error {
        public:
            using base = logic_error;

            explicit invalid_argument(const char *message, const source &location = source::current()) :
                base(message,location)  {
            }

            explicit invalid_argument(const std::string &message, const source &location = source::current()) :
                base(message,location)  {
            }
        };

        class length_error final : public logic_error {
        public:
            using base = logic_error;

            explicit length_error(const char *message, const source &location = source::current()) :
                base(message,location)  {
            }

            explicit length_error(const std::string &message, const source &location = source::current()) :
                base(message,location)  {
            }
        };

        class domain_error final : public logic_error {
        public:
            using base = logic_error;

            explicit domain_error(const char *message, const source &location = source::current()) :
                base(message,location)  {
            }

            explicit domain_error(const std::string &message, const source &location = source::current()) :
                base(message,location)  {
            }
        };
    }
}

#endif
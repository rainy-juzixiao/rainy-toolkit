#ifndef RAINY_FOUNDATION_PAL_ASIO_TIMER_QUEUE_SET
#define RAINY_FOUNDATION_PAL_ASIO_TIMER_QUEUE_SET
#include <rainy/core/core.hpp>
#include <rainy/foundation/pal/asio/implements/op/queue.hpp>

namespace rainy::foundation::pal::asio::timer {
    class queue_set;
}

namespace rainy::foundation::pal::asio::timer::implements {
    class timer_queue_base : private type_traits::helper::non_copyable {
    public:
        friend class timer::queue_set;

        timer_queue_base() : next_(nullptr) {
        }
        virtual ~timer_queue_base() {
        }
        virtual bool empty() const = 0;
        virtual long wait_duration_msec(long max_duration) const = 0;
        virtual long wait_duration_usec(long max_duration) const = 0;
        virtual void get_ready_timers(asio::implements::op_queue<asio::implements::operation> &ops) = 0;
        virtual void get_all_timers(asio::implements::op_queue<asio::implements::operation> &ops) = 0;

    private:
        timer_queue_base *next_;
    };
}

namespace rainy::foundation::pal::asio::timer {
    class queue_set {
    public:
        queue_set() : first_{nullptr} {
        }

        void insert(implements::timer_queue_base *q) {
            q->next_ = first_;
            first_ = q;
        }

        void erase(implements::timer_queue_base *q) {
            if (first_) {
                if (q == first_) {
                    first_ = q->next_;
                    q->next_ = 0;
                    return;
                }

                for (implements::timer_queue_base *p = first_; p->next_; p = p->next_) {
                    if (p->next_ == q) {
                        p->next_ = q->next_;
                        q->next_ = 0;
                        return;
                    }
                }
            }
        }

        bool all_empty() const {
            for (implements::timer_queue_base *p = first_; p; p = p->next_) {
                if (!p->empty()) {
                    return false;
                }
            }
            return true;
        }

        long wait_duration_msec(long max_duration) const {
            long min_duration = max_duration;
            for (implements::timer_queue_base *p = first_; p; p = p->next_) {
                min_duration = p->wait_duration_msec(min_duration);
            }
            return min_duration;
        }

        long wait_duration_usec(long max_duration) const {
            long min_duration = max_duration;
            for (implements::timer_queue_base *p = first_; p; p = p->next_) {
                min_duration = p->wait_duration_usec(min_duration);
            }
            return min_duration;
        }

        void get_ready_timers(asio::implements::op_queue<asio::implements::operation> &ops) {
            for (implements::timer_queue_base *p = first_; p; p = p->next_) {
                p->get_ready_timers(ops);
            }
        }

        void get_all_timers(asio::implements::op_queue<asio::implements::operation> &ops) {
            for (implements::timer_queue_base *p = first_; p; p = p->next_) {
                p->get_all_timers(ops);
            }
        }

    private:
        implements::timer_queue_base *first_;
    };
}

#endif
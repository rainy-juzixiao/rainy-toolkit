/*
* Copyright 2026 rainy-juzixiao
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef RAINY_FOUNDATION_IO_NET_IMPLEMENTS_QUEUE_HPP
#define RAINY_FOUNDATION_IO_NET_IMPLEMENTS_QUEUE_HPP
#include <rainy/core/core.hpp>

namespace rainy::foundation::io::implements {
    template <typename Operation>
    class op_queue;

    class op_queue_access {
    public:
        template <typename Operation>
        static Operation *next(Operation *o) {
            return static_cast<Operation *>(o->next_);
        }

        template <typename Operation1, typename Operation2>
        static void next(Operation1 *&o1, Operation2 *o2) {
            o1->next_ = o2;
        }

        template <typename Operation>
        static void destroy(Operation *o) {
            o->destroy();
        }

        template <typename Operation>
        static Operation *&front(op_queue<Operation> &q) {
            return q.front_;
        }

        template <typename Operation>
        static Operation *&back(op_queue<Operation> &q) {
            return q.back_;
        }
    };

    template <typename Operation>
    class op_queue : private type_traits::helper::non_copyable {
    public:
        op_queue() : front_(nullptr), back_(nullptr) {
        }

        ~op_queue() {
            Operation *op = front_;
            while (op) {
                pop();
                op_queue_access::destroy(op);
            }
        }

        Operation *front() {
            return front_;
        }

        void pop() {
            if (front_) {
                Operation *tmp = front_;
                front_ = op_queue_access::next(front_);
                if (!front_) {
                    back_ = nullptr;
                }
                op_queue_access::next(tmp, static_cast<Operation *>(nullptr));
            }
        }

        void push(Operation *h) {
            op_queue_access::next(h, static_cast<Operation *>(0));
            if (back_) {
                op_queue_access::next(back_, h);
                back_ = h;
            } else {
                front_ = back_ = h;
            }
        }

        template <typename OtherOperation>
        void push(op_queue<OtherOperation> &q) {
            if (Operation *other_front = op_queue_access::front(q)) {
                if (back_)
                    op_queue_access::next(back_, other_front);
                else
                    front_ = other_front;
                back_ = op_queue_access::back(q);
                op_queue_access::front(q) = 0;
                op_queue_access::back(q) = 0;
            }
        }

        bool empty() const {
            return front_ == 0;
        }

        bool is_enqueued(Operation *o) const {
            return op_queue_access::next(o) != nullptr || back_ == nullptr;
        }

    private:
        friend class op_queue_access;

        Operation *front_;
        Operation *back_;
    };
}

#endif

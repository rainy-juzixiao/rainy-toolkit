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
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <rainy/core/annotations/smf_control.hpp>
#include <string>
#include <type_traits>

using namespace rainy::annotations::smf_control;

struct dummy_base {
    dummy_base() = default;
    dummy_base(const dummy_base &) = default;
    dummy_base(dummy_base &&) = default;
    dummy_base &operator=(const dummy_base &) = default;
    dummy_base &operator=(dummy_base &&) = default;
};

struct construct_impl : constructible_base<construct_impl> {
    int value = 0;

    construct_impl() = default;
    construct_impl(int v) : value(v) {
    }

    template <typename Ty, typename = std::enable_if_t<std::is_arithmetic_v<Ty>>>
    void construct_impl_(Ty &&other) {
        value = static_cast<int>(other);
    }

    void construct_impl_(const construct_impl &other) {
        value = other.value;
    }

    void construct_impl_(construct_impl &&other) {
        value = other.value;
    }
};

struct assign_impl : assignable_base<assign_impl> {
    int value = 0;

    assign_impl() = default;
    assign_impl(int v) : value(v) {
    }

    template <typename Ty, typename = std::enable_if_t<std::is_arithmetic_v<Ty>>>
    void assign_impl_(Ty &&other) {
        value = static_cast<int>(other);
    }

    void assign_impl_(const assign_impl &other) {
        value = other.value;
    }

    void assign_impl_(assign_impl &&other) {
        value = other.value;
    }
};

struct both_impl : constructible_base<both_impl>, assignable_base<both_impl> {
    int value = 0;

    both_impl() = default;
    both_impl(int v) : value(v) {
    }

    template <typename Ty, typename = std::enable_if_t<std::is_arithmetic_v<Ty>>>
    void construct_impl_(Ty &&other) {
        value = static_cast<int>(other);
    }

    template <typename Ty, typename = std::enable_if_t<std::is_arithmetic_v<Ty>>>
    void assign_impl_(Ty &&other) {
        value = static_cast<int>(other);
    }

    void construct_impl_(const both_impl &other) {
        value = other.value;
    }

    void construct_impl_(both_impl &&other) {
        value = other.value;
    }

    void assign_impl_(const both_impl &other) {
        value = other.value;
    }

    void assign_impl_(both_impl &&other) {
        value = other.value;
    }
};

TEST_CASE("constructible_base forwards to construct_impl_", "[smf_control]") {
    construct_impl forty_two = 42;
    construct_impl pi = 3.14;
    construct_impl obj;
    rainy::core::builtin::construct_at(rainy::core::builtin::addressof(obj), forty_two);
    REQUIRE(obj.value == 42);

    rainy::core::builtin::construct_at(rainy::core::builtin::addressof(obj), pi);
    REQUIRE(obj.value == 3);
}

TEST_CASE("assignable_base forwards to assign_impl_", "[smf_control]") {
    assign_impl forty_two = 42;
    assign_impl pi = 3.14;
    assign_impl obj;
    obj = forty_two;
    REQUIRE(obj.value == 42);

    obj = pi;
    REQUIRE(obj.value == 3);
}

TEST_CASE("non_trivial_copy copies correctly", "[smf_control]") {
    struct test_base : both_impl {
        test_base() = default;
        test_base(int v) : both_impl(v) {
        }
    };

    using wrapped = non_trivial_copy<test_base>;

    wrapped original(42);
    wrapped copy(original);
    REQUIRE(copy.value == 42);
}

struct tracking_base1 : constructible_base<tracking_base1> {
    int value = 0;
    int construct_calls = 0;

    template <typename Ty, typename = std::enable_if_t<std::is_arithmetic_v<Ty>>>
    void construct_impl_(Ty &&other) {
        value = static_cast<int>(other);
        ++construct_calls;
    }

    void construct_impl_(const tracking_base1 &other) {
        value = other.value;
        ++construct_calls;
    }

    void construct_impl_(tracking_base1 &&other) {
        value = other.value;
        ++construct_calls;
    }
};

TEST_CASE("non_trivial_copy uses construct_from", "[smf_control]") {
    using wrapped = non_trivial_copy<tracking_base1>;

    wrapped original;
    original.value = 100;

    wrapped copy(original);
    REQUIRE(copy.value == 100);
    REQUIRE(copy.construct_calls == 1);
}

TEST_CASE("deleted_copy has deleted copy constructor", "[smf_control]") {
    struct test_base : both_impl {
        test_base() = default;
        test_base(int v) : both_impl(v) {
        }
    };

    using wrapped = deleted_copy<test_base>;

    STATIC_REQUIRE_FALSE(std::is_copy_constructible_v<wrapped>);
    STATIC_REQUIRE(std::is_move_constructible_v<wrapped>);
    STATIC_REQUIRE(std::is_copy_assignable_v<wrapped>);
    STATIC_REQUIRE(std::is_move_assignable_v<wrapped>);
}

TEST_CASE("copy_control selects trivially copyable types", "[smf_control]") {
    struct test_base : both_impl {
        test_base() = default;
        test_base(int v) : both_impl(v) {
        }
    };

    using trivial_types = copy_control<test_base, int, float>;
    STATIC_REQUIRE(std::is_same_v<trivial_types, test_base>);
}

TEST_CASE("copy_control selects non_trivial_copy for non-trivial types", "[smf_control]") {
    struct test_base : both_impl {
        test_base() = default;
        test_base(int v) : both_impl(v) {
        }
    };

    using non_trivial_types = copy_control<test_base, std::string>;
    STATIC_REQUIRE(std::is_same_v<non_trivial_types, non_trivial_copy<test_base>>);
}

TEST_CASE("copy_control selects deleted_copy for non-copyable types", "[smf_control]") {
    struct test_base : both_impl {
        test_base() = default;
        test_base(int v) : both_impl(v) {
        }
    };

    struct non_copyable {
        non_copyable() = default;
        non_copyable(const non_copyable &) = delete;
        non_copyable &operator=(const non_copyable &) = delete;
    };

    using deleted_types = copy_control<test_base, non_copyable>;
    STATIC_REQUIRE(std::is_same_v<deleted_types, deleted_copy<test_base>>);
}

TEST_CASE("non_trivial_move moves correctly", "[smf_control]") {
    struct test_base : both_impl {
        test_base() = default;
        test_base(int v) : both_impl(v) {
        }
    };

    using wrapped = non_trivial_move<test_base, int>;

    wrapped original(42);
    wrapped moved(std::move(original));
    REQUIRE(moved.value == 42);
}

struct tracking_base2 : constructible_base<tracking_base2> {
    int value = 0;
    int construct_calls = 0;

    template <typename Ty, typename = std::enable_if_t<std::is_arithmetic_v<Ty>>>
    void construct_impl_(Ty &&other) {
        value = static_cast<int>(other);
        ++construct_calls;
    }

    void construct_impl_(const tracking_base2 &other) {
        value = other.value;
        ++construct_calls;
    }

    void construct_impl_(tracking_base2 &&other) {
        value = other.value;
        ++construct_calls;
    }
};

TEST_CASE("non_trivial_move uses construct_from for move", "[smf_control]") {
    using wrapped = non_trivial_move<tracking_base2, int>;

    wrapped original;
    original.value = 100;

    wrapped moved(std::move(original));
    REQUIRE(moved.value == 100);
    REQUIRE(moved.construct_calls == 1);
}

struct test_base : both_impl {
    test_base() = default;
    test_base(int v) : both_impl(v) {
    }
};

TEST_CASE("deleted_move has deleted move constructor", "[smf_control]") {
    using wrapped = deleted_move<test_base, std::shared_ptr<int>>;

    STATIC_REQUIRE(std::is_copy_constructible_v<wrapped>);
    STATIC_REQUIRE_FALSE(std::is_move_constructible_v<wrapped>);
    STATIC_REQUIRE(std::is_copy_assignable_v<wrapped>);
    STATIC_REQUIRE(std::is_move_assignable_v<wrapped>);
}

TEST_CASE("move_control selects trivially movable types", "[smf_control]") {
    struct test_base : both_impl {
        test_base() = default;
        test_base(int v) : both_impl(v) {
        }
    };

    using trivial_types = move_control<test_base, int, float>;
    STATIC_REQUIRE(std::is_same_v<trivial_types, copy_control<test_base, int, float>>);
}

TEST_CASE("move_control selects non_trivial_move for movable types", "[smf_control]") {
    struct test_base : both_impl {
        test_base() = default;
        test_base(int v) : both_impl(v) {
        }
    };

    using movable_types = move_control<test_base, std::string>;
    STATIC_REQUIRE(std::is_same_v<movable_types, non_trivial_move<test_base, std::string>>);
}

TEST_CASE("move_control selects deleted_move for non-movable types", "[smf_control]") {
    struct test_base : both_impl {
        test_base() = default;
        test_base(int v) : both_impl(v) {
        }
    };

    struct non_movable {
        non_movable() = default;
        non_movable(non_movable &&) = delete;
        non_movable &operator=(non_movable &&) = delete;
    };

    using deleted_types = move_control<test_base, non_movable>;
    STATIC_REQUIRE(std::is_same_v<deleted_types, deleted_move<test_base, non_movable>>);
}

TEST_CASE("non_trivial_copy_assign assigns correctly", "[smf_control]") {
    struct test_base : both_impl {
        test_base() = default;
        test_base(int v) : both_impl(v) {
        }
    };

    using wrapped = non_trivial_copy_assign<test_base, std::string>;

    wrapped original(42);
    wrapped copy;
    copy = original;
    REQUIRE(copy.value == 42);
}

struct tracking_base3 : assignable_base<tracking_base3> {
    int value = 0;
    int assign_calls = 0;

    template <typename Ty, typename = std::enable_if_t<std::is_arithmetic_v<Ty>>>
    void assign_impl_(Ty &&other) {
        value = static_cast<int>(other);
        ++assign_calls;
    }

    void assign_impl_(const tracking_base3 &other) {
        value = other.value;
        ++assign_calls;
    }

    void assign_impl_(tracking_base3 &&other) {
        value = other.value;
        ++assign_calls;
    }
};

TEST_CASE("non_trivial_copy_assign uses assign_from", "[smf_control]") {
    using wrapped = non_trivial_copy_assign<tracking_base3, std::string>;

    wrapped original;
    original.value = 100;

    wrapped copy;
    copy = original;
    REQUIRE(copy.value == 100);
    REQUIRE(copy.assign_calls == 1);
}

TEST_CASE("deleted_copy_assign has deleted copy assignment", "[smf_control]") {
    struct test_base : both_impl {
        test_base() = default;
        test_base(int v) : both_impl(v) {
        }
    };

    using wrapped = deleted_copy_assign<test_base, std::shared_ptr<int>>;

    STATIC_REQUIRE(std::is_copy_constructible_v<wrapped>);
    STATIC_REQUIRE(std::is_move_constructible_v<wrapped>);
    STATIC_REQUIRE_FALSE(std::is_copy_assignable_v<wrapped>);
    STATIC_REQUIRE(std::is_move_assignable_v<wrapped>);
}

TEST_CASE("copy_assign_control selects trivial copy assignment", "[smf_control]") {
    struct test_base : both_impl {
        test_base() = default;
        test_base(int v) : both_impl(v) {
        }
    };

    using trivial_types = copy_assign_control<test_base, int, float>;
    STATIC_REQUIRE(std::is_same_v<trivial_types, move_control<test_base, int, float>>);
}

TEST_CASE("copy_assign_control selects non_trivial_copy_assign", "[smf_control]") {
    struct test_base : both_impl {
        test_base() = default;
        test_base(int v) : both_impl(v) {
        }
    };

    using non_trivial_types = copy_assign_control<test_base, std::string>;
    STATIC_REQUIRE(std::is_same_v<non_trivial_types, non_trivial_copy_assign<test_base, std::string>>);
}

TEST_CASE("copy_assign_control selects deleted_copy_assign", "[smf_control]") {
    struct test_base : both_impl {
        test_base() = default;
        test_base(int v) : both_impl(v) {
        }
    };

    struct non_copyable {
        non_copyable() = default;
        non_copyable(const non_copyable &) = delete;
        non_copyable &operator=(const non_copyable &) = delete;
    };

    using deleted_types = copy_assign_control<test_base, non_copyable>;
    STATIC_REQUIRE(std::is_same_v<deleted_types, deleted_copy_assign<test_base, non_copyable>>);
}

TEST_CASE("non_trivial_move_assign move assigns correctly", "[smf_control]") {
    struct test_base : both_impl {
        test_base() = default;
        test_base(int v) : both_impl(v) {
        }
    };

    using wrapped = non_trivial_move_assign<test_base, std::string>;

    wrapped original(42);
    wrapped target;
    target = std::move(original);
    REQUIRE(target.value == 42);
}

struct tracking_base4 : assignable_base<tracking_base4> {
    int value = 0;
    int assign_calls = 0;

    template <typename Ty, typename = std::enable_if_t<std::is_arithmetic_v<Ty>>>
    void assign_impl_(Ty &&other) {
        value = static_cast<int>(other);
        ++assign_calls;
    }

    void assign_impl_(const tracking_base4 &other) {
        value = other.value;
        ++assign_calls;
    }

    void assign_impl_(tracking_base4 &&other) {
        value = other.value;
        ++assign_calls;
    }
};

TEST_CASE("non_trivial_move_assign uses assign_from for move", "[smf_control]") {
    using wrapped = non_trivial_move_assign<tracking_base4, std::string>;

    wrapped original;
    original.value = 100;

    wrapped target;
    target = std::move(original);
    REQUIRE(target.value == 100);
    REQUIRE(target.assign_calls == 1);
}

TEST_CASE("deleted_move_assign has deleted move assignment", "[smf_control]") {
    struct test_base : both_impl {
        test_base() = default;
        test_base(int v) : both_impl(v) {
        }
    };

    using wrapped = deleted_move_assign<test_base, std::shared_ptr<int>>;

    STATIC_REQUIRE(std::is_copy_constructible_v<wrapped>);
    STATIC_REQUIRE(std::is_move_constructible_v<wrapped>);
    STATIC_REQUIRE(std::is_copy_assignable_v<wrapped>);
    STATIC_REQUIRE_FALSE(std::is_move_assignable_v<wrapped>);
}

TEST_CASE("move_assign_control selects trivial move assignment", "[smf_control]") {
    struct test_base : both_impl {
        test_base() = default;
        test_base(int v) : both_impl(v) {
        }
    };

    using trivial_types = move_assign_control<test_base, int, float>;
    STATIC_REQUIRE(std::is_same_v<trivial_types, copy_assign_control<test_base, int, float>>);
}

TEST_CASE("move_assign_control selects non_trivial_move_assign", "[smf_control]") {
    struct test_base : both_impl {
        test_base() = default;
        test_base(int v) : both_impl(v) {
        }
    };

    using non_trivial_types = move_assign_control<test_base, std::string>;
    STATIC_REQUIRE(std::is_same_v<non_trivial_types, non_trivial_move_assign<test_base, std::string>>);
}

TEST_CASE("move_assign_control selects deleted_move_assign", "[smf_control]") {
    struct test_base : both_impl {
        test_base() = default;
        test_base(int v) : both_impl(v) {
        }
    };

    struct non_movable {
        non_movable() = default;
        non_movable(non_movable &&) = delete;
        non_movable &operator=(non_movable &&) = delete;
    };

    using deleted_types = move_assign_control<test_base, non_movable>;
    STATIC_REQUIRE(std::is_same_v<deleted_types, deleted_move_assign<test_base, non_movable>>);
}

TEST_CASE("control alias works correctly", "[smf_control]") {
    struct test_base : both_impl {
        test_base() = default;
        test_base(int v) : both_impl(v) {
        }
    };

    using controlled = control<test_base, int, float>;
    STATIC_REQUIRE(std::is_same_v<controlled, move_assign_control<test_base, int, float>>);
}

TEST_CASE("control with multiple types", "[smf_control]") {
    struct test_base : both_impl {
        test_base() = default;
        test_base(int v) : both_impl(v) {
        }
    };

    using controlled = control<test_base, std::string, std::vector<int>>;
    STATIC_REQUIRE(std::is_same_v<controlled, non_trivial_move_assign<test_base, std::string, std::vector<int>>>);
}

TEST_CASE("control with mixed types", "[smf_control]") {
    struct test_base : both_impl {
        test_base() = default;
        test_base(int v) : both_impl(v) {
        }
    };

    using controlled = control<test_base, int, std::string>;
    STATIC_REQUIRE(std::is_same_v<controlled, non_trivial_move_assign<test_base, int, std::string>>);
}
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
#include <catch2/catch_all.hpp>
#include <rainy/foundation/container/any.hpp>

using namespace rainy::foundation::container;

TEST_CASE("any - arithmetic - addition", "[any]") {
    any a = 10;
    any b = 20;
    any c = a + b;
    CHECK(c.has_value() == true);
    CHECK(c.is<int>() == true);
    CHECK(c.as<int>() == 30);

    any d = c + a;
    CHECK(d.as<int>() == 40);
}

TEST_CASE("any - arithmetic - subtraction", "[any]") {
    any a = 50;
    any b = 20;
    any c = a - b;
    CHECK(c.has_value() == true);
    CHECK(c.is<int>() == true);
    CHECK(c.as<int>() == 30);

    any d = c - b;
    CHECK(d.as<int>() == 10);
}

TEST_CASE("any - arithmetic - multiplication", "[any]") {
    any a = 5;
    any b = 6;
    any c = a * b;
    CHECK(c.has_value() == true);
    CHECK(c.is<int>() == true);
    CHECK(c.as<int>() == 30);

    any d = c * any(2);
    CHECK(d.as<int>() == 60);
}

TEST_CASE("any - arithmetic - division", "[any]") {
    any a = 100;
    any b = 4;
    any c = a / b;
    CHECK(c.has_value() == true);
    CHECK(c.is<int>() == true);
    CHECK(c.as<int>() == 25);

    any d = a / any(10);
    CHECK(d.as<int>() == 10);
}

TEST_CASE("any - arithmetic - modulo", "[any]") {
    any a = 17;
    any b = 5;
    any c = a % b;
    CHECK(c.has_value() == true);
    CHECK(c.is<int>() == true);
    CHECK(c.as<int>() == 2);

    any d = any(10) % any(3);
    CHECK(d.as<int>() == 1);
}

TEST_CASE("any - arithmetic - floating point", "[any]") {
    any a = 10.5;
    any b = 2.0;
    any c = a + b;
    CHECK(c.has_value() == true);
    CHECK(c.as<double>() == 12.5);

    any d = a - b;
    CHECK(d.as<double>() == 8.5);

    any e = a * b;
    CHECK(e.as<double>() == 21.0);

    any f = a / b;
    CHECK(f.as<double>() == 5.25);
}

TEST_CASE("any - arithmetic - double precision operations", "[any]") {
    any a = 3.0;
    any b = 7.0;
    any c = a + b;
    CHECK(c.as<double>() == 10.0);

    any d = b - a;
    CHECK(d.as<double>() == 4.0);

    any e = a * b;
    CHECK(e.as<double>() == 21.0);
}

TEST_CASE("any - arithmetic - increment and decrement", "[any]") {
    any a = 10;
    any b = ++a;
    CHECK(b.has_value() == true);

    any c = 10;
    any d = c++;
    CHECK(d.has_value() == true);

    any e = 10;
    any f = --e;
    CHECK(f.has_value() == true);

    any g = 10;
    any h = g--;
    CHECK(h.has_value() == true);
}

TEST_CASE("any - arithmetic - compound add assign", "[any]") {
    any a = 10;
    any b = 5;
    a += b;
    CHECK(a.has_value() == true);
}

TEST_CASE("any - arithmetic - compound subtract assign", "[any]") {
    any a = 30;
    any b = 10;
    a -= b;
    CHECK(a.has_value() == true);
}

TEST_CASE("any - arithmetic - compound multiply assign", "[any]") {
    any a = 5;
    any b = 4;
    a *= b;
    CHECK(a.has_value() == true);
}

TEST_CASE("any - arithmetic - compound divide assign", "[any]") {
    any a = 100;
    any b = 5;
    a /= b;
    CHECK(a.has_value() == true);
}

TEST_CASE("any - arithmetic - compound modulo assign", "[any]") {
    any a = 17;
    any b = 5;
    a %= b;
    CHECK(a.has_value() == true);
}

TEST_CASE("any - arithmetic - compound assignment chained", "[any]") {
    any a = 10;
    a += any(5);
    CHECK(a.has_value() == true);

    any b = 100;
    b -= any(30);
    CHECK(b.has_value() == true);

    b /= any(7);
    CHECK(b.has_value() == true);
}

TEST_CASE("any - arithmetic - chained operations", "[any]") {
    any a = 10;
    any b = 20;
    any c = 30;
    any d = a + b + c;
    CHECK(d.has_value() == true);
    CHECK(d.as<int>() == 60);

    any e = a + b - c;
    CHECK(e.has_value() == true);

    any f = a * b + c;
    CHECK(f.has_value() == true);
    CHECK(f.as<int>() == 230);
}

TEST_CASE("any - arithmetic - mixed int and double", "[any]") {
    any a = 10;
    any b = 3.5;
    any c = a + b;
    CHECK(c.has_value() == true);

    any d = b + a;
    CHECK(d.has_value() == true);

    any e = a * b;
    CHECK(e.has_value() == true);

    any f = b / a;
    CHECK(f.has_value() == true);
}

TEST_CASE("any - arithmetic - dereference pointer", "[any]") {
    int value = 42;
    int *ptr = &value;
    any a = ptr;
    any b = *a;
    CHECK(b.has_value() == true);
}

TEST_CASE("any - arithmetic - zero and negative", "[any]") {
    any a = 0;
    any b = -5;
    any c = 10;

    any d = a + c;
    CHECK(d.as<int>() == 10);

    any e = b + c;
    CHECK(e.as<int>() == 5);

    any f = c - c;
    CHECK(f.as<int>() == 0);
}

TEST_CASE("any - arithmetic - large values", "[any]") {
    any a = 1000000;
    any b = 2000000;
    any c = a + b;
    CHECK(c.as<int>() == 3000000);

    any d = b - a;
    CHECK(d.as<int>() == 1000000);

    any e = a * any(3);
    CHECK(e.as<int>() == 3000000);
}

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
// 这是一个测试代码单元文件，用于测试prism-moc的生成行为
#define RAINY_ENABLE_MOC(...)

#define RAINY_GRANT_ACCESS_NAMESPACE(x)
#define RAINY_GRANT_ACCESS_USING(x)
#define RAINY_MOC_METADATA(...)
#include <string>
#include <vector>

class Foo {};

class Bar {};

/*
应该生成如下的插桩
```cpp
registration::class_<a>("a")
        .constructor<>()
        .method("foo",rainy::utility::get_overloaded_func<a, void()>(&a::foo))
        .method("foo",rainy::utility::get_overloaded_func<a, void(int, int)>(&a::foo))
        .method("mem",rainy::utility::get_overloaded_func<a, int()>(&a::mem))
        .method("bar",rainy::utility::get_overloaded_func<void()>(&a::bar))
        .property("aa", &a::aa);
```
 */
class a {
public:
    RAINY_ENABLE_MOC()

    a() {
    }

    void foo() {
    }

    void foo(int, int) {
    }

    int mem() {
        return 0;
    }

    static void bar() {
    }

    int aa;

private:
    int b;
};

class b {
public:
    RAINY_ENABLE_MOC(RAINY_GRANT_ACCESS_NAMESPACE("xxx"), RAINY_GRANT_ACCESS_NAMESPACE("x1xx"), RAINY_GRANT_ACCESS_USING("xxx"))

    b() {
    }

    void foo() {
    }

    void foo(int, int) {
    }

    static void bar() {
    }

    int field;
};

RAINY_ENABLE_MOC(RAINY_GRANT_ACCESS_NAMESPACE("xxx"), RAINY_GRANT_ACCESS_USING("xxx"))
void function() {
}

RAINY_ENABLE_MOC()
void function1() {
}

namespace test_namespace {
    RAINY_ENABLE_MOC()
    void function2() {
    }
}

namespace test_namespace {
    namespace nest {
        class c {
        public:
            RAINY_ENABLE_MOC()

            c() {
            }

            void foo() {
            }

            void foo(int, int) {
            }

            int mem() {
                return 0;
            }

            static void bar() {
            }

            int aa;

        private:
            int b;
        };
    }
}

class nest_class {
public:
    RAINY_ENABLE_MOC()
    nest_class() {
    }

    class clazz {
    public:
        // RAINY_ENABLE_MOC()

        clazz() {
        }
    };
};

class test_class {
public:
    RAINY_ENABLE_MOC()
    test_class() {
    }

    test_class(const test_class &) {
    }

    test_class(int) {
    }
};

RAINY_ENABLE_MOC()
enum test_enum {
    a,
    b,
    c
};

RAINY_ENABLE_MOC()
enum class enum_class {
    a,
    b,
    c
};

namespace test_namespace {
    RAINY_ENABLE_MOC()
    enum class test_enum {
        b,
        c,
        d
    };

    class test_clazz {
    public:
        RAINY_ENABLE_MOC()

        test_clazz() {
        }

        RAINY_ENABLE_MOC()
        enum class test_enum {
            a,
            b,
            c
        };
    };

    class test_clazz1 {
    public:
        RAINY_ENABLE_MOC()

        test_clazz1() {
        }

        struct nest1 {

            nest1() {
            }

            class nest2 {
            public:
                nest2() {
                }

                RAINY_ENABLE_MOC()
            };
        };
    };
}

class nest_class_mo {
public:
    RAINY_ENABLE_MOC()

    nest_class_mo() {
    }

    void fun();
};

RAINY_ENABLE_MOC()
void function1111();

class Player {
public:
    RAINY_ENABLE_MOC()

    Player();

    Player(std::string n, int h, int l);

    void PrintInfo();

    void TakeDamage(int damage);

    // 成员变量
    std::string name;
    int health;
    int level;
};

RAINY_ENABLE_MOC()
std::string test_str(std::string, std::vector<std::string>);

RAINY_ENABLE_MOC()
void print_plugin_test_info();
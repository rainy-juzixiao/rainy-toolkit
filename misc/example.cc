#define RAINY_ENABLE_MOC(...)

#define RAINY_GRANT_ACCESS_NAMESPACE(x)
#define RAINY_GRANT_ACCESS_USING(x)

class Foo {
};

class Bar {
};

class a {
public:
    RAINY_ENABLE_MOC()

    a() { }

    void foo() {

    }

    void foo(int, int) {
    }

    int mem() {
        return 0;
    }

    static void bar() {}

    int aa;

private:
    int b;
};

class b {
public:
    RAINY_ENABLE_MOC(
        RAINY_GRANT_ACCESS_NAMESPACE("xxx"),
        RAINY_GRANT_ACCESS_NAMESPACE("x1xx"),
        RAINY_GRANT_ACCESS_USING("xxx")
    )

    b() { }

    void foo() {

    }

    void foo(int, int) {

    }

    static void bar() {}

    int field;
};

RAINY_ENABLE_MOC(
    RAINY_GRANT_ACCESS_NAMESPACE("xxx"),
    RAINY_GRANT_ACCESS_USING("xxx")
)
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
            RAINY_ENABLE_MOC();

            c() {}

            void foo() {}
            
            void foo(int, int) {
            }

            int mem() {
            }

            static void bar() {}

            int aa;

        private:
            int b;
        };
    }
}
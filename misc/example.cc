#define RAINY_ENABLE_MOC(...)

class Foo {
};

class Bar {
};

class a {
public:
    RAINY_ENABLE_MOC();

    a() { }

    void foo() {

    }

    void foo(int, int) {

    }

    int mem() {

    }

    static void bar() {}
};

class b {
public:
    RAINY_ENABLE_MOC();

    b() { }

    void foo() {

    }

    void foo(int, int) {

    }

    static void bar() {}
};

RAINY_ENABLE_MOC()
void function() {

}
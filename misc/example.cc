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

    int aa;

private:
    int b;
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

RAINY_ENABLE_MOC()
void function1() {

}

RAINY_ENABLE_MOC()
void function2() {

}
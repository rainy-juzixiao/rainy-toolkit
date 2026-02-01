#define RAINY_ENABLE_MOC() static constexpr bool inline this_class_is_enable_for_moc = false

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

    static void bar() {}
};
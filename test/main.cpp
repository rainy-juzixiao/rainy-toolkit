#include <iostream>

#include <cstdio>
#include <rainy/experimental/format.hpp>
#include <rainy/system/exceptions.hpp>
#include <span>

template <typename Fn, typename... Args>
void benchmark(const char *const label, Fn &&func, Args &&...args) {
    const auto start = std::chrono::high_resolution_clock::now();
    (void) std::forward<Fn>(func)(std::forward<Args>(args)...);
    const auto end = std::chrono::high_resolution_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << label << " took " << time.count() << " nanoseconds\n";
}

#include <utility>
#include <variant>
#include <bitset>
#include <sstream>
#include <unordered_set>
#include <rainy/containers/any.hpp>
#include <rainy/meta/hash.hpp>
#include <rainy/utility.hpp>
#include <rainy/containers/stack.hpp>

union unions {
    int a;
    int b;
};

template <typename Ty,std::size_t N>
class array {
public:
    using value_type = Ty;
    using size_type = std::size_t;

    constexpr array() noexcept(std::is_nothrow_constructible_v<Ty>) = default;

    RAINY_CONSTEXPR20 array(std::initializer_list<Ty> ilist) noexcept(std::is_nothrow_copy_assignable_v<Ty>) {
        _size = ilist.size();
#if RAINY_HAS_CXX20
        if (std::is_constant_evaluated()) {
            std::copy_n(ilist.begin(), _size, elems);
        } else
#endif
        {
            rainy::utility::expects(ilist.size() <= _size);
            if constexpr (rainy::type_traits::type_properties::is_pod_v<Ty>) {
                std::uninitialized_copy_n(ilist.begin(), _size, elems);
            } else {
                std::copy_n(ilist.begin(), _size, elems);
            }
        }
    }

    RAINY_CONSTEXPR20 array(const array &right) {
        
    }

    constexpr std::size_t size() const noexcept {
        return _size;
    }

    constexpr size_type length() const noexcept {
        return N;
    }

    constexpr std::size_t empty() const noexcept {
        return _size == 0;
    }

    template <typename... Args>
    RAINY_CONSTEXPR20 void emplace_back(Args... args) noexcept {
        range_check(++_size);
        
    }

private:
    void range_check(const size_type idx) {
        if (idx <= N) {
            rainy::foundation::system::exceptions::logic::throw_out_of_range("Invalid array subscript");
        }
    }

    static void check_zero_length_error() {
        if (N == 0) {
            std::terminate();
        }
    }

    std::size_t _size;
    value_type elems[N];
};

namespace static_reflection {
    template <typename Ty>
    struct field {
        using type = Ty;

        constexpr field(std::string_view name, type *pointer, const bool is_const, const bool is_static) :
            name(name), pointer(pointer), is_const(is_const), is_static(is_static) {
        }

        std::string_view name;
        type *pointer;
        bool is_const;
        bool is_static;
    };

    template <typename Class,typename Ty>
    struct field<Ty Class::*> {
        using type = Ty Class::*;

        constexpr field(std::string_view name, Ty Class::* pointer, const bool is_const, const bool is_static) :
            name(name), pointer(pointer), is_const(is_const), is_static(is_static) {
        }

        std::string_view name;
        type pointer;
        bool is_const;
        bool is_static;
    };

    template <typename... Es>
    struct element_list {
        constexpr element_list(Es... es) : elements(es...) {
        }

        template <typename Fx>
        constexpr void for_each(Fx && func) const {
            std::apply([&func](const auto &...e) {
                (func(e), ...); 
            }, elements);
        }

        template <typename Pred>
        constexpr std::size_t find_if(Pred &&pred) const {
            rainy_let index = static_cast<std::size_t>(-1);
            std::size_t i = 0;
            ((pred(std::get<Es>(elements)) ? (index = i, true) : (++i, false)) || ...);
            return index;
        }

        template <typename Name>
        constexpr auto find() const {
            return find_if([](const auto &e) {
                return std::is_same_v<typename decltype(e)::name_type, Name>; 
            });
        }

        template <std::size_t I>
        constexpr const auto &get() const {
            return std::get<I>(elements);
        }

        std::tuple<Es...> elements;
    };

    template <typename... Fields>
    struct field_list : element_list<Fields...> {
        constexpr field_list(Fields... fields) : element_list<Fields...>{fields...} {
        }
    };

    template <typename Class,typename Ty>
    constexpr auto make_field(std::string_view name, Ty Class::*member) {
        return field<Ty Class::*>(name, member, std::is_const_v<Ty Class::*>, false);
    }

    template <typename Ty>
    struct demo {
        using type = Ty;
        int a;

        void fun() {
            std::cout << "fun"
                      << "\n";
        }
    };

    template <typename Ty, typename... Fields>
    struct type_info {
        static constexpr std::string_view name = "unknown";
        using fields = std::tuple<Fields...>;
    };

    template <typename type>
    struct type_info<demo<type>> {
        static constexpr std::string_view name = "demo";
        static constexpr field_list fields = {
            make_field("a", &demo<int>::a), 
            make_field("fun",&demo<int>::fun)
        };
    };
}


class low {
public:
    int low_field = 0;

    virtual void virtual_method() {
    }
};

class base : public low {
public:
    int base_field = 0;

    void virtual_method() override {
        std::cout << "Hello Virtual"
                  << "\n";
    }
};

class my_class : public rainy::component::object<my_class> {
public:
    my_class() {
        std::cout << "default constructer"
                  << "\n";
    };

    my_class(const my_class &) {
        std::cout << "copy constructer"
                  << "\n";
    };

    my_class(int, int) {
        std::cout << "constructer(int,int)"
                  << "\n";
    }

    ~my_class() {
        std::cout << "destructer"
                  << "\n";
    }

    void another() {
        std::cout << "another"
                  << "\n";
    }

    void another() const {
        std::cout << "const another"
                  << "\n";
    }

    void hello() {
        std::cout << "normal method"
                  << "\n";
    }

    void hello(const int a, const int b) const {
    }

    static void hello_static() {
        std::cout << "static method"
                  << "\n";
    }

    static void hello_static(int, int) {
        std::cout << "static method"
                  << "\n";
    }

    my_class &operator=(const my_class &) {
        return *this;
    }

    int a = 10;
    const int b = 10;
    static const int c;
};

const int my_class::c = 10;

#if RAINY_USING_WINDOWS
template <typename CharType>
LSTATUS reg_openkey(HKEY key,DWORD options,const CharType* sub_key, REGSAM desired,PHKEY result) {
    if (!result || !key) {
        return ERROR_BAD_ARGUMENTS;
    }
    if constexpr (rainy::type_traits::helper::is_wchar_t<CharType>) {
        return RegOpenKeyExW(key, sub_key, options, desired, result);
    } else {
        return RegOpenKeyExA(key, sub_key, options, desired, result);
    }
}

class registry {
public:
    enum class predefinekey {
        HKCR,
        HKCC,
        HKCU,
        HKLM,
        HKUSERS
    };

    constexpr registry() noexcept : context(HKEY{}){};

    ~registry() {
        close();
    }

    template <typename CharType>
    LSTATUS switch_context(predefinekey predefine_key, const std::basic_string_view<CharType> path) {
        HKEY key = get_predefine_key(predefine_key);
        if (LSTATUS status = reg_openkey(key, path.data(), 0, KEY_ALL_ACCESS, &context); status == ERROR_SUCCESS) {
            return status;
        } else if (status = reg_openkey(key, path.data(), 0, KEY_READ | KEY_WRITE, &context); status == ERROR_SUCCESS) {
            return status;
        } else if (status = reg_openkey(key, path.data(), 0, KEY_READ, &context); status == ERROR_SUCCESS) {
            return status;
        } else {
            return status;
        }
    }

    template <typename CharType>
    LSTATUS switch_context(HKEY key, const std::basic_string_view<CharType> path) {
        if (!key) {
            return ERROR_BAD_ARGUMENTS;
        }
        if (LSTATUS status = reg_openkey(key, path.data(), 0, KEY_ALL_ACCESS, &context); status == ERROR_SUCCESS) {
            return status;
        } else if (status = reg_openkey(key, path.data(), 0, KEY_READ | KEY_WRITE, &context); status == ERROR_SUCCESS) {
            return status;
        } else if (status = reg_openkey(key, path.data(), 0, KEY_READ, &context); status == ERROR_SUCCESS) {
            return status;
        } else {
            return status;
        }
    }

    void close() {
        if (!valid()) {
            return;
        }
        if (LSTATUS error_code = RegCloseKey(context); error_code != ERROR_SUCCESS) {
            std::string error_message = rainy::winapi::error_process::last_error_message<char>(error_code);
            (void)std::fwrite(error_message.c_str(), sizeof(char), sizeof(char) * error_message.length(), stderr);
        }
        context = nullptr;
    }

    bool valid() const noexcept {
        return static_cast<bool>(context);
    }

private:
    static HKEY get_predefine_key(predefinekey predefine_key) noexcept {
        HKEY open_key{nullptr};
        switch (predefine_key) {
            case predefinekey::HKCR:
                open_key = HKEY_CLASSES_ROOT;
                break;
            case predefinekey::HKCC:
                open_key = HKEY_CURRENT_CONFIG;
                break;
            case predefinekey::HKCU:
                open_key = HKEY_CURRENT_USER;
                break;
            case predefinekey::HKLM:
                open_key = HKEY_LOCAL_MACHINE;
                break;
            case predefinekey::HKUSERS:
                open_key = HKEY_USERS;
                break;
            default:
                break;
        }
        return open_key;
    }

    HKEY context{};
};
#endif

#include <rainy/utility.hpp>



#include <future>




constexpr int fooooo() {
    return 10 + 20;
}



#include <rainy/system/nebula_ptr.hpp>

class A {
public:
    virtual ~A() {
        std::puts("~A()\n");
    }

    int value = 0;

    virtual void hello() const = 0;
};

class B : public A {
public:
    ~B() {
        std::puts("~B()\n");
    }

    void hello() const {
        std::cout << "Hello World"
                  << "\n";
    }
};

#include <execution>

constexpr int test() {
    std::array<int, 5> a = {1, 2, 3, 4, 5};
    std::array<int, 5> a2{};
    //rainy::algorithm::container_operater::copy_n(rainy::algorithm::execution::par,a.begin(), a.size(), a2.begin());
    return 1;
}

//#include <rainy/functional.hpp>

#include <rainy/containers/vector.hpp>
#include <rainy/winapi/environment.hpp>

constexpr int test_fun() {
    /*int *p = rainy::foundation::system::memory::allocator<int>{}.allocate(1);
    rainy::utility::construct_at(p,10);
    int value = *p;
    rainy::utility::ensures(*p == 10);
    rainy::foundation::system::memory::allocator<int>{}.deallocate(p, 1);
    return value;*/
    return 0;
}

#include <rainy/containers/variant.hpp>

void run_visit_test() {
    using namespace rainy::containers;
    struct visitor {
        void operator()(int val) noexcept {
            std::cout << "int\n";
        }

        void operator()(char val) noexcept {
            std::cout << "char\n";
        }

        void operator()(std::string &val) noexcept {
            std::cout << "string\n";
        }
    };
    variant<std::string,int,char> variant1 = "Hello World";
    visit(visitor{}, variant1);
    variant<std::string, int, char> variant2 = 10;
    visit(visitor{}, variant2);
    variant<std::string, int, char> variant3 = 'c';
    visit(visitor{}, variant3);
    visit(visitor{}, variant1, variant2, variant3);

    variant<std::string, const char *, int> var(std::in_place_index<2>, 10);
}

struct demo_class {
    int x, y;
    int custom_field;

    int mult() {
        return x * y;
    }
};

struct derived_class : public demo_class {
};

struct foo {
    foo(int val) : val(val) {
    }

    // 这是前置递增
    int operator++() {
        val = val + 1;
        return val;
    }

    // 这是后置递增
    int operator++(int) {
        int old_val = val; // 保存了当前的值
        val = val + 1; // 然后我再自增
        return old_val; // 最后返回的是我保存的，而不是val
    }

    // 这是前置递减
    int operator--() {
        val = val - 1;
        return val;
    }

    // 这是后置递减
    int operator--(int) {
        int old_val = val; // 保存了当前的值
        val = val - 1; // 然后我再自增
        return old_val; // 最后返回的是我保存的，而不是val
    }

    int val;
};

#include <rainy/winapi/ui/window.hpp>

//class my_window : public rainy::winapi::ui::window<char> {
//public:
//    virtual LRESULT handle_message(UINT msg, WPARAM wparam, LPARAM lparam) override {
//        return DefWindowProcA(this->handle,msg,wparam,lparam);
//    }
//};

unsigned int my_function(void*) {
    std::cout << "Hello World"
              << "\n";
    return 0;
}

//#include <process.h>
#include <fstream>

class localization {
public:
    localization() = default;

    localization(std::string_view filename) {
        std::fstream fstream(filename.data(), std::ios::in);
        if (!fstream) {
            
        }
    }

private:
    std::unordered_map<std::string, std::string> data;
};

class user_event : public rainy::component::sync_event::event {
public:
    user_event() : msg(666) {
    }

    int message() const noexcept {
        return msg;
    }

    void set_message(int msg) noexcept {
        this->msg = msg;
    }

private:
    int msg;
};

class listener : public rainy::component::sync_event::event_handler<user_event> {
public:
    void on(rainy::component::sync_event::event &e) override {
    }
};

struct pod {
    int a{};
    char b{};
    std::string c;
    std::vector<int> d;
public:
    static constexpr auto __refl_struct_members() noexcept {
        return std::make_tuple(&pod::a, &pod::b, &pod::c, &pod::d);
    }
};

class secret {
    int m_i = 3;
    int m_f(int p) {
        return 14 * p;
    }
    int m_2 = 10;
    std::string m_3;
};

class secret2 {
    int m_i = 3;
    int m_f(int p) {
        return 14 * p;
    }
};

int virtual_function() noexcept {
    return 555;
}

#include <set>
#include <rainy/utility/invoke.hpp>

namespace rainy::utility::internals {
    struct try_to_invoke_size {
        template <typename Uty, typename = type_traits::other_trans::void_t<decltype(declval<const Uty>().size())>>
        static constexpr auto invoke(const Uty &container) -> decltype(container.size()) {
            return container.size();
        }

        template <typename Uty, typename = type_traits::other_trans::void_t<decltype(declval<const Uty>().Size()), int>>
        static constexpr auto invoke(const Uty &container) -> decltype(container.Size()) {
            return container.Size();
        }
    };

    struct try_to_invoke_length {
        template <typename Uty,
                  typename = type_traits::other_trans::void_t<decltype(declval<const Uty>().Length()), int, int>>
        static constexpr auto invoke(const Uty &container) noexcept(noexcept(container.Length())) -> decltype(container.Length()) {
            return container.Length();
        }

        template <typename Uty,
                  typename = type_traits::other_trans::void_t<decltype(declval<const Uty>().length()), int, int, int>>
        static constexpr auto invoke(const Uty &container) noexcept(noexcept(container.length())) -> decltype(container.length()) {
            return container.length();
        }
    };
}

namespace rainy::utility {
    template <typename Ty, std::size_t N>
    constexpr std::size_t size(const Ty (&)[N]) noexcept {
        return N;
    }

    template <typename Container>
    constexpr auto size(const Container &container) noexcept -> decltype(internals::try_to_invoke_size::invoke(container)) {
        return internals::try_to_invoke_size::invoke(container);
    }

    template <typename Container>
    constexpr auto length(const Container &container) noexcept -> decltype(internals::try_to_invoke_length::invoke(container)) {
        return internals::try_to_invoke_length::invoke(container);
    }
}

#include <rainy/internals/reflection_core/method.hpp>

namespace rainy_test {
    template <typename Fn, typename... Args>
    void benchmark(const char *const label, Fn &&func, Args &&...args) {
        const auto start = std::chrono::high_resolution_clock::now();
        (void) std::forward<Fn>(func)(std::forward<Args>(args)...);
        const auto end = std::chrono::high_resolution_clock::now();
        auto time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << label << " took " << time.count() << " milliseconds\n";
    }

    struct test_class {
        void noexcept_fun() noexcept {
            std::cout << "Hello test_class"
                      << "\n";
        }

        void virtual_function() noexcept {
        }

        int length() const noexcept {
            return 0;
        }

        int size() const & {
            return 0;
        }

        int test_4args_method(int,int,int,int) {
            return 0;
        }

        int a;
    };

    void test() {
        auto size_method = rainy::foundation::reflection::make_method("test_class::size()", &test_class::size);
        for (int i = 0; i < 10; ++i) {
            benchmark("test_class::size() 100000 invocations", [&size_method]() {
                for (int i = 0; i < 100000; ++i) {
                    size_method(nullptr, {});
                }
            });
        }
        auto test_4args_method =
            rainy::foundation::reflection::make_method("test_class::test_4args_method()", &test_class::test_4args_method);
        for (int i = 0; i < 10; ++i) {
            benchmark("test_class::test_4args_method(int,int,int,int) 100000 invocations", [&test_4args_method]() {
                for (int i = 0; i < 100000; ++i) {
                    test_4args_method(nullptr, {1, 2, 3, 4});
                }
            });
        }
        std::vector<rainy::containers::any> param_list = {1, 2, 3, 4};
        for (int i = 0; i < 10; ++i) {
            benchmark("test_class::test_4args_method(int,int,int,int) 100000 invocations with static param_list", [&test_4args_method,&param_list]() {
                for (int i = 0; i < 100000; ++i) {
                    test_4args_method(nullptr,param_list);
                }
            });
        }
        rainy::foundation::reflection::make_method("vf", &test_class::length);
    }
}




int main() {
    {
        const std::vector<int> a;
        auto p = std::mem_fn(rainy::utility::overload_cmethod(&std::vector<int>::at));
        p(a, 0);
    }
    rainy_test::test();
    /*{
        int a{};
        rainy::utility::reference_wrapper ref = virtual_function;
        ref();
        const std::vector<int> v{};
        rainy::utility::length(test_class{});
    }*/
    std::random_device random_device;
    std::mt19937 gen(random_device());
    std::uniform_int_distribution<> dist(1, 28);
    std::set<int> my_set;
    while (my_set.size() < 10) {
        auto number = dist(gen);
        if (number == 19) {
            while (number == 19) {
                number = dist(gen);
            }
        }
        my_set.insert(dist(gen));
    }
    for (const auto &i: my_set) {
        std::cout << i << "\n";
    }

    

    std::uniform_int_distribution<> rc_dist(1, 10);
    std::cout << "random choice = " << rc_dist(gen) << "\n";
    //rainy::foundation::system::async::internals::init_async_moudle_abi();
    //auto thread_instance = rainy::foundation::comint::create_instance<rainy::foundation::system::async::internals::native_thread>();



    //thread_instance->start(nullptr, 0, my_function, nullptr, 0);
    //rainy::foundation::system::async::internals::abi::thread_sleep_for(1);
    //std::cout << thread_instance.use_count() << "\n";
    //std::cout << thread_instance->get_id() << "\n";
    //std::shared_ptr<rainy::foundation::comint::the_unknown> move_ptr;
    //thread_instance->move(move_ptr);
    //std::cout << thread_instance->get_id() << "\n";
    //std::cout << std::dynamic_pointer_cast<rainy::foundation::system::async::internals::native_thread>(move_ptr)->get_id() << "\n";
    //std::dynamic_pointer_cast<rainy::foundation::system::async::internals::native_thread>(move_ptr)->join();
    //std::cout << thread_instance.use_count() << "\n";
    auto reg = rainy::component::sync_event::dispatcher::instance()->subscribe<user_event, listener>();
    std::cout << reg->name() << "\n";
    (void) rainy::component::sync_event::dispatcher::instance()->subscribe<user_event>(
        [](rainy::component::sync_event::event &e) {});
    user_event e;
    std::cout << e.type_info().name() << "\n";
    for (int i = 0; i < 10; ++i) {
        benchmark("sync_event", [&e]() {
            for (int i = 0; i < 10000; ++i) {
                rainy::component::sync_event::dispatcher::instance()->publish(e);
            }
        });
    }

    rainy::component::sync_event::dispatcher::instance()->for_each_in_handlers<user_event>(
        +[](rainy::component::sync_event::event_handler<user_event> *handler) { 
            std::cout << handler->handler_name() << "\n";
        });
    rainy::component::sync_event::dispatcher::instance()->clear();
    std::tuple<int,char,std::string> t;
    //rainy::winapi::ui::window<> main_window;
    //RECT window_rect = {100, 100, 800, 600};
    //main_window.create(NULL, "Main Window", WS_OVERLAPPEDWINDOW, 0, window_rect);
    //main_window.show_window();
    //main_window.message_loop();
    system("pause");
    foo a = 2;
    foo b = ++a;
    std::cout << a.val << "\n";
    std::cout << b.val << "\n";
    run_visit_test();
    rainy::containers::any any_object = 10;
    std::cout << rainy::utility::get<int&>(any_object) << "\n";
    rainy::utility::get<int &>(any_object) = 666;
    std::cout << rainy::utility::get<int &>(any_object) << "\n";


    /*rainy::winapi::environment::environment_manager manager;
    for (const auto &i: manager.get_environment_var_by_list("PATH").vector_list) {
        std::cout << i << "\n";
    }*/
    constexpr int v = test_fun();
    std::cout << v << "\n";

    rainy::foundation::system::memory::allocator<int,
                                                    rainy::foundation::system::memory::allocation_method::RAINY_allocator_no_check>
        allocator;

    int *get = allocator.allocate(10);
    std::tuple<int, char, int> tuple(1, 'a', 3);

    my_class o2;
    o2.another();

    o2.a = 666;

    auto o3 = o2.clone();

    std::cout << o3->a << "\n";

    o3->another();

    std::cout << std::get<2>(tuple) << "\n";
    rainy::foundation::system::memory::nebula_ptr<int[]> smart_ptr(get, 10);
    smart_ptr.fill_with_ilist({1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
    for (const int &i: smart_ptr) {
        std::cout << i << ' ';
    }
    allocator.deallocate(smart_ptr.release(), 10);

    std::pmr::memory_resource* std_memres = new rainy::foundation::system::memory::pmr::std_memory_resource;
    smart_ptr = static_cast<int*>(std_memres->allocate(sizeof(int) * 10, alignof(int)));
    smart_ptr.fill_with_ilist({1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
    for (const int &i: smart_ptr) {
        std::cout << i << ' ';
    }
    std_memres->deallocate(smart_ptr.release(),sizeof(int) * 10, alignof(int));

    constexpr int i = test();
    auto p = rainy::foundation::system::memory::make_nebula<int[]>(10, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
    for (const int &i: p) {
        std::cout << i << ' ';
    }
    std::cout.put('\n');
    p.reset();
    int array[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    p = &array;
    rainy::foundation::system::output::stdout_print("p.length = ", p.length(), " \n");
    for (const int &i: p) {
        std::cout << i << ' ';
    }
    std::cout.put('\n');
    p.reset();
    rainy::foundation::system::output::stdout_print("p.length = ", p.length(), " \n");

    int array1[5] = {1, 2, 3, 4, 5};
    p = &array1;

    rainy::foundation::system::output::stdout_print("p.length = ", p.length(), " \n");
    for (const int &i: p) {
        std::cout << i << ' ';
    }
    std::cout.put('\n');

    rainy::foundation::system::output::stdout_print("p.length = ", p.length(), " \n");

    return 0;
}

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
#include <rainy/utility/any.hpp>
#include <rainy/user/hash/hash.hpp>
#include <rainy/utility.hpp>
#include <rainy/collections/stack.hpp>

union unions {
    int a;
    int b;
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

#include <rainy/collections/vector.hpp>
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

#include <rainy/collections/variant.hpp>

void run_visit_test() {
    using namespace rainy::collections;
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

namespace rainy::utility::implements {
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
    constexpr auto size(const Container &container) noexcept -> decltype(implements::try_to_invoke_size::invoke(container)) {
        return implements::try_to_invoke_size::invoke(container);
    }

    template <typename Container>
    constexpr auto length(const Container &container) noexcept -> decltype(implements::try_to_invoke_length::invoke(container)) {
        return implements::try_to_invoke_length::invoke(container);
    }
}

#include <rainy/implements/reflection_core/method.hpp>

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
        std::vector<rainy::utility::any> param_list = {1, 2, 3, 4};
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

constexpr int my_test() {
    rainy::utility::expects(true);

    return 0;
}

#include <rainy/algorithm.hpp>
#include <rainy/user/hash/sha.hpp>

void test_sha512() {
    std::vector<std::string> test_inputs = {"123456789", "Hello, World!", "TestCase"};

    for (const auto &input: test_inputs) {
        std::string result = rainy::user::hash::implements::make_sha<rainy::user::hash::implements::sha_type::sha256>(input);
        std::cout << "Input: " << input << std::endl;
        std::cout << "SHA-256 hash: " << result << std::endl << std::endl;
    }

    for (const auto &input: test_inputs) {
        std::string result = rainy::user::hash::implements::make_sha<rainy::user::hash::implements::sha_type::sha512>(input);
        std::cout << "Input: " << input << std::endl;
        std::cout << "SHA-512 hash: " << result << std::endl << std::endl;
    }
    std::string result = rainy::user::hash::implements::make_sha_from_file<rainy::user::hash::implements::sha_type::sha512>("cpp.hint");
    std::cout << result << "\n";
    result = rainy::user::hash::implements::make_sha_from_file<rainy::user::hash::implements::sha_type::sha512>("cpp.hint");
    std::cout << result << "\n";
}

constexpr int it_test() {
    rainy::collections::array<int, 5> my_array = {1, 2, 3, 4, 5};
    auto it = my_array.cend();
    it -= 1;
    return *it.begin();
}







int main() {
    std::vector<int> my_vec123 = {1, 2, 3, 4, 5};
    rainy::utility::ostream_iterator<int> os(std::cout, " ");
    std::copy(my_vec123.begin(),my_vec123.end(), os);
    std::istream_iterator<int> is_iter;
    {
        std::size_t len{8192};
        std::vector<uint8_t> source(8192);
        std::vector<uint8_t> destination(8192);
        std::cout << "-------8192 sizes block copy-------"
                  << "\n";
        for (std::size_t i = 0; i < 10; ++i) {
            benchmark("memcpy", std::memcpy, &destination[0], &source[0], len);
        }
        for (std::size_t i = 0; i < 10; ++i) {
            benchmark("fast_memcpy", fast_memcpy, &destination[0], &source[0], len);
        }
        len = 4196;
        std::cout << "-------4096 sizes block copy-------"
                  << "\n";
        for (std::size_t i = 0; i < 10; ++i) {
            benchmark("memcpy", std::memcpy, &destination[0], &source[0],len);
        }
        for (std::size_t i = 0; i < 10; ++i) {
            benchmark("fast_memcpy", fast_memcpy, &destination[0], &source[0], len);
        }
        std::cout << "-------2048 sizes block copy-------"
                  << "\n";
        len = 2048;
        for (std::size_t i = 0; i < 10; ++i) {
            benchmark("memcpy", std::memcpy, &destination[0], &source[0], len);
        }
        for (std::size_t i = 0; i < 10; ++i) {
            benchmark("fast_memcpy", fast_memcpy, &destination[0], &source[0], len);
        }
        std::cout << "-------1024 sizes block copy-------"
                  << "\n";
        len = 1024;
        for (std::size_t i = 0; i < 10; ++i) {
            benchmark("memcpy", std::memcpy, &destination[0], &source[0], len);
        }
        for (std::size_t i = 0; i < 10; ++i) {
            benchmark("fast_memcpy", fast_memcpy, &destination[0], &source[0], len);
        }
        std::cout << "-------512 sizes block copy-------"
                  << "\n";
        len = 512;
        for (std::size_t i = 0; i < 10; ++i) {
            benchmark("memcpy", std::memcpy, &destination[0], &source[0], len);
        }
        for (std::size_t i = 0; i < 10; ++i) {
            benchmark("fast_memcpy", fast_memcpy, &destination[0], &source[0], len);
        }
        std::cout << "-------256 sizes block copy-------"
                  << "\n";
        len = 256;
        for (std::size_t i = 0; i < 10; ++i) {
            benchmark("memcpy", std::memcpy, &destination[0], &source[0], len);
        }
        for (std::size_t i = 0; i < 10; ++i) {
            benchmark("fast_memcpy", fast_memcpy, &destination[0], &source[0], len);
        }
        std::cout << "-------128 sizes block copy-------"
                  << "\n";
        len = 128;
        for (std::size_t i = 0; i < 10; ++i) {
            benchmark("memcpy", std::memcpy, &destination[0], &source[0], len);
        }
        for (std::size_t i = 0; i < 10; ++i) {
            benchmark("fast_memcpy", fast_memcpy, &destination[0], &source[0], len);
        }
        std::cout << "-------64 sizes block copy-------"
                  << "\n";
        len = 64;
        for (std::size_t i = 0; i < 10; ++i) {
            benchmark("memcpy", std::memcpy, &destination[0], &source[0], len);
        }
        for (std::size_t i = 0; i < 10; ++i) {
            benchmark("fast_memcpy", fast_memcpy, &destination[0], &source[0], len);
        }
        std::cout << "-------32 sizes block copy-------"
                  << "\n";
        len = 32;
        for (std::size_t i = 0; i < 10; ++i) {
            benchmark("memcpy", std::memcpy, &destination[0], &source[0], len);
        }
        for (std::size_t i = 0; i < 10; ++i) {
            benchmark("fast_memcpy", fast_memcpy, &destination[0], &source[0], len);
        }
        std::cout << "-------28 sizes block copy-------"
                  << "\n";
        len = 28;
        for (std::size_t i = 0; i < 10; ++i) {
            benchmark("memcpy", std::memcpy, &destination[0], &source[0], len);
        }
        for (std::size_t i = 0; i < 10; ++i) {
            benchmark("fast_memcpy", fast_memcpy, &destination[0], &source[0], len);
        }
    }
    {
        constexpr int r = []() constexpr noexcept {
            int num1 = 32;
            int num2 = 20;
            int sum = num1 - num2 + 10;
            sum = sum + 10;
            sum = sum - 20;
            return sum;
        }();
        auto my_lmb = [](int) noexcept { return 0; };

        rainy::utility::invoker<decltype(my_lmb)>::category;

        rainy::type_traits::type_properties::is_invocable_v<int, decltype(my_lmb), int>;
        rainy::type_traits::primary_types::function_traits<decltype(my_lmb)>::valid;

    }
    
    {
        constexpr int i = my_test();
        std::vector<int> a = {1, 2, 3, 4, 5};
        auto p = rainy::utility::mem_fn(rainy::utility::overload_cmethod(&std::vector<int>::at));
        p(a, 0);
        // rainy::utility::iterator<typename std::vector<int>::iterator> iterator_begin(a.begin());
        // rainy::utility::iterator<typename std::vector<int>::iterator> iterator_end(a.end());
        // rainy::collections::array<int, 5> arr = {1, 2, 3, 4, 5};
        // rainy::utility::reverse_iterator<typename std::vector<int>::iterator> r_begin(a.end());
        // rainy::utility::reverse_iterator<typename std::vector<int>::iterator> r_end(a.begin());
        // for (; r_begin != r_end; ++r_begin) {
        //     std::cout << *r_begin << "\n";
        // }
        rainy::collections::array<int, 5> my_array = {1, 2, 3, 4, 5};
        auto it = my_array.end();
        //std::cout << *it << "\n";
        constexpr int o = it_test();        
    }

    //rainy_test::test();
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
    //rainy::foundation::system::async::implements::init_async_moudle_abi();
    //auto thread_instance = rainy::foundation::comint::create_instance<rainy::foundation::system::async::implements::native_thread>();



    //thread_instance->start(nullptr, 0, my_function, nullptr, 0);
    //rainy::foundation::system::async::implements::abi::thread_sleep_for(1);
    //std::cout << thread_instance.use_count() << "\n";
    //std::cout << thread_instance->get_id() << "\n";
    //std::shared_ptr<rainy::foundation::comint::the_unknown> move_ptr;
    //thread_instance->move(move_ptr);
    //std::cout << thread_instance->get_id() << "\n";
    //std::cout << std::dynamic_pointer_cast<rainy::foundation::system::async::implements::native_thread>(move_ptr)->get_id() << "\n";
    //std::dynamic_pointer_cast<rainy::foundation::system::async::implements::native_thread>(move_ptr)->join();
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
    foo a = 2;
    foo b = ++a;
    std::cout << a.val << "\n";
    std::cout << b.val << "\n";
    run_visit_test();
    rainy::utility::any any_object = 10;
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

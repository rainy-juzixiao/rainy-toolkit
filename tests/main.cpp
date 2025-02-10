#include <set>
#include <iostream>
#include <rainy/core/core.hpp>
#include <rainy/frozen/containers/vector.hpp>
#include <rainy/internals/reflection_core/method.hpp>
#include <rainy/winapi/api_core.h>
#include <rainy/winapi/environment.hpp>
#include <any>

struct test {
    std::string sample_function(std::string name, int age, std::vector<std::string> hobbits) {
        return name;
   }

    std::string another(std::string name, int age) {
        return name;
   }

    void test_ref(int &v) {
        v = 666;
   }

    void my_test() {
   }
};
 
template <typename Fn, typename... Args>
void benchmark(const char *const label, Fn &&func, Args &&...args) {
    const auto start = std::chrono::high_resolution_clock::now();
    (void) std::forward<Fn>(func)(std::forward<Args>(args)...);
    const auto end = std::chrono::high_resolution_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << label << " took " << time.count() << " nanoseconds\n";
}

using type_name_prober = void;

constexpr std::string_view type_name_prober_ = "void";

template <typename Ty>
constexpr std::string_view wrapped_type_name() {
    constexpr bool always_false = rainy::type_traits::internals::always_false<Ty>;
    static_assert(!always_false);
#if RAINY_USING_CLANG || RAINY_USING_GCC
    return __PRETTY_FUNCTION__;
#elif RAINY_USING_MSVC
    return __FUNCSIG__;
#else
    static_assert(always_false, "unsuporrted complier");
#endif
}

static constexpr std::size_t wrapped_type_name_prefix_length() {
    return wrapped_type_name<type_name_prober>().find(type_name_prober_);
}

static constexpr std::size_t wrapped_type_name_suffix_length() {
    return wrapped_type_name<type_name_prober>().length() - wrapped_type_name_prefix_length() - type_name_prober_.length();
}

template <auto Variable>
constexpr std::string_view wrapped_variable_name() {
#if RAINY_USING_CLANG || RAINY_USING_GCC
    return __PRETTY_FUNCTION__;
#elif RAINY_USING_MSVC
    return __FUNCSIG__;
#else
    static_assert(false, "unsupported compiler");
#endif
}

template <typename Ty>
constexpr std::string_view type_name() {
    constexpr auto wrapped_name = wrapped_type_name<Ty>();
    constexpr auto prefix_length = wrapped_type_name_prefix_length();
    constexpr auto suffix_length = wrapped_type_name_suffix_length();
    constexpr auto type_name_length = wrapped_name.length() - prefix_length - suffix_length;
    return wrapped_name.substr(prefix_length, type_name_length);
}

template <auto Variable>
inline constexpr std::string_view variable_name() {
#if RAINY_USING_MSVC
    constexpr std::string_view func_name = __FUNCSIG__;
#else
    constexpr std::string_view func_name = __PRETTY_FUNCTION__;
#endif
#if RAINY_USING_CLANG
    auto split = func_name.substr(0, func_name.size() - 2);
    return split.substr(split.find_last_of(":.") + 1);
#elif RAINY_USING_GCC
    auto split = func_name.substr(0, func_name.rfind(")}"));
    return split.substr(split.find_last_of(":") + 1);
#elif RAINY_USING_MSVC
    auto split = func_name.substr(func_name.rfind("variable_name<") + 13);
    auto split_again = split.substr(split.rfind("->") + 2);
    return split_again.substr(0, split_again.rfind(">(void)"));
#else
    static_assert(false, "You are using an unsupported compiler. Please use GCC, Clang or MSVC");
#endif
}

RAINY_CONSTEXPR20 int frozen_test() {
     rainy::containers::frozen::frozen_vector<int, 10> vec{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
     return vec[0];
}
 
namespace rainy_reflection_impl {
    struct arguments_base {
        virtual std::size_t arguments_count() const noexcept = 0;
        virtual bool is_view() const noexcept = 0;
        virtual ~arguments_base() = default;
    };

    template <typename... Args>
    class arguments final : public arguments_base {
    public:
        arguments(Args &&...args) {
            if constexpr (is_large) {
                ::new (buffer) std::tuple<Args...>(rainy::utility::forward<Args>(args)...);
                tuple_ptr = reinterpret_cast<std::tuple<Args...> *>(buffer);
            } else {
                tuple_ptr =
                    rainy::foundation::system::memory::make_nebula<std::tuple<Args...>>(rainy::utility::forward<Args>(args)...);
            }
        }

        ~arguments() final {
            if (!soo_active()) {
                tuple_ptr.reset();
            }
            (void) tuple_ptr.release();
        }

        bool is_view() const noexcept override {
            return false;
        }

        RAINY_NODISCARD std::size_t arguments_count() const noexcept override {
            return sizeof...(Args);
        }

        RAINY_NODISCARD const std::tuple<Args...> &get_args() const {
            return *tuple_ptr;
        }

        RAINY_NODISCARD bool soo_active() const noexcept {
            return tuple_ptr.get() == reinterpret_cast<const std::tuple<Args...> *>(&buffer);
        }

        template <std::size_t Idx, typename Ty,
                  rainy::type_traits::other_trans::enable_if_t<
                      rainy::type_traits::type_relations::is_convertible_v<Ty, decltype(std::get<Idx>())>, int> = 0>
        void change_argument(Ty arugment) noexcept(std::is_nothrow_assignable_v<Ty, decltype(std::get<Idx>())>) {
            std::get<Idx>(*tuple_ptr) = arugment;
        }

        template <std::size_t Idx, typename = decltype(std::get<Idx>())>
        RAINY_NODISCARD decltype(auto) get_arg() const {
            return std::get<Idx>(*tuple_ptr);
        }

    private:
        static constexpr size_t buffer_size = 64;
        static constexpr bool is_large = sizeof(std::tuple<Args...>) <= buffer_size;
        alignas(std::max_align_t) char buffer[buffer_size]{};
        rainy::foundation::system::memory::nebula_ptr<std::tuple<Args...>> tuple_ptr{nullptr};
    };

    template <typename... Args>
    arguments<Args...> make_arguments(Args... args) {
        return arguments<Args...>(args...);
    }

    template <typename... Args>
    class arguments_view final : public arguments_base {
    public:
        arguments_view(std::tuple<Args...> &ref_tuple) : tuple_view(ref_tuple) {
        }

        arguments_view(const std::tuple<Args...> &ref_tuple) : tuple_view(ref_tuple) {
        }

        ~arguments_view() final = default;

        bool is_view() const noexcept override {
            return true;
        }

        RAINY_NODISCARD std::size_t arguments_count() const noexcept override {
            return sizeof...(Args);
        }

        RAINY_NODISCARD const std::tuple<Args...> &get_args() const {
            return tuple_view.get();
        }

        template <std::size_t Idx, typename Ty,
                  rainy::type_traits::other_trans::enable_if_t<
                      rainy::type_traits::type_relations::is_convertible_v<Ty, decltype(std::get<Idx>())>, int> = 0>
        void change_argument(Ty arugment) noexcept(std::is_nothrow_assignable_v<Ty, decltype(std::get<Idx>())>) {
            std::get<Idx>(tuple_view.get()) = arugment;
        }

        template <std::size_t Idx, typename = decltype(std::get<Idx>())>
        RAINY_NODISCARD decltype(auto) get_arg() const {
            return std::get<Idx>(tuple_view.get());
        }

    private:
        rainy::utility::reference_wrapper<std::tuple<Args...>> tuple_view;
    };

    template <typename... Args>
    arguments_view<Args...> make_arguments_view(std::tuple<Args...> &ref_tuple) {
        return arguments_view(ref_tuple);
    }

    template <typename... Args>
    const arguments_view<Args...> make_arguments_view(const std::tuple<Args...> &ref_tuple) {
        return arguments_view(ref_tuple);
    }

    template <typename Class, typename ReturnType, typename... Args>
    struct method_invoker {
        using any = rainy::containers::any;

        method_invoker() = default;

        method_invoker(ReturnType (Class::*invoker)(Args...)) noexcept : method(invoker) {
        }

        any invoke(void *obj, const arguments_base &params) const {
            if (params.is_view()) {
                rainy_ref typed_args = static_cast<const arguments_view<Args...> &>(params);
                rainy::utility::expects(params.arguments_count() == sizeof...(Args));
                if constexpr (std::is_void_v<ReturnType>) {
                    std::apply([obj, this, &typed_args](auto &...params) { (static_cast<Class *>(obj)->*method)(params...); },
                               typed_args.get_args());
                    return {};
                } else {
                    return rainy::containers::any(std::apply(
                        [obj, this, &typed_args](auto &...params) { return (static_cast<Class *>(obj)->*method)(params...); },
                        typed_args.get_args()));
                }
            } else {
                rainy_ref typed_args = static_cast<const arguments<Args...> &>(params);
                rainy::utility::expects(params.arguments_count() == sizeof...(Args));
                if constexpr (std::is_void_v<ReturnType>) {
                    std::apply([obj, this, &typed_args](auto &...params) { (static_cast<Class *>(obj)->*method)(params...); },
                               typed_args.get_args());
                    return {};
                } else {
                    return rainy::containers::any(std::apply(
                        [obj, this, &typed_args](auto &...params) { return (static_cast<Class *>(obj)->*method)(params...); },
                        typed_args.get_args()));
                }
            }
        }

        ReturnType (Class::*method)(Args...);
    };

    template <typename ReturnType, typename... Args>
    struct method_invoker<void, ReturnType, Args...> {
        using any = rainy::containers::any;
        method_invoker() = default;
        template <typename Class>
        method_invoker(ReturnType (Class::*invoker)(Args...)) noexcept {
        }

        any invoke(void *, const std::vector<any> &) const {
            return {};
        }
    };

    struct invoker {
        virtual ~invoker() = default;
        virtual rainy::containers::any invoke(void *obj, const arguments_base &args) = 0;
    };

    template <typename Class, typename ReturnType, typename... Args>
    struct invoker_impl : invoker {
        template <typename Method>
        invoker_impl(Method method) {
            rainy::utility::construct_at(&invoker, method);
        }

        rainy::containers::any invoke(void *obj, const arguments_base &args) override {
            return invoker.invoke(obj, args);
        }

        method_invoker<Class, ReturnType, Args...> invoker;
    };

    class method {
    public:
        template <typename Class, typename ReturnType, typename... Args>
        method(ReturnType (Class::*method)(Args...)) {
            invoker_ =
                rainy::utility::construct_at(reinterpret_cast<invoker_impl<Class, ReturnType, Args...> *>(invoker_storage), method);
        }

        rainy::containers::any invoke(void *obj, const arguments_base &args) {
            return invoker_->invoke(obj, args);
        }

    private:
        alignas(std::max_align_t) char invoker_storage[24]{};
        invoker *invoker_{nullptr};
    };
}

#include <format>
//#include <rainy/experimental/format.hpp>
//#include <rainy/diagnostics/contract.hpp>

constexpr int ceval() {
    rainy::utility::contract(true, "assertion", "contract it hit", rainy::utility::contract_option::use_compile_time);
    return 0;
}
#include <rainy/user/hash/sha.hpp>

//#include <rainy/winapi/ui/notifcations.hpp>
#include <rainy/component/logger/message.hpp>

int add(int a, int b) {
    return a + b;
}

#include <rainy/foundation/io/print.hpp>

#if RAINY_USING_WINDOWS
int main() {
    rainy::foundation::functional::function_pointer myf = &add;
    myf = nullptr;
    myf = &add;
    std::cout << myf(10, 20) << std::endl;



    std::function<int(HWND, LPCSTR, LPCSTR, UINT)> f;
    rainy::foundation::system::dynamic_library::load_runtime_fn("User32.dll", "MessageBoxA", f);
    f(NULL, "111", "111", MB_OK);
    
    rainy::foundation::functional::function_pointer<int(HWND, LPCSTR, LPCSTR, UINT)> rf;
    rainy::foundation::system::dynamic_library::load_runtime_fn("User32.dll", "MessageBoxA", rf);
    rf(NULL, "111", "111", MB_OK);

    int(*raw_ptr)(HWND,LPCSTR,LPCSTR,UINT);
    rainy::foundation::system::dynamic_library::load_runtime_fn("User32.dll", "MessageBoxA", raw_ptr);
    raw_ptr(NULL, "111", "111", MB_OK);
    {
        auto handle = rainy::foundation::system::dynamic_library::load_library("User32.dll");
        if (handle.code == rainy::foundation::system::dynamic_library::load_library_errcode::ok) {
            rainy::winapi::dynamic_library::load_runtime_fn(handle.handle, "MessageBoxA", rf);
        }
        rainy::winapi::dynamic_library::internals::load_runtime_fn_proxy<int (*)(HWND, LPCSTR, LPCSTR, UINT)>(
            "User32.dll", "MessageBoxA", raw_ptr);
        std::is_same_v<decltype(raw_ptr), int (*)(HWND, LPCSTR, LPCSTR, UINT)>;
           
    }
    constexpr int eval = ceval();
    benchmark("any test", []() {
        for (int i = 0; i < 10; i++) {

            benchmark("rainy's toolkit any", []() {
                for (int i = 0; i < 100000; ++i) {
                    rainy::containers::any sti_any(std::in_place_type<std::string>, "Hello World");
                    rainy::utility::any_cast<std::string>(&sti_any)->append("Who");
                }
            });
            benchmark("standard-libray any", []() {
                for (int i = 0; i < 100000; ++i) {
                    std::any rtti_any(std::in_place_type<std::string>, "Hello World");
                    std::any_cast<std::string>(&rtti_any)->append("Who");
                }
            });

            std::endl(std::cout);
        }
    });

    auto my_arugment_tuple = std::make_tuple(std::string{"Bob"}, 18, std::vector<std::string>{"打", "看书", "玩游戏"});
    auto argument_view = rainy_reflection_impl::make_arguments_view(my_arugment_tuple);
    rainy_reflection_impl::method method(&test::sample_function);
    benchmark("view test", [&]() {
        for (int i = 0; i < 100000; ++i) {
            method.invoke(nullptr, argument_view);
        }
    });
    {
        std::size_t times = 10;
        for (std::size_t i = 0; i < 5; ++i, times *= 10) {
            std::cout << times << " times invoke test" << '\n';
            /* Rainy's Reflection */
            auto old_method = rainy::foundation::reflection::make_method("sample_function", &test::sample_function);
            auto old_test = rainy::foundation::reflection::make_method("sample_function", &test::my_test);
            /* Rainy's New Reflection */
            rainy_reflection_impl::method method(&test::sample_function);
            rainy_reflection_impl::method my_test(&test::my_test);
            int value{};
            std::cout << "-------invoke test-------" << "\n";
            benchmark("rainy's reflection", [&]() {
                for (int i = 0; i < times; ++i) {
                    old_method.invoke_with_paramspack(nullptr, std::string{"Bob"}, 18,
                                                      std::vector<std::string>{"打", "看书", "玩游戏"});
                }
            });
            benchmark("rainy's new reflection", [&]() {
                for (int i = 0; i < times; ++i) {
                    method.invoke(nullptr, rainy_reflection_impl::arguments(std::string{"Bob"}, 18,
                                                                            std::vector<std::string>{"打", "看书", "玩游戏"}));
                }
            });

            benchmark("rainy's reflection", [&]() {
                for (int i = 0; i < times; ++i) {
                    old_method.invoke_with_paramspack(nullptr, std::string{"Bob"}, 18,
                                                      std::vector<std::string>{"打", "看书", "玩游戏"});
                }
            });
            benchmark("rainy's new reflection", [&]() {
                for (int i = 0; i < times; ++i) {
                    method.invoke(nullptr, rainy_reflection_impl::arguments(std::string{"Bob"}, 18,
                                                                            std::vector<std::string>{"打", "看书", "玩游戏"}));
                }
            });

            benchmark("rainy's reflection 单次调用", [&]() {
                old_method.invoke_with_paramspack(nullptr, std::string{"Bob"}, 18, std::vector<std::string>{"打", "看书", "玩游戏"});
            });
            benchmark("rainy's new reflection 单次调用", [&]() {
                method.invoke(nullptr, rainy_reflection_impl::arguments(std::string{"Bob"}, 18,
                                                                        std::vector<std::string>{"打", "看书", "玩游戏"}));
            });

            benchmark("rainy's reflection void() 单次调用", [&]() { old_test.invoke_with_paramspack(nullptr); });
            benchmark("rainy's new reflection void() 单次调用",
                      [&]() { my_test.invoke(nullptr, rainy_reflection_impl::make_arguments()); });
            benchmark("rainy's reflection 单次调用 with void()", [&]() {
                for (int i = 0; i < times; ++i) {
                    old_test.invoke_with_paramspack(nullptr);
                }
            });
            benchmark("rainy's new reflection void() 单次调用", [&]() {
                for (int i = 0; i < times; ++i) {
                    my_test.invoke(nullptr, rainy_reflection_impl::make_arguments());
                }
            });
            std::cout << "-------test again-------" << "\n";
            benchmark("rainy's reflection", [&]() {
                for (int i = 0; i < times; ++i) {
                    old_method.invoke_with_paramspack(nullptr, std::string{"Bob"}, 18,
                                                      std::vector<std::string>{"打", "看书", "玩游戏"});
                }
            });
            benchmark("rainy's new reflection", [&]() {
                for (int i = 0; i < times; ++i) {
                    method.invoke(nullptr, rainy_reflection_impl::arguments(std::string{"Bob"}, 18,
                                                                            std::vector<std::string>{"打", "看书", "玩游戏"}));
                }
            });

            benchmark("rainy's reflection", [&]() {
                for (int i = 0; i < times; ++i) {
                    old_method.invoke_with_paramspack(nullptr, std::string{"Bob"}, 18,
                                                      std::vector<std::string>{"打", "看书", "玩游戏"});
                }
            });
            benchmark("rainy's new reflection", [&]() {
                for (int i = 0; i < times; ++i) {
                    method.invoke(nullptr, rainy_reflection_impl::arguments(std::string{"Bob"}, 18,
                                                                            std::vector<std::string>{"打", "看书", "玩游戏"}));
                }
            });

            benchmark("rainy's reflection 单次调用", [&]() {
                old_method.invoke_with_paramspack(nullptr, std::string{"Bob"}, 18, std::vector<std::string>{"打", "看书", "玩游戏"});
            });
            benchmark("rainy's new reflection 单次调用", [&]() {
                method.invoke(nullptr, rainy_reflection_impl::arguments(std::string{"Bob"}, 18,
                                                                        std::vector<std::string>{"打", "看书", "玩游戏"}));
            });

            benchmark("rainy's reflection void() 单次调用", [&]() { old_test.invoke_with_paramspack(nullptr); });
            benchmark("rainy's new reflection void() 单次调用",
                      [&]() { my_test.invoke(nullptr, rainy_reflection_impl::make_arguments()); });
            benchmark("rainy's reflection void() 单次调用", [&]() {
                for (int i = 0; i < times; ++i) {
                    old_test.invoke_with_paramspack(nullptr);
                }
            });
            benchmark("rainy's new reflection void() 单次调用", [&]() {
                for (int i = 0; i < times; ++i) {
                    my_test.invoke(nullptr, rainy_reflection_impl::make_arguments());
                }
            });
            std::endl(std::cout);
        }
    }
    rainy::information::byte_t byte{};
    {
        rainy::utility::scope_guard<rainy::information::byte_t,
                                    rainy::foundation::system::memory::default_deleter<rainy::information::byte_t>>
            scope_guard(new rainy::information::byte_t);
    }
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
}

#endif


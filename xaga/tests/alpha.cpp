#include <rainy/foundation/system/memory/allocator.hpp>
#include <rainy/utility/iterator.hpp>
#include <rainy/collections/array.hpp>
#include <rainy/foundation/io/stream_print.hpp>
#include <iostream>

template<typename Ty>
class vector_base {
public:
    template<typename, typename>
    class vector;

    using value_type = Ty;
    using pointer = value_type *;
    using size_type = std::size_t;

    RAINY_NODISCARD RAINY_CONSTEXPR20 size_type size() const noexcept {
        return last - begin;
    }

    RAINY_NODISCARD RAINY_CONSTEXPR20 size_type capacity() const noexcept {
        return last - end;
    }

private:
    pointer begin;
    pointer last;
    pointer end;
};

template<typename Ty, typename Alloc = rainy::foundation::system::memory::allocator<Ty>>
class vector : private vector_base<Ty> {
public:
    using value_type = Ty;
    using pointer = value_type *;
    using allocator_type = Alloc;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type &;
    using const_reference = const value_type &;
    using const_pointer = const value_type *;


private:


};

constexpr int test() {
    rainy::foundation::system::memory::allocator<int> alloc;
    auto memory = rainy::foundation::system::memory::implements::std_allocator_traits<rainy::foundation::system::memory::allocator<int>>::allocate(
            alloc, 1);
    *memory = 10;
    int ret = *memory;
    rainy::foundation::system::memory::implements::std_allocator_traits<rainy::foundation::system::memory::allocator<int>>::deallocate(
            alloc, memory, 1);
    return ret;
}

struct Student {
    int id;
    char name[50];
    float score;
};

void write() {
    Student student1;
    student1.id = 101;
    strcpy(student1.name, "Alice");
    student1.score = 89.5;

    // 打开文件以二进制方式写入
    FILE *file = fopen("student.dat", "wb");
    if (file == NULL) {
        printf("无法打开文件!\n");
        return;
    }

    // 使用 fwrite 函数写入结构体数据到文件
    size_t written = fwrite(&student1, sizeof(struct Student), 1, file);
    if (written == 1) {
        printf("数据成功写入文件！\n");
    } else {
        printf("数据写入失败！\n");
    }

    // 关闭文件
    fclose(file);
}

void read() {
    FILE *file = fopen("student.dat", "rb");
    Student student{};
    rainy::core::pal::read_binray(file, &student, sizeof(student), 1);
    std::cout << student.id << "\n";
    std::cout << student.name << "\n";
}

void write1() {
    FILE *file = fopen("new.txt", "w");
    if (file == NULL) {
        std::printf("无法打开文件!\n");
        return;
    }

    // 使用 fwrite 函数写入结构体数据到文件
    std::wstring_view my_data = L"Hello World 你好 \n";
    size_t written = std::fwrite(my_data.data(), sizeof(my_data[0]), my_data.size(), stdout);
    if (written == 1) {
        std::printf("数据成功写入文件！\n");
    } else {
        std::printf("数据写入失败！\n");
    }

    // 关闭文件
    std::fclose(file);
}

void read1() {
    FILE *file = fopen("new.txt", "r");
    wchar_t buffer[100]{};
    std::fread(buffer, sizeof(wchar_t), 100, file);
    std::wcout << buffer << "\n";
    std::fclose(file);
}



void fun() {

}

#if RAINY_USING_WINDOWS
#include <rainy/winapi/system/multithread/mutex.h>

using namespace rainy::winapi::system::multithread;

std::atomic<int> errors_in_mtx_lock;
std::atomic<int> errors_in_mtx_lock_with_error_data;
std::atomic<int> errors_in_mtx_trylock_lock;
std::atomic<int> errors_in_mtx_rec_lock;

// 共享资源
int shared_counter{0};

// 测试函数：多个线程竞争加锁
void test_mtx_lock_multithread() {
    using namespace rainy::winapi::system::multithread;
    void* mtx{};
    mtx_create(&mtx, mutex_types::plain_mtx | mutex_types::try_mtx);
    const int num_threads = 10;
    const int increments_per_thread = 10;
    std::vector<std::thread> threads;
    // 启动多个线程
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&mtx]() {
            for (int j = 0; j < increments_per_thread; ++j) {
                win32_thread_result result = mtx_trylock(&mtx);
                if (result != win32_thread_result::success) {
                    if (result == win32_thread_result::busy) {
                        result = mtx_lock(&mtx);
                    }
                    if (result != win32_thread_result::success) {
                        errors_in_mtx_lock.fetch_add(1);
                        std::cout << "errors in mtx_lock -> " << errors_in_mtx_lock << "\n";
                    }
                }
                // 修改共享资源
                shared_counter++;
                result = mtx_unlock(&mtx);
                if (result != win32_thread_result::success) {
                    errors_in_mtx_lock++;
                    std::cout << static_cast<int>(result) << "\n";
                    std::cout << "errors in mtx_unlock -> " << errors_in_mtx_lock << "\n";
                }
            }
        });
    }

    // 等待所有线程完成
    for (auto &t: threads) {
        t.join();
    }

    // 验证共享资源的最终值
    if (shared_counter != num_threads * increments_per_thread) {
        errors_in_mtx_lock_with_error_data++;
        std::cout << "errors in mtx_lock : data -> " << errors_in_mtx_lock_with_error_data << "\n";
    }
    mtx_destroy(&mtx);
}

void test_rec_mtx_lock_multithread() {
    void *mtx{};
    // 创建递归锁
    mtx_create(&mtx, mutex_types::try_mtx | mutex_types::recursive_mtx);
    const int num_threads = 10;
    const int increments_per_thread = 10;
    std::vector<std::thread> threads;
    std::atomic<int> errors_in_mtx_lock{0};
    std::atomic<int> errors_in_mtx_lock_with_error_data{0};
    int shared_counter = 0;

    // 启动多个线程
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&mtx, &shared_counter, &errors_in_mtx_lock]() {
            for (int j = 0; j < increments_per_thread; ++j) {
                win32_thread_result result = mtx_trylock(&mtx);
                if (result != win32_thread_result::success) {
                    if (result == win32_thread_result::busy) {
                        result = mtx_lock(&mtx);
                    }
                    if (result != win32_thread_result::success) {
                        errors_in_mtx_lock.fetch_add(1);
                        std::cout << "errors in mtx_lock -> " << errors_in_mtx_lock << "\n";
                    }
                }
                // 递归加锁：模拟需要递归访问共享资源的情况
                win32_thread_result result_recursive = mtx_lock(&mtx); // 递归锁
                if (result_recursive != win32_thread_result::success) {
                    errors_in_mtx_lock.fetch_add(1);
                    std::cout << "Recursive lock error -> " << errors_in_mtx_lock << "\n";
                }

                // 修改共享资源
                shared_counter++;

                // 解锁
                result_recursive = mtx_unlock(&mtx); // 递归解锁
                if (result_recursive != win32_thread_result::success) {
                    errors_in_mtx_lock.fetch_add(1);
                    std::cout << "Recursive unlock error -> " << errors_in_mtx_lock << "\n";
                }

                result = mtx_unlock(&mtx); // 最外层解锁
                if (result != win32_thread_result::success) {
                    errors_in_mtx_lock.fetch_add(1);
                    std::cout << "errors in mtx_unlock -> " << errors_in_mtx_lock << "\n";
                }
            }
        });
    }

    // 等待所有线程完成
    for (auto &t: threads) {
        t.join();
    }

    // 验证共享资源的最终值
    if (shared_counter != num_threads * increments_per_thread) {
        errors_in_mtx_lock_with_error_data.fetch_add(1);
        std::cout << "errors in mtx_lock : data -> " << errors_in_mtx_lock_with_error_data << "\n";
    }
}

#endif

#include<bitset>

#include <rainy/foundation/pal/threading.hpp>


#include <array>

#if RAINY_USING_WINDOWS

template <typename Fn, typename... Args>
void benchmark(const char *const label, Fn &&func, Args &&...args) {
    const auto start = std::chrono::high_resolution_clock::now();
    (void) std::forward<Fn>(func)(std::forward<Args>(args)...);
    const auto end = std::chrono::high_resolution_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << label << " took " << time.count() << " milliseconds\n";
}


template <typename T, template <typename...> class Template>
struct is_specialization_of : std::false_type {};

template <template <typename...> class Template, typename... Args>
struct is_specialization_of<Template<Args...>, Template> : std::true_type {};

constexpr std::size_t test_chash() {
    std::string c = "Hello World";
    return constexpr_hash<std::string>{}(c);
}

template <typename T, typename = void>
struct has_constexpr_hash : std::false_type {};

template <typename T>
struct has_constexpr_hash<T, std::void_t<decltype(std::declval<constexpr_hash<T>>()(std::declval<T>(), 0))>> : std::true_type {};

int main() {
    hybrid_unordered_map<std::string, int, 1> fmap;
    std::unordered_map<std::string, int> m;
    std::cout << m.size() << '\n';
    volatile int it{};
    for (int i = 0; i < 10; ++i) {
        benchmark("std", [&]() {
            for (int i = 0; i < 100000; ++i) {
                for (const auto &i: m) {
                    it += i.second;
                }
            }
        });
        benchmark("hybrid", [&]() {
            for (int i = 0; i < 100000; ++i) {
                for (const auto &i: fmap) {
                    it += i.second;
                }
            }
        });
        std::endl(std::cout);
    }
    std::array<std::pair<std::string, int>, 80> std_array;
    rainy::collections::array<std::pair<std::string, int>, 80> rainy_array;
    for (auto it = m.begin(); it != m.end(); it++) {
        std::cout << it->first << "\n";   
    }
    using omap = decltype(fmap);
    std::cout << fmap.size() << "\n";
    fmap.empty();
    hybrid_unordered_map<std::string, int, 1, constexpr_hash<std::string>> h;
    has_constexpr_hash<std::string>::value;
    /*for (const auto &i: fmap.occupied) {
        std::cout << i << "\n";
    }*/
    /* for (const auto &i: fmap) {
        std::cout << i.first << '\n';
    }*/
    //std::cout << "Alice: " << fmap["Alice"] << std::endl;
    //fmap["Steve"] = 80;
    //fmap["Curis"] = 80;
    //std::cout << "Contains Bob? " << fmap.contains("Bob") << std::endl;
    
    /*for (const auto &i: fmap) {
        std::cout << i.first << "\n";
    }*/
    //auto it = fmap.begin();
    /*for (int i = 0; i < 100000; ++i) {
        if (i == 0) {
            std::cout << "1 times\n";
        } else if (i % 1000 == 0) {
            std::cout << i <<"times\n";
        }
        shared_counter = 0;
        test_mtx_lock_multithread();
    }*/

    //std::cout << "All multithreaded tests passed!" << std::endl;
    std::cout << "是否为主线程： " << rainy::core::is_main_thread() << "\n";
    std::thread thread([]() {
        std::cout << "jthread---" << "\n";
        std::cout << "是否为主线程： " << rainy::core::is_main_thread() << "\n";
    });
    thread.join();
    std::thread thread1([]() {
        std::cout << "jthread---" << "\n";
        std::cout << "是否为主线程： " << rainy::core::is_main_thread() << "\n";
    });
    thread1.join();
    // std::allocator_traits<std::allocator<int>>;
    // std::vector<int>::size;
    // std::allocator_traits<rainy::foundation::system::memory::allocator<int>>::propagate_on_container_move_assignment;
    // rainy::utility::implements::has_to_address<rainy::utility::const_iterator<int*>>;
    rainy::collections::array<int, 5> a{1, 2, 3, 4, 5};
    for (const auto &i: a) {
        rainy::foundation::io::stdout_print(i, " ");
    }
    rainy::foundation::io::stdout_println();
    return 0;
}
#else

#include <rainy/linux_api/system/multithread/thread.h>

unsigned int test_thread(void* arg) {
    std::cout << *static_cast<int*>(arg) << "\n";
    std::cout << "jthread---" << "\n";
    std::cout << rainy::linux_api::system::multithread::get_thread_id() << "\n";
    return 0;
}

int main() {
    auto thread = rainy::foundation::pal::threading::make_thread([](int a,int b ,int c){
        std::cout<< (a+b+c)<< '\n';
        return 0;
    },1,2,3);
    thread.join();
    std::cout << "是否为主线程： " << rainy::core::is_main_thread() << "\n";
    return 0;
}

#endif
#include <rainy/foundation/system/memory/allocator.hpp>
#include <rainy/utility/iterator.hpp>
#include <rainy/containers/array.hpp>
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
    auto memory = rainy::foundation::system::memory::internals::std_allocator_traits<rainy::foundation::system::memory::allocator<int>>::allocate(
            alloc, 1);
    *memory = 10;
    int ret = *memory;
    rainy::foundation::system::memory::internals::std_allocator_traits<rainy::foundation::system::memory::allocator<int>>::deallocate(
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

template <typename Ty = void>
struct constexpr_hash {
    static_assert(std::is_integral<Ty>::value || std::is_enum<Ty>::value, "only supports integral types, specialize for other types");

    constexpr std::size_t operator()(Ty const &value, std::size_t seed) const {
        std::size_t key = seed ^ static_cast<std::size_t>(value);
        key = (~key) + (key << 21); // key = (key << 21) - key - 1;
        key = key ^ (key >> 24);
        key = (key + (key << 3)) + (key << 8); // key * 265
        key = key ^ (key >> 14);
        key = (key + (key << 2)) + (key << 4); // key * 21
        key = key ^ (key >> 28);
        key = key + (key << 31);
        return key;
    }
};

template <typename Elem, typename Traits, typename Alloc>
struct constexpr_hash<std::basic_string<Elem, Traits, Alloc>> {
    constexpr std::size_t operator()(const std::basic_string<Elem, Traits, Alloc> &value, std::size_t seed = 0) const {
        std::size_t hash_value = seed;
        for (auto c: value) {
            hash_value = (hash_value ^ static_cast<std::size_t>(c)) * 1099511628211U;
        }
        return hash_value;
    }
};

template <typename Key, typename Val, std::size_t N, typename Hasher = std::hash<Key>, typename KeyEqual = std::equal_to<Key>,
    bool UseCompileTimeVersion = 
        (std::is_integral_v<Key> || std::is_enum_v<Key> || rainy::type_traits::primary_types::is_specialization_v<Key,constexpr_hash>) && rainy::type_traits::type_relations::is_same_v<Hasher, constexpr_hash<Key>>
>
class hybrid_unordered_map {
public:
    template <typename Container, typename PredicateType, typename ProtoTypeGetData, typename ProtoTypeGetSize>
    class const_bidirectional_iterator {
    public:
        using container_type = Container;
        using iterator_traits =
            rainy::utility::make_iterator_traits<std::ptrdiff_t, std::bidirectional_iterator_tag, typename container_type::pointer,
                                                 typename container_type::reference, typename container_type::value_type>;
        using iterator_category = typename iterator_traits::iterator_category;
        using value_type = typename iterator_traits::value_type;
        using difference_type = typename iterator_traits::difference_type;
        using pointer = typename iterator_traits::pointer;
        using reference = typename iterator_traits::reference;
        using const_reference = const value_type &;
        using const_pointer = const value_type *;

        const_bidirectional_iterator(container_type *container, std::size_t idx) : container(container), current_index(idx) {
            find_next_occupied_forward();
        }

        const value_type &operator*() {
            return get_data(current_index);
        }

        const_bidirectional_iterator &operator++() {
            ++current_index;
            find_next_occupied_forward();
            return *this;
        }

        const_bidirectional_iterator &operator--() {
            --current_index;
            find_next_occupied_backward();
            return *this;
        }

        bool operator!=(const const_bidirectional_iterator &other) const {
            return current_index != other.current_index;
        }

    private:
        void find_next_occupied_forward() {
            while (current_index < get_size() && !predicate(current_index)) {
                ++current_index;
            }
        }

        void find_next_occupied_backward() {
            while (current_index > 0 && !predicate(current_index - 1)) {
                --current_index;
            }
        }

        bool predicate(std::size_t current_index) const noexcept {
            return functions.get_first()(container, current_index);
        }

        const_reference get_data(std::size_t current_index) {
            return functions.get_second().get_first()(container, current_index);
        }

        std::size_t get_size() {
            return functions.get_second().get_second()(container);
        }

        Container *container;
        std::size_t current_index;
        rainy::utility::compressed_pair<PredicateType, rainy::utility::compressed_pair<ProtoTypeGetData, ProtoTypeGetSize>> functions;
    };

    struct is_occupied_pred;
    struct data_getter;
    struct get_size;

    using key_type = Key;
    using mapped_type = Val;
    using value_type = std::pair<const key_type, mapped_type>;
    using hasher = Hasher;
    using key_equal = KeyEqual;
    using size_type = std::size_t;
    using reference = value_type &;
    using const_reference = const value_type &;
    using pointer = value_type *;
    using const_pointer = const value_type *;
    using iterator = const_bidirectional_iterator<hybrid_unordered_map,is_occupied_pred,data_getter,get_size>;
    using const_iterator = const_bidirectional_iterator<hybrid_unordered_map,is_occupied_pred,data_getter,get_size>;

    struct is_occupied_pred {
        bool operator()(const hybrid_unordered_map *this_pointer, std::size_t idx) const {
            return this_pointer->occupied[idx];
        }
    };

    struct data_getter {
        const_reference operator()(const hybrid_unordered_map *this_pointer, std::size_t idx) const {
            return this_pointer->data[idx];
        }
    };
    struct get_size {
        std::size_t operator()(const hybrid_unordered_map *this_pointer) const {
            return this_pointer->max_size();
        }
    };

    iterator begin() {
        return iterator{this, 0};
    }

    iterator end() {
        return iterator{this, N};
    }

   hybrid_unordered_map(std::initializer_list<value_type> init_list) :
        occupied{}, occupied_count(0), data{}, hasher_{}, key_equal_{} {
        for (const auto &item: init_list) {
            std::size_t index = find_index(item.first);
            if (!occupied[index]) {
                rainy::utility::construct_at(&data[index], rainy::utility::move(item));
                occupied[index] = true;
                ++occupied_count;
            }
        }
    }

    hybrid_unordered_map() : occupied{}, occupied_count(0), data{}, hasher_{}, key_equal_{} {
    }

    hybrid_unordered_map(const hybrid_unordered_map &) = default;
    hybrid_unordered_map &operator=(const hybrid_unordered_map &) = default;
    hybrid_unordered_map(hybrid_unordered_map &&) noexcept = default;
    hybrid_unordered_map &operator=(hybrid_unordered_map &&) noexcept = default;

    mapped_type &operator[](const Key &key) {
        std::size_t index = find_index(key);
        if (!occupied[index]) {
            insert(key, mapped_type{});
            index = find_index(key);
        }
        return data[index].second;
    }

    const mapped_type &operator[](const Key &key) const {
        return at(key);
    }

    Val &at(const Key &key) {
        std::size_t index = find_index(key);
        if (!occupied[index]) {
            throw std::out_of_range("Key not found");
        }
        return data[index].second;
    }

    const Val &at(const Key &key) const {
        std::size_t index = find_index(key);
        if (!occupied[index]) {
            throw std::out_of_range("Key not found");
        }
        return data[index].second;
    }

    bool contains(const Key &key) const {
        std::size_t hash_value = hasher_(key);
        std::size_t index = hash_value % N;
        std::size_t original_index = index;
        std::size_t probe = 0;
        do {
            if (!occupied[index]) {
                return false;
            }
            if (key_equal_(data[index].first, key)) {
                return true;
            }
            probe++;
            index = (hash_value + probe) % N;
        } while (index != original_index);
        return false;
    }

    void clear() {
        occupied.fill(false);
        occupied_count = 0;
    }

    size_type bucket(const Key &key) const {
        return find_index(key);
    }

    std::size_t size() const noexcept {
        return occupied_count;
    }

    bool empty() const noexcept {
        return size() == 0;
    }

    constexpr size_type max_size() const noexcept {
        return N;
    }

    float load_factor() const noexcept {
        return static_cast<float>(size()) / N;
    }

private:
    rainy::containers::array<std::pair<const Key, Val>, N> data;
    rainy::containers::array<bool, N> occupied;
    std::size_t occupied_count;
    rainy::utility::compressed_pair<hasher, key_equal> utils;
    hasher hasher_;
    key_equal key_equal_;

    std::size_t find_index(const Key &key) const {
        std::size_t hash_value = hasher_(key);
        std::size_t index = hash_value % N;
        std::size_t original_index = index;
        std::size_t probe = 0;
        do {
            if (!occupied[index] || key_equal_(data[index].first, key)) {
                return index;
            }
            probe++;
            index = (hash_value + probe) % N;
        } while (index != original_index);
        throw std::runtime_error("Map is full");
    }
};

template <typename Key, typename Val, std::size_t N, typename Hasher,typename KeyEqual>
class hybrid_unordered_map<Key, Val, N, Hasher, KeyEqual, true> {

};

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
    rainy::containers::array<std::pair<std::string, int>, 80> rainy_array;
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
    // rainy::utility::internals::has_to_address<rainy::utility::const_iterator<int*>>;
    rainy::containers::array<int, 5> a{1, 2, 3, 4, 5};
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
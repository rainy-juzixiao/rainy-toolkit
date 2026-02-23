#include <rainy/foundation/concurrency/atomic.hpp>
#include <iostream>
using namespace rainy;

int main() {
    rainy::foundation::concurrency::atomic<float> atomics = 10;
    std::cout << atomics << '\n';
    std::cout << atomics + 3.14f << '\n';
    std::cout << atomics + 3.14 << '\n';
    std::cout << atomics.fetch_sub(10) << '\n';
    std::cout << atomics << '\n';
    std::cout << atomics.fetch_sub(10) << '\n';
    rainy::foundation::concurrency::atomic_fetch_add(&atomics, 100);
    std::cout << atomics << '\n';   
    return 0;
}

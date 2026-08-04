/*
* Copyright 2026 rainy-juzixiao
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <rainy/core/platform.hpp>

namespace rainy::core::layer {
    bool is_aligned(void *const ptr, const std::size_t alignment) {
        return (reinterpret_cast<uintptr_t>(ptr) & (alignment - 1)) == 0;
    }

    void *allocate(std::size_t size) noexcept {
        if (size == 0) {
            return nullptr;
        }
        return operator new[](size, std::nothrow);
    }

    void *allocate(const std::size_t size, const std::size_t alignment) noexcept {
        if (size == 0) {
            return nullptr;
        }
#ifdef __cpp_aligned_new
        return operator new[](size, std::align_val_t{alignment}, std::nothrow); // 由调用它的人，负责处理分配失败的问题
#else
        const std::size_t offset = alignment - 1 + sizeof(void *);
        const std::size_t total = size + offset;
        void *ptr = operator new[](total, std::nothrow);
        if (!ptr) {
            return nullptr; // 由调用它的人，负责处理分配失败的问题
        }
        rainy_const raw_location = reinterpret_cast<std::size_t>(ptr);
        const std::size_t aligned_location = (raw_location + offset) & ~(alignment - 1);
        rainy_let aligned_ptr = reinterpret_cast<void *>(aligned_location);
        *(reinterpret_cast<void **>(aligned_location - sizeof(void *))) = ptr;
        return aligned_ptr;
#endif
    }

    void deallocate(void *block) {
        if (!block) {
            return;
        }
        operator delete[](block);
    }

    void deallocate(void *block, const std::size_t alignment) {
        if (!block) {
            return;
        }
#ifdef __cpp_aligned_new
        operator delete[](block, std::align_val_t{alignment});
#else
        if (is_aligned(block, alignment)) {
            rainy_const aligned_location = reinterpret_cast<std::size_t>(block);
            void *original_ptr = *(reinterpret_cast<void **>(aligned_location - sizeof(void *)));
            operator delete[](original_ptr);
        }
#endif
    }

    void deallocate(void *block, const std::size_t byte_count, const std::size_t alignment) {
        if (!block || byte_count == 0) {
            return;
        }
#ifdef __cpp_aligned_new
        operator delete[](block, byte_count, std::align_val_t{alignment});
#else
        if (is_aligned(block, alignment)) {
            rainy_const aligned_location = reinterpret_cast<std::size_t>(block);
            void *original_ptr = *(reinterpret_cast<void **>(aligned_location - sizeof(void *)));
            operator delete[](original_ptr, byte_count);
        }
#endif
    }
}

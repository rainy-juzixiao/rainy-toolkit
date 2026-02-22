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
#ifndef RAINY_FOUNDATION_CONCURRENCY_HPP
#define RAINY_FOUNDATION_CONCURRENCY_HPP // NOLINT
/**
 * @file concurrency.hpp
 * @brief 此头文件是rainy-toolkit用于实现跨平台线程API的库，若要访问内部的API，请参阅CONCURRENCY文件夹的implements.hpp
 */
// NOLINTBEGIN
#include <chrono>
#include <rainy/core/core.hpp>
#include <rainy/foundation/concurrency/mutex.hpp>
#include <rainy/foundation/concurrency/thread.hpp>
#include <rainy/foundation/concurrency/tss_ptr.hpp>
#include <rainy/foundation/concurrency/condition_variable.hpp>
#include <rainy/foundation/concurrency/pool.hpp>
#include <rainy/foundation/concurrency/executor.hpp>
// NOLINTEND

#endif
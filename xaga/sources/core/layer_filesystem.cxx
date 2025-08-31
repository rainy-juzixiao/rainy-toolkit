/*
 * Copyright 2025 rainy-juzixiao
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
#include <rainy/core/core.hpp>

//#if RAINY_USING_WINDOWS
//#include <rainy/winapi/system/filesystem.hpp>
//#else
//#include <rainy/linux_api/system/filesystem.hpp>
//#endif
//
//// 每个pal接口，都会通过layer直接与对应平台的接口进行调用
//namespace rainy::core::pal::layer {
//#if RAINY_USING_WINDOWS
//    using namespace winapi::filesystem; // layer -> rainy::winapi::filesystem;
//#else
//    using namespace linux_api::filesystem; // layer -> rainy::linux_api::filesystem;
//#endif
//}
//
//#if RAINY_USING_WINDOWS
//
//namespace rainy::core::pal {
//    file_handle open_file(czstring filepath, open_mode mode) {
//        return layer::open_file(filepath, static_cast<layer::win32_file_open_mode>(mode));
//    }
//
//    bool close_file(file_handle handle) {
//        return layer::close_file(handle);
//    }
//
//    bool file_exists(czstring file_path) {
//        return layer::file_exists(file_path);
//    }
//
//    file_status get_file_status(czstring file_path) noexcept {
//        auto status = layer::get_file_status(file_path);
//        file_status result{};
//        result.attributes = static_cast<file_attributes>(status.attributes);
//        result.reparse_tag = static_cast<reparse_tag>(status.reparse_tag);
//        result.file_size = status.file_size;
//        result.last_write_time = status.last_write_time;
//        result.link_count = status.link_count;
//        return result;
//    }
//
//    file_type get_file_type(core::czstring file_path) noexcept {
//        return static_cast<file_type>(layer::get_file_type(file_path));
//    }
//}
//
//#else
//
//namespace rainy::core::pal {
//    file_handle open_file(czstring filepath, open_mode mode) {
//        return layer::open_file(filepath, static_cast<layer::linux_file_open_mode>(mode));
//    }
//
//    bool close_file(file_handle handle) {
//        return layer::close_file(handle);
//    }
//
//    bool file_exists(czstring file_path) {
//        return layer::file_exists(file_path);
//    }
//
//    file_status get_file_status(czstring file_path) noexcept {
//        auto status = layer::get_file_status(file_path);
//        file_status result{};
//        result.attributes = static_cast<file_attributes>(status.attributes);
//        result.reparse_tag = static_cast<reparse_tag>(status.reparse_tag);
//        result.file_size = status.file_size;
//        result.last_write_time = status.last_write_time;
//        result.link_count = status.link_count;
//        return result;
//    }
//
//    file_type get_file_type(core::czstring file_path) noexcept {
//        return static_cast<file_type>(layer::get_file_type(file_path));
//    }
//}
//
//#endif

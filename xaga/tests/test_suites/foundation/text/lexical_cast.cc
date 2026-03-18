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
#include <catch2/catch_all.hpp>
#include <iostream>
#include <rainy/foundation/text/lexical_cast.hpp>

#if RAINY_USING_WINDOWS
#include <windows.h>
#endif

#if 0

TEST_CASE("MY") {
#if RAINY_USING_WINDOWS
    SetConsoleOutputCP(CP_UTF8);
#endif
    using namespace rainy::foundation::text;
    std::cout << lexical_cast<bool>("false") << '\n';
    std::cout << lexical_cast<bool>(L"true") << '\n';
    std::cout << lexical_cast<bool>(u8"true") << '\n';
    std::cout << lexical_cast<bool>(u"true") << '\n';
    std::cout << lexical_cast<bool>(U"true") << '\n';
    std::cout << lexical_cast<bool>(U" [ true ] ") << '\n';
    std::cout << lexical_cast<bool>(U"[ false ] ") << '\n';

    std::cout << "----\n";
    std::cout << lexical_cast<char>("c") << '\n';
    std::cout << lexical_cast<char>(L"c") << '\n';
    std::cout << lexical_cast<char>(u8"c") << '\n';
    std::cout << lexical_cast<char>(U"c") << '\n';
    std::cout << lexical_cast<char>(u"c") << '\n';

    std::cout << "----\n";
    std::cout << lexical_cast<float>("3.14f") << '\n';
    std::cout << lexical_cast<float>(L"3.14f") << '\n';
    std::cout << lexical_cast<float>(u8"3.14f") << '\n';
    std::cout << lexical_cast<float>(u"3.14f") << '\n';
    std::cout << lexical_cast<float>(U"3.14f") << '\n';

    std::cout << "----\n";
    std::cout << lexical_cast<int>("3.14f") << '\n';
    std::cout << lexical_cast<int>(L"3.14f") << '\n';
    std::cout << lexical_cast<int>(u8"3.14f") << '\n';
    std::cout << lexical_cast<int>(u"3.14f") << '\n';
    std::cout << lexical_cast<int>(U"3.14f") << '\n';

    std::cout << "----\n";
    std::cout << lexical_cast<string>(L"Hello World 这是字符串") << '\n';
    wstring ws = L"This is rainy-toolkit string with 中文UTF-8✔";
    std::cout << lexical_cast<std::string>(ws.data()) << '\n';
}

#endif
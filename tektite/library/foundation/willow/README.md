# willow

willow 是 rainy-toolkit 的多语言统一文档标记配置协议库

## 快速入门

```cpp
#include <rainy/foundation/willow/json.hpp>

using namespace rainy::foundation::willow;

auto doc = json::parse(R"({"name": "rainy", "port": 8080})");
doc["name"].as_string();   // "rainy"
doc["port"].as_integer();  // 8080

json::facade<document> built = json::facade<document>::object({{"a", 1}});
json::dump(built);         // {"a":1}
json::dump(built, 2);      // pretty print，缩进 2
```

手写文档、跨语言转换：

```cpp
json::facade<document> doc = json::facade<document>::object({{"a", 1}});
doc["b"] = true;
yaml::facade<document> converted{from_other_document, doc};
converted["a"].as_integer();
```

不同字符/整数宽度的文档类型：`document`（char/int32）、`document64`、
`wdocument`（wchar_t）、`u16document`，以及 C++20 下的 `u8document`：

```cpp
auto wide = json::parse<wdocument>(LR"({"k": 1})");
wide[L"k"].as_integer();  // 1
```

## 支持的文档语言/协议

- ini
- json/json5
- yaml
- jsonnet (实现中)
- xml (实现中)
- toml (实现中)
- hjson (实现中)
- msgpack (实现中)
- bson (实现中)
- cbor (实现中)
- protobuf

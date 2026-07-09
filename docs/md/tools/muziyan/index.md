# MuZiYan（沐子言）文档生成器

MuZiYan（沐子言）是一个基于 Rust + libClang 开发的 C++ 文档生成工具。

## 概述

MuZiYan 通过解析 C++ 头文件中的 Doxygen 风格注释，自动生成结构化的文档。它直接使用 libClang 解析 AST，支持完整的 C++20/23 语法。

## 功能特性

- **多格式输出** — 支持 Markdown、HTML、LaTeX、XML、VitePress Markdown
- **国际化** — 文档注释支持多语言标签，输出可按语言切换
- **继承文档** — 自动继承基类文档，支持 `@using` 覆盖
- **命名空间过滤** — 可忽略 `implements`、`detail`、`impl` 等内部命名空间
- **宏注入** — 解析时自动注入 `__MUZIYAN_IS_HERE__`，头文件可检测文档生成环境
- **`@mergeto` 标签** — 支持将多个头文件合并到同一份文档
- **`@NODOCBEGIN / @NODOCEND`** — 控制区域是否纳入文档

## 配置

MuZiYan 通过根目录下的 `sleepy.yaml` 配置文件驱动：

```yaml
output_dir: docs/api
lang: chinese

ignored_namespaces:
  - implements
  - detail
  - impl

sources:
  - name: my_library
    include_dirs:
      - include/
    extensions:
      - h
      - hpp
    compile_flags:
      std: c++20
      defines: []
      includes: []
      extra: []
    files:
      - include/core/core.hpp
```

### 配置项说明

| 字段 | 说明 |
|------|------|
| `output_dir` | 文档输出目录 |
| `lang` | 文档语言（english / chinese / japanese） |
| `ignored_namespaces` | 要忽略的命名空间列表，其中的内容不会出现在文档中 |
| `sources` | 源码配置组，可配置多个 |
| `sources[].name` | 源名称，输出时作为子目录名 |
| `sources[].include_dirs` | 头文件搜索路径 |
| `sources[].extensions` | 要处理的文件后缀 |
| `sources[].compile_flags` | 编译参数（std / defines / includes / extra） |
| `sources[].files` | 精确指定文件列表（与 include_dirs 扫描二选一） |

## `__MUZIYAN_IS_HERE__`

MuZiYan 在解析时会自动向 clang 注入 `-D__MUZIYAN_IS_HERE__`。你可以在头文件中利用此宏条件编译仅在文档生成时可见的内容：

```cpp
#ifdef __MUZIYAN_IS_HERE__
/** @brief 仅在文档生成时可见 */
void doc_only_function();
#endif
```

## 标签使用示例

以下示例各自独立，每个标签按需单独使用，无需组合。

### 基础文档标签

`@brief` / `@description` 用于描述函数或类的作用：

```cpp
/// @brief 计算两个数值的平方和
/// @description 该函数会分别计算两个参数的平方，然后返回它们的和。
///              支持所有支持加减乘除运算的类型。
/// @param a 第一个数值
/// @param b 第二个数值
/// @return 两数平方之和
template <typename T>
auto sum_of_squares(T a, T b) -> decltype(a * a + b * b);
```

### 参数方向

`@param[in]` / `@param[out]` / `@param[in,out]` 标明参数是输入、输出还是读写：

```cpp
/// @brief 将字节序列解析为配置项
/// @param[in]  data      原始字节数据
/// @param[in]  size      数据长度
/// @param[out] output    解析后的配置结构体
/// @param[out] error_msg 解析失败时的错误信息
/// @return true 表示解析成功，false 表示失败
bool parse_config(const char* data, size_t size,
                  Config& output, std::string& error_msg);
```

### 模板参数

`@tparam` 用于说明模板参数的含义：

```cpp
/// @brief 通用的缓存容器
/// @tparam Key       键类型，要求可哈希
/// @tparam Value     值类型
/// @tparam Capacity  最大缓存条目数，默认 64
template <typename Key, typename Value, size_t Capacity = 64>
class lru_cache;
```

### 异常说明

`@throws` 标记函数可能抛出的异常：

```cpp
/// @brief 从文件路径读取内容
/// @param path 文件路径
/// @return 文件内容字符串
/// @throws std::filesystem_error 文件不存在或权限不足
/// @throws std::bad_alloc        内存不足
std::string read_file(const std::string& path);
```

### 返回值细节

`@retval` 描述特定返回值含义：

```cpp
/// @brief 尝试从队列中取出一个元素
/// @param[out] value 存储取出的元素
/// @return 操作结果
/// @retval true  成功取出一个元素
/// @retval false 队列为空，value 未被修改
bool try_pop(T& value);
```

### 注意事项与警告

```cpp
/// @brief 执行关键操作
/// @note  该操作可能耗时较长，建议在后台线程调用
/// @warning 调用前请确保资源已初始化
/// @attention 此接口将在下一版本中移除，请迁移至 new_api()
/// @remark 内部实现使用了写时复制技术
void critical_operation();
```

### 生命周期标签

```cpp
/// @brief 获取系统版本号
/// @since 2.0.0
/// @version 1.3
/// @date 2024-06-01
/// @author rainy-juzixiao
/// @deprecated 请使用 get_system_info() 替代
const char* get_version();
```

### 版权与许可

```cpp
/// @copyright Copyright (c) 2026 rainy-juzixiao
/// @license Apache License, Version 2.0
/// @see https://www.apache.org/licenses/LICENSE-2.0
class licensed_api;
```

### 参见与交叉引用

```cpp
/// @brief 快速排序实现
/// @see stable_sort 如果需要稳定排序
/// @ref  quick_sort_detail 详细算法说明
/// @xref external:algorithms 外部算法参考手册
void quick_sort(Iterator begin, Iterator end);
```

### 代码示例

`@code` / `@endcode` 嵌入示例代码，可配合 `@snippet` 引用外部文件：

```cpp
/// @brief 配置管理器
/// @code
///    ConfigManager cm;
///    cm.load("config.json");
///    auto val = cm.get<int>("timeout");
/// @endcode
class ConfigManager;
```

### 前置 / 后置 / 不变式

```cpp
/// @brief 执行数据传输
/// @pre   buffer 不为空且长度大于 0
/// @post  transferred >= 0
/// @invariant 传输过程中 buffer 内容不变
size_t transmit(const char* buffer, size_t length);
```

### 自定义段落

`@par` 在文档中插入带标题的独立段落：

```cpp
/// @brief 网络请求封装
/// @par 线程安全
///      本类的所有方法都是线程安全的，可在多线程环境中直接使用。
/// @par 性能说明
///      每次请求会复用内部连接池，减少握手开销。
class http_client;
```

### 待办 / 缺陷 / 测试

```cpp
/// @brief 实验性特性
/// @todo 支持 IPv6
/// @bug   高并发下可能出现超时
/// @test  需覆盖边缘情况：空输入、超大输入
void experimental_feature();
```

## MuZiYan 拓展标签

以下标签为 MuZiYan 特有，标准 Doxygen 不支持。

### `@mergeto` — 合并文档

在头文件首行使用，将该文件的文档合并到另一个文件输出：

```cpp
// @mergeto core.hpp
/// @brief 内部算法实现
void internal_algorithm();
```

处理后 `bit.hpp` 的内容会合并到 `core.hpp` 的文档中输出，而非独立生成。

### `@NODOCBEGIN` / `@NODOCEND` — 跳过文档区域

在源码中标记一段区域跳过文档生成：

```cpp
// @NODOCBEGIN
/// @brief 这段注释不会被提取
void internal_helper();
// @NODOCEND

/// @brief 这段注释会被正常提取
void public_api();
```

### `@main_template` / `@spec_template` — 模板特化文档

为主模板和特化版本分别编写文档：

```cpp
/// @brief 主模板：默认实现
/// @main_template
template <typename T, typename Enable = void>
class hash;

/// @brief 特化：数值类型的哈希实现
/// @spec_template<T, std::enable_if_t<std::is_arithmetic_v<T>>>
class hash<T, std::enable_if_t<std::is_arithmetic_v<T>>>;
```

### `__MUZIYAN_IS_HERE__` — 编译时检测

MuZiYan 在解析时会自动向 clang 注入 `-D__MUZIYAN_IS_HERE__`。可在头文件中条件编译仅在文档生成时可见的内容，无需手动配置：

```cpp
#ifdef __MUZIYAN_IS_HERE__
/** @brief 仅在文档生成时暴露 */
void doc_only_function();
#endif
```

## 多语言文档

MuZiYan 通过 `\lang` 标签切换语言上下文，后续所有标签均归属于该语言：

```cpp
/// @brief 计算两个数值的平方和
/// @param a 第一个数值
/// @param b 第二个数值
///
/// \lang en
/// @brief calculate the sum of squares of two numbers
/// @param a the first number
/// @param b the second number
///
/// \lang ja
/// @brief 2つの数値の二乗和を計算します
/// @param a 最初の数値
/// @param b 2番目の数値
template <typename T>
auto sum_of_squares(T a, T b) -> decltype(a * a + b * b);
```

未加 `\lang` 前缀的内容为默认语言。在 `sleepy.yaml` 中设置 `lang` 控制输出语言。

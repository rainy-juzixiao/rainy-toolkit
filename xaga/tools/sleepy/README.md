# MuZiYan（沐子言）/ sleepy

MuZiYan（沐子言）是一个基于 Rust + libClang 开发的 C++ 程序语言文档生成工具。 内部代号 **sleepy**，可执行文件名为 `MuZiYan`。

其预计设计目标如下：

- 提供统一的 Doxygen 中间文档树表达结构
- 支持 `markdown`、`vitepress`、`latex`、`html`、`xml` 格式的生成
- 国际化支持
- 控制语法（`@brief`、`@param`、`@NODOCBEGIN` 等）

当前状态：`markdown` 与 `rt_vitepress_markdown`（VitePress 站点）生成器可用；
`xml` / `html` / `latex` 生成器尚未实现（占位文件）。

## 特性总览

- 基于 libClang 的 AST 级解析，识别 9 类 C++ 实体（类/枚举/别名/概念/函数/宏/命名空间/变量/常量）
- `implements` / `detail` / `impl` 等内部命名空间参与解析（基类查找、继承补全）但不出现在文档中
- 可选注入 `__MUZIYAN_IS_HERE__` 宏（`compile_flags.muziyan`），头文件可据此调整自身内容以配合解析
- `@module` 模块标注：VitePress 侧边栏按模块组织
- 跨目录配置引用（`includes`）：一个根配置聚合多个库的文档
- 依赖图生成（Mermaid 图 + 边列表）与文件/模块两级循环依赖检查
- 文件级 `@hide` / `@notpublic` 排除伞头与实现文件，`@umbrella` 生成仅含 `#include` 列表的伞头页；空文档源文件与编译诊断即时报告
- `--dry-run` 试运行、`--allow-cycles` 放行、`--no-graph` 跳过图生成
- 中英双语文档与双语文诊断输出

## 构建

依赖：Rust（stable）与 libClang（`clang-sys` 可找到的动态库）。

```bash
# 指定 libClang 位置（示例：pip 安装的 clang 包）
export LIBCLANG_PATH=/path/to/clang/native
cargo build --release
# 产物位于 workspace 根 target/release/MuZiYan
```

运行期同样需要能加载 `libclang`，必要时设置 `LD_LIBRARY_PATH=$LIBCLANG_PATH`。

## 快速上手

在项目根目录准备 `sleepy.yaml`：

```yaml
output_dir: docs_out
lang: chinese            # 或 english；影响生成文档与诊断报告语言
sources:
  - name: lib-alpha      # 输出子目录名
    include_dirs:
      - include          # 相对本配置文件所在目录
    compile_flags:
      std: c++20
```

运行：

```bash
MuZiYan                 # 在当前目录查找 sleepy.{json,yaml,yml}
MuZiYan --root path/to/project
MuZiYan -c some/dir/sleepy.yaml
```

输出结构（完整模式）：

```text
docs_out/
├── lib-alpha/           # 每个 source 一个目录，逐头文件生成 .md
├── dependency_graph.md  # 依赖图（Mermaid + 边列表）
├── index.md             # 文档索引页
└── docs/                # VitePress 站点（config.mts + index.md + reference/）
```

## 命令行参考

```text
Usage: MuZiYan [OPTIONS]

Options:
      --root <DIR>       在查找/加载配置前切换工作目录
  -c, --config <PATH>    显式指定配置文件路径
      --partial          仅生成 reference 页面（VitePress 或 HTML，嵌入已有站点时用）
      --no-graph         跳过依赖图与索引页生成；循环依赖检查仍会执行
      --dry-run          试运行：解析与检查照常，但不写任何输出文件
      --allow-cycles     发现循环依赖时仅警告，不作为错误退出
  -h, --help             打印帮助
  -V, --version          打印版本

退出码 / exit codes: 0 = 成功, 1 = 失败或检测到循环依赖
```

典型组合：

```bash
MuZiYan --dry-run                     # CI 里可用于先验证解析与循环依赖，不会产生对文件的修改
MuZiYan --allow-cycles                # 临时放行已知循环，仅警告
MuZiYan --no-graph --partial          # 只要 reference 页面
```

## 文件级 @hide / @notpublic

伞头（只做 `#include`）和实现文件通常不该出现在文档里。在文件开头的注释里写
`@hide` 或 `@notpublic`，整个文件就会被排除：不解析、不生成页面、不参与依赖图，
也不会触发空文档警告。

```cpp
/**
 * @file collections.hpp
 * @brief 汇总所有集合头文件。
 * @hide
 */
#pragma once
#include <rainy/core/yesod/collections/array.hpp>
```

识别范围仅限文件开头的注释（第一个非注释行之前），因此实体或成员上的
`@hide` 不会被误当成文件级标记：

```cpp
/// @brief 正常归档
struct Outer {
    /// @hide      ← 只隐藏这个成员
    int x;
};
```

被跳过的文件静默处理：不生成页面、不参与依赖图，也不打印提示。

### 伞头：@umbrella

如果伞头仍然需要出现在文档里（让读者知道某个模块由哪些头文件组成），用
`@umbrella` 代替 `@hide` / `@notpublic`：不解析实体，页面只列出它 `#include` 的
头文件名。

```cpp
/**
 * @file collections.hpp
 * @brief 汇总所有集合头文件。
 * @umbrella
 */
#pragma once
#include <rainy/core/yesod/collections/array.hpp>
#include <rainy/core/yesod/collections/vector.hpp>
```

生成结果：

```markdown
# collections.hpp

汇总所有集合头文件。

## 包含的头文件

- `rainy/core/yesod/collections/array.hpp`
- `rainy/core/yesod/collections/vector.hpp`
```

`#include` 名称直接按源码文本读取，**不做宏展开**，也不解析被包含的文件；
注释里的 `#include` 会被忽略。伞头照常出现在索引页、侧边栏和依赖图中。

## 空文档源文件检查

扫描范围内、却没有生成任何文档的头文件会立即报告（不是攒到最后汇总）：

```text
[sleepy warn] 未生成任何文档: lib-alpha/empty.hpp: 未解析到可归档的实体，确认它是否只是聚合头文件或前置声明，或整体位于 ignored_namespaces 中
[sleepy warn] 未生成任何文档: lib-alpha/legacy.hpp: 实体全部被 @notpublic/@hide 过滤，确认是否应从扫描范围排除
[sleepy warn] 未生成任何文档: lib-alpha/gone.hpp: 文件不存在或无法读取，确认路径与权限
```

编译诊断同样在解析每个翻译单元时即时打印：

```text
[sleepy][parse warning] include/foo.hpp:2:2: "this header is deprecated"
```

## 可见性与 @show

成员默认按访问级别过滤：只有 **public** 的成员（成员函数、成员变量、常量、变量模板、嵌套类/枚举/别名）会出现在文档里；顶层实体不受此限制。

需要公开一个非 public 实体时，在它的文档注释里写 `@show`：

```cpp
class Cache {
private:
    /**
     * @brief 仅供诊断工具读取的条目数
     * @show
     */
    int entry_count() const;

    /// @brief 这个不会出现在文档里
    void evict();
};
```

`@show` 可以越过 private / protected 限制，但对 `@hide` / `@notpublic` 无效——
两者同时出现时仍然隐藏，方便在保持隐藏的前提下临时改写注释。

匿名结构体/联合体（`union { ... };`）没有可引用的名字，libclang 会合成
`(anonymous union at path:line:col)` 这样的伪名字，sleepy 直接跳过它们，既不生成
也不计入统计。

## 批量定义组：\scope / \forscope

一组声明共用同一份文档时（例如 `_1`…`_20` 占位符），把模板注释写在第一个声明上，
其余成员用 `\forscope` 引用并代入变量：

```cpp
namespace rainy::functional::placeholders {
    /**
     * \lang simp-chinese
     * @brief 用于绑定参数的占位符对象 _{N}。
     *
     * \scope name [placeholders]
     * \scope Var [N=1]
     */
    inline constexpr implements::placeholder_t<1> _1{};

    /**
     * \forscope [placeholders]
     * \forscope Var N=2
     */
    inline constexpr implements::placeholder_t<2> _2{};

    /**
     * \forscope [placeholders]
     * \forscope Var N=3
     */
    inline constexpr implements::placeholder_t<3> _3{};
}
```

规则：

- `\scope name [S]` 给组命名，`\scope Var [N=值]` 声明模板变量并给出初值；
  初值必填，写 `\scope Var [N]` 会报错——组里的每个成员（包括声明处那个）都要
  由显式赋值确定 `{N}`，不按声明顺序推断；
- 写了 `\scope` 的声明同样是组成员，会正常生成文档；
- `\forscope [S]` 引用组（省略组名时取最近声明的组），`\forscope Var N=XXX` 赋值，
  正文里的 `{N}` 替换成 `XXX`；
- 组只在**当前文件**内有效，不跨文件；不需要结束标记，组一直延续到文件末尾；
- 组内每个成员各生成一条完整条目，并视为已文档化，不进入未文档化统计；
- 每个语言块（`\lang ...`）独立替换，中英文可以各自取值。

以下情况会给出警告（`批量定义组:` 前缀）：

| 情况 | 说明 |
|------|------|
| `\scope Var [N]` 缺少初值 | 无法确定该成员的 `{N}` |
| `\forscope [S]` 引用了未声明的组 | 组名拼错，或组在别的文件里 |
| `\forscope` 没有组名且之前没有组 | 省略组名时必须有可沿用的组 |

## 未文档化实体统计

除了“整份文件没生成文档”，sleepy 还会报告**可见却没有文档注释**的有效实体：

```text
[sleepy warn] 未文档化的有效实体: lib-alpha/core.hpp: 共 47 个（2 类、1 枚举、22 枚举项、4 别名、4 函数、1 常量、13 成员函数）
  |- 成员函数 `hash::operator()` (L974)
  |- 成员函数 `hash::hash_this_val` (L978)
  |- 别名 `hash::argument_type` (L971)
  |- 枚举项 `ctti::traits::is_const` (L42)
  |- ... 另有 27 个未列出
```

口径与文档生成器一致：

- 成员只有 **public** 的才会生成进文档；`@show` 可以让 private / protected 成员也生成（但对 `@hide` / `@notpublic` 无效）；
- `@hide` / `@notpublic` 排除的实体、`// @NODOCBEGIN` 区间内的实体不统计；
- 只写了 `@notpublic` / `@hide` / `@file` 这类标记、没有实际文档内容的注释不算文档；
- 伞头（`@umbrella`）不解析实体，因此不参与统计。

每个文件最多逐条列出 20 个实体（类型、全名、行号），其余按种类汇总成“另有 N 个未列出”。

枚举项的文档归属按源码位置判断：只有紧挨着该枚举项声明行的注释才算它的文档。
libclang 会把某个枚举项的注释一并挂到其后的枚举项上，sleepy 会剥离这类误挂，
避免把未文档化的枚举项误判为已文档化。

## 配置参考（sleepy.yaml）

```yaml
output_dir: docs_out          # 必填：所有输出根目录
lang: chinese                 # 可选：english（默认）/ chinese
ignored_namespaces: # 可选：跳过这些命名空间（默认 implements/detail/impl）
  - detail

includes: # 可选：引用其它目录的 sleepy 配置（相对本文件）
  - ../other-lib/sleepy.yaml

dependency_graph: # 可选：依赖图与循环依赖检查
  enabled: true               #    是否生成依赖图与索引页（默认 true）
  out_name: dependency_graph  #    输出文件名（默认 dependency_graph.md）
  fail_on_cycle: true         #    发现循环是否视为致命错误（默认 true）
  ignore: #    豁免的包含边（与依赖图同步剔边）
    - "engine.hpp -> vehicle.hpp"

sources:
  - name: lib-alpha           # 必填：输出子目录 / 图中模块名
    include_dirs: [ include ]   # 必填：递归扫描的头文件目录（相对本配置文件）
    extensions: [ h, hpp, hxx ] # 可选：扫描的扩展名
    exclude_dirs: [ include/detail ]   # 可选：排除目录
    exclude_files: [ include/impl_old.hpp ]  # 可选：排除单个文件
    files: [ ]                 # 可选：显式列出文件（优先于目录扫描）
    compile_flags:
      std: c++20
      defines: [ FOO=1 ]
      includes: [ third_party/include ]
      extra: [ -fno-exceptions ]
      inherit_from: build/compile_commands.json  # 复用编译数据库参数
      compiler: /usr/bin/clang++-18               # 手动指定编译器
```

## 跨库配置引用（includes）

根配置可以通过 `includes` 引用其它库的 `sleepy.yaml`，一次生成多库文档与跨库依赖图：

```yaml
# site/sleepy.yaml
output_dir: docs_out
lang: chinese
includes:
  - ../lib-alpha/sleepy.yaml
  - ../lib-beta/sleepy.yaml
```

合并语义：

| 项                                         | 行为                                                             |
|--------------------------------------------|------------------------------------------------------------------|
| `sources`                                  | 追加合并；**路径相对该配置文件自身所在目录重写**，无需写绝对路径 |
| `ignored_namespaces`                       | 取并集                                                           |
| `output_dir` / `lang` / `dependency_graph` | 以根配置为准                                                     |
| 引用链成环（A→B→A）                        | **报错**退出，打印完整引用链并标注 `cycle closes here`           |
| 菱形引用（A→B、A→C、B/C→D）                | 静默跳过，D 只合并一次                                           |

另外，所有被引用库的 `include_dirs` 会自动注入每个 source 的编译参数（`-I`）， 使头文件中的跨库 `#include` 能被正确解析。

## 依赖图与循环依赖检查

每次运行会基于各头文件实际记录的 `#include` 指令构建内部包含图，并做两级检查：

- **文件级**：头文件之间的循环包含（如 `engine.hpp ↔ vehicle.hpp`），自包含（`a.hpp include 自己`）单独报告；
- **模块级**：把文件图投影到 source（库）粒度，检测库与库之间的循环依赖。

发现循环时的行为：

1. **不写入任何输出文件**；
2. 以 exit code 1 退出，除非配置 `fail_on_cycle: false` 或命令行 `--allow-cycles`；
3. 打印双语诊断，例如（`lang: chinese`）：

```text
✘ 检测到循环依赖 (circular includes detected)
  文件级循环: 1 处，涉及 2 个文件:
    [1] lib-alpha/engine.hpp -> lib-beta/beta/vehicle.hpp -> lib-alpha/engine.hpp  ← 闭环
  模块级循环: 1 处:
    [1] lib-alpha <-> lib-beta  ← 闭环

  修复建议:
    1. 优先使用前置声明 (forward declaration) 打断头文件之间的相互包含;
    2. 若该包含是预期行为，可在 sleepy.yaml 的 dependency_graph.ignore 中豁免，
       例如: ignore: ["a.hpp -> b.hpp"];
    3. 或使用 --allow-cycles 跳过本次检查（不推荐）.
```

`ignore` 中豁免的边会同时从依赖图中剔除；`--no-graph` 只跳过依赖图/索引页 **生成**， 不影响循环检查本身。

## 国际化

`lang` 字段同时控制三处语言：

1. 生成文档中标签（"Parameters"、"成员" 等）；
2. 循环依赖诊断报告语言；
3. `@brief` 等多语言注释标签的取值优先级（优先取 `lang` 对应翻译，其次 default）。
4.

## License

Apache License 2.0。详见仓库根目录的 LICENSE 文件。

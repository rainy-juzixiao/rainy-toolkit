# MuZiYan（沐子言）/ sleepy

MuZiYan（沐子言）是一个基于 Rust + libClang 开发的 C++ 程序语言文档生成工具。 内部代号 **sleepy**，可执行文件名为 `MuZiYan`。

其预计设计目标如下：

- 提供统一的 Doxygen 中间文档树表达结构
- 支持 `markdown`、`vitepress`、`latex`、`html`、`xml` 格式的生成
- 国际化支持
- 控制语法（`@brief`、`@param`、`@mergeto`、`@NODOCBEGIN` 等）

当前状态：`markdown` 与 `rt_vitepress_markdown`（VitePress 站点）生成器可用；
`xml` / `html` / `latex` 生成器尚未实现（占位文件）。

## 特性总览

- 基于 libClang 的 AST 级解析，识别 9 类 C++ 实体（类/枚举/别名/概念/函数/宏/命名空间/变量/常量）
- `implements` / `detail` / `impl` 等内部命名空间参与解析（基类查找、继承补全）但不出现在文档中
- 可选注入 `__MUZIYAN_IS_HERE__` 宏（`compile_flags.muziyan`），头文件可据此调整自身内容以配合解析
- `@mergeto` 头文件合并：多个实现片段合并到同一个主页面
- `@module` 模块标注：VitePress 侧边栏按模块组织
- 跨目录配置引用（`includes`）：一个根配置聚合多个库的文档
- 依赖图生成（Mermaid 图 + 边列表）与文件/模块两级循环依赖检查
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
      --partial          仅生成 VitePress 的 reference 页面（嵌入已有站点时用）
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

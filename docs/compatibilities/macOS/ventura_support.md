## Ventura

### 测试环境信息

以下结果通过`fastfetch`获取

| 项目       | 信息                                      |
|----------|-----------------------------------------|
| 操作系统     | macOS Ventura 13.7.8 (22H730) x86_64    |
| 处理器架构    | x86_64                                  |
| macOS版本号 | 13.7.8                                  |
| 内核版本     | Darwin 22.6.0                           |
| CPU      | 4 x Intel Core (Skylake) (4) @ 3.18 GHz |
| 内存       | 4.0GIB                                  |
| swap     | Disabled                                |
| locale   | zh_CN.UTF-8                             |

### 编译工具链信息

`clang++ --version`

| 项目   | 信息                               |
|------|----------------------------------|
| 版本   | 18.1.8                           |
| 目标   | x86_64-apple-darwin22.6.0        |
| 线程模型 | posix                            |
| 安装于  | `/opt/local/libexec/llvm-18/bin` |

另注：

使用MacPorts进行包管理，不使用Ventura自带的Apple Clang工具链（存在一定兼容性问题）
该版本部分库处于实验特性（构建系统已处理并尽可能启用实验特性）

```shell
sudo port install clang-18
```

### 构建系统信息

| 项目    | 版本号     |
|-------|---------|
| CMake | 3.31.10 |
| Ninja | 1.13.2  |

另注：

使用MacPorts进行包管理，不进行版本制定，仅供参考

```shell
sudo port install ninja cmake
```

### 目前测试进展

1. 在foundation::text::format的locale支持测试中，检查到格式化结果有时并未得到预期结果。因此，后续考虑增强改进实现代码。

所有测试用例基本工作良好

#### 概览

| 项目                          | 结果     | 
|-----------------------------|--------|
| [collections](#collections) | Passed | 
| [core](#core)               | Passed |
| [foundation](#foundation)   | Passed |
| [meta](#meta)               | Passed |
| [utility](#utility)         | Passed |

#### collections

已覆盖的测试

- `concurrency.forward_list`
- `array`
- `bit_vector`
- `unordered_map`
- `vector`

#### core

已覆盖的测试：

- `text`
    - `format(*)`
    - `string`
    - `string_view`
- `atomic_pal`

说明：

text.format在部分locale测试中存在问题，推测是系统环境问题导致（搁置修复）

#### foundation

已覆盖的测试：

- `foundation`
    - `concurrency`
        - `atomic`
        - `barrier`
        - `condition_variable`
        - `condition_variable_any`
        - `executor`
        - `future`
        - `mutex`
        - `shared_mutex`
    - `container`
        - `indirect`
        - `optional`
        - `polymorphic`
        - `variant`
    - `io`
        - `net`
            - `address`
            - `buffer`
            - `internet`
            - `io_context`
            - `socket`
            - `timer`
        - `memory`
            - `hazard_pointer`

#### meta

已覆盖的测试：

- `meta`
    - `wtf`
        - `express_center`

#### utility

已覆盖的测试：

- `utility`
    - `basic_any`

##### 兼容性进展

- 添加了大部分macOS兼容层，部分暂未添加，等到后续分支补充
- 基本可工作

### 备注说明

#### 构建

在构建时，使用了如下cmake的参数对cmake项目进行构建生成

```
-G Ninja
-DCMAKE_OSX_SYSROOT=/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk
-DCMAKE_CXX_FLAGS="-isystem /opt/local/libexec/llvm-18/include/c++/v1"
-DCMAKE_EXE_LINKER_FLAGS="
    -L/opt/local/libexec/llvm-18/lib/libc++ -Wl,
    -rpath,
    /opt/local/libexec/llvm-18/lib/libc++ -Wl,
    -rpath,
    /opt/local/lib -L/opt/local/lib -lunwind
"
-DCMAKE_SHARED_LINKER_FLAGS="
    -L/opt/local/libexec/llvm-18/lib/libc++ -Wl,
    -rpath,/opt/local/libexec/llvm-18/lib/libc++ -Wl,
    -rpath,/opt/local/lib -L/opt/local/lib -lunwind
"
```

#### 修复的问题

1. 修正了foundation::text::string的行为，修复了clang在constexpr上下文计算中的生命周期检查错误
2. 移除了不支持macOS的检查
3. 对basic_any的数值类型转换，添加了专属的unsigned long类型检查支持，确保兼容性和正确性
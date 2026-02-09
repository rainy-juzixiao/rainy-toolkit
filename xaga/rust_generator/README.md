## rainy-toolkit反射moc代码生成器

### 概述

此子项目通过使用tree-sitter-cpp库实现对C++代码的元反射解析和生成注册代码。

为了确保该生成器正常工作，你必须安装rust + cargo工具，编译生成器。

目前，主要研究rainy-toolkit动态反射的moc生成器

该生成器将被设计为一个使用rust语言对C++代码进行解析和代码生成的命令行工具，该命令行工具将优先作为重点进行规划。

国际化支持部分：

优先支持英文界面，中文界面将通过语言toml提供

预定在首个正式可用的技术预览支持中提供如下功能：

- 基础Cli命令行支持
- 生成针对位于头文件的类成员，全局函数，静态函数的插桩代码
- 国际化支持

生成器目前预定规划支持如下参数：

- `-i --input <input_file_path>` 指定moc生成器需要读取生成的源文件
- `-o --out <output_file_path>` 指定moc生成器输出到的路径
- `-h --help` 打印帮助输出

### 使用

**将rt_moc安装到当前机器中：**

```bash
cargo run --bin install
```

这将会调用install.rs，并进行项目编译，在完成之后，即可调用rt_moc命令进行操作。

**使用cargo run进行预览：**

```bash
cargo run -- [指定工具的参数]
```

### 快速开始

**扫描一个源文件**

确保在你当前工作目录或是绝对路径中存在该文件，以确保rt_moc找到该文件。

假设，有如下文件，确保你已经包含了`<rainy/core/core.hpp>`头

```cpp
class a {
public:
    RAINY_ENABLE_MOC();

    a() { }

    void foo() {

    }

    void foo(int, int) {

    }

    int mem() {

    }

    static void bar() {}

    int aa;

private:
    int b;
};
```

我们暂且将该文件命名为example.cc，假设在当前目录打开终端，执行如下指令

```bash
rt_moc --input example.cc
# 或者...
cargo run -- --input example.cc # cargo默认添加了default-run目标，因此可以直接使用run
```

执行完成后，将在当前目录中，将会出现example.cc文件，在文件内，将能看到如下的插桩代码

```cpp
RAINY_REFLECTION_REGISTRATION {
    registration::class_<a>("a")
        .constructor<>()
        .method("foo",rainy::utility::get_overloaded_func<a, void()>(&a::foo))
        .method("foo",rainy::utility::get_overloaded_func<a, void(int, int)>(&a::foo))
        .method("mem",rainy::utility::get_overloaded_func<a, int()>(&a::mem))
        .method("bar",rainy::utility::get_overloaded_func<void()>(&a::bar))
        .property("aa", &a::aa)
}

```

**更多示例**

更多的示例演示，将会通过后续的更新与文档被展示，请时刻查阅
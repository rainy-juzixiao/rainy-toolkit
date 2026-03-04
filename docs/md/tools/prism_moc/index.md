# rainy-toolkit prism-moc 反射代码生成器

prism-moc是一个基于rust语言开发的命令行工具，负责解析moc代码生成标准的C++注册代码。

一般情况下，你不需要操作prism-moc，cmake会自动帮助你调用prism-moc，并自动替换编译单元帮助你编译。

但考虑存在个别情况，如需要控制prism-moc的行为，例如：

- 是否允许增量生成
- 是否需要查看AST解析结果
- 集成到其它分布式构建系统

那么，需查阅本文档以了解详细信息。

目前，prism-moc处于技术预览阶段。

你可以使用prism-moc，但是，我们不保证，prism-moc百分百完全可用。

prism-moc现在仅支持如下操作：

- 生成类内的函数注册、生成类体
- 
---
# https://vitepress.dev/reference/default-theme-home-page
layout: home

hero:
  name: "rainy-toolkit 文档"
  text: "Keep It Simple, Stupid"
  tagline: "基于C++17，提供便携的模板元编程、反射、跨平台平台函数功能实现等等。让C++如同积木一样使用"
  actions:
    - theme: brand
      text: 快速上手
      link: '/'
    - theme: alt
      text: '查阅文档'
      link: '/md'
features:
  - title: 开源
    details: rainy-toolkit采用了Apache 2.0许可证进行分发。在不违反Apache 2.0许可证的条件下，所有人都可自由查看和修改rainy-toolkit的源代码
  - title: 跨平台、跨编译器
    details: rainy-toolkit支持MSVC、LLVM-Clang、GCC三大编译器。且，跨操作系统，可在Windows和Linux平台可用
  - title: 高性能
    details: rainy-toolkit的大量代码以头文件形式进行提供，进行了大规模优化，且易用。
  - title: 可用于高性能的集合
    details: 借助hybrid_jenova模块提供的可创建固定大小的高性能集合容器。使得C++更加灵活
  - title: 统一工具集
    details: 你仅需在cmake构建系统中使用add_subdirectory，cmake将会自动配置构建
  - title: 现代C++
    details: 基于C++17进行开发，对C++20、C++23提供向后兼容
---


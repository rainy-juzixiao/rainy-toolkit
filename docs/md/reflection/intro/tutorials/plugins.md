# 插件样例

## 前言

此章节将指导用户如何创建适用于rainy-toolkit的插件模块，插件模块是rainy-toolkit反射的高级应用，通过动态加载并执行反射注册，可在业务中实现更灵活的编程体验。

## 前置条件

- 已阅读hello_world的tutorial
- 已阅读prism-moc文档并了解其集成规则（可选）

## 编写Player类

首先，让我们按照常规流程，包含需要的头文件。

```cpp
#include <rainy/meta/reflection/registration.hpp>
```

然后编写Player类。

```cpp
class Player {
public:
    Player();

    Player(std::string n, int h, int l);

    void PrintInfo();

    void TakeDamage(int damage);

    std::string name;
    int health;
    int level;
};
```

实现略去，在你完成编写之后，即完成了基础插件的最基本建模。 现在，让我们按照以往的习惯，执行注册。

```cpp
RAINY_REFLECTION_REGISTRATION {
    registration::class_<Player>("Player")
        .constructor<>()
        .constructor<std::string, int, int>()
        .method("PrintInfo",rainy::utility::get_overloaded_func<Player, void()>(&Player::PrintInfo))
        .method("TakeDamage",rainy::utility::get_overloaded_func<Player, void(int)>(&Player::TakeDamage))
        .property("name", &Player::name)
        .property("health", &Player::health)
        .property("level", &Player::level);
}
```

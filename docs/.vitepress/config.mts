import { defineConfig } from 'vitepress'

export default defineConfig({
  title: "rainy-toolkit 文档",
  description: "A VitePress Site",
  lang: 'zh-CN',
  base: '/rainy-toolkit/',
  themeConfig: {
    sidebar: [
      {
        text: '反射',
        base: '/md/reflection/',
        collapsed: true,
        items: [
          {
            text: '&lt;function.hpp&gt;',
            collapsed: true,
            items: [
              {
                text: '&lt;function.hpp&gt;',
                link: 'function/'
              },          
              {
                text: '<function.hpp> 函数',
                link: 'function/pubfn'
              },
              {
                text: '<function.hpp> 运算符',
                link: 'function/operators'
              },
              {
                text: 'function 类',
                link: 'function/function_class'
              }
            ]
          }
        ]
      }
    ],
    socialLinks: [
      {
        icon: 'bilibili', link: 'https://space.bilibili.com/670859148'
      }
    ]
  }
});

import { defineConfig } from 'vitepress'
import { change_log_sidebar_item } from './change_log.mts';
import { utility_toolkit_index } from './utility_document/index.mjs';

export default defineConfig({
  title: "rainy-toolkit 文档",
  description: "A VitePress Site",
  lang: 'zh-CN',
  base: '/rainy-toolkit/',
  themeConfig: {
    sidebar: [
      change_log_sidebar_item,
      utility_toolkit_index,
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
                text: '&lt;function.hpp&gt;函数',
                link: 'function/pubfn'
              },
              {
                text: '&lt;function.hpp&gt;运算符',
                link: 'function/operators'
              },
              {
                text: 'method_flags 枚举',
                link: 'function/method_flags'
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
  },
  markdown: {
    theme: {
      dark: 'dracula-soft',
      light: 'github-dark',
    }
  }
});

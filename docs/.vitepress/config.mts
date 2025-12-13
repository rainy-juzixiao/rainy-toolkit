import { defineConfig } from 'vitepress'
import { change_log_sidebar_items } from './change_log.mts';
import { utility_toolkit_index } from './config/utility_document/index.mjs';
import { core_toolkit_index } from './core_document/index.mts';
import { meta_index } from './config/meta_document/index.mts';

export default defineConfig({
  title: "rainy-toolkit 文档",
  description: "A VitePress Site",
  lang: 'zh-CN',
  base: '/rainy-toolkit/',
  themeConfig: {
    sidebar: {
      '/': [
        {
          text: '首页',
          link: '/md/',
        },
        {
          text: '更新日志',
          base: '/change_log/',
          collapsed: true,
          items: change_log_sidebar_items,
        },
        {
          text: '核心库',
          link: '/md/core/',
        },
        {
          text: 'meta库',
          link: '/md/meta/'
        }
      ],
      '/md/meta/': meta_index, 
      '/md/core/': core_toolkit_index,
      '/md/reflection/': [
        {
          text: '反射',
          base: '/md/reflection/',
          collapsed: true,
          items: [
            {
              text: '简介',
              collapsed: true,
              items: [
                {
                  text: '绪论',
                  link: 'intro/',
                },
                {
                  text: '教程',
                  items: [
                    {
                      text: 'Hello World样例',
                      link: 'intro/tutorials/hello_world'
                    }
                  ]
                }
              ]
            },
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
      ]
    },
    socialLinks: [
      {
        icon: 'bilibili',
        link: 'https://space.bilibili.com/670859148'
      }
    ]
  },
  markdown: {
    theme: {
      light: 'material-theme-palenight',
      dark: 'material-theme-palenight'
    },
    async config(md) {
      const mod = await import('markdown-it-multimd-table')

      // 关键：CJS 模块在 ESM 下 "mod.default" 有时是 undefined
      const multi = mod.default || mod

      md.use(multi, {
        rowspan: true,
        colspan: true
      })

    }
  },

  vite: {
    css: {
      preprocessorOptions: {
        scss: {
          api: 'modern',
        }
      }
    }
  }
});

import { defineConfig } from 'vitepress'
import { change_log_sidebar_items } from './change_log.mts';
import { core_toolkit_index } from './core_document/index.mts';
import { meta_index } from './config/meta_document/index.mts';
import { tools_index } from './config/tools/index.mts';
import { api_index } from './config/api.mts';
import rainyWarm from './theme/shiki/rainy-warm';

export default defineConfig({
  title: "rainy-toolkit",
  description: "面向现代 C++ 的基础设施集合：模板元编程、反射、平台抽象与组件库",
  lang: 'zh-CN',
  base: '/rainy-toolkit/',
  head: [
    ['link', { rel: 'icon', type: 'image/png', href: '/rainy-toolkit/images/rainy-toolkit-icon.png' }],
    ['meta', { name: 'theme-color', content: '#f0eee6' }]
  ],
  themeConfig: {
    nav: [
      { text: '首页', link: '/' },
      { text: '核心库', link: '/md/core/' },
      { text: 'meta 库', link: '/md/meta/' },
      { text: '反射', link: '/md/reflection/' },
      ...(api_index ? [{ text: 'API', link: '/api/' }] : []),
      { text: '更新日志', link: '/change_log/xaga/xaga-0.7.4' }
    ],
    logo: '/images/rainy-toolkit-icon.png',
    siteTitle: 'rainy-toolkit',
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
        },
        tools_index,
      ],
      '/md/meta/': meta_index,
      '/md/core/': core_toolkit_index,
      ...(api_index
        ? {
            '/api/': [
              api_index,
              { text: '返回文档', link: '/md/' }
            ]
          }
        : {}),
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
            }
          ]
        }
      ],
      '/compatibilities': [
        {
          text: 'macOS',
          link: '/compatibilities/macOS',
          items: [
            {
              text: 'ventura',
              link: '/compatibilities/macOS/ventura_support'
            }
          ]
        }
      ]
    },
    outline: {
      level: [2, 3],
      label: '本页目录'
    },
    docFooter: {
      prev: '上一篇',
      next: '下一篇'
    },
    lastUpdated: {
      text: '最后更新',
      formatOptions: { dateStyle: 'medium' }
    },
    editLink: {
      pattern: 'https://github.com/rainy-juzixiao/rainy-toolkit/edit/master/docs/:path',
      text: '在 GitHub 上编辑此页'
    },
    footer: {
      message: 'Apache-2.0 · 面向现代 C++ 的基础设施集合',
      copyright: '© 2026 rainy-toolkit'
    },
    socialLinks: [
      {
        icon: 'github',
        link: 'https://github.com/rainy-juzixiao/rainy-toolkit'
      },
      {
        icon: 'bilibili',
        link: 'https://space.bilibili.com/670859148'
      }
    ],
    search: {
      provider: 'local',
      options: {
        translations: {
          button: {
            buttonText: '搜索文档',
            buttonAriaLabel: '搜索文档'
          },
          modal: {
            displayDetails: '显示详情',
            resetButtonTitle: '清除查询',
            backButtonTitle: '返回',
            noResultsText: '没有找到结果',
            footer: {
              selectText: '选择',
              selectKeyAriaLabel: '回车',
              navigateText: '切换',
              navigateUpKeyAriaLabel: '上箭头',
              navigateDownKeyAriaLabel: '下箭头',
              closeText: '关闭',
              closeKeyAriaLabel: 'Esc'
            }
          }
        }
      }
    }
  },
  markdown: {
    theme: rainyWarm,
    async config(md) {
      const mod = await import('markdown-it-multimd-table')

      // 关键：CJS 模块在 ESM 下 "mod.default" 有时是 undefined
      const multi = mod.default || mod

      md.use(multi, {
        rowspan: true,
        colspan: true,
        multibody: false
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

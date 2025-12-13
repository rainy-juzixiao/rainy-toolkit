import { DefaultTheme } from 'vitepress'

export const utility_toolkit_index: DefaultTheme.SidebarItem = {
  text: '实用工具集',
  base: '/md/utility/',
  collapsed: true,
  items: [
    {
      text: '&lt;any.hpp&gt;',
      collapsed: true,
      items: [
        {
          text: '&lt;any.hpp&gt;',
          link: 'basic_any/'
        },
        {
          text: '&lt;any.hpp&gt;函数',
          link: 'basic_any/pubfn'
        },
        {
          text: 'basic_any 类',
          link: 'basic_any/basic_any_class'
        }
      ]
    }
  ]
}
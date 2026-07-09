import { DefaultTheme } from 'vitepress'

export const tools_index: DefaultTheme.SidebarItem = {
  text: '工具',
  base: '/md/tools/',
  collapsed: true,
  items: [
    {
      text: 'prism-moc 反射代码生成器',
      link: 'prism_moc/'
    },
    {
      text: 'MuZiYan 文档生成器',
      link: 'muziyan/'
    }
  ]
}

import {DefaultTheme} from 'vitepress'

export const change_log_sidebar_item : DefaultTheme.SidebarItem = {
  text: '更新日志',
  base: '/change_log',
  collapsed: true,
  items: [
    {
      text: 'xaga',
      collapsed: true,
      items: [
        {
          text: '0.7.4',
          link: '/xaga/xaga-0.7.4'
        }
      ]
    }
  ]
}

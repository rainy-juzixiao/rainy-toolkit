import DefaultTheme from 'vitepress/theme'
import { h } from 'vue' // h函数
// 组件1
import layout from "./components/layout.vue";
import './style/index.scss'

export default {
  extends: DefaultTheme,
  Layout() {
    return h(layout, null, {
    })
  }
}
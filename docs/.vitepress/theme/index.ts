import DefaultTheme from 'vitepress/theme'
import { h } from 'vue' // h函数
// 组件1
import layout from "./components/layout.vue";
import './style/index.scss'
import DeclarationTable from './components/DeclarationTable.vue';
import ParameterSection from './components/ParameterSection.vue';
import ReturnValueSection from './components/ReturnValueSection.vue';
import DescriptSection from './components/DescriptSection.vue';

export default {
  extends: DefaultTheme,
  Layout() {
    return h(layout, null, {
    })
  },
  enhanceApp({ app }) {
    // 在此引入文档组件
    app.component('DeclarationTable', DeclarationTable);
    app.component('ParameterSection', ParameterSection);
    app.component('ReturnValueSection', ReturnValueSection);
    app.component('DescriptSection', DescriptSection);
  }
}
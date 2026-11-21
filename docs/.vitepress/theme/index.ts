import DefaultTheme from 'vitepress/theme'
import type { Theme } from 'vitepress'
import { h, App } from 'vue'
import layout from './components/layout.vue'
import './style/index.scss'
import DeclarationTable from './components/DeclarationTable.vue'
import ParameterSection from './components/ParameterSection.vue'
import ReturnValueSection from './components/ReturnValueSection.vue'
import DescriptSection from './components/DescriptSection.vue'
import AttentionSection from './components/AttentionSection.vue'
import RemarkSection from './components/RemarkSection.vue'
import InnerMemberDefine from './components/InnerMemberDefine.vue'

export default {
  extends: DefaultTheme,
  Layout() {
    return h(layout, null, {})
  },
  enhanceApp({ app }: { app: App }) {
    app.component('DeclarationTable', DeclarationTable)
    app.component('ParameterSection', ParameterSection)
    app.component('ReturnValueSection', ReturnValueSection)
    app.component('DescriptSection', DescriptSection)
    app.component('AttentionSection', AttentionSection)
    app.component('RemarkSection', RemarkSection)
    app.component('InnerMemberDefine', InnerMemberDefine)
  }
} satisfies Theme

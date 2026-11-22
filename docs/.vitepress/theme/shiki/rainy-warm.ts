import type { ThemeRegistration } from 'shiki'

const warm: ThemeRegistration = {
  name: 'rainy-warm',
  type: 'dark',
  colors: {
    'editor.background': '#1d1c19',
    'editor.foreground': '#ded8ca'
  },
  settings: [
    {
      settings: {
        background: '#1d1c19',
        foreground: '#ded8ca'
      }
    },
    {
      scope: ['comment', 'punctuation.definition.comment', 'string.comment'],
      settings: { foreground: '#6e6c64', fontStyle: 'italic' }
    },
    {
      scope: [
        'keyword',
        'storage',
        'storage.type',
        'storage.modifier',
        'keyword.control',
        'keyword.operator.new',
        'keyword.operator.expression',
        'keyword.operator.cast'
      ],
      settings: { foreground: '#d08a6b' }
    },
    {
      scope: ['keyword.operator', 'punctuation.separator', 'punctuation.terminator'],
      settings: { foreground: '#a8a296' }
    },
    {
      scope: [
        'entity.name.type',
        'entity.name.class',
        'entity.name.namespace',
        'support.type',
        'support.class',
        'storage.type.primitive'
      ],
      settings: { foreground: '#c9b98a' }
    },
    {
      scope: [
        'entity.name.function',
        'support.function',
        'variable.function',
        'meta.function-call'
      ],
      settings: { foreground: '#a9b99a' }
    },
    {
      scope: ['variable', 'variable.other', 'variable.parameter', 'meta.definition.variable'],
      settings: { foreground: '#ded8ca' }
    },
    {
      scope: ['variable.other.member', 'meta.object-literal.key', 'variable.object.property'],
      settings: { foreground: '#c7c0ae' }
    },
    {
      scope: ['string', 'string.quoted', 'punctuation.definition.string'],
      settings: { foreground: '#9db4c4' }
    },
    {
      scope: ['constant.numeric', 'constant.language', 'constant.character', 'support.constant'],
      settings: { foreground: '#c98f8a' }
    },
    {
      scope: ['entity.name.tag', 'punctuation.definition.tag'],
      settings: { foreground: '#d08a6b' }
    },
    {
      scope: ['entity.other.attribute-name'],
      settings: { foreground: '#c9b98a' }
    },
    {
      scope: ['meta.preprocessor', 'keyword.control.directive'],
      settings: { foreground: '#b7a4c4' }
    },
    {
      scope: ['support.type.property-name', 'support.type.property-name.json'],
      settings: { foreground: '#c9b98a' }
    },
    {
      scope: ['punctuation', 'meta.brace', 'meta.delimiter'],
      settings: { foreground: '#8d897d' }
    },
    {
      scope: ['invalid', 'invalid.illegal'],
      settings: { foreground: '#e6978e' }
    }
  ]
}

export default warm

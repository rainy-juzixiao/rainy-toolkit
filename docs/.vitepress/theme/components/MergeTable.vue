/**
* 说明：
* 插槽中的纯文本会被解析为表格数据。
* 行之间以 rowSep（默认 ";;"）分隔。
* 列之间以 sep（默认 "|"）分隔。
* 当 header 为 true 时，第一行会作为表头。
* 支持 markdown 行内语法。
* 单元格可附加元数据，如 [rowspan=2,colspan=3]。
  */
<script setup>
import { computed, useSlots } from "vue";
import MarkdownIt from "markdown-it";

const props = defineProps({
  sep: { type: String, default: "|" },
  rowSep: { type: String, default: ";;" },
  header: { type: Boolean, default: false }
});

const slots = useSlots();

// 初始化 markdown-it
const md = new MarkdownIt({
  html: true,        // 允许 HTML 标签
  breaks: false,     // 不自动转换换行
  linkify: true,     // 自动识别链接
});

// 自定义链接渲染规则,区分锚点和外部链接
const defaultRender = md.renderer.rules.link_open || function(tokens, idx, options, env, self) {
  return self.renderToken(tokens, idx, options);
};

md.renderer.rules.link_open = function (tokens, idx, options, env, self) {
  const token = tokens[idx];
  const hrefIndex = token.attrIndex('href');
  
  if (hrefIndex >= 0) {
    const href = token.attrs[hrefIndex][1];
    
    // 如果是外部链接,添加 target="_blank"
    if (href.startsWith('http://') || href.startsWith('https://') || href.startsWith('//')) {
      token.attrPush(['target', '_blank']);
      token.attrPush(['rel', 'noopener noreferrer']);
    }
    // 锚点链接和相对路径不需要新开标签页
  }
  
  return defaultRender(tokens, idx, options, env, self);
};

function getRawText() {
  const vnodes = slots.default ? slots.default() : [];
  let text = "";

  function extractText(node) {
    if (!node) return;
    
    if (typeof node.children === "string") {
      text += node.children;
    } 
    else if (Array.isArray(node.children)) {
      node.children.forEach(child => extractText(child));
    }
    else if (node.children && typeof node.children === "object") {
      extractText(node.children);
    }
  }

  vnodes.forEach(vnode => extractText(vnode));
  
  return text.trim();
}

const rows = computed(() => {
  const raw = getRawText();
  return raw.split(props.rowSep).filter(line => line.trim()).map(line =>
    line.split(props.sep).map(cell => cell.trim())
  );
});

const headerRow = computed(() => {
  return props.header && rows.value.length > 0 ? rows.value[0] : null;
});

const bodyRows = computed(() => {
  return props.header && rows.value.length > 0 ? rows.value.slice(1) : rows.value;
});

function parseCell(cell) {
  // 先保存所有 Markdown 链接,避免被误识别为元数据
  const links = [];
  let tempCell = cell.replace(/\[([^\]]+)\]\(([^)]+)\)/g, (match, text, url) => {
    links.push({ text, url });
    return `__LINK_${links.length - 1}__`;
  });

  // 现在匹配元数据 [rowspan=2,colspan=3]
  // 元数据格式: 必须包含 = 号
  const metaMatch = tempCell.match(/\[([^\]]*=+[^\]]*)\]/);
  let text = tempCell;
  const meta = {};

  if (metaMatch) {
    // 找到了元数据
    metaMatch[1].split(",").forEach(pair => {
      const [k, v] = pair.split("=").map(s => s.trim());
      if (k && v) {
        meta[k] = Number(v);
      }
    });
    // 移除元数据部分
    text = tempCell.replace(/\[([^\]]*=+[^\]]*)\]/, "").trim();
  }

  // 恢复 Markdown 链接
  links.forEach((link, idx) => {
    text = text.replace(`__LINK_${idx}__`, `[${link.text}](${link.url})`);
  });

  // 使用 markdown-it 渲染(renderInline 避免生成 <p> 标签)
  text = md.renderInline(text);

  return { text, ...meta };
}
</script>

<template>
  <div class="vp-raw">
    <table>
      <thead v-if="headerRow">
        <tr>
          <th v-for="(raw, cIdx) in headerRow" :key="cIdx" v-bind="parseCell(raw)">
            <span v-html="parseCell(raw).text"></span>
          </th>
        </tr>
      </thead>
      <tbody>
        <tr v-for="(row, rIdx) in bodyRows" :key="rIdx">
          <td v-for="(raw, cIdx) in row" :key="cIdx" v-bind="parseCell(raw)">
            <span v-html="parseCell(raw).text"></span>
          </td>
        </tr>
      </tbody>
    </table>
  </div>
</template>

<style scoped>
:deep(table) {
  display: table;
  border-collapse: collapse;
  margin: 16px 0;
  width: 100%;
  overflow-x: auto;
}

:deep(thead) {
  border-bottom: 1px solid var(--vp-c-divider);
}

:deep(th) {
  border: 1px solid var(--vp-c-divider);
  padding: 8px 16px;
  font-weight: 600;
  text-align: left;
  background-color: var(--vp-c-bg-soft);
  color: var(--vp-c-text-1);
}

:deep(td) {
  border: 1px solid var(--vp-c-divider);
  padding: 8px 16px;
  color: var(--vp-c-text-2);
}

:deep(tbody tr) {
  background-color: var(--vp-c-bg);
  transition: background-color 0.25s;
}

:deep(tbody tr:nth-child(even)) {
  background-color: var(--vp-c-bg-soft);
}

:deep(tbody tr:hover) {
  background-color: var(--vp-c-bg-mute);
}

/* Markdown 元素样式 */
:deep(code) {
  background-color: var(--vp-c-bg-mute);
  padding: 2px 6px;
  border-radius: 4px;
  font-size: 0.875em;
  font-family: var(--vp-font-family-mono);
  color: var(--vp-c-text-code);
}

:deep(a) {
  color: var(--vp-c-brand-1);
  text-decoration: none;
  transition: color 0.25s;
}

:deep(a:hover) {
  color: var(--vp-c-brand-2);
  text-decoration: underline;
}

:deep(strong) {
  font-weight: 600;
  color: var(--vp-c-text-1);
}

:deep(em) {
  font-style: italic;
}

:deep(del) {
  text-decoration: line-through;
  opacity: 0.7;
}
</style>
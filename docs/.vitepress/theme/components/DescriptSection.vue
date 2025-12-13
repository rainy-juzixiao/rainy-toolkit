<template>
  <div class="descript-section vp-doc">
    <div ref="slotContainer" style="display: none;">
      <slot></slot>
    </div>
    <strong style="font-size: 18px; display: block; margin-bottom: 8px;">描述：</strong>
    <div v-html="htmlContent"></div>
    <strong v-if="category != 'none'" style="font-size: 18px; display: block; margin-bottom: 8px;">
      类别：{{ categoryName }}
    </strong>
  </div>
</template>

<script setup>
import { ref, onMounted, nextTick, onBeforeMount } from "vue";

const props = defineProps({
  rowSep: { type: String, default: ";;" },
  category: { type: String, default: "none" }
});

const slotContainer = ref(null);
const htmlContent = ref('');

let categoryName = "";

let categoryMap = new Map([
  ["ctor", "构造函数"],
  ["dtor", "析构函数"],
  ["copy", "拷贝函数"],
  ["move", "移动函数"],
  ["assign", "赋值"],
  ["accessor", "访问器"],
  ["modifier", "修改器"],
  ["comparator", "比较"],
  ["converter", "转换"]
]);

onMounted(async () => {
  await nextTick();
  if (slotContainer.value) {
    let html = slotContainer.value.innerHTML;
    if (html.includes(props.rowSep)) {
      const parts = html.split(props.rowSep);
      htmlContent.value = parts
        .map(part => part.trim())
        .filter(part => part)
        .join('<br>');
    } else {
      htmlContent.value = html;
    }
  }
});

onBeforeMount(() => {
  if (props.category !== "none" && categoryMap.has(props.category)) {
    categoryName = categoryMap.get(props.category);
  }
});
</script>

<style scoped>
.descript-section {
  margin: 16px 0;
}

.descript-section :deep(.header-anchor) {
  display: none;
}

.descript-section :deep(h1:first-child),
.descript-section :deep(h2:first-child),
.descript-section :deep(h3:first-child),
.descript-section :deep(p:first-child),
.descript-section :deep(ul:first-child),
.descript-section :deep(ol:first-child) {
  margin-top: 0;
}
</style>
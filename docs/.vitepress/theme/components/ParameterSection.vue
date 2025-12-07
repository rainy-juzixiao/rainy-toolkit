<template>
  <div class="parameter-section vp-doc">
    <div ref="slotContainer" style="display: none;">
      <slot></slot>
    </div>
    <strong v-if="isTypeParameter" style="font-size: 18px; display: block; margin-bottom: 8px;">类型参数：</strong>
    <strong v-else style="font-size: 18px; display: block; margin-bottom: 8px;">参数：</strong>
    <div v-html="htmlContent"></div>
  </div>
</template>

<script setup>
import { ref, onMounted, nextTick } from "vue";

const props = defineProps({
  rowSep: { type: String, default: ";;" },
  isTypeParameter: { type: Boolean, default: false }
});

const slotContainer = ref(null);
const htmlContent = ref('');

onMounted(async () => {
  await nextTick();
  if (slotContainer.value) {
    let html = slotContainer.value.innerHTML;
    if (html.includes(props.rowSep)) {
      const parts = html.split(props.rowSep);
      htmlContent.value = parts
        .map(part => part.trim())
        .filter(part => part)
        .join('<br>'); // 添加间距
    } else {
      htmlContent.value = html;
    }
  }
});
</script>

<style scoped>
.parameter-section {
  margin: 16px 0;
}

.parameter-section :deep(.header-anchor) {
  display: none;
}

.parameter-section :deep(h1:first-child),
.parameter-section :deep(h2:first-child),
.parameter-section :deep(h3:first-child),
.parameter-section :deep(p:first-child),
.parameter-section :deep(ul:first-child),
.parameter-section :deep(ol:first-child) {
  margin-top: 0;
}
</style>
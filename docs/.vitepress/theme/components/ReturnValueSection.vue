<template>
  <div class="return-value-section vp-doc">
    <div ref="slotContainer" style="display: none;">
      <slot></slot>
    </div>
    <strong style="font-size: 18px; display: block; margin-bottom: 8px;">返回值：</strong>
    <div v-html="htmlContent"></div>
    <br>
    <div v-if="noDiscardForRet">
        <strong>注意：该方法返回值应被调用方处理。</strong>
        <br>
        <strong v-if="noDiscardReaason != null && typeof noDiscardReaason == String && noDiscardReaason.length > 0">
            理由：{{ noDiscardReaason }}
        </strong>
    </div>
  </div>
</template>

<script setup>
import { ref, onMounted, nextTick } from "vue";

const props = defineProps({
  rowSep: { type: String, default: ";;" },
  noDiscardForRet: { type: Boolean, default: false },
  noDiscardReaason: { type: String, default: "" }
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
        .join('<br>');
    } else {
      htmlContent.value = html;
    }
  }
});
</script>

<style scoped>
.return-value-section {
  margin: 16px 0;
}

.return-value-section :deep(.header-anchor) {
  display: none;
}

.return-value-section :deep(h1:first-child),
.return-value-section :deep(h2:first-child),
.return-value-section :deep(h3:first-child),
.return-value-section :deep(p:first-child),
.return-value-section :deep(ul:first-child),
.return-value-section :deep(ol:first-child) {
  margin-top: 0;
}
</style>
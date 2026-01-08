<template>
  <div class="attention-section vp-doc">
    <div ref="slotContainer" style="display: none;">
      <slot></slot>
    </div>
    <strong style="font-size: 18px; display: block; margin-bottom: 8px;"> {{ levelName }}</strong>
    <div v-html="htmlContent"></div>
    <br>
  </div>
</template>

<script setup>
import { ref, onMounted, nextTick, onBeforeMount } from "vue";

const props = defineProps({
  rowSep: { type: String, default: ";;" },
  level: { type: String, default: "as_reminder" }
});

const slotContainer = ref(null);
const htmlContent = ref('');

let levelName = "";

let levelMap = new Map([
  ["as_reminder", "提醒："],
  ["as_warning", "警告："],
  ["as_must_attention", "必须注意："]
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
  levelName = levelMap.get(props.level) || "注意：";
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
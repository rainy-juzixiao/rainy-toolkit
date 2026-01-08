<template>
  <table class="array-table">
    <thead>
      <tr>
        <th v-for="(title, index) in titles" :key="index">{{ title }}</th>
      </tr>
    </thead>
    <tbody>
      <tr v-for="(row, rowIndex) in data" :key="rowIndex">
        <td v-for="(cell, cellIndex) in row" :key="cellIndex">{{ cell }}</td>
      </tr>
    </tbody>
  </table>
</template>

<script setup>
defineProps({
  titles: { type: Array, required: true },
  data: { type: Array, required: true }
})
</script>

<style scoped>
.array-table {
  width: 100%;               /* 占满容器宽度 */
  border-collapse: collapse; 
  table-layout: auto;        /* 根据内容自动分配列宽 */
}

.array-table th,
.array-table td {
  padding: 0.75em 1em;
  border: 1px solid var(--vp-c-divider);
  text-align: left;          /* 可改为 center 居中 */
  min-width: 120px;          /* 每列最小宽度，防止太紧凑 */
}

.array-table th {
  background-color: var(--vp-c-bg-secondary);
  font-weight: 600;
}

.array-table td {
  background-color: var(--vp-c-bg-primary);
}
</style>

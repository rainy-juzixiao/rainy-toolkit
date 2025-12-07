<script setup lang="ts">
import { ref, onMounted } from 'vue';
import { codeToHtml } from 'shiki';

interface Props {
  header?: string;
  content: string;
  no_syntax_label?: boolean;
}

const props = defineProps<Props>();

interface SinceInfo {
  type: 'since' | 'constexpr';
  version: string;
}

interface Declaration {
  number: string;
  code: string;
  since: SinceInfo[];
}

interface HighlightedDeclaration extends Declaration {
  lightHighlighted: string;
  darkHighlighted: string;
}

interface GroupedDeclaration {
  number: string;
  declarations: HighlightedDeclaration[];
}

const highlightedGroups = ref<GroupedDeclaration[]>([]);
const isLoading = ref(true);

function parseDeclarations(content: string): Declaration[] {
  const lines = content
    .trim()
    .split('\n')
    .filter((line) => line.trim());
  const declarations: Declaration[] = [];

  let currentDeclaration = '';
  let currentNumber = '';
  let currentSince: SinceInfo[] = [];
  let inDeclaration = false;

  for (const line of lines) {
    const trimmedLine = line.trim();

    const numberMatch = trimmedLine.match(/^\((\S+)\)(.*)$/);
    if (numberMatch) {
      if (currentDeclaration && currentNumber) {
        declarations.push({
          number: currentNumber,
          code: currentDeclaration.trim(),
          since: currentSince,
        });
      }

      currentNumber = numberMatch[1];
      currentDeclaration = '';
      currentSince = [];
      inDeclaration = true;

      const sinceText = numberMatch[2];
      const sinceMatches = sinceText.matchAll(
        /\((?:(constexpr\s+)?since\s+(.+?))\)/g,
      );

      for (const match of sinceMatches) {
        const isConstexpr = !!match[1];
        const version = match[2];
        currentSince.push({
          type: isConstexpr ? 'constexpr' : 'since',
          version: version,
        });
      }

      continue;
    }

    if (inDeclaration) {
      currentDeclaration += line + '\n';
    } else if (!currentNumber) {
      currentDeclaration += line + '\n';
      currentNumber = '1';
      inDeclaration = true;
    }
  }

  if (currentDeclaration && currentNumber) {
    declarations.push({
      number: currentNumber,
      code: currentDeclaration.trim(),
      since: currentSince,
    });
  }

  return declarations;
}

function groupDeclarations(declarations: Declaration[]) {
  const grouped: { [key: string]: Declaration[] } = {};

  for (const decl of declarations) {
    if (!grouped[decl.number]) {
      grouped[decl.number] = [];
    }
    grouped[decl.number].push(decl);
  }

  return Object.entries(grouped).map(([number, decls]) => ({
    number,
    declarations: decls,
  }));
}

async function highlightDeclarations() {
  const declarations = parseDeclarations(props.content);
  const groupedDeclarations = groupDeclarations(declarations);

  const groups = await Promise.all(
    groupedDeclarations.map(async (group) => {
      const highlightedDeclarations = await Promise.all(
        group.declarations.map(async (decl) => {
          const [lightHighlighted, darkHighlighted] = await Promise.all([
            codeToHtml(decl.code, {
              lang: 'cpp',
              theme: 'github-light',
            }),
            codeToHtml(decl.code, {
              lang: 'cpp',
              theme: 'github-dark',
            }),
          ]);

          return {
            ...decl,
            lightHighlighted,
            darkHighlighted,
          };
        }),
      );

      return {
        number: group.number,
        declarations: highlightedDeclarations,
      };
    }),
  );

  return groups;
}

onMounted(async () => {
  highlightedGroups.value = await highlightDeclarations();
  isLoading.value = false;
});
</script>

<template>
  <strong v-if="no_syntax_label != true" style="font-size: 18px;">语法：</strong>
  <div v-if="!isLoading" class="declaration-table not-content">
    <div class="declaration-header" v-if="header != null">
      Defined in header &lt;<span class="header-name">{{ header }}</span>&gt;
    </div>
    <div class="declarations-container">
      <div
        v-for="group in highlightedGroups"
        :key="group.number"
        class="declaration-group"
      >
        <div class="declaration-row">
          <div class="declaration-content-wrapper">
            <div
              v-for="(decl, index) in group.declarations"
              :key="index"
              class="declaration-content"
            >
              <div class="declaration-code-wrapper">
                <div class="light-code" v-html="decl.lightHighlighted" />
                <div class="dark-code" v-html="decl.darkHighlighted" />
              </div>
              <div v-if="decl.since.length > 0" class="since-info">
                <span
                  v-for="(since, sinceIndex) in decl.since"
                  :key="sinceIndex"
                  :class="['since-tag', since.type]"
                >
                  {{
                    since.type === 'constexpr'
                      ? `constexpr since ${since.version}`
                      : `since ${since.version}`
                  }}
                </span>
              </div>
            </div>
          </div>
          <div class="declaration-meta">
            <span class="declaration-number">({{ group.number }})</span>
          </div>
        </div>
      </div>
    </div>
  </div>
  <div v-else class="declaration-loading">
    Loading...
  </div>
</template>

<style scoped>
.declaration-table {
  border: 1px solid #e1e5e9;
  border-radius: 6px;
  margin: 1.5rem 0;
  overflow: hidden;
  background: white;
}

.declaration-loading {
  padding: 1rem;
  text-align: center;
  color: #656d76;
}

.declaration-header {
  background: #f6f8fa;
  padding: 0.75rem 1rem;
  border-bottom: 1px solid #e1e5e9;
  font-size: 0.9rem;
  color: #656d76;
}

.header-name {
  color: #0969da;
  font-family: 'SFMono-Regular', Consolas, 'Liberation Mono', Menlo, monospace;
}

.declarations-container {
  background: white;
}

.declaration-group:not(:last-child) {
  border-bottom: 1px solid #e1e5e9;
}

.declaration-row {
  display: flex;
  align-items: flex-start;
  padding: 1rem;
  gap: 1rem;
  align-items: center;
}

.declaration-content-wrapper {
  flex: 1;
  min-width: 0;
  position: relative;
  display: flex;
  flex-direction: column;
}

.declaration-content {
  display: flex;
  flex-direction: row;
  padding: 0.5rem;
  align-items: center;
  justify-content: space-between;
}

.declaration-content:not(:last-child) {
  border-bottom: 1px solid var(--border-color);
}

.declaration-code-wrapper {
  flex: 1;
  overflow-x: auto;
  overflow-y: hidden;
  position: relative;
  scrollbar-width: none;
  -ms-overflow-style: none;
  mask: linear-gradient(
    to right,
    black 0%,
    black calc(100% - 20px),
    transparent 100%
  );
  -webkit-mask: linear-gradient(
    to right,
    black 0%,
    black calc(100% - 20px),
    transparent 100%
  );
}

.declaration-content::-webkit-scrollbar {
  display: none;
}

.since-info {
  display: flex;
  gap: 0.5rem;
  flex-wrap: wrap;
  flex-direction: column;
  align-items: flex-end;
}

.since-tag {
  font-size: 0.75rem;
  padding: 0.15rem 0.5rem;
  border-radius: 12px;
  white-space: nowrap;
  font-weight: 500;
}

.since-tag.since {
  color: #1a7f37;
  background: #dafbe1;
}

.since-tag.constexpr {
  color: #8250df;
  background: #fbefff;
}

.declaration-meta {
  display: flex;
  align-items: center;
  gap: 0.25rem;
  flex-shrink: 0;
  margin: 0;
}

.declaration-number {
  font-size: 0.9rem;
  color: #656d76;
  font-weight: 500;
}

.light-code {
  display: block;
}

.dark-code {
  display: none;
}

:deep(.declaration-content pre),
:deep(.declaration-content pre[class*='shiki']) {
  margin: 0 !important;
  padding: 0 !important;
  background: transparent !important;
  background-color: transparent !important;
  border: none !important;
  border-radius: 0 !important;
  box-shadow: none !important;
  font-size: 0.9rem !important;
  line-height: 1.5 !important;
  overflow: visible !important;
}

:deep(.declaration-content code),
:deep(.declaration-content code[class*='shiki']) {
  background: transparent !important;
  background-color: transparent !important;
  padding: 0 !important;
  border: none !important;
  border-radius: 0 !important;
  box-shadow: none !important;
  display: block !important;
}

/* Dark mode styles - VitePress uses .dark class */
.dark .declaration-table {
  border-color: #30363d;
  background: #0d1117;
}

.dark .declaration-header {
  background: #161b22;
  border-color: #30363d;
  color: #8b949e;
}

.dark .header-name {
  color: #58a6ff;
}

.dark .declarations-container {
  background: #0d1117;
}

.dark .declaration-group:not(:last-child) {
  border-color: #21262d;
}

.dark .declaration-number {
  color: #8b949e;
}

.dark .since-tag.since {
  color: #238636;
  background: #1a2e1a;
}

.dark .since-tag.constexpr {
  color: #a5a3ff;
  background: #2d1b3d;
}

.dark .light-code {
  display: none;
}

.dark .dark-code {
  display: block;
}

@media (max-width: 768px) {
  .declaration-row {
    flex-direction: column;
    gap: 0.75rem;
  }

  .declaration-meta {
    align-items: center;
    align-self: flex-start;
  }

  .declaration-content-wrapper {
    width: 100%;
  }
}
</style>
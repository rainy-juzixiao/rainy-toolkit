<script setup lang="ts">
import { ref, onMounted } from 'vue';
import { codeToHtml } from 'shiki';
import rainyWarm from '../shiki/rainy-warm';

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
  highlighted: string;
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
          const highlighted = await codeToHtml(decl.code, {
            lang: 'cpp',
            theme: rainyWarm,
          });

          return {
            ...decl,
            highlighted,
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
                <div class="declaration-code" v-html="decl.highlighted" />
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
  border: 1px solid rgba(240, 238, 230, 0.1);
  border-radius: 10px;
  margin: 1.5rem 0;
  overflow: hidden;
  background: var(--rt-code-bg);
  box-shadow: 0 1px 0 rgba(20, 20, 19, 0.04);
}

.declaration-loading {
  padding: 1rem;
  text-align: center;
  color: var(--rt-code-muted);
}

.declaration-header {
  background: rgba(240, 238, 230, 0.04);
  padding: 0.7rem 1rem;
  border-bottom: 1px solid rgba(240, 238, 230, 0.08);
  font-family: var(--rt-font-mono);
  font-size: 0.7rem;
  letter-spacing: 0.1em;
  text-transform: uppercase;
  color: rgba(240, 238, 230, 0.45);
}

.header-name {
  color: var(--rt-accent-bright);
  font-family: var(--rt-font-mono);
  text-transform: none;
  letter-spacing: 0;
}

.declarations-container {
  background: transparent;
}

.declaration-group:not(:last-child) {
  border-bottom: 1px solid rgba(240, 238, 230, 0.08);
}

.declaration-row {
  display: flex;
  align-items: center;
  padding: 0.9rem 1rem;
  gap: 1rem;
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
  border-bottom: 1px solid rgba(240, 238, 230, 0.06);
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
  gap: 0.4rem;
  flex-wrap: wrap;
  flex-direction: column;
  align-items: flex-end;
}

.since-tag {
  font-family: var(--rt-font-mono);
  font-size: 0.64rem;
  letter-spacing: 0.06em;
  padding: 0.15rem 0.5rem;
  border-radius: 4px;
  white-space: nowrap;
  font-weight: 500;
}

.since-tag.since {
  color: #a9b99a;
  background: rgba(169, 185, 154, 0.14);
}

.since-tag.constexpr {
  color: #b09ac7;
  background: rgba(176, 154, 199, 0.16);
}

.declaration-meta {
  display: flex;
  align-items: center;
  gap: 0.25rem;
  flex-shrink: 0;
  margin: 0;
}

.declaration-number {
  font-family: var(--rt-font-mono);
  font-size: 0.76rem;
  color: rgba(240, 238, 230, 0.42);
  font-weight: 500;
}

.declaration-code {
  display: block;
}

:deep(.declaration-code pre),
:deep(.declaration-code pre[class*='shiki']) {
  margin: 0 !important;
  padding: 0 !important;
  background: transparent !important;
  background-color: transparent !important;
  border: none !important;
  border-radius: 0 !important;
  box-shadow: none !important;
  font-size: 0.86rem !important;
  line-height: 1.65 !important;
  overflow: visible !important;
}

:deep(.declaration-code code),
:deep(.declaration-code code[class*='shiki']) {
  background: transparent !important;
  background-color: transparent !important;
  padding: 0 !important;
  border: none !important;
  border-radius: 0 !important;
  box-shadow: none !important;
  display: block !important;
}
</style>

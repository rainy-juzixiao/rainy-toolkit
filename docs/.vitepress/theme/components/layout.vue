<script setup lang="ts">
import DefaultTheme from "vitepress/theme";
import { useData } from "vitepress";
import { nextTick, provide } from "vue";

const { isDark } = useData();

const enableTransitions = () =>
  "startViewTransition" in document &&
  window.matchMedia("(prefers-reduced-motion: no-preference)").matches;

provide("toggle-appearance", async ({ clientX: x, clientY: y }: MouseEvent) => {
  if (!enableTransitions()) {
    isDark.value = !isDark.value;
    return;
  }

  const clipPath = [
    `circle(0px at ${x}px ${y}px)`,
    `circle(${Math.hypot(
      Math.max(x, innerWidth - x),
      Math.max(y, innerHeight - y)
    )}px at ${x}px ${y}px)`,
  ];

  await document.startViewTransition(async () => {
    isDark.value = !isDark.value;
    await nextTick();
  }).ready;

  document.documentElement.animate(
    { clipPath: isDark.value ? clipPath.reverse() : clipPath },
    {
      duration: 300,
      easing: "ease-in",
      pseudoElement: `::view-transition-${isDark.value ? "old" : "new"}(root)`,
    }
  );
});
</script>

<template>
  <DefaultTheme.Layout>
    <template #home-hero-image>
      <svg class="rt-hero-mark" viewBox="0 0 320 360" fill="none" aria-hidden="true">
        <g class="rt-rain">
          <path d="M44 236 L32 266" />
          <path d="M92 268 L80 298" />
          <path d="M148 240 L136 270" />
          <path d="M206 274 L194 304" />
          <path d="M266 238 L254 268" />
          <path d="M22 190 L10 220" />
          <path d="M298 196 L286 226" />
        </g>
        <path
          class="rt-canopy"
          d="M44 176 C70 88 250 88 276 176
             Q247 200 218 176 Q189 200 160 176
             Q131 200 102 176 Q73 200 44 176 Z"
        />
        <path class="rt-panel" d="M160 78 L160 176 Q189 200 218 176 L276 176 C250 88 160 78 160 78 Z" />
        <g class="rt-ribs">
          <path d="M160 78 L44 176" />
          <path d="M160 78 L102 176" />
          <path d="M160 78 L160 176" />
          <path d="M160 78 L218 176" />
          <path d="M160 78 L276 176" />
        </g>
        <path class="rt-shaft" d="M160 78 L160 300 Q160 322 142 322 Q126 322 126 306" />
      </svg>
    </template>
  </DefaultTheme.Layout>
</template>

<style>
::view-transition-old(root),
::view-transition-new(root) {
  animation: none;
  mix-blend-mode: normal;
}

::view-transition-old(root),
.dark::view-transition-new(root) {
  z-index: 1;
}

::view-transition-new(root),
.dark::view-transition-old(root) {
  z-index: 9999;
}

.VPSwitchAppearance .check {
  transform: none !important;
}

/* 修正因视图过渡导致的按钮图标偏移 */
.VPSwitchAppearance .check .icon {
  top: -2px;
}

</style>

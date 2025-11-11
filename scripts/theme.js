// scripts/theme.js
(() => {
  const KEY = "site-theme";
  const THEMES = ["", "theme-emerald", "theme-dark", "theme-indigo"]; // order = dropdown options
  const html = document.documentElement;

  function applyTheme(theme) {
    // remove any existing theme-* classes
    Array.from(html.classList)
      .filter(c => c.startsWith("theme-"))
      .forEach(c => html.classList.remove(c));

    if (theme) html.classList.add(theme);
    localStorage.setItem(KEY, theme);

    // optional: tint mobile address bar to header color
    const meta = document.querySelector('meta[name="theme-color"]') || (() => {
      const m = document.createElement('meta');
      m.name = 'theme-color';
      document.head.appendChild(m);
      return m;
    })();
    const cs = getComputedStyle(html);
    meta.setAttribute('content', cs.getPropertyValue('--header-bg').trim() || '#000000');
  }

  // Apply saved theme ASAP (default = "")
  applyTheme(localStorage.getItem(KEY) || "");

  // Event delegation so it works even if nav is injected later
  document.addEventListener("change", (e) => {
    const sel = e.target.closest("#themeSelect");
    if (!sel) return;
    applyTheme(sel.value);
  });

  // Ensure the select exists & reflects current theme
  function ensureSelectValue() {
    const sel = document.getElementById("themeSelect");
    if (!sel) return;
    const saved = localStorage.getItem(KEY) || "";
    sel.value = THEMES.includes(saved) ? saved : "";
  }

  if (document.readyState === "loading") {
    document.addEventListener("DOMContentLoaded", ensureSelectValue);
  } else {
    ensureSelectValue();
  }
})();

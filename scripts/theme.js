const THEME_KEY = "site-theme";
  const THEMES = ["", "theme-indigo", "theme-emerald", "theme-dark"]; 
  const html = document.documentElement;

  // Apply saved theme ASAP (works even before the button exists)
  (function applySavedTheme(){
    const saved = localStorage.getItem(THEME_KEY);
    if (saved) {
      // clear any previous theme-* classes first
      html.classList.forEach(c => { if (c.startsWith("theme-")) html.classList.remove(c); });
      if (saved) html.classList.add(saved);
    }
  })();

  // Event delegation: works even if #themeToggle is injected later
  document.addEventListener("click", (e) => {
    const btn = e.target.closest("#themeToggle");
    if (!btn) return;

    // find current theme class on <html>
    const current = Array.from(html.classList).find(c => c.startsWith("theme-")) || "";
    // compute next theme
    const nextIndex = (THEMES.indexOf(current) + 1) % THEMES.length;

    // remove any theme-* classes, then add next (if not default "")
    THEMES.forEach(t => t && html.classList.remove(t));
    const next = THEMES[nextIndex];
    if (next) html.classList.add(next);

    localStorage.setItem(THEME_KEY, next);
  });

console.log("[theme] init");
document.addEventListener("click", (e) => {
  const btn = e.target.closest("#themeToggle");
  if (!btn) return;
  console.log("[theme] button clicked");
});
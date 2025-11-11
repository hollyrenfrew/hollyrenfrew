document.addEventListener('DOMContentLoaded', () => {
  const inProjects = window.location.pathname.includes('/projects/');
  const prefix = inProjects ? '..' : '.';

  fetch(`${prefix}/partials/nav.html`)
    .then(res => res.text())
    .then(html => {
      const target = document.getElementById('nav');
      if (!target) return;
      target.innerHTML = html;

      // Build correct hrefs for GitHub Pages and highlight the active link
      const repo = window.location.pathname.split('/')[1] || '';
      const root = repo ? `/${repo}` : '';
      let current = window.location.pathname.replace(root + '/', '');
      if (current === '' || current.endsWith('/')) current = 'index.html';

      // Supports nav links written as: <a data-path="projects/airgead.html">Airgead</a>
      const links = target.querySelectorAll('a[data-path]');
      links.forEach(a => {
        const rel = a.getAttribute('data-path');
        if (!rel) return;
        a.href = `${root}/${rel}`;
        if (rel === current) a.classList.add('active');
      });
    })
    .catch(err => console.error('Nav load error:', err));
});

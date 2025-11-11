document.addEventListener('DOMContentLoaded', () => {
  const inProjects = window.location.pathname.includes('/projects/');
  const prefix = inProjects ? '..' : '.';

  fetch(`${prefix}/partials/header.html`)
    .then(res => res.text())
    .then(html => {
      const target = document.getElementById('header');
      if (target) target.innerHTML = html;
    })
    .catch(err => console.error('Header load error:', err));
});

document.addEventListener('DOMContentLoaded', () => {
  const inProjects = window.location.pathname.includes('/projects/');
  const prefix = inProjects ? '..' : '.';

  fetch(`${prefix}/partials/footer.html`)
    .then(res => res.text())
    .then(html => {
      const target = document.getElementById('footer');
      if (target) target.innerHTML = html;
    })
    .catch(err => console.error('Footer load error:', err));
});

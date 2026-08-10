(() => {
  const REPO = 'masarray/obs-gamepad-hotkeys';
  const FALLBACK_RELEASE = `https://github.com/${REPO}/releases/latest`;

  const header = document.querySelector('[data-header]');
  const updateHeader = () => {
    if (!header) return;
    header.classList.toggle('scrolled', window.scrollY > 18);
  };

  updateHeader();
  window.addEventListener('scroll', updateHeader, { passive: true });

  const revealNodes = [...document.querySelectorAll('.reveal')];
  const reducedMotion = window.matchMedia('(prefers-reduced-motion: reduce)').matches;

  if (reducedMotion || !('IntersectionObserver' in window)) {
    revealNodes.forEach((node) => node.classList.add('visible'));
  } else {
    const observer = new IntersectionObserver((entries, obs) => {
      entries.forEach((entry) => {
        if (!entry.isIntersecting) return;
        entry.target.classList.add('visible');
        obs.unobserve(entry.target);
      });
    }, { rootMargin: '0px 0px -7% 0px', threshold: 0.08 });

    revealNodes.forEach((node) => observer.observe(node));
  }

  const downloadLinks = [...document.querySelectorAll('[data-download]')];
  const versionNodes = [...document.querySelectorAll('[data-version]')];

  const setRelease = (release) => {
    if (!release) return;

    const version = release.tag_name || 'Latest';
    versionNodes.forEach((node) => {
      node.textContent = version;
    });

    const installer = Array.isArray(release.assets)
      ? release.assets.find((asset) => /^OBS-Gamepad-Hotkeys-Setup-v.+\.exe$/i.test(asset.name))
      : null;

    const target = installer?.browser_download_url || release.html_url || FALLBACK_RELEASE;
    downloadLinks.forEach((link) => {
      link.href = target;
    });

    const schema = document.querySelector('script[type="application/ld+json"]');
    if (schema) {
      try {
        const data = JSON.parse(schema.textContent);
        data.softwareVersion = version.replace(/^v/i, '');
        data.downloadUrl = target;
        schema.textContent = JSON.stringify(data);
      } catch (_) {
        // Static metadata remains valid if dynamic enhancement is unavailable.
      }
    }
  };

  fetch(`https://api.github.com/repos/${REPO}/releases/latest`, {
    headers: { Accept: 'application/vnd.github+json' }
  })
    .then((response) => {
      if (!response.ok) throw new Error(`GitHub release lookup failed: ${response.status}`);
      return response.json();
    })
    .then(setRelease)
    .catch(() => {
      downloadLinks.forEach((link) => {
        link.href = FALLBACK_RELEASE;
      });
    });

  document.querySelectorAll('.faq-item').forEach((details) => {
    details.addEventListener('toggle', () => {
      if (!details.open) return;
      document.querySelectorAll('.faq-item[open]').forEach((other) => {
        if (other !== details) other.open = false;
      });
    });
  });
})();

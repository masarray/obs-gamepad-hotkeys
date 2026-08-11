(() => {
  const REPO = 'masarray/obs-gamepad-hotkeys';
  const FALLBACK_RELEASE = `https://github.com/${REPO}/releases/latest`;

  const polish = document.createElement('style');
  polish.textContent = `
    .hero-reassurance {
      display: flex;
      flex-wrap: wrap;
      gap: 8px 16px;
      margin-top: 18px;
      color: #8294a9;
      font-size: 12px;
      font-weight: 650;
    }
    .hero-reassurance span { white-space: nowrap; }
    .steps-four { grid-template-columns: repeat(4, 1fr); }
    .step a { color: #b9d7fb; text-decoration: underline; text-decoration-color: rgba(185,215,251,.25); text-underline-offset: 3px; }
    .security-section {
      padding-top: 30px;
      padding-bottom: 100px;
    }
    .security-panel {
      position: relative;
      overflow: hidden;
      padding: clamp(30px, 5vw, 54px);
      border: 1px solid rgba(110,231,183,.18);
      border-radius: 28px;
      background:
        radial-gradient(circle at 90% 0%, rgba(16,185,129,.1), transparent 30rem),
        linear-gradient(145deg, rgba(11,24,37,.94), rgba(5,13,23,.92));
      box-shadow: 0 28px 80px rgba(0,0,0,.28), inset 0 1px 0 rgba(255,255,255,.035);
    }
    .security-heading {
      display: grid;
      grid-template-columns: auto 1fr;
      gap: 22px;
      align-items: start;
    }
    .security-heading h2,
    .beginner-copy h2 {
      margin: 0;
      font-size: clamp(38px, 4.6vw, 60px);
      font-weight: 760;
      letter-spacing: -.05em;
      line-height: 1.02;
    }
    .security-heading h2 span,
    .beginner-copy h2 span {
      background: linear-gradient(100deg, #d1fae5 0%, #93c5fd 55%, #60a5fa 100%);
      -webkit-background-clip: text;
      background-clip: text;
      color: transparent;
    }
    .security-heading > div > p:last-child {
      max-width: 850px;
      margin: 20px 0 0;
      color: #91a3b5;
      font-size: 16px;
    }
    .security-icon {
      width: 58px;
      height: 58px;
      display: grid;
      place-items: center;
      border: 1px solid rgba(110,231,183,.22);
      border-radius: 18px;
      background: rgba(16,185,129,.08);
      color: #86efac;
      box-shadow: inset 0 1px 0 rgba(255,255,255,.04);
    }
    .security-icon svg {
      width: 28px;
      height: 28px;
      fill: none;
      stroke: currentColor;
      stroke-width: 1.7;
      stroke-linecap: round;
      stroke-linejoin: round;
    }
    .security-trust-grid {
      display: grid;
      grid-template-columns: repeat(3, 1fr);
      gap: 14px;
      margin-top: 34px;
    }
    .security-trust-item {
      min-height: 185px;
      padding: 22px;
      border: 1px solid rgba(148,163,184,.13);
      border-radius: 18px;
      background: rgba(15,23,42,.48);
    }
    .security-trust-item strong { color: #e6eef6; font-size: 15px; }
    .security-trust-item p { margin: 9px 0 0; color: #8293a7; font-size: 13px; }
    .security-trust-item .text-link { margin-top: 16px; font-size: 12px; }
    .security-note {
      display: flex;
      align-items: center;
      justify-content: space-between;
      gap: 26px;
      margin-top: 18px;
      padding: 20px 22px;
      border: 1px solid rgba(96,165,250,.14);
      border-radius: 17px;
      background: rgba(37,99,235,.055);
    }
    .security-note strong { color: #dce8f5; font-size: 13px; }
    .security-note p { max-width: 760px; margin: 5px 0 0; color: #8293a7; font-size: 12.5px; }
    .security-note .button { min-height: 42px; flex: none; font-size: 12px; }
    .beginner-section {
      border-block: 1px solid rgba(148,163,184,.08);
      background: rgba(4,10,18,.4);
    }
    .beginner-grid {
      display: grid;
      grid-template-columns: .78fr 1.22fr;
      gap: clamp(44px, 7vw, 90px);
      align-items: center;
    }
    .beginner-copy > p:not(.section-kicker) {
      max-width: 540px;
      margin: 22px 0 0;
      color: #8f9fb2;
      font-size: 16px;
    }
    .beginner-flow {
      display: grid;
      grid-template-columns: 1fr auto 1fr auto 1fr;
      gap: 12px;
      align-items: center;
    }
    .beginner-flow > div:not(.flow-arrow) {
      min-height: 160px;
      padding: 20px;
      border: 1px solid var(--line);
      border-radius: 18px;
      background: linear-gradient(150deg, rgba(15,23,42,.7), rgba(8,17,31,.6));
    }
    .beginner-flow span {
      width: 30px;
      height: 30px;
      display: grid;
      place-items: center;
      border-radius: 999px;
      background: rgba(59,130,246,.12);
      color: #bfdbfe;
      font-size: 11px;
      font-weight: 800;
    }
    .beginner-flow strong { display: block; margin-top: 25px; color: #e5edf6; font-size: 14px; }
    .beginner-flow small { display: block; margin-top: 5px; color: #718399; font-size: 11px; line-height: 1.5; }
    .flow-arrow { color: #45617e; font-size: 18px; }
    @media (max-width: 1000px) {
      .steps-four { grid-template-columns: 1fr; }
      .security-trust-grid { grid-template-columns: 1fr; }
      .security-trust-item { min-height: auto; }
      .security-note { align-items: flex-start; flex-direction: column; }
      .beginner-grid { grid-template-columns: 1fr; }
    }
    @media (max-width: 700px) {
      .hero-reassurance { display: grid; grid-template-columns: 1fr 1fr; }
      .security-panel { padding: 28px 20px; border-radius: 22px; }
      .security-heading { grid-template-columns: 1fr; }
      .security-heading h2, .beginner-copy h2 { font-size: clamp(36px, 11vw, 48px); }
      .security-note .button { width: 100%; }
      .beginner-flow { grid-template-columns: 1fr; }
      .flow-arrow { transform: rotate(90deg); justify-self: center; }
    }
  `;
  document.head.appendChild(polish);

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
  const checksumLinks = [...document.querySelectorAll('[data-checksum]')];
  const releasePageLinks = [...document.querySelectorAll('[data-release-page]')];
  const versionNodes = [...document.querySelectorAll('[data-version]')];
  const installerNameNodes = [...document.querySelectorAll('[data-installer-name]')];

  const setRelease = (release) => {
    if (!release) return;

    const version = release.tag_name || 'Latest';
    versionNodes.forEach((node) => {
      node.textContent = version;
    });

    const assets = Array.isArray(release.assets) ? release.assets : [];
    const installer = assets.find((asset) => /^OBS-Gamepad-Hotkeys-Setup-v.+\.exe$/i.test(asset.name));
    const checksum = installer
      ? assets.find((asset) => asset.name.toLowerCase() === `${installer.name}.sha256`.toLowerCase())
      : null;

    const downloadTarget = installer?.browser_download_url || release.html_url || FALLBACK_RELEASE;
    const releaseTarget = release.html_url || FALLBACK_RELEASE;
    const checksumTarget = checksum?.browser_download_url || releaseTarget;

    downloadLinks.forEach((link) => {
      link.href = downloadTarget;
    });
    checksumLinks.forEach((link) => {
      link.href = checksumTarget;
    });
    releasePageLinks.forEach((link) => {
      link.href = releaseTarget;
    });
    installerNameNodes.forEach((node) => {
      node.textContent = installer?.name || 'Setup EXE';
    });

    const schema = document.querySelector('script[type="application/ld+json"]');
    if (schema) {
      try {
        const data = JSON.parse(schema.textContent);
        data.softwareVersion = version.replace(/^v/i, '');
        data.downloadUrl = downloadTarget;
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
      downloadLinks.forEach((link) => { link.href = FALLBACK_RELEASE; });
      checksumLinks.forEach((link) => { link.href = FALLBACK_RELEASE; });
      releasePageLinks.forEach((link) => { link.href = FALLBACK_RELEASE; });
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

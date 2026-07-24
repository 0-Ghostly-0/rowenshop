/* =========================================================
   ROWEN — site behavior
   Reads content from SITE_CONFIG (assets/js/config.js, loaded
   before this file) and wires up every interactive piece:
   router, portfolio grids + filters, project detail modal,
   pricing/how-it-works/FAQ/reviews rendering, policy
   modals, mobile nav, and the commission form.
   ========================================================= */
(function(){
"use strict";

const CFG = window.SITE_CONFIG;

/* ---------- year ---------- */
const yearEl = document.getElementById('year');
if (yearEl) yearEl.textContent = new Date().getFullYear();

/* ---------- apply business info to static links ---------- */
document.querySelectorAll('[data-discord-link]').forEach(el => el.href = CFG.business.discordInvite);
document.querySelectorAll('[data-email-link]').forEach(el => el.href = `mailto:${CFG.business.email}?subject=Design%20Inquiry`);
document.querySelectorAll('[data-email-text]').forEach(el => el.textContent = CFG.business.email);
document.querySelectorAll('[data-instagram-link]').forEach(el => el.href = CFG.business.instagram);
document.querySelectorAll('[data-tiktok-link]').forEach(el => el.href = CFG.business.tiktok);
document.querySelectorAll('[data-cashapp-link]').forEach(el => el.href = CFG.business.cashapp);
document.querySelectorAll('[data-venmo-link]').forEach(el => el.href = CFG.business.venmo);
document.querySelectorAll('[data-min-budget]').forEach(el => el.textContent = CFG.business.minBudget);

// Stripe pay link is optional — only show it once a real Payment Link URL
// is set in config.js (avoids a dead/broken "Card" button before setup).
document.querySelectorAll('[data-stripe-link]').forEach(el => {
  if (CFG.business.stripe) {
    el.href = CFG.business.stripe;
    el.hidden = false;
  } else {
    el.hidden = true;
  }
});

/* ---------- preloader (opening animation) ---------- */
(function(){
  const preloader = document.getElementById('preloader');
  if(!preloader) return;
  const reduceMotion = window.matchMedia('(prefers-reduced-motion: reduce)').matches;
  const dismiss = () => preloader.classList.add('hide');
  if(reduceMotion){
    setTimeout(dismiss, 50);
  } else {
    const start = Date.now();
    const minShow = 1500;
    window.addEventListener('load', ()=>{
      const elapsed = Date.now() - start;
      setTimeout(dismiss, Math.max(0, minShow - elapsed));
    });
    setTimeout(dismiss, 4000);
  }
  preloader.addEventListener('transitionend', (e)=>{
    if(e.propertyName === 'opacity') preloader.remove();
  });
})();

/* ---------- custom cursor ---------- */
const dot = document.getElementById('cursor-dot');
const ring = document.getElementById('cursor-ring');
const hasFinePointer = window.matchMedia('(hover:hover) and (pointer:fine)').matches;
if (dot && ring && hasFinePointer) {
  let rx=0, ry=0, mx=0, my=0;
  window.addEventListener('mousemove', e=>{
    mx=e.clientX; my=e.clientY;
    dot.style.left=mx+'px'; dot.style.top=my+'px';
  });
  function loop(){
    rx += (mx-rx)*0.18; ry += (my-ry)*0.18;
    ring.style.left=rx+'px'; ring.style.top=ry+'px';
    requestAnimationFrame(loop);
  }
  loop();
}
function bindCursorTargets(){
  if (!ring) return;
  document.querySelectorAll('a,button,.card,input,textarea,select,.file-drop').forEach(el=>{
    if (el.dataset.cursorBound) return;
    el.dataset.cursorBound = '1';
    el.addEventListener('mouseenter', ()=>ring.classList.add('grow'));
    el.addEventListener('mouseleave', ()=>ring.classList.remove('grow'));
  });
}

/* ---------- header scroll state + progress bar ---------- */
const header = document.getElementById('siteHeader');
const progressBar = document.getElementById('scrollProgress');
window.addEventListener('scroll', ()=>{
  header.classList.toggle('scrolled', window.scrollY>30);
  const max = document.documentElement.scrollHeight - window.innerHeight;
  const pct = max>0 ? (window.scrollY/max)*100 : 0;
  progressBar.style.width = pct+'%';
},{passive:true});

/* ---------- page router ---------- */
const PAGE_KEYS = ['home','cover-art','thumbnails','ads','pfps','beats','vsts','reviews','order'];
const pageEls = {};
PAGE_KEYS.forEach(k=>{ pageEls[k] = document.querySelector(`.page[data-page="${k}"]`); });
const navLinkEls = document.querySelectorAll('.nav-links a[data-page], .mobile-nav-panel a[data-page]');
const dotLinkEls = document.querySelectorAll('.side-dots a[data-page]');
const PAGE_TITLES = {
  'home': `${CFG.business.name} — Design & Beats`,
  'cover-art': `Cover Art — ${CFG.business.name}`,
  'thumbnails': `YouTube Thumbnails — ${CFG.business.name}`,
  'ads': `Ad Creative — ${CFG.business.name}`,
  'pfps': `PFPs — ${CFG.business.name}`,
  'beats': `Beats — ${CFG.business.name}`,
  'vsts': `VSTs — ${CFG.business.name}`,
  'reviews': `Reviews — ${CFG.business.name}`,
  'order': `Start a Project — ${CFG.business.name}`
};

/* Groups pages into the nav's "Graphic Design" / "Music" dropdowns
   (header, side-dots, and mobile drawer). Any element carrying
   data-nav-group="design"/"music" gets .active whenever the current
   page is one of its members — see setActivePage below. */
const NAV_GROUPS = {
  design: ['cover-art','thumbnails','ads','pfps'],
  music: ['beats','vsts']
};
const navGroupEls = document.querySelectorAll('[data-nav-group]');
const serviceSelect = document.getElementById('service');
const SERVICE_LABELS = {
  'cover-art': 'Cover Art',
  'thumbnails': 'YouTube Thumbnail',
  'ads': 'Advertisement Design',
  'pfps': 'Profile Picture'
};

function setActivePage(key, serviceParam){
  if(!pageEls[key]) key = 'home';
  PAGE_KEYS.forEach(k=>{
    if(!pageEls[k]) return;
    pageEls[k].classList.toggle('active', k===key);
  });
  navLinkEls.forEach(a=>a.classList.toggle('active', a.dataset.page===key));
  dotLinkEls.forEach(d=>d.classList.toggle('active', d.dataset.page===key));
  navGroupEls.forEach(el=>{
    const isActive = (NAV_GROUPS[el.dataset.navGroup] || []).includes(key);
    el.classList.toggle('active', isActive);
    // landing directly on a page inside a mobile-nav group (e.g. #beats)
    // should auto-expand that group so it's not hidden behind a collapsed
    // toggle — but never auto-collapse a group the visitor opened by hand.
    if (isActive && el.classList.contains('mobile-nav-group-toggle')) {
      setMobileGroupOpen(el.closest('.mobile-nav-group'), true);
    }
  });
  document.querySelectorAll('.filter-tab').forEach(t=>t.classList.toggle('active', t.dataset.page===key));
  document.title = PAGE_TITLES[key] || PAGE_TITLES.home;
  window.scrollTo(0,0);
  closeMobileNav();
  if(key==='order' && serviceSelect){
    serviceSelect.value = serviceParam ? (SERVICE_LABELS[serviceParam] || '') : '';
  }
  // move focus to the page heading for screen-reader / keyboard users on route change
  const heading = pageEls[key] && pageEls[key].querySelector('h1,h2');
  if (heading) {
    // tabindex=-1 keeps it out of the normal tab order — this focus move is
    // purely so screen readers announce the new page title, not a visual
    // affordance, so it deliberately suppresses the focus outline below.
    heading.setAttribute('tabindex','-1');
    heading.classList.add('sr-route-focus');
    heading.focus({preventScroll:true});
  }
}

function routeFromHash(){
  const raw = location.hash.replace('#','');
  const [pageKey, serviceParam] = raw.split('/');
  setActivePage(pageKey || 'home', serviceParam);
  // "#beats/submit" is the deep link the header + icon and every "Upload a
  // Beat" button point to — land straight on step 1 of the beat wizard
  // instead of just the top of the Beats page.
  if (pageKey === 'beats' && serviceParam === 'submit' && typeof goToBeatStep === 'function') {
    goToBeatStep(0);
  }
}
window.addEventListener('hashchange', routeFromHash);

/* ---------- scroll reveal ---------- */
const io = new IntersectionObserver((entries)=>{
  entries.forEach(en=>{
    if(en.isIntersecting){ en.target.classList.add('in-view'); io.unobserve(en.target); }
  });
},{threshold:0.15});
function observeReveal(){
  document.querySelectorAll('.reveal:not([data-observed])').forEach(el=>{ el.dataset.observed='1'; io.observe(el); });
}
const ioTall = new IntersectionObserver((entries)=>{
  entries.forEach(en=>{
    if(en.isIntersecting){ en.target.classList.add('in-view'); ioTall.unobserve(en.target); }
  });
},{threshold:0.01});
function observeRevealStagger(){
  document.querySelectorAll('.reveal-stagger:not([data-observed])').forEach(el=>{ el.dataset.observed='1'; ioTall.observe(el); });
}

/* ---------- magnetic buttons ---------- */
function bindMagnetic(){
  document.querySelectorAll('.btn:not([data-magnetic-bound])').forEach(btn=>{
    btn.dataset.magneticBound = '1';
    btn.addEventListener('mousemove', e=>{
      const r = btn.getBoundingClientRect();
      const x = e.clientX - r.left - r.width/2;
      const y = e.clientY - r.top - r.height/2;
      btn.style.transform = `translate(${x*0.18}px, ${y*0.35}px)`;
    });
    btn.addEventListener('mouseleave', ()=>{ btn.style.transform='translate(0,0)'; });
  });
}

/* ---------- background canvas: drifting grid + particles ---------- */
const canvas = document.getElementById('bg-canvas');
const reduceMotionMQ = window.matchMedia('(prefers-reduced-motion: reduce)');
if (canvas && !reduceMotionMQ.matches) {
  const ctx = canvas.getContext('2d');
  let w,h,particles=[];
  function resize(){
    w = canvas.width = window.innerWidth;
    h = canvas.height = window.innerHeight;
  }
  function initParticles(){
    particles = Array.from({length:46}, ()=>({
      x: Math.random()*w, y: Math.random()*h,
      r: Math.random()*1.4+0.3,
      vx:(Math.random()-0.5)*0.15, vy:(Math.random()-0.5)*0.15,
      a: Math.random()*0.5+0.1
    }));
  }
  resize(); initParticles();
  window.addEventListener('resize', resize);
  let t=0;
  function draw(){
    t += 0.002;
    ctx.clearRect(0,0,w,h);
    ctx.strokeStyle='rgba(244,244,242,0.045)';
    ctx.lineWidth=1;
    const gap=64;
    const offset=(t*20)%gap;
    for(let x=-gap; x<w+gap; x+=gap){
      ctx.beginPath(); ctx.moveTo(x+offset,0); ctx.lineTo(x+offset,h); ctx.stroke();
    }
    for(let y=-gap; y<h+gap; y+=gap){
      ctx.beginPath(); ctx.moveTo(0,y+offset*0.4); ctx.lineTo(w,y+offset*0.4); ctx.stroke();
    }
    particles.forEach(p=>{
      p.x+=p.vx; p.y+=p.vy;
      if(p.x<0)p.x=w; if(p.x>w)p.x=0;
      if(p.y<0)p.y=h; if(p.y>h)p.y=0;
      ctx.beginPath();
      ctx.fillStyle=`rgba(244,244,242,${p.a})`;
      ctx.arc(p.x,p.y,p.r,0,Math.PI*2);
      ctx.fill();
    });
    requestAnimationFrame(draw);
  }
  draw();
} else if (canvas) {
  canvas.remove();
}

/* =========================================================
   PORTFOLIO — build items from config, render grids + filters
   ========================================================= */
function pad(n){ return String(n).padStart(2,'0'); }

function buildCategoryItems(cat){
  return Array.from({length: cat.count}, (_, i) => {
    const n = i + 1;
    const key = `${cat.key}-${pad(n)}`;
    const override = CFG.portfolio.overrides[key] || {};
    return {
      key,
      title: `${cat.shortLabel} ${pad(n)}`,
      src: `${cat.path}${pad(n)}.webp`,
      category: cat.key,
      categoryLabel: cat.label,
      wide: cat.wide,
      ...override
    };
  });
}

const PORTFOLIO = {};
CFG.portfolio.categories.forEach(cat=>{ PORTFOLIO[cat.key] = buildCategoryItems(cat); });

function buildGrid(gridEl, items, category){
  if(!gridEl) return;
  gridEl.innerHTML = items.map((c,i)=>`
    <button type="button" class="card" data-index="${i}" aria-label="View ${c.title}">
      <div class="art"><img src="${c.src}" alt="${c.title} — ${c.categoryLabel}" loading="lazy" width="600" height="${c.wide?338:600}"></div>
      <div class="card-info"><span class="g">${c.categoryLabel}</span><h4>${c.title}</h4></div>
      <div class="card-expand" aria-hidden="true">↗</div>
    </button>
  `).join('');
  gridEl.querySelectorAll('.card').forEach((card,i)=>{
    card.addEventListener('click', ()=>openLightbox(items[i], category));
  });
}

CFG.portfolio.categories.forEach(cat=>{
  if (cat.comingSoon) return;
  const gridId = cat.key === 'cover-art' ? 'grid' : `grid-${cat.key}`;
  buildGrid(document.getElementById(gridId), PORTFOLIO[cat.key], cat.key);
});

/* filter tabs shown above each portfolio grid, linking between categories */
document.querySelectorAll('.filter-tabs').forEach(tabsEl=>{
  tabsEl.innerHTML = CFG.portfolio.categories.map(cat=>
    `<a href="#${cat.key}" class="filter-tab" data-page="${cat.key}">${cat.shortLabel}</a>`
  ).join('');
});

/* =========================================================
   BEATS — audio catalog (hidden automatically when empty)
   Reads from CFG.beats — see config.js and the README for how
   to add a new one. Only one beat preview plays at a time.
   ========================================================= */
const ICON_PLAY = '<svg viewBox="0 0 24 24" width="15" height="15" fill="currentColor" aria-hidden="true"><path d="M6 4.5v15l14-7.5z"/></svg>';
const ICON_PAUSE = '<svg viewBox="0 0 24 24" width="15" height="15" fill="currentColor" aria-hidden="true"><rect x="5" y="4" width="5" height="16"/><rect x="14" y="4" width="5" height="16"/></svg>';
let currentlyPlayingAudio = null;

function buildBeatCard(beat){
  const card = document.createElement('div');
  card.className = 'beat-card';

  const cover = document.createElement('div');
  cover.className = 'beat-cover';
  if (beat.cover) {
    const img = document.createElement('img');
    img.src = beat.cover; img.alt = ''; img.loading = 'lazy';
    cover.appendChild(img);
  } else {
    cover.innerHTML = '<svg viewBox="0 0 24 24" width="26" height="26" fill="none" stroke="currentColor" stroke-width="1.6" aria-hidden="true"><path d="M9 18V5l12-2v13"/><circle cx="6" cy="18" r="3"/><circle cx="18" cy="16" r="3"/></svg>';
  }
  card.appendChild(cover);

  const title = document.createElement('h3');
  title.className = 'beat-title';
  title.textContent = beat.title || 'Untitled Beat';
  card.appendChild(title);

  // Credits the original producer for beats hosted on behalf of someone
  // else — omitted entirely for beats Rowen made, so nothing changes for
  // the existing catalog. Built with textContent/append (not innerHTML)
  // so a producer's name or link text can never be interpreted as markup.
  if (beat.producer && beat.producer.name) {
    const credit = document.createElement('div');
    credit.className = 'beat-producer';
    if (beat.producer.link) {
      const link = document.createElement('a');
      link.href = beat.producer.link;
      link.target = '_blank';
      link.rel = 'noopener';
      link.textContent = beat.producer.name;
      credit.append('Produced by ', link);
    } else {
      credit.textContent = `Produced by ${beat.producer.name}`;
    }
    card.appendChild(credit);
  }

  const metaBits = [];
  if (beat.bpm) metaBits.push(`${beat.bpm} BPM`);
  if (beat.key) metaBits.push(beat.key);
  (beat.tags || []).forEach(t => metaBits.push(t));
  if (metaBits.length) {
    const meta = document.createElement('div');
    meta.className = 'beat-meta';
    meta.innerHTML = metaBits.map(b => `<span class="beat-tag">${b}</span>`).join('');
    card.appendChild(meta);
  }

  if (beat.audio) {
    const player = document.createElement('div');
    player.className = 'beat-player';
    const audio = new Audio(beat.audio);
    audio.preload = 'none';

    const playBtn = document.createElement('button');
    playBtn.type = 'button';
    playBtn.className = 'beat-play-btn';
    playBtn.setAttribute('aria-label', `Play preview of ${beat.title || 'this beat'}`);
    playBtn.innerHTML = ICON_PLAY;

    const progress = document.createElement('div');
    progress.className = 'beat-progress';
    progress.setAttribute('role', 'slider');
    progress.setAttribute('aria-label', 'Playback position');
    progress.setAttribute('tabindex', '0');
    progress.setAttribute('aria-valuemin', '0');
    progress.setAttribute('aria-valuemax', '100');
    const progressFill = document.createElement('div');
    progressFill.className = 'beat-progress-fill';
    progress.appendChild(progressFill);

    playBtn.addEventListener('click', () => {
      if (audio.paused) {
        if (currentlyPlayingAudio && currentlyPlayingAudio !== audio) currentlyPlayingAudio.pause();
        audio.play().catch(() => {});
        currentlyPlayingAudio = audio;
      } else {
        audio.pause();
      }
    });
    audio.addEventListener('play', () => playBtn.innerHTML = ICON_PAUSE);
    audio.addEventListener('pause', () => playBtn.innerHTML = ICON_PLAY);
    audio.addEventListener('ended', () => { playBtn.innerHTML = ICON_PLAY; progressFill.style.width = '0%'; });
    audio.addEventListener('timeupdate', () => {
      if (audio.duration) {
        const pct = (audio.currentTime / audio.duration) * 100;
        progressFill.style.width = pct + '%';
        progress.setAttribute('aria-valuenow', String(Math.round(pct)));
      }
    });
    function seek(clientX){
      const rect = progress.getBoundingClientRect();
      const ratio = Math.min(1, Math.max(0, (clientX - rect.left) / rect.width));
      if (audio.duration) audio.currentTime = ratio * audio.duration;
    }
    progress.addEventListener('click', e => seek(e.clientX));
    progress.addEventListener('keydown', e => {
      if (e.key === 'ArrowRight') audio.currentTime = Math.min(audio.duration || 0, audio.currentTime + 5);
      if (e.key === 'ArrowLeft') audio.currentTime = Math.max(0, audio.currentTime - 5);
    });

    player.appendChild(playBtn);
    player.appendChild(progress);
    card.appendChild(player);
  }

  const foot = document.createElement('div');
  foot.className = 'beat-foot';
  const price = document.createElement('span');
  price.className = 'beat-price';
  price.textContent = beat.price != null ? `$${beat.price}` : 'Ask for price';
  foot.appendChild(price);

  const buy = document.createElement('a');
  buy.target = '_blank'; buy.rel = 'noopener';
  if (beat.stripeLink) {
    buy.href = beat.stripeLink;
    buy.className = 'btn solid small beat-buy';
    buy.innerHTML = '<span>Buy</span>';
  } else {
    buy.href = CFG.business.discordInvite;
    buy.className = 'btn ghost small beat-buy';
    buy.innerHTML = '<span>Ask on Discord</span>';
  }
  foot.appendChild(buy);
  card.appendChild(foot);

  return card;
}

/* Beats come from two places, combined: CFG.beats (hand-added in
   config.js, per the README "Adding a new beat" section — always
   works even if the API below fails) plus whatever's been
   auto-approved through the producer submission form (see
   api/beats.js, api/approve-beat.js, and the README "Getting beat
   submissions from producers" section). The fetch is wrapped so a
   network hiccup, or the automatic-approval env vars not being set
   up yet, just falls back to CFG.beats alone instead of breaking the
   page. */
async function fetchAutoApprovedBeats(){
  try {
    const res = await fetch('/api/beats');
    if (!res.ok) return [];
    const data = await res.json();
    return Array.isArray(data) ? data : [];
  } catch (err) {
    return [];
  }
}

async function renderBeats(){
  const beatsEmptyEl = document.getElementById('beatsEmpty');
  const beatsGridEl = document.getElementById('beatsGrid');
  if (!beatsEmptyEl || !beatsGridEl) return;
  const autoApproved = await fetchAutoApprovedBeats();
  const beats = [...(CFG.beats || []), ...autoApproved];
  if (!beats.length) {
    beatsEmptyEl.hidden = false;
    beatsGridEl.hidden = true;
    return;
  }
  beatsEmptyEl.hidden = true;
  beatsGridEl.hidden = false;
  beatsGridEl.innerHTML = '';
  beats.forEach(beat => beatsGridEl.appendChild(buildBeatCard(beat)));
}
renderBeats();

/* =========================================================
   VSTS — plugin catalog + cart + Stripe checkout
   Reads from CFG.vsts (see config.js). Empty array keeps the
   "in development" teaser visible; a non-empty array switches
   the page to a real product grid with a cart and checkout,
   same empty/real-catalog pattern as beats above.
   ========================================================= */
const VST_BY_KEY = {};
(CFG.vsts || []).forEach(v => { VST_BY_KEY[v.key] = v; });

function formatUSD(n){
  return `$${Number(n).toLocaleString('en-US', { minimumFractionDigits: 0, maximumFractionDigits: 2 })}`;
}

function buildVstCard(v, index){
  const card = document.createElement('div');
  card.className = 'vst-card';

  const cover = document.createElement('div');
  cover.className = 'vst-card-cover';
  if (v.images && v.images[0]) {
    const img = document.createElement('img');
    img.src = v.images[0]; img.alt = ''; img.loading = 'lazy';
    cover.appendChild(img);
  } else {
    cover.innerHTML = '<svg viewBox="0 0 24 24" width="26" height="26" fill="none" stroke="currentColor" stroke-width="1.6" aria-hidden="true"><rect x="4" y="4" width="16" height="16" rx="3"/><path d="M9 9h.01M9 15h.01M15 9h.01M15 15h.01"/></svg>';
  }
  if (v.badge) {
    const badge = document.createElement('span');
    badge.className = 'vst-card-badge';
    badge.textContent = v.badge;
    cover.appendChild(badge);
  }
  cover.addEventListener('click', () => openVstModal(v));
  card.appendChild(cover);

  const body = document.createElement('div');
  body.className = 'vst-card-body';

  const title = document.createElement('h3');
  title.className = 'vst-card-title';
  title.textContent = v.name || 'Untitled Plugin';
  title.addEventListener('click', () => openVstModal(v));
  body.appendChild(title);

  const tagline = document.createElement('p');
  tagline.className = 'vst-card-tagline';
  tagline.textContent = v.tagline || '';
  body.appendChild(tagline);

  const foot = document.createElement('div');
  foot.className = 'vst-card-foot';
  const price = document.createElement('span');
  price.className = 'vst-card-price';
  price.textContent = v.price != null ? formatUSD(v.price) : '';
  foot.appendChild(price);

  const addBtn = document.createElement('button');
  addBtn.type = 'button';
  addBtn.className = 'btn solid small';
  addBtn.innerHTML = '<span>Add to Cart</span>';
  addBtn.addEventListener('click', () => {
    RowenCart.addItem(v.key, 1);
    showToast(`Added "${v.name}" to your cart`);
  });
  foot.appendChild(addBtn);

  body.appendChild(foot);
  card.appendChild(body);
  return card;
}

function renderVstCatalog(){
  const teaserEl = document.getElementById('vstTeaser');
  const gridEl = document.getElementById('vstGrid');
  const earlyAccessEl = document.getElementById('vstEarlyAccessBand');
  if (!teaserEl || !gridEl) return;
  const vsts = CFG.vsts || [];
  if (!vsts.length) {
    teaserEl.hidden = false;
    gridEl.hidden = true;
    if (earlyAccessEl) earlyAccessEl.hidden = false;
    return;
  }
  teaserEl.hidden = true;
  gridEl.hidden = false;
  // Once there are real, buyable products, the "get notified when they
  // launch" CTA reads oddly right under working Add to Cart buttons —
  // hide it so the page doesn't send a mixed "buy now" / "not out yet" signal.
  if (earlyAccessEl) earlyAccessEl.hidden = true;
  gridEl.innerHTML = '';
  vsts.forEach((v,i) => gridEl.appendChild(buildVstCard(v,i)));
}
renderVstCatalog();

/* ---------- VST product detail modal ---------- */
const vstModal = document.getElementById('vstModal');
const vstModalBadge = document.getElementById('vstModalBadge');
const vstModalTitle = document.getElementById('vstModalTitle');
const vstModalTagline = document.getElementById('vstModalTagline');
const vstModalGalleryMain = document.getElementById('vstModalGalleryMain');
const vstModalGalleryThumbs = document.getElementById('vstModalGalleryThumbs');
const vstModalPrice = document.getElementById('vstModalPrice');
const vstModalDesc = document.getElementById('vstModalDesc');
const vstModalFeatures = document.getElementById('vstModalFeatures');
const vstModalQty = document.getElementById('vstModalQty');
const vstModalAddToCart = document.getElementById('vstModalAddToCart');
let currentVstModalItem = null;
let currentVstModalQty = 1;

function setVstModalMainImage(src){
  if (!vstModalGalleryMain) return;
  vstModalGalleryMain.innerHTML = src ? `<img src="${src}" alt="">` : '';
  vstModalGalleryThumbs.querySelectorAll('button').forEach(b => b.classList.toggle('active', b.dataset.src === src));
}

function openVstModal(v){
  if (!vstModal) return;
  currentVstModalItem = v;
  currentVstModalQty = 1;
  if (vstModalQty) vstModalQty.textContent = '1';
  if (vstModalBadge) vstModalBadge.textContent = v.badge || '';
  if (vstModalTitle) vstModalTitle.textContent = v.name || 'Untitled Plugin';
  if (vstModalTagline) vstModalTagline.textContent = v.tagline || '';
  if (vstModalPrice) vstModalPrice.textContent = v.price != null ? formatUSD(v.price) : '';
  if (vstModalDesc) { vstModalDesc.textContent = v.description || ''; vstModalDesc.hidden = !v.description; }
  if (vstModalFeatures) {
    vstModalFeatures.innerHTML = '';
    (v.features || []).forEach(f => {
      const li = document.createElement('li');
      li.textContent = f;
      vstModalFeatures.appendChild(li);
    });
  }
  const images = v.images && v.images.length ? v.images : [];
  setVstModalMainImage(images[0]);
  if (vstModalGalleryThumbs) {
    vstModalGalleryThumbs.innerHTML = '';
    vstModalGalleryThumbs.hidden = images.length < 2;
    images.forEach(src => {
      const btn = document.createElement('button');
      btn.type = 'button';
      btn.dataset.src = src;
      btn.innerHTML = `<img src="${src}" alt="">`;
      btn.addEventListener('click', () => setVstModalMainImage(src));
      vstModalGalleryThumbs.appendChild(btn);
    });
    setVstModalMainImage(images[0]);
  }
  openModal(vstModal);
}
wireModalDismiss(vstModal, 'vstModalBg', 'vstModalClose');

document.querySelectorAll('#vstModalQtyStepper [data-qty-down]').forEach(btn => {
  btn.addEventListener('click', () => {
    currentVstModalQty = Math.max(1, currentVstModalQty - 1);
    if (vstModalQty) vstModalQty.textContent = String(currentVstModalQty);
  });
});
document.querySelectorAll('#vstModalQtyStepper [data-qty-up]').forEach(btn => {
  btn.addEventListener('click', () => {
    currentVstModalQty = Math.min(99, currentVstModalQty + 1);
    if (vstModalQty) vstModalQty.textContent = String(currentVstModalQty);
  });
});
if (vstModalAddToCart) {
  vstModalAddToCart.addEventListener('click', () => {
    if (!currentVstModalItem) return;
    RowenCart.addItem(currentVstModalItem.key, currentVstModalQty);
    showToast(`Added "${currentVstModalItem.name}" to your cart`);
    closeModal(vstModal);
  });
}

/* ---------- cart badge + drawer ---------- */
const cartDrawer = document.getElementById('cartDrawer');
const cartDrawerItems = document.getElementById('cartDrawerItems');
const cartDrawerEmpty = document.getElementById('cartDrawerEmpty');
const cartDrawerFoot = document.getElementById('cartDrawerFoot');
const cartSubtotalEl = document.getElementById('cartSubtotal');
const cartCheckoutBtn = document.getElementById('cartCheckoutBtn');
const cartCheckoutText = document.getElementById('cartCheckoutText');
const cartCheckoutError = document.getElementById('cartCheckoutError');

function updateCartBadges(){
  const count = RowenCart.getCount();
  ['cartBadge','cartBadgeMobile'].forEach(id => {
    const el = document.getElementById(id);
    if (!el) return;
    el.dataset.count = String(count);
    el.textContent = count > 0 ? String(count) : '';
  });
}

function buildCartItemRow(entry){
  const v = VST_BY_KEY[entry.key];
  if (!v) return null; // product no longer in the catalog — skip gracefully
  const row = document.createElement('div');
  row.className = 'cart-item';

  const cover = document.createElement('div');
  cover.className = 'cart-item-cover';
  if (v.images && v.images[0]) {
    const img = document.createElement('img');
    img.src = v.images[0]; img.alt = '';
    cover.appendChild(img);
  }
  row.appendChild(cover);

  const info = document.createElement('div');
  info.className = 'cart-item-info';
  const name = document.createElement('div');
  name.className = 'cart-item-name';
  name.textContent = v.name;
  info.appendChild(name);
  const price = document.createElement('div');
  price.className = 'cart-item-price';
  price.textContent = `${formatUSD(v.price)} × ${entry.qty}`;
  info.appendChild(price);
  row.appendChild(info);

  const stepper = document.createElement('div');
  stepper.className = 'qty-stepper';
  const down = document.createElement('button');
  down.type = 'button'; down.textContent = '−'; down.setAttribute('aria-label', `Decrease quantity of ${v.name}`);
  down.addEventListener('click', () => RowenCart.setQty(entry.key, entry.qty - 1));
  const qtySpan = document.createElement('span');
  qtySpan.textContent = String(entry.qty);
  const up = document.createElement('button');
  up.type = 'button'; up.textContent = '+'; up.setAttribute('aria-label', `Increase quantity of ${v.name}`);
  up.addEventListener('click', () => RowenCart.setQty(entry.key, entry.qty + 1));
  stepper.append(down, qtySpan, up);
  row.appendChild(stepper);

  const removeBtn = document.createElement('button');
  removeBtn.type = 'button';
  removeBtn.className = 'cart-item-remove';
  removeBtn.setAttribute('aria-label', `Remove ${v.name} from cart`);
  removeBtn.innerHTML = '✕';
  removeBtn.addEventListener('click', () => RowenCart.removeItem(entry.key));
  row.appendChild(removeBtn);

  return row;
}

function renderCartDrawer(){
  if (!cartDrawerItems) return;
  const entries = RowenCart.getItems();
  const rows = entries.map(buildCartItemRow).filter(Boolean);
  if (!rows.length) {
    cartDrawerEmpty.hidden = false;
    cartDrawerItems.hidden = true;
    cartDrawerFoot.hidden = true;
    return;
  }
  cartDrawerEmpty.hidden = true;
  cartDrawerItems.hidden = false;
  cartDrawerFoot.hidden = false;
  cartDrawerItems.innerHTML = '';
  rows.forEach(r => cartDrawerItems.appendChild(r));
  const subtotal = entries.reduce((sum, entry) => {
    const v = VST_BY_KEY[entry.key];
    return v ? sum + v.price * entry.qty : sum;
  }, 0);
  if (cartSubtotalEl) cartSubtotalEl.textContent = formatUSD(subtotal);
  if (cartCheckoutError) cartCheckoutError.hidden = true;
}

RowenCart.subscribe(() => { updateCartBadges(); renderCartDrawer(); });
updateCartBadges();

['cartTrigger','cartTriggerMobile'].forEach(id => {
  const btn = document.getElementById(id);
  if (btn) btn.addEventListener('click', () => { renderCartDrawer(); openModal(cartDrawer); });
});
wireModalDismiss(cartDrawer, 'cartDrawerBg', 'cartDrawerClose');

if (cartCheckoutBtn) {
  cartCheckoutBtn.addEventListener('click', async () => {
    const entries = RowenCart.getItems().filter(e => VST_BY_KEY[e.key]);
    if (!entries.length) return;
    cartCheckoutBtn.disabled = true;
    cartCheckoutText.innerHTML = '<span class="spinner" aria-hidden="true"></span> Redirecting…';
    if (cartCheckoutError) cartCheckoutError.hidden = true;
    try {
      const res = await fetch('/api/create-checkout-session', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ items: entries.map(e => ({ key: e.key, qty: e.qty })) })
      });
      const data = await res.json().catch(() => ({}));
      if (res.ok && data.url) {
        window.location.href = data.url;
        return;
      }
      throw new Error(data.error || 'Checkout could not be started.');
    } catch (err) {
      if (cartCheckoutError) {
        cartCheckoutError.textContent = 'Something went wrong starting checkout — please try again in a moment.';
        cartCheckoutError.hidden = false;
      }
    }
    cartCheckoutBtn.disabled = false;
    cartCheckoutText.textContent = 'Checkout';
  });
}

/* hero "selected work" strip — first few cover-art pieces */
const heroStrip = document.getElementById('heroStripTrack');
if (heroStrip) {
  const picks = PORTFOLIO['cover-art'].slice(0,8);
  heroStrip.innerHTML = picks.map((c,i)=>`
    <button type="button" class="card" data-index="${i}" aria-label="View ${c.title}">
      <div class="art"><img src="${c.src}" alt="${c.title} — ${c.categoryLabel}" loading="lazy" width="300" height="300"></div>
    </button>
  `).join('');
  heroStrip.querySelectorAll('.card').forEach((card,i)=>{
    card.addEventListener('click', ()=>openLightbox(picks[i], 'cover-art'));
  });
}

/* =========================================================
   REFERENCE FILE MANAGER (order form)
   ========================================================= */
const fileDrop = document.getElementById('fileDrop');
const fileInput = document.getElementById('fileInput');
const fileDropText = document.getElementById('fileDropText');
const fileList = document.getElementById('fileList');
const fileError = document.getElementById('fileError');
let referenceFiles = [];

const MAX_FILE_MB = 15;
const MAX_FILES = 6;
const ACCEPTED_TYPES = ['image/png','image/jpeg','image/webp','image/gif'];

function validateFiles(files){
  const errors = [];
  const accepted = [];
  const combined = referenceFiles.length + files.length;
  files.forEach(f=>{
    if (!ACCEPTED_TYPES.includes(f.type)) {
      errors.push(`"${f.name}" isn't a supported image type (PNG, JPG, WEBP, or GIF only).`);
      return;
    }
    if (f.size > MAX_FILE_MB * 1024 * 1024) {
      errors.push(`"${f.name}" is over the ${MAX_FILE_MB}MB limit.`);
      return;
    }
    accepted.push(f);
  });
  if (combined > MAX_FILES) {
    errors.push(`You can attach up to ${MAX_FILES} reference files.`);
  }
  return { accepted: accepted.slice(0, Math.max(0, MAX_FILES - referenceFiles.length)), errors };
}

function syncFileInput(){
  const dt = new DataTransfer();
  referenceFiles.forEach(f=>dt.items.add(f));
  fileInput.files = dt.files;
  updateFileText();
}
function appendReferenceFiles(newFiles){
  const { accepted, errors } = validateFiles(Array.from(newFiles));
  if (errors.length && fileError) {
    fileError.textContent = errors.join(' ');
    fileError.hidden = false;
  } else if (fileError) {
    fileError.hidden = true;
  }
  if (accepted.length) {
    referenceFiles = referenceFiles.concat(accepted);
    syncFileInput();
  }
}
function removeReferenceFile(index){
  referenceFiles.splice(index, 1);
  syncFileInput();
}
function updateFileText(){
  const n = referenceFiles.length;
  fileDropText.textContent = n ? `${n} file${n>1?'s':''} selected` : 'Drop images here or click to browse';
  if(fileList){
    fileList.innerHTML = referenceFiles.map((f,i)=>`
      <div class="file-chip">
        <span class="chip-name" title="${f.name}">${f.name}</span>
        <button type="button" class="chip-remove" data-index="${i}" aria-label="Remove ${f.name}">✕</button>
      </div>
    `).join('');
  }
}
if (fileDrop && fileInput) {
  if(fileList){
    fileList.addEventListener('click', e=>{
      const btn = e.target.closest('.chip-remove');
      if(!btn) return;
      removeReferenceFile(Number(btn.dataset.index));
    });
  }
  ['dragenter','dragover'].forEach(evt=>fileDrop.addEventListener(evt,e=>{e.preventDefault();fileDrop.classList.add('drag');}));
  ['dragleave','drop'].forEach(evt=>fileDrop.addEventListener(evt,e=>{e.preventDefault();fileDrop.classList.remove('drag');}));
  fileDrop.addEventListener('drop', e=>{ appendReferenceFiles(e.dataTransfer.files); });
  fileInput.addEventListener('change', ()=>{ appendReferenceFiles(fileInput.files); });
}

/* =========================================================
   MODAL HELPERS — shared focus-trap / Escape / restore-focus
   logic for the project lightbox, payment modal, and policy
   modals so keyboard and screen-reader users aren't stranded.
   ========================================================= */
let lastFocusedEl = null;
function openModal(modalEl){
  lastFocusedEl = document.activeElement;
  modalEl.classList.add('open');
  modalEl.setAttribute('aria-hidden','false');
  const focusable = modalEl.querySelector('.lightbox-close, button, [href], input, select, textarea');
  if (focusable) focusable.focus();
  document.addEventListener('keydown', trapHandler(modalEl));
  document.body.style.overflow = 'hidden';
}
function closeModal(modalEl){
  modalEl.classList.remove('open');
  modalEl.setAttribute('aria-hidden','true');
  document.body.style.overflow = '';
  if (lastFocusedEl && typeof lastFocusedEl.focus === 'function') lastFocusedEl.focus();
}
function trapHandler(modalEl){
  function handler(e){
    if (!modalEl.classList.contains('open')) {
      document.removeEventListener('keydown', handler);
      return;
    }
    if (e.key === 'Escape') { closeModal(modalEl); return; }
    if (e.key === 'Tab') {
      const focusables = Array.from(modalEl.querySelectorAll('button, [href], input, select, textarea, [tabindex]:not([tabindex="-1"])')).filter(el=>!el.disabled && el.offsetParent !== null);
      if (!focusables.length) return;
      const first = focusables[0], last = focusables[focusables.length-1];
      if (e.shiftKey && document.activeElement === first) { e.preventDefault(); last.focus(); }
      else if (!e.shiftKey && document.activeElement === last) { e.preventDefault(); first.focus(); }
    }
  }
  return handler;
}
function wireModalDismiss(modalEl, bgId, closeId){
  const bg = document.getElementById(bgId);
  const closeBtn = document.getElementById(closeId);
  if (bg) bg.addEventListener('click', ()=>closeModal(modalEl));
  if (closeBtn) closeBtn.addEventListener('click', ()=>closeModal(modalEl));
}

/* =========================================================
   PROJECT DETAIL LIGHTBOX
   ========================================================= */
const lightbox = document.getElementById('lightbox');
const lbArt = document.getElementById('lightboxArt');
const lbTitle = document.getElementById('lightboxTitle');
const lbCta = document.getElementById('lightboxCta');
const lbMeta = document.getElementById('lightboxMeta');
const lbDesc = document.getElementById('lightboxDesc');
let currentLightboxItem = null;

function metaRow(label, value){
  if (!value) return '';
  return `<div><span class="dm-label">${label}</span><span class="dm-value">${value}</span></div>`;
}

function openLightbox(c, category){
  const wide = !!c.wide;
  currentLightboxItem = {...c, category};
  document.querySelector('.lightbox-content').classList.toggle('wide', wide);
  lbArt.classList.toggle('wide', wide);

  if (c.before) {
    lbArt.innerHTML = `
      <div class="compare ${wide?'wide':''}">
        <img src="${c.before}" alt="Before">
        <div class="compare-after"><img src="${c.src}" alt="After"></div>
        <span class="compare-label before">Before</span>
        <span class="compare-label after">After</span>
        <div class="compare-handle" style="left:50%"></div>
        <input type="range" class="compare-slider" min="0" max="100" value="50" aria-label="Drag to compare before and after">
      </div>`;
    const slider = lbArt.querySelector('.compare-slider');
    const afterEl = lbArt.querySelector('.compare-after');
    const handle = lbArt.querySelector('.compare-handle');
    slider.addEventListener('input', ()=>{
      afterEl.style.clipPath = `inset(0 0 0 ${slider.value}%)`;
      handle.style.left = slider.value + '%';
    });
  } else {
    lbArt.innerHTML = `<div class="art" style="position:absolute;inset:0;"><img src="${c.src}" alt="${c.title}"></div>`;
  }

  lbTitle.textContent = c.title;
  if (lbMeta) {
    lbMeta.innerHTML = [
      metaRow('Project type', c.type || c.categoryLabel),
      metaRow('Client', c.client || 'Personal / portfolio piece'),
      metaRow('Tools used', c.tools),
    ].join('');
  }
  if (lbDesc) {
    lbDesc.textContent = c.description || '';
    lbDesc.hidden = !c.description;
  }
  if(lbCta) lbCta.href = `#order/${category}`;
  openModal(lightbox);
}
wireModalDismiss(lightbox, 'lightboxBg', 'lightboxClose');

if(lbCta){
  lbCta.addEventListener('click', (e)=>{
    e.preventDefault();
    const item = currentLightboxItem;
    const targetHash = lbCta.getAttribute('href');
    closeModal(lightbox);
    location.hash = targetHash;
    if(item){
      (async ()=>{
        try{
          const res = await fetch(item.src);
          const blob = await res.blob();
          const ext = (blob.type.split('/')[1] || 'webp').replace('jpeg','jpg');
          const filename = `${item.title.replace(/\s+/g,'-').toLowerCase()}.${ext}`;
          const file = new File([blob], filename, {type: blob.type || 'image/webp'});
          appendReferenceFiles([file]);
          showToast(`Added "${item.title}" as a reference image`);
        }catch(err){
          console.warn('Could not auto-attach reference image', err);
        }
      })();
    }
  });
}

/* =========================================================
   POLICY MODALS
   ========================================================= */
const policyModal = document.getElementById('policyModal');
const policyModalTitle = document.getElementById('policyModalTitle');
const policyModalBody = document.getElementById('policyModalBody');
wireModalDismiss(policyModal, 'policyModalBg', 'policyModalClose');

function renderPolicy(key){
  const policy = CFG.policies[key];
  if (!policy || !policyModalBody) return;
  policyModalTitle.textContent = policy.title;
  policyModalBody.innerHTML = `
    ${policy.body.map(sec => `<h4>${sec.h}</h4><p>${sec.p}</p>`).join('')}
  `;
  openModal(policyModal);
}
document.querySelectorAll('[data-policy]').forEach(btn=>{
  btn.addEventListener('click', (e)=>{
    e.preventDefault();
    renderPolicy(btn.dataset.policy);
  });
});

/* =========================================================
   PRICING SECTION (rendered from config)
   ========================================================= */
const pricingGrid = document.getElementById('pricingGrid');
if (pricingGrid) {
  pricingGrid.innerHTML = CFG.services.map(s => `
    <div class="price-card">
      <h3>${s.name}</h3>
      <p style="color:var(--dim);font-size:13px;line-height:1.5;">${s.blurb}</p>
      <span class="price-badge" style="align-self:flex-start;margin:14px 0 4px;">Custom quote</span>
      <ul>
        ${s.includes.map(i=>`<li>${i}</li>`).join('')}
        <li>Turnaround: ${s.turnaround}</li>
        <li>Source files: ${s.sourceFiles}</li>
      </ul>
      <a href="#order/${s.key}" class="btn small"><span>Start this project</span></a>
    </div>
  `).join('');
}
const pricingFactorsEl = document.getElementById('pricingFactors');
if (pricingFactorsEl && CFG.pricingFactors) {
  pricingFactorsEl.innerHTML = `
    <h3>What affects your quote</h3>
    <ul>${CFG.pricingFactors.map(f=>`<li>${f}</li>`).join('')}</ul>
  `;
}
document.querySelectorAll('[data-min-budget-line]').forEach(el=>{
  el.textContent = `$${CFG.business.minBudget} minimum on every project — final pricing is always a custom quote, never a fixed rate.`;
});

/* =========================================================
   HOW IT WORKS (rendered from config)
   ========================================================= */
const processEl = document.getElementById('processSteps');
if (processEl) {
  processEl.innerHTML = CFG.howItWorks.map((s,i)=>`
    <div class="step">
      <div class="num">0${i+1}</div>
      <h3>${s.title}</h3>
      <p>${s.body}</p>
    </div>
  `).join('');
}

/* =========================================================
   REVIEWS — homepage teaser + dedicated Reviews page.
   Hidden automatically when CFG.reviews is empty (homepage teaser
   only — the Reviews page's submission form always stays visible
   so the first review has somewhere to come from). Average rating
   is computed here, not stored in config, so it's always accurate.
   ========================================================= */

/* Two-stacked-row star display: a background row of 5 empty stars
   with a filled row on top, clipped to (rating/5) width via a CSS
   custom property. Works for both whole numbers (a single review)
   and fractional averages (e.g. 4.9) with one implementation. */
function buildStarRatingHTML(rating, size){
  const r = Math.max(0, Math.min(5, Number(rating) || 0));
  const sizeStyle = size ? `--star-size:${size};` : '';
  return `<span class="star-rating" style="--rating:${r};${sizeStyle}" role="img" aria-label="${r.toFixed(1)} out of 5 stars">
    <span class="star-row star-row-empty" aria-hidden="true">★★★★★</span>
    <span class="star-row star-row-filled" aria-hidden="true">★★★★★</span>
  </span>`;
}
function buildReviewCard(r){
  return `
    <div class="review-card">
      ${buildStarRatingHTML(r.rating, '14px')}
      <p class="review-quote">${r.quote}</p>
      <div class="review-who">
        ${r.avatar ? `<img class="review-avatar" src="${r.avatar}" alt="" loading="lazy">` : `<span class="review-avatar-fallback" aria-hidden="true">${(r.name||'?').charAt(0)}</span>`}
        <div>
          <div class="review-name">${r.name}</div>
          ${r.role ? `<div class="review-role">${r.role}</div>` : ''}
        </div>
      </div>
    </div>
  `;
}
function buildRatingBannerHTML(reviews){
  const avg = reviews.reduce((sum,r)=> sum + (Number(r.rating)||0), 0) / reviews.length;
  return `
    ${buildStarRatingHTML(avg, '22px')}
    <span class="rating-number">${avg.toFixed(1)}</span>
    <span class="rating-detail">from ${reviews.length} review${reviews.length===1?'':'s'}</span>
  `;
}

const reviewsSection = document.getElementById('reviewsSection');
const homeRatingBanner = document.getElementById('homeRatingBanner');
const homeReviewGrid = document.getElementById('homeReviewGrid');
const reviewsRatingBanner = document.getElementById('reviewsRatingBanner');
const reviewsEmptyEl = document.getElementById('reviewsEmpty');
const reviewsGridEl = document.getElementById('reviewsGrid');

function renderReviews(){
  const reviews = CFG.reviews || [];

  // Homepage teaser — the whole section hides when there's nothing to show yet.
  if (reviewsSection && homeRatingBanner && homeReviewGrid) {
    if (reviews.length) {
      homeRatingBanner.innerHTML = buildRatingBannerHTML(reviews);
      homeReviewGrid.innerHTML = reviews.slice(0, 6).map(buildReviewCard).join('');
      reviewsSection.hidden = false;
    } else {
      reviewsSection.hidden = true;
    }
  }

  // Dedicated Reviews page — the "leave a review" form further down the
  // page always stays visible regardless of this; only the rating banner
  // and grid vs. the "no reviews yet" panel toggle here.
  if (reviewsRatingBanner && reviewsEmptyEl && reviewsGridEl) {
    if (reviews.length) {
      reviewsRatingBanner.innerHTML = buildRatingBannerHTML(reviews);
      reviewsRatingBanner.hidden = false;
      reviewsEmptyEl.hidden = true;
      reviewsGridEl.hidden = false;
      reviewsGridEl.innerHTML = reviews.map(buildReviewCard).join('');
    } else {
      reviewsRatingBanner.hidden = true;
      reviewsEmptyEl.hidden = false;
      reviewsGridEl.hidden = true;
      reviewsGridEl.innerHTML = '';
    }
  }
}
renderReviews();

/* =========================================================
   FAQ ACCORDION (rendered from config)
   ========================================================= */
const faqList = document.getElementById('faqList');
if (faqList) {
  faqList.innerHTML = CFG.faq.map((item, i) => `
    <div class="faq-item" data-open="false">
      <h3>
        <button type="button" class="faq-q" id="faqQ${i}" aria-expanded="false" aria-controls="faqA${i}">
          <span>${item.q}</span>
          <span class="faq-icon" aria-hidden="true"></span>
        </button>
      </h3>
      <div class="faq-a" id="faqA${i}" role="region" aria-labelledby="faqQ${i}">
        <p>${item.a}</p>
      </div>
    </div>
  `).join('');
  faqList.querySelectorAll('.faq-q').forEach(btn=>{
    btn.addEventListener('click', ()=>{
      const item = btn.closest('.faq-item');
      const answer = item.querySelector('.faq-a');
      const isOpen = item.dataset.open === 'true';
      item.dataset.open = isOpen ? 'false' : 'true';
      btn.setAttribute('aria-expanded', String(!isOpen));
      answer.style.maxHeight = isOpen ? '0px' : answer.scrollHeight + 'px';
    });
  });
}

/* =========================================================
   MOBILE NAV
   ========================================================= */
const burger = document.getElementById('burger');
const mobileNav = document.getElementById('mobileNav');
function openMobileNav(){
  if (!mobileNav) return;
  mobileNav.classList.add('open');
  burger.setAttribute('aria-expanded','true');
  document.body.style.overflow = 'hidden';
  // A group can get auto-expanded (via setActivePage, e.g. landing on
  // #beats) while the drawer is still display:none — scrollHeight reads
  // as 0 in that state, so the max-height set at the time didn't actually
  // reveal anything. Recompute it now that the drawer has real layout.
  document.querySelectorAll('.mobile-nav-group[data-open="true"]').forEach(group=>{
    const panel = group.querySelector('.mobile-nav-subpanel');
    if (panel) panel.style.maxHeight = panel.scrollHeight + 'px';
  });
  const firstLink = mobileNav.querySelector('a');
  if (firstLink) firstLink.focus();
}
function closeMobileNav(){
  if (!mobileNav || !mobileNav.classList.contains('open')) return;
  mobileNav.classList.remove('open');
  burger.setAttribute('aria-expanded','false');
  document.body.style.overflow = '';
}
if (burger && mobileNav) {
  burger.addEventListener('click', ()=>{
    mobileNav.classList.contains('open') ? closeMobileNav() : openMobileNav();
  });
  mobileNav.querySelectorAll('a').forEach(a=>a.addEventListener('click', closeMobileNav));
  document.querySelector('.mobile-nav-bg')?.addEventListener('click', closeMobileNav);
  document.addEventListener('keydown', e=>{ if (e.key === 'Escape') closeMobileNav(); });
}

/* "Graphic Design" / "Music" accordion groups inside the mobile drawer —
   same open/close mechanics as the FAQ accordion above (max-height
   transition driven from JS, since the true content height isn't knowable
   from CSS alone). Toggling a group never closes the drawer itself, since
   these are <button>s, not the <a> elements closeMobileNav is bound to. */
function setMobileGroupOpen(groupEl, open){
  if (!groupEl) return;
  const btn = groupEl.querySelector('.mobile-nav-group-toggle');
  const panel = groupEl.querySelector('.mobile-nav-subpanel');
  groupEl.dataset.open = open ? 'true' : 'false';
  if (btn) btn.setAttribute('aria-expanded', String(open));
  if (panel) panel.style.maxHeight = open ? panel.scrollHeight + 'px' : '0px';
}
document.querySelectorAll('.mobile-nav-group-toggle').forEach(btn=>{
  btn.addEventListener('click', ()=>{
    const group = btn.closest('.mobile-nav-group');
    setMobileGroupOpen(group, group.dataset.open !== 'true');
  });
});

/* =========================================================
   ORDER FORM — validation, honeypot, submit
   ========================================================= */
const orderForm = document.getElementById('orderForm');
const formStatus = document.getElementById('formStatus');
const submitBtn = document.getElementById('submitBtn');
const submitText = document.getElementById('submitText');
const formLoadedAt = Date.now();

/* budget is a required select of ranges (not free text) — populate its
   options from config so the ranges stay editable in one place. */
const budgetSelect = document.getElementById('budget');
if (budgetSelect && CFG.budgetRanges) {
  CFG.budgetRanges.forEach(range=>{
    const opt = document.createElement('option');
    opt.value = range;
    opt.textContent = range;
    budgetSelect.appendChild(opt);
  });
}

function setFieldError(fieldEl, message){
  const wrap = fieldEl.closest('.field');
  const errorEl = wrap ? wrap.querySelector('.field-error') : null;
  if (message) {
    wrap.classList.add('has-error');
    if (errorEl) errorEl.textContent = message;
    fieldEl.setAttribute('aria-invalid','true');
  } else {
    wrap.classList.remove('has-error');
    if (errorEl) errorEl.textContent = '';
    fieldEl.removeAttribute('aria-invalid');
  }
}

/* Validates the required fields within a container (a single step, or the
   whole form). Returns the first invalid field, or null if all pass. */
function validateStep(container){
  if (!container) return null;
  let firstInvalid = null;
  const required = container.querySelectorAll('[required]');
  required.forEach(field=>{
    let message = '';
    if (field.type === 'checkbox') {
      if (!field.checked) message = 'You need to agree before sending your request.';
    } else if (!field.value.trim()) {
      message = 'This field is required.';
    } else if (field.type === 'email' && !/^[^\s@]+@[^\s@]+\.[^\s@]+$/.test(field.value)) {
      message = 'Enter a valid email address.';
    }
    setFieldError(field, message);
    if (message && !firstInvalid) firstInvalid = field;
  });
  return firstInvalid;
}

/* Final safety-net validation across the whole form (in case a field
   somehow ends up invalid without going through the step Next button).
   If something fails, jumps to the step that contains it before focusing
   — focusing a field inside a currently-hidden step is a no-op. */
function validateForm(){
  if (!orderForm) return true;
  const firstInvalid = validateStep(orderForm);
  if (firstInvalid) {
    const stepEl = firstInvalid.closest('.form-step');
    if (stepEl) goToStep(Number(stepEl.dataset.step), { focusField: firstInvalid });
    else firstInvalid.focus();
    return false;
  }
  return true;
}
if (orderForm) {
  orderForm.querySelectorAll('[required]').forEach(f=>{
    f.addEventListener('input', ()=>setFieldError(f, ''));
    f.addEventListener('change', ()=>setFieldError(f, ''));
  });
}

/* =========================================================
   ORDER FORM — one-question-at-a-time wizard
   ========================================================= */
const formSteps = orderForm ? Array.from(orderForm.querySelectorAll('#formSteps .form-step')) : [];
const TOTAL_STEPS = formSteps.length;
const REVIEW_STEP_INDEX = TOTAL_STEPS - 1;
let currentStep = 0;
const stepBackBtn = document.getElementById('stepBack');
const stepNextBtn = document.getElementById('stepNext');
const formProgressFill = document.getElementById('formProgressFill');
const formStepCount = document.getElementById('formStepCount');
const reviewSummaryEl = document.getElementById('reviewSummary');

function goToStep(index, opts){
  if (!formSteps.length) return;
  opts = opts || {};
  currentStep = Math.max(0, Math.min(index, TOTAL_STEPS - 1));
  formSteps.forEach((el,i)=> el.classList.toggle('active', i === currentStep));
  const isReview = currentStep === REVIEW_STEP_INDEX;
  if (stepBackBtn) stepBackBtn.hidden = currentStep === 0;
  if (stepNextBtn) stepNextBtn.hidden = isReview;
  if (submitBtn) submitBtn.hidden = !isReview;
  const pct = Math.round(((currentStep + 1) / TOTAL_STEPS) * 100);
  if (formProgressFill) formProgressFill.style.width = pct + '%';
  if (formStepCount) formStepCount.textContent = `Step ${currentStep + 1} of ${TOTAL_STEPS}`;
  if (isReview) renderReviewSummary();
  if (opts.focusField) {
    opts.focusField.focus();
  } else {
    const heading = formSteps[currentStep].querySelector('.form-step-heading');
    if (heading) heading.focus({ preventScroll: false });
  }
}

if (stepNextBtn) {
  stepNextBtn.addEventListener('click', ()=>{
    const firstInvalid = validateStep(formSteps[currentStep]);
    if (firstInvalid) { firstInvalid.focus(); return; }
    goToStep(currentStep + 1);
  });
}
if (stepBackBtn) {
  stepBackBtn.addEventListener('click', ()=> goToStep(currentStep - 1));
}
if (orderForm) {
  // Enter advances to the next step instead of doing nothing — except in a
  // textarea, where it should just insert a newline like normal.
  orderForm.addEventListener('keydown', (e)=>{
    if (e.key !== 'Enter' || e.target.tagName === 'TEXTAREA') return;
    if (currentStep === REVIEW_STEP_INDEX) return; // let the Send Request button handle this step
    e.preventDefault();
    if (stepNextBtn) stepNextBtn.click();
  });
}

/* Builds a plain-text summary of every answer on the final review step.
   Uses textContent (not innerHTML) throughout so nothing a visitor typed
   can be interpreted as markup. */
function reviewRow(dl, label, value){
  const dt = document.createElement('dt');
  dt.textContent = label;
  const dd = document.createElement('dd');
  dd.textContent = value;
  dl.appendChild(dt);
  dl.appendChild(dd);
}
function renderReviewSummary(){
  if (!reviewSummaryEl) return;
  reviewSummaryEl.innerHTML = '';
  const serviceEl = document.getElementById('service');
  const serviceLabel = serviceEl && serviceEl.selectedIndex >= 0 ? serviceEl.options[serviceEl.selectedIndex].text : 'Not sure yet';
  const val = id => { const el = document.getElementById(id); return el ? el.value.trim() : ''; };
  reviewRow(reviewSummaryEl, 'What you need', serviceLabel || 'Not sure yet');
  reviewRow(reviewSummaryEl, 'Name', val('name') || '—');
  reviewRow(reviewSummaryEl, 'Email', val('email') || '—');
  reviewRow(reviewSummaryEl, 'Discord username', val('discord') || '—');
  reviewRow(reviewSummaryEl, 'Preferred contact', val('contactMethod') || '—');
  reviewRow(reviewSummaryEl, 'Project / channel', val('artist') || '—');
  reviewRow(reviewSummaryEl, 'Deadline', val('deadline') || 'Flexible');
  reviewRow(reviewSummaryEl, 'Budget range', val('budget') || '—');
  reviewRow(reviewSummaryEl, 'Project description', val('brief') || '—');
  reviewRow(reviewSummaryEl, 'Reference links', val('referenceLinks') || '—');
  reviewRow(reviewSummaryEl, 'Reference files', referenceFiles.length ? `${referenceFiles.length} file${referenceFiles.length > 1 ? 's' : ''} attached` : 'None');
}

if (orderForm) {
  orderForm.addEventListener('submit', (e)=>{
    e.preventDefault();
    if(!validateForm()) return;
    // basic bot check: honeypot filled, or submitted implausibly fast
    const honeypot = orderForm.querySelector('input[name="_gotcha"]');
    const elapsed = Date.now() - formLoadedAt;
    if ((honeypot && honeypot.value) || elapsed < 2500) { return; } // silently drop — likely a bot
    submitOrderForm();
  });
}

goToStep(0);

async function submitOrderForm(){
  if (!orderForm) return;
  submitBtn.disabled = true;
  submitBtn.setAttribute('aria-disabled','true');
  submitText.innerHTML = '<span class="spinner" aria-hidden="true"></span> Sending…';
  formStatus.textContent = '';
  formStatus.className = 'form-status';

  if(!CFG.formspreeId || CFG.formspreeId === 'YOUR_FORM_ID'){
    const data = new FormData(orderForm);
    const body = encodeURIComponent(
      `Service: ${data.get('service')||'Not sure yet'}\nName: ${data.get('name')}\nEmail: ${data.get('email')}\nDiscord: ${data.get('discord')||'—'}\nProject/Channel: ${data.get('artist')}\nDeadline: ${data.get('deadline')}\nBudget: ${data.get('budget')}\nPreferred contact: ${data.get('contactMethod')||'—'}\nReference links: ${data.get('referenceLinks')||'—'}\n\nWhat they want:\n${data.get('brief')}`
    );
    window.location.href = `mailto:${CFG.business.email}?subject=${encodeURIComponent('New Design Request: '+(data.get('service')||'General')+' — '+(data.get('artist')||data.get('name')))}&body=${body}`;
    resetSubmitButton();
    showToast('Opening your email client to send this request…');
    return;
  }

  const hadFiles = referenceFiles.length > 0;

  try{
    let res = await fetch(`https://formspree.io/f/${CFG.formspreeId}`, {
      method:'POST',
      headers:{'Accept':'application/json'},
      body:new FormData(orderForm)
    });

    let sentWithoutFiles = false;

    // Some Formspree plans don't support file attachments, which makes the
    // whole submission fail even though the text fields are fine. If a
    // submission with attached files gets rejected, retry once without the
    // files so the request still gets through — the visitor is told to send
    // the files another way instead of losing the whole message.
    if(!res.ok && hadFiles){
      const retryData = new FormData(orderForm);
      retryData.delete('attachment');
      retryData.set('filesNote', `This visitor attached ${referenceFiles.length} reference file(s) that could not be delivered automatically (file uploads may require a Formspree plan upgrade). Ask them to resend via Discord or email.`);
      res = await fetch(`https://formspree.io/f/${CFG.formspreeId}`, {
        method:'POST',
        headers:{'Accept':'application/json'},
        body: retryData
      });
      sentWithoutFiles = res.ok;
    }

    if(res.ok){
      orderForm.reset();
      referenceFiles = [];
      updateFileText();
      const successMsg = sentWithoutFiles
        ? "Request sent! Your attached files couldn't come through automatically — please DM them to me on Discord so I have everything."
        : "Request sent — I'll be in touch soon!";
      formStatus.textContent = successMsg;
      formStatus.classList.add('is-success');
      showToast(successMsg);
    } else {
      formStatus.textContent = 'Something went wrong sending that. Please try the email link below instead.';
      formStatus.classList.add('is-error');
      showToast('Could not send — please try the email link instead.', true);
    }
  }catch(err){
    formStatus.textContent = 'Network error — please try the email link below instead.';
    formStatus.classList.add('is-error');
    showToast('Network error — please try the email link instead.', true);
  }
  resetSubmitButton();
}
function resetSubmitButton(){
  submitBtn.disabled = false;
  submitBtn.removeAttribute('aria-disabled');
  submitText.textContent = 'Send Request';
}

/* =========================================================
   REVIEW SUBMISSION FORM (Reviews page) — separate, single-screen
   form; not part of the order form wizard above. Submissions never
   publish automatically — they land in the inbox the same way
   commission requests do, and get added to CFG.reviews by hand.
   ========================================================= */
const reviewForm = document.getElementById('reviewForm');
const reviewFormStatus = document.getElementById('reviewFormStatus');
const reviewSubmitBtn = document.getElementById('reviewSubmitBtn');
const reviewSubmitText = document.getElementById('reviewSubmitText');
const reviewFormLoadedAt = Date.now();

function setRatingError(message){
  const starInput = reviewForm && reviewForm.querySelector('.star-input');
  const wrap = starInput ? starInput.closest('.field') : null;
  const errorEl = document.getElementById('ratingError');
  if (message) {
    if (wrap) wrap.classList.add('has-error');
    if (errorEl) errorEl.textContent = message;
  } else {
    if (wrap) wrap.classList.remove('has-error');
    if (errorEl) errorEl.textContent = '';
  }
}

/* Validates the text fields first (reusing the order form's generic
   validateStep, which no-ops harmlessly on the rating radios since they
   always carry a static, non-empty value), then checks the rating group
   separately last so its error state isn't immediately cleared again by
   validateStep's own pass over the same container. */
function validateReviewForm(){
  if (!reviewForm) return null;
  const textInvalid = validateStep(reviewForm);
  const ratingChecked = reviewForm.querySelector('input[name="rating"]:checked');
  let ratingInvalid = null;
  if (!ratingChecked) {
    setRatingError('Pick a star rating.');
    ratingInvalid = reviewForm.querySelector('input[name="rating"]');
  } else {
    setRatingError('');
  }
  return ratingInvalid || textInvalid;
}

if (reviewForm) {
  reviewForm.querySelectorAll('input[name="rating"]').forEach(r=>{
    r.addEventListener('change', ()=>setRatingError(''));
  });
  reviewForm.querySelectorAll('[required]').forEach(f=>{
    f.addEventListener('input', ()=>setFieldError(f, ''));
    f.addEventListener('change', ()=>setFieldError(f, ''));
  });

  reviewForm.addEventListener('submit', (e)=>{
    e.preventDefault();
    const firstInvalid = validateReviewForm();
    if (firstInvalid) { firstInvalid.focus(); return; }
    // basic bot check: honeypot filled, or submitted implausibly fast
    const honeypot = reviewForm.querySelector('input[name="_gotcha"]');
    const elapsed = Date.now() - reviewFormLoadedAt;
    if ((honeypot && honeypot.value) || elapsed < 2000) { return; } // silently drop — likely a bot
    submitReviewForm();
  });
}

async function submitReviewForm(){
  if (!reviewForm) return;
  const formspreeId = CFG.reviewsFormspreeId || CFG.formspreeId;
  reviewSubmitBtn.disabled = true;
  reviewSubmitBtn.setAttribute('aria-disabled','true');
  reviewSubmitText.innerHTML = '<span class="spinner" aria-hidden="true"></span> Sending…';
  reviewFormStatus.textContent = '';
  reviewFormStatus.className = 'form-status';

  if (!formspreeId || formspreeId === 'YOUR_FORM_ID') {
    const data = new FormData(reviewForm);
    const body = encodeURIComponent(
      `Rating: ${data.get('rating')}/5\nName: ${data.get('name')}\nEmail: ${data.get('email')}\nProject/Channel: ${data.get('role')||'—'}\n\nReview:\n${data.get('quote')}`
    );
    window.location.href = `mailto:${CFG.business.email}?subject=${encodeURIComponent('New Review Submission')}&body=${body}`;
    resetReviewSubmitButton();
    showToast('Opening your email client to send this review…');
    return;
  }

  try{
    const res = await fetch(`https://formspree.io/f/${formspreeId}`, {
      method:'POST',
      headers:{'Accept':'application/json'},
      body:new FormData(reviewForm)
    });
    if(res.ok){
      reviewForm.reset();
      const successMsg = "Review submitted — thank you! It'll show up on the site once it's been added.";
      reviewFormStatus.textContent = successMsg;
      reviewFormStatus.classList.add('is-success');
      showToast(successMsg);
    } else {
      reviewFormStatus.textContent = 'Something went wrong sending that. Please try again or email instead.';
      reviewFormStatus.classList.add('is-error');
      showToast('Could not send — please try again or email instead.', true);
    }
  }catch(err){
    reviewFormStatus.textContent = 'Network error — please try again or email instead.';
    reviewFormStatus.classList.add('is-error');
    showToast('Network error — please try again.', true);
  }
  resetReviewSubmitButton();
}
function resetReviewSubmitButton(){
  reviewSubmitBtn.disabled = false;
  reviewSubmitBtn.removeAttribute('aria-disabled');
  reviewSubmitText.textContent = 'Submit Review';
}

/* =========================================================
   PRODUCER BEAT SUBMISSION FORM (Beats page) — lets a producer send
   over a beat + their info. Unlike the review form above, this ONE
   DOES publish automatically — just not instantly and not without a
   check: submitting uploads the files straight to Blob storage, then
   sends Rowen a review email with every field plus one-click Approve
   / Reject links (see api/submit-beat.js, api/approve-beat.js, and
   the README "Getting beat submissions from producers" section).
   Approving is the entire "add it to the site" step; nothing needs
   editing config.js or pushing to GitHub. One beat per submission — a
   producer with several beats just submits this again for each one.
   ========================================================= */
const beatSubmitForm = document.getElementById('beatSubmitForm');
const beatSubmitStatus = document.getElementById('beatSubmitStatus');
const beatSubmitBtn = document.getElementById('beatSubmitBtn');
const beatSubmitText = document.getElementById('beatSubmitText');
const beatSubmitLoadedAt = Date.now();

const BEAT_FILE_MAX_MB = 15;
const BEAT_IMAGE_TYPES = ['image/png','image/jpeg','image/webp','image/gif'];
const BEAT_AUDIO_TYPES = ['audio/mpeg','audio/mp3'];
const BEAT_MP3_MIN_SECONDS = 30;
const BEAT_MP3_MAX_SECONDS = 5 * 60 + 30; // 5:30
// True while getAudioDurationSeconds() is mid-flight for the MP3 field —
// checked by the submit handler below so a very fast submit right after
// dropping a file can't sneak past the duration check before it resolves
// (the check itself is near-instant, but this closes the gap either way).
let beatMp3DurationCheckPending = false;

function formatSeconds(totalSeconds){
  const s = Math.round(totalSeconds);
  const m = Math.floor(s / 60);
  const rem = s % 60;
  return `${m}:${String(rem).padStart(2,'0')}`;
}

/* Reads an audio file's duration without ever uploading it, by loading
   it into a throwaway <audio> element via an object URL. Rejects if
   the browser can't determine a duration at all (corrupt file, or a
   type it can't decode) — treated as invalid, same as a wrong file
   type, rather than silently letting it through unchecked. */
function getAudioDurationSeconds(file){
  return new Promise((resolve, reject) => {
    const url = URL.createObjectURL(file);
    const audio = new Audio();
    function cleanup(){
      audio.removeEventListener('loadedmetadata', onLoaded);
      audio.removeEventListener('error', onError);
      URL.revokeObjectURL(url);
    }
    function onLoaded(){
      const duration = audio.duration;
      cleanup();
      if (!isFinite(duration) || duration <= 0) reject(new Error('Could not read this file\'s length.'));
      else resolve(duration);
    }
    function onError(){
      cleanup();
      reject(new Error('Could not read this file\'s length.'));
    }
    audio.addEventListener('loadedmetadata', onLoaded);
    audio.addEventListener('error', onError);
    audio.src = url;
  });
}

/* Wires a single-file drop zone (used for producer photo, beat cover,
   and the MP3 preview). Simpler than the order form's multi-file
   reference uploader above since each of these only ever holds one
   file — just validates type/size (and, when durationRange is given,
   audio length) and swaps the drop-zone text to show the selected
   filename (or an error, clearing the bad file). */
function wireSingleFileDrop({ inputId, dropId, textId, types, typeLabel, placeholder, durationRange }){
  const input = document.getElementById(inputId);
  const drop = document.getElementById(dropId);
  const text = document.getElementById(textId);
  if (!input || !drop || !text) return;

  // Reuses the same setFieldError() the generic required-field validator
  // uses (toggles the wrapping .field's has-error class + its
  // .field-error span) instead of a separate show/hide mechanism —
  // otherwise a required file input's "This field is required" error
  // (set by validateStep) and this drop zone's own type/size errors
  // would fight over two different ways of hiding the same element.
  function showError(message){
    setFieldError(input, message);
  }

  // The duration check is async (reading audio metadata takes a beat),
  // so a fast double-pick (drop one file, immediately drop another)
  // could otherwise let an old check's result land after a newer one.
  // A simple increasing token per call makes any stale check a no-op.
  let checkToken = 0;

  function reject(file, message){
    showError(message);
    input.value = '';
    text.textContent = placeholder;
  }

  async function applyFile(file){
    const myToken = ++checkToken;
    if (!file) { text.textContent = placeholder; showError(''); return; }
    if (!types.includes(file.type)) {
      reject(file, `"${file.name}" isn't a supported ${typeLabel} type.`);
      return;
    }
    if (file.size > BEAT_FILE_MAX_MB * 1024 * 1024) {
      reject(file, `"${file.name}" is over the ${BEAT_FILE_MAX_MB}MB limit.`);
      return;
    }
    if (durationRange) {
      showError('');
      text.textContent = `Checking "${file.name}"…`;
      beatMp3DurationCheckPending = true;
      let duration;
      try {
        duration = await getAudioDurationSeconds(file);
      } catch (err) {
        if (myToken === checkToken) beatMp3DurationCheckPending = false;
        if (myToken !== checkToken) return; // superseded by a newer pick
        reject(file, `Couldn't read "${file.name}" — try a different file.`);
        return;
      }
      if (myToken === checkToken) beatMp3DurationCheckPending = false;
      if (myToken !== checkToken) return; // a newer pick already replaced this one
      if (duration < durationRange.min) {
        reject(file, `"${file.name}" is ${formatSeconds(duration)} — previews must be at least ${formatSeconds(durationRange.min)}.`);
        return;
      }
      if (duration > durationRange.max) {
        reject(file, `"${file.name}" is ${formatSeconds(duration)} — previews must be ${formatSeconds(durationRange.max)} or shorter.`);
        return;
      }
    }
    showError('');
    text.textContent = file.name;
  }

  input.addEventListener('change', () => applyFile(input.files && input.files[0]));
  ['dragover','dragenter'].forEach(evt => drop.addEventListener(evt, (e) => { e.preventDefault(); drop.classList.add('drag'); }));
  ['dragleave','drop'].forEach(evt => drop.addEventListener(evt, (e) => { e.preventDefault(); drop.classList.remove('drag'); }));
  drop.addEventListener('drop', (e) => {
    const file = e.dataTransfer && e.dataTransfer.files && e.dataTransfer.files[0];
    if (!file) return;
    const dt = new DataTransfer();
    dt.items.add(file);
    input.files = dt.files;
    applyFile(file);
  });
}

if (beatSubmitForm) {
  wireSingleFileDrop({
    inputId: 'beatProducerPhoto', dropId: 'beatProducerPhotoDrop', textId: 'beatProducerPhotoText',
    types: BEAT_IMAGE_TYPES, typeLabel: 'image',
    placeholder: 'Drop an image here or click to browse'
  });
  wireSingleFileDrop({
    inputId: 'beatCover', dropId: 'beatCoverDrop', textId: 'beatCoverText',
    types: BEAT_IMAGE_TYPES, typeLabel: 'image',
    placeholder: 'Drop an image here or click to browse'
  });
  wireSingleFileDrop({
    inputId: 'beatMp3', dropId: 'beatMp3Drop', textId: 'beatMp3Text',
    types: BEAT_AUDIO_TYPES, typeLabel: 'MP3',
    placeholder: 'Drop an MP3 here or click to browse',
    durationRange: { min: BEAT_MP3_MIN_SECONDS, max: BEAT_MP3_MAX_SECONDS }
  });
}

function setBeatLicenseError(message){
  const wrap = document.getElementById('beatLicenseField');
  const errorEl = document.getElementById('beatLicenseError');
  if (message) {
    if (wrap) wrap.classList.add('has-error');
    if (errorEl) errorEl.textContent = message;
  } else {
    if (wrap) wrap.classList.remove('has-error');
    if (errorEl) errorEl.textContent = '';
  }
}

/* Same two-part pattern as validateReviewForm above: run the generic
   required-field validator first, then check the license checkbox
   group separately since "at least one of several checkboxes" isn't
   something validateStep's single-field logic covers. Used both as
   the per-step check (scoped to just that step, below) and as a final
   whole-form safety net on submit. */
function validateBeatSubmitForm(container){
  if (!beatSubmitForm) return null;
  const scope = container || beatSubmitForm;
  const textInvalid = validateStep(scope);
  if (textInvalid) return textInvalid;
  // Only check the license group if it's actually within the scope being
  // validated (so validating a single early step doesn't wrongly flag a
  // later step's license checkboxes).
  const licenseField = document.getElementById('beatLicenseField');
  if (licenseField && scope.contains(licenseField)) {
    const licenseChecked = beatSubmitForm.querySelector('input[name="license"]:checked');
    if (!licenseChecked) {
      setBeatLicenseError('Pick at least one license type.');
      return beatSubmitForm.querySelector('input[name="license"]');
    }
    setBeatLicenseError('');
  }
  return null;
}

if (beatSubmitForm) {
  beatSubmitForm.querySelectorAll('input[name="license"]').forEach(c=>{
    c.addEventListener('change', ()=>setBeatLicenseError(''));
  });
  // Skips type="file" here on purpose — wireSingleFileDrop's own change
  // handler already sets the correct error (or clears it) after actually
  // validating the picked file. This blind "clear on change" is only
  // safe for fields where any change means the field is now fine.
  beatSubmitForm.querySelectorAll('[required]:not([type="file"])').forEach(f=>{
    f.addEventListener('input', ()=>setFieldError(f, ''));
    f.addEventListener('change', ()=>setFieldError(f, ''));
  });
}

/* =========================================================
   BEAT SUBMISSION FORM — same one-question-at-a-time wizard as the
   order form above (see ORDER FORM — one-question-at-a-time wizard),
   just wired to its own set of elements/ids so the two forms don't
   interfere with each other.
   ========================================================= */
const beatFormSteps = beatSubmitForm ? Array.from(beatSubmitForm.querySelectorAll('#beatFormSteps .form-step')) : [];
const BEAT_TOTAL_STEPS = beatFormSteps.length;
const BEAT_REVIEW_STEP_INDEX = BEAT_TOTAL_STEPS - 1;
let beatCurrentStep = 0;
const beatStepBackBtn = document.getElementById('beatStepBack');
const beatStepNextBtn = document.getElementById('beatStepNext');
const beatFormProgressFill = document.getElementById('beatFormProgressFill');
const beatFormStepCount = document.getElementById('beatFormStepCount');

function goToBeatStep(index, opts){
  if (!beatFormSteps.length) return;
  opts = opts || {};
  beatCurrentStep = Math.max(0, Math.min(index, BEAT_TOTAL_STEPS - 1));
  beatFormSteps.forEach((el,i)=> el.classList.toggle('active', i === beatCurrentStep));
  const isReview = beatCurrentStep === BEAT_REVIEW_STEP_INDEX;
  if (beatStepBackBtn) beatStepBackBtn.hidden = beatCurrentStep === 0;
  if (beatStepNextBtn) beatStepNextBtn.hidden = isReview;
  if (beatSubmitBtn) beatSubmitBtn.hidden = !isReview;
  const pct = Math.round(((beatCurrentStep + 1) / BEAT_TOTAL_STEPS) * 100);
  if (beatFormProgressFill) beatFormProgressFill.style.width = pct + '%';
  if (beatFormStepCount) beatFormStepCount.textContent = `Step ${beatCurrentStep + 1} of ${BEAT_TOTAL_STEPS}`;
  if (isReview) renderBeatReviewSummary();
  if (opts.focusField) {
    opts.focusField.focus();
  } else {
    const heading = beatFormSteps[beatCurrentStep].querySelector('.form-step-heading');
    if (heading) heading.focus({ preventScroll: false });
  }
}

if (beatStepNextBtn) {
  beatStepNextBtn.addEventListener('click', ()=>{
    const firstInvalid = validateBeatSubmitForm(beatFormSteps[beatCurrentStep]);
    if (firstInvalid) { firstInvalid.focus(); return; }
    goToBeatStep(beatCurrentStep + 1);
  });
}
if (beatStepBackBtn) {
  beatStepBackBtn.addEventListener('click', ()=> goToBeatStep(beatCurrentStep - 1));
}
if (beatSubmitForm) {
  // Enter advances to the next step instead of doing nothing — except in
  // a textarea, where it should just insert a newline like normal.
  beatSubmitForm.addEventListener('keydown', (e)=>{
    if (e.key !== 'Enter' || e.target.tagName === 'TEXTAREA') return;
    if (beatCurrentStep === BEAT_REVIEW_STEP_INDEX) return; // let the Submit Beat button handle this step
    e.preventDefault();
    if (beatStepNextBtn) beatStepNextBtn.click();
  });
}

/* Plain-text summary of every answer, shown on the final review step —
   same reviewRow() helper the order form's review step uses above
   (textContent throughout, so nothing typed in can be interpreted as
   markup). */
function renderBeatReviewSummary(){
  const beatReviewSummaryEl = document.getElementById('beatReviewSummary');
  if (!beatReviewSummaryEl || !beatSubmitForm) return;
  beatReviewSummaryEl.innerHTML = '';
  const val = id => { const el = document.getElementById(id); return el ? el.value.trim() : ''; };
  const fileName = id => { const el = document.getElementById(id); return el && el.files && el.files[0] ? el.files[0].name : null; };
  const licenses = Array.from(beatSubmitForm.querySelectorAll('input[name="license"]:checked')).map(c => c.value);
  reviewRow(beatReviewSummaryEl, 'Producer / artist', val('beatProducerName') || '—');
  reviewRow(beatReviewSummaryEl, 'Logo / photo', fileName('beatProducerPhoto') || 'None');
  reviewRow(beatReviewSummaryEl, 'Bio', val('beatBio') || '—');
  reviewRow(beatReviewSummaryEl, 'Social links', val('beatSocialLinks') || '—');
  reviewRow(beatReviewSummaryEl, 'Beat store link', val('beatStoreLink') || '—');
  reviewRow(beatReviewSummaryEl, 'Beat title', val('beatTitle') || '—');
  reviewRow(beatReviewSummaryEl, 'BPM', val('beatBpm') || '—');
  reviewRow(beatReviewSummaryEl, 'Key', val('beatKey') || '—');
  reviewRow(beatReviewSummaryEl, 'Genre', val('beatGenre') || '—');
  reviewRow(beatReviewSummaryEl, 'Mood / tags', val('beatMood') || '—');
  reviewRow(beatReviewSummaryEl, 'Price', val('beatPrice') ? `$${val('beatPrice')}` : 'Default ($12)');
  reviewRow(beatReviewSummaryEl, 'License types', licenses.length ? licenses.join(', ') : '—');
  reviewRow(beatReviewSummaryEl, 'Cover image', fileName('beatCover') || 'None');
  reviewRow(beatReviewSummaryEl, 'MP3 preview', fileName('beatMp3') || 'None');
  reviewRow(beatReviewSummaryEl, 'WAV link', val('beatWavLink') || '—');
}

if (beatSubmitForm) {
  beatSubmitForm.addEventListener('submit', (e)=>{
    e.preventDefault();
    if (beatMp3DurationCheckPending) {
      beatSubmitStatus.textContent = 'Still checking the MP3\'s length — try submitting again in a second.';
      beatSubmitStatus.className = 'form-status is-error';
      return;
    }
    // Final whole-form safety net, in case something invalid somehow
    // slipped past the per-step Continue checks — jumps back to
    // whichever step actually has the problem, same as the order form.
    const firstInvalid = validateBeatSubmitForm();
    if (firstInvalid) {
      const stepEl = firstInvalid.closest('.form-step');
      if (stepEl) goToBeatStep(Number(stepEl.dataset.step), { focusField: firstInvalid });
      else firstInvalid.focus();
      return;
    }
    // basic bot check: honeypot filled, or submitted implausibly fast
    const honeypot = beatSubmitForm.querySelector('input[name="_gotcha"]');
    const elapsed = Date.now() - beatSubmitLoadedAt;
    if ((honeypot && honeypot.value) || elapsed < 2000) { return; } // silently drop — likely a bot
    submitBeatForm();
  });
  goToBeatStep(0);
}

// Pinned version, loaded on demand (not at page load) so a slow or
// blocked CDN never delays the Beats page itself — only submitting
// the form needs it. Pinned rather than "@latest" so a future release
// of @vercel/blob can't silently change this form's behavior; bump
// this deliberately (and re-test a real submission) if it ever needs
// updating.
const VERCEL_BLOB_CLIENT_CDN_URL = 'https://esm.sh/@vercel/blob@0.27.1/client';

// Uploads one <input type="file"> field straight to Blob storage (see
// api/beat-upload.js, which authorizes this) and resolves to its
// public URL — or null if nothing was selected, since photo/cover are
// optional. Doing this client-side (instead of sending the file
// through a server function) is what lets a 15MB MP3 through at all —
// Vercel functions cap request bodies at a few MB, well under that.
async function uploadBeatFile(upload, submissionFolder, fieldName, subfolder){
  const input = beatSubmitForm.querySelector(`[name="${fieldName}"]`);
  const file = input && input.files && input.files[0];
  if (!file) return null;
  const blob = await upload(`pending/${submissionFolder}/${subfolder}-${file.name}`, file, {
    access: 'public',
    handleUploadUrl: '/api/beat-upload'
  });
  return blob.url;
}

async function submitBeatForm(){
  if (!beatSubmitForm) return;
  beatSubmitBtn.disabled = true;
  beatSubmitBtn.setAttribute('aria-disabled','true');
  beatSubmitText.innerHTML = '<span class="spinner" aria-hidden="true"></span> Sending…';
  beatSubmitStatus.textContent = '';
  beatSubmitStatus.className = 'form-status';

  const data = new FormData(beatSubmitForm);
  const honeypot = data.get('_gotcha');
  const elapsedMs = Date.now() - beatSubmitLoadedAt;

  try {
    const { upload } = await import(VERCEL_BLOB_CLIENT_CDN_URL);
    // Just a folder name to keep this submission's own files together
    // in Blob storage — not a security boundary (the approve/reject
    // links are what's actually signed), so a simple random string is
    // fine here.
    const submissionFolder = `${Date.now()}-${Math.random().toString(36).slice(2, 10)}`;

    const [photoUrl, coverUrl, mp3Url] = await Promise.all([
      uploadBeatFile(upload, submissionFolder, 'producerPhoto', 'photo'),
      uploadBeatFile(upload, submissionFolder, 'coverImage', 'cover'),
      uploadBeatFile(upload, submissionFolder, 'mp3Preview', 'mp3')
    ]);

    const payload = {
      producerName: data.get('producerName'),
      bio: data.get('bio'),
      socialLinks: data.get('socialLinks'),
      storeLink: data.get('storeLink'),
      beatTitle: data.get('beatTitle'),
      bpm: data.get('bpm'),
      key: data.get('key'),
      genre: data.get('genre'),
      mood: data.get('mood'),
      price: data.get('price'),
      license: data.getAll('license'),
      wavLink: data.get('wavLink'),
      permission: data.get('permission') === 'on',
      photoUrl, coverUrl, mp3Url,
      gotcha: honeypot,
      elapsedMs
    };

    const res = await fetch('/api/submit-beat', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(payload)
    });
    const result = await res.json().catch(() => ({}));

    if (res.ok) {
      beatSubmitForm.reset();
      ['beatProducerPhotoText','beatCoverText'].forEach(id=>{
        const el = document.getElementById(id);
        if (el) el.textContent = 'Drop an image here or click to browse';
      });
      const mp3Text = document.getElementById('beatMp3Text');
      if (mp3Text) mp3Text.textContent = 'Drop an MP3 here or click to browse';
      goToBeatStep(0);
      const successMsg = "Beat submitted — thank you! I'll take a quick look and it'll go live once I approve it.";
      beatSubmitStatus.textContent = successMsg;
      beatSubmitStatus.classList.add('is-success');
      showToast(successMsg);
    } else {
      const errorMsg = result.error || 'Something went wrong sending that. Please try again or send it on Discord instead.';
      beatSubmitStatus.textContent = errorMsg;
      beatSubmitStatus.classList.add('is-error');
      showToast('Could not send — please try again or use Discord instead.', true);
    }
  } catch (err) {
    beatSubmitStatus.textContent = 'Network error — please try again or send it on Discord instead.';
    beatSubmitStatus.classList.add('is-error');
    showToast('Network error — please try again.', true);
  }
  resetBeatSubmitButton();
}
function resetBeatSubmitButton(){
  beatSubmitBtn.disabled = false;
  beatSubmitBtn.removeAttribute('aria-disabled');
  beatSubmitText.textContent = 'Submit Beat';
}

function showToast(msg, isError){
  const toast = document.getElementById('toast');
  toast.textContent = msg;
  toast.classList.toggle('is-error', !!isError);
  toast.classList.add('show');
  setTimeout(()=>toast.classList.remove('show'), 3600);
}

/* ---------- kick things off ---------- */
routeFromHash();
observeReveal();
observeRevealStagger();
bindCursorTargets();
bindMagnetic();

})();

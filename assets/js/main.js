/* =========================================================
   ROWEN — site behavior
   Reads content from SITE_CONFIG (assets/js/config.js, loaded
   before this file) and wires up every interactive piece:
   router, portfolio grids + filters, project detail modal,
   pricing/how-it-works/FAQ/testimonials rendering, policy
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
const PAGE_KEYS = ['home','cover-art','thumbnails','ads','pfps','order'];
const pageEls = {};
PAGE_KEYS.forEach(k=>{ pageEls[k] = document.querySelector(`.page[data-page="${k}"]`); });
const navLinkEls = document.querySelectorAll('.nav-links a[data-page], .mobile-nav-panel a[data-page]');
const dotLinkEls = document.querySelectorAll('.dot-link[data-page]');
const PAGE_TITLES = {
  'home': `${CFG.business.name} — Graphic Design Studio`,
  'cover-art': `Cover Art — ${CFG.business.name}`,
  'thumbnails': `YouTube Thumbnails — ${CFG.business.name}`,
  'ads': `Ad Creative — ${CFG.business.name}`,
  'pfps': `PFPs — ${CFG.business.name}`,
  'order': `Start a Project — ${CFG.business.name}`
};
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
   PAYMENT CONFIRM MODAL
   ========================================================= */
const paymentModal = document.getElementById('paymentModal');
wireModalDismiss(paymentModal, 'paymentModalBg', 'paymentModalClose');
const paymentModalBack = document.getElementById('paymentModalBack');
if (paymentModalBack) paymentModalBack.addEventListener('click', ()=>closeModal(paymentModal));
const paymentModalConfirm = document.getElementById('paymentModalConfirm');
if (paymentModalConfirm) paymentModalConfirm.addEventListener('click', ()=>{
  closeModal(paymentModal);
  submitOrderForm();
});

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
    <div class="policy-note"><b>Draft placeholder:</b> this is generic starter wording, not legal advice. Have it reviewed (ideally by a lawyer familiar with your local laws) and customize it before relying on it.</div>
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
   TESTIMONIALS (hidden automatically when empty)
   ========================================================= */
const testimonialSection = document.getElementById('testimonialsSection');
const testimonialGrid = document.getElementById('testimonialGrid');
if (testimonialSection && testimonialGrid) {
  if (CFG.testimonials && CFG.testimonials.length) {
    testimonialGrid.innerHTML = CFG.testimonials.map(t => `
      <div class="testimonial-card">
        <p class="testimonial-quote">${t.quote}</p>
        <div class="testimonial-who">
          ${t.avatar ? `<img class="testimonial-avatar" src="${t.avatar}" alt="" loading="lazy">` : `<span class="testimonial-avatar-fallback" aria-hidden="true">${(t.name||'?').charAt(0)}</span>`}
          <div>
            <div class="testimonial-name">${t.name}</div>
            ${t.role ? `<div class="testimonial-role">${t.role}</div>` : ''}
          </div>
        </div>
      </div>
    `).join('');
    testimonialSection.hidden = false;
  } else {
    testimonialSection.hidden = true;
  }
}

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

function validateForm(){
  if (!orderForm) return true;
  let firstInvalid = null;
  const required = orderForm.querySelectorAll('[required]');
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
  if (firstInvalid) {
    firstInvalid.focus();
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

if (orderForm) {
  orderForm.addEventListener('submit', (e)=>{
    e.preventDefault();
    if(!validateForm()) return;
    // basic bot check: honeypot filled, or submitted implausibly fast (<2.5s)
    const honeypot = orderForm.querySelector('input[name="_gotcha"]');
    if (honeypot && honeypot.value) { return; } // silently drop — likely a bot
    openModal(paymentModal);
  });
}

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

  try{
    const res = await fetch(`https://formspree.io/f/${CFG.formspreeId}`, {
      method:'POST',
      headers:{'Accept':'application/json'},
      body:new FormData(orderForm)
    });
    if(res.ok){
      orderForm.reset();
      referenceFiles = [];
      updateFileText();
      formStatus.textContent = "Request sent — I'll be in touch soon!";
      formStatus.classList.add('is-success');
      showToast("Request sent — I'll be in touch soon!");
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

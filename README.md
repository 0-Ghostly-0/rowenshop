# rowen.work

Monochrome graphic design studio site for Rowen — cover art, YouTube thumbnails, ads & PFPs. Built as one HTML page that feels like a multi-page site: navigating between Cover Art / Thumbnails / Ads / PFPs / Order swaps the visible page with a transition, and each has its own shareable URL (e.g. `rowen.work/#cover-art`), but it's all a single page load — no build step, no framework, no server routing needed.

## Files
- `index.html` — page structure/markup only
- `assets/css/styles.css` — all styles
- `assets/js/config.js` — **the one file you'll edit most.** Business info, service pricing, FAQ, policies, testimonials, and portfolio category settings all live here.
- `assets/js/main.js` — all interactive behavior (router, portfolio grids, modals, form handling, etc.), reads from `config.js`
- `assets/covers/` — cover art portfolio images
- `assets/thumbnails/` — thumbnail portfolio images
- `assets/brand/` — logo, favicon, background texture
- `robots.txt`, `sitemap.xml` — SEO
- `favicon.ico`

## Pages (hash-routed, all client-side)
- **Home** (`#home`) — hero, selected work, services, pricing, how it works, testimonials (auto-hidden if empty), FAQ, CTA
- **Cover Art** (`#cover-art`) — full portfolio grid (26 real pieces)
- **Thumbnails** (`#thumbnails`) — full portfolio grid (32 real pieces)
- **Ads** (`#ads`) — "coming soon" panel until real examples are added
- **PFPs** (`#pfps`) — "coming soon" panel until real examples are added
- **Order** (`#order`) — commission form + business contact section; the "What do you need?" field auto-fills based on which service page you arrived from

## Editing content — how to use `assets/js/config.js`
This is a plain text file, not a program you run — no coding tools required. Any text editor works (Notepad, VS Code, even editing the file directly on GitHub's web UI).

**How to edit it:**
1. Open `assets/js/config.js` in a text editor.
2. Find the value you want to change, edit the text between the quotes, save.
3. See the change: if you're testing locally, just refresh the page. If it's already deployed on Vercel, push the changed file to your GitHub repo and Vercel auto-redeploys in about a minute — no build step, no rebuild command needed.

**What's still a placeholder you should fill in yourself**, roughly in priority order:
- Lines marked `// EDIT ME` (turnaround times, revision counts, file formats per service) — these are guesses to keep the pricing section from looking empty. Swap in your real numbers.
- `testimonials: []` — empty on purpose. Add real ones as `{ quote, name, role }` objects and that section appears on the homepage automatically; leave it empty and it stays hidden. Never invent fake ones.
- `policies: {...}` (Terms of Service / Commission Policy / Privacy Policy / Refund Policy) — the text is generic boilerplate. Fine to publish short-term, but worth a real read-through (or a lawyer's) before you rely on it.
- `portfolio.overrides` — empty by default. If you want a specific cover or thumbnail to show its project type, tools used, or a short description in the detail popup, add an entry there — otherwise those fields just stay hidden, no fake info shown.
- Ads/PFPs galleries — still `count: 0, comingSoon: true`. Once you have real examples, drop the images in `assets/ads/`/`assets/pfps/` following the existing `ad-01.webp`, `ad-02.webp`... naming pattern, bump that category's `count`, and flip `comingSoon` to `false`.

Everything else — business name, email, Discord/social links, budget ranges, the Formspree form ID, FAQ answers — lives in the same file and is safe to tweak anytime the same way.

## Status
- Order form: connected to Formspree (form ID `xojgvwbp`), sending submissions to rowenapichardo@gmail.com. Falls back to opening the visitor's email client if Formspree is ever unreachable. Includes a honeypot field for basic spam prevention.
- Contact: Discord is the featured/preferred channel throughout (nav, hero badge, footer, order page); email and CashApp/Venmo/Apple Pay are secondary, framed as "pay after you accept a quote," not instant checkout.
- Opening animation: a short logo intro plays once on every page load, then fades out (respects `prefers-reduced-motion`).
- Portfolio: filterable by category via the tabs above each portfolio grid; clicking any piece opens an accessible project detail modal (type, client label, tools, description, optional before/after slider where configured).
- Policies (Terms of Service, Commission Policy, Privacy Policy, Refund & Cancellation Policy) open as modals from the footer — placeholder text, flagged for legal review before publishing.

## Going live
Deploy to Vercel and connect the `rowen.work` domain — see the project chat history for the full walkthrough, summarized below.

### Run locally
No build step. From the project folder:
```
npx serve .
```
or just open `index.html` directly in a browser (the file:// protocol works fine since there's no server-side code).

### Deploy to Vercel
1. Push this folder to a GitHub repo.
2. In Vercel, "Add New Project" → import that repo. Framework preset: **Other** (static site) — no build command needed.
3. Deploy.

### Connect rowen.work
1. In the Vercel project → Settings → Domains → add `rowen.work` (and `www.rowen.work` if you want both).
2. Vercel gives you DNS records (usually an A record for the apex domain and a CNAME for `www`) — add those at your domain registrar.
3. Wait for DNS to propagate (usually minutes, sometimes longer) and Vercel to issue the SSL certificate automatically.

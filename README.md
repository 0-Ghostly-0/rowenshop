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
- **Order** (`#order`) — commission form + business contact section; the "What do you need?" field auto-fills based on which service page you arrived from. The form is a one-question-at-a-time wizard (`assets/js/main.js`, "ORDER FORM — one-question-at-a-time wizard" section): each `.form-step` in `index.html` is a screen, a shared Back/Continue/Send Request bar at the bottom drives navigation, and the second-to-last step explains how payment works before a final review screen. If you ever add or remove a field, add/remove its `.form-step` (or add it to an existing one) and update `renderReviewSummary()` in main.js so the review screen picks it up.

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
- `business.stripe` — set to a **test-mode** Stripe Payment Link right now. Swap it for a live-mode link before you launch, or the "Card" button will only accept Stripe test cards. See "Setting up the Stripe 'Card' payment option" above.

Everything else — business name, email, Discord/social links, budget ranges, the Formspree form ID, FAQ answers — lives in the same file and is safe to tweak anytime the same way.

### Setting up the Stripe "Card" payment option
The site has a spot for a Stripe payment option next to CashApp/Venmo/Apple Pay. It's already wired up with a real Payment Link (product: "Design Commission Payment," customer chooses the amount at checkout — min $25, preset $50, max $1,000 — plus a required "Project name or Discord username" field so you can match a payment to a commission).

**⚠ It's currently a TEST MODE link.** It was created through your connected Stripe account, which is a sandbox — so right now it only accepts Stripe's test cards (like `4242 4242 4242 4242`, any future expiry, any CVC), not real money. Before you launch:
1. In your Stripe Dashboard, switch from Sandbox/Test to **Live mode** (toggle top-left).
2. Recreate the same payment link there — Payment links → Create payment link → a product with price type **"Customer chooses price"** (this matters because your pricing is a custom quote, not fixed) → same $25 min / $1,000 max if you want. Or just ask me to do it once your Stripe connection points at the live account and I can set it up the same way.
3. Copy the new `https://buy.stripe.com/xxxxxxxx` URL (no `test_` in it — that's how you know it's live).
4. Open `assets/js/config.js`, find `stripe:` near the top (under `business:`), and replace the test URL with the live one.
5. Save and push to GitHub — the button keeps working, just now for real payments.

The button stays hidden automatically if this field is ever emptied out, so it's safe to deploy at any point. When a client pays, they land on Stripe's own secure checkout page (not something hosted on rowen.work) — Stripe handles all the card data, receipts, and security; your site never touches payment info.

## Status
- Order form: connected to Formspree (form ID `xojgvwbp`), sending submissions to rowenapichardo@gmail.com. Falls back to opening the visitor's email client if Formspree is ever unreachable. Includes a honeypot field for basic spam prevention. If a submission has reference files attached and Formspree rejects it (e.g. file uploads aren't included on your current Formspree plan), the form automatically retries without the files so the message still gets through, and tells the visitor to DM the files on Discord instead.
- Contact: Discord is the featured/preferred channel throughout (nav, hero badge, footer, order page); email, CashApp/Venmo/Apple Pay, and card via Stripe are secondary, framed as "pay after you accept a quote," not instant checkout.
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

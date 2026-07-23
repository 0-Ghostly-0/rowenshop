# rowen.work

Monochrome studio site for Rowen — cover art, YouTube thumbnails, ads, PFPs, beats, and (soon) VSTs. Built as one HTML page that feels like a multi-page site: navigating between pages swaps the visible page with a transition, and each has its own shareable URL (e.g. `rowen.work/#cover-art`), but it's all a single page load — no build step, no framework, no server routing needed.

## Files
- `index.html` — page structure/markup only
- `assets/css/styles.css` — all styles
- `assets/js/config.js` — **the one file you'll edit most.** Business info, service pricing, FAQ, policies, reviews, beats catalog, and portfolio category settings all live here.
- `assets/js/main.js` — all interactive behavior (router, portfolio grids, beats player, modals, form handling, etc.), reads from `config.js`
- `assets/covers/` — cover art portfolio images
- `assets/thumbnails/` — thumbnail portfolio images
- `assets/beats/` — beat audio files and optional cover art (create this folder when you add your first beat)
- `assets/brand/` — logo, favicon, background texture
- `robots.txt`, `sitemap.xml` — SEO
- `favicon.ico`

## Pages (hash-routed, all client-side)
- **Home** (`#home`) — hero, selected work, services, pricing, how it works, reviews teaser with an average rating banner (auto-hidden if empty), FAQ, CTA
- **Cover Art** (`#cover-art`) — full portfolio grid (26 real pieces)
- **Thumbnails** (`#thumbnails`) — full portfolio grid (32 real pieces)
- **Ads** (`#ads`) — "coming soon" panel until real examples are added
- **PFPs** (`#pfps`) — "coming soon" panel until real examples are added
- **Beats** (`#beats`) — beat catalog with preview player + instant Stripe checkout; shows a "coming soon" panel automatically until you add your first entry to `CFG.beats` (see "Adding a new beat" below)
- **VSTs** (`#vsts`) — "coming soon" placeholder for now — add real content here whenever those are ready (same pattern as Ads/PFPs: build the real page, remove the coming-soon panel)
- **Reviews** (`#reviews`) — average rating banner + full grid of approved reviews (shows a "no reviews yet" panel until you add the first one), plus a review submission form for clients. Submissions never publish automatically — see "Approving a submitted review" below.
- **Order** (`#order`) — commission form + business contact section; the "What do you need?" field auto-fills based on which service page you arrived from. The form is a one-question-at-a-time wizard (`assets/js/main.js`, "ORDER FORM — one-question-at-a-time wizard" section): each `.form-step` in `index.html` is a screen, a shared Back/Continue/Send Request bar at the bottom drives navigation, and the second-to-last step explains how payment works before a final review screen. If you ever add or remove a field, add/remove its `.form-step` (or add it to an existing one) and update `renderReviewSummary()` in main.js so the review screen picks it up. Beats/VSTs are sold separately from this form (instant checkout, not a custom quote), so "Ask About a Custom Beat" links to Discord instead.

## Editing content — how to use `assets/js/config.js`
This is a plain text file, not a program you run — no coding tools required. Any text editor works (Notepad, VS Code, even editing the file directly on GitHub's web UI).

**How to edit it:**
1. Open `assets/js/config.js` in a text editor.
2. Find the value you want to change, edit the text between the quotes, save.
3. See the change: if you're testing locally, just refresh the page. If it's already deployed on Vercel, push the changed file to your GitHub repo and Vercel auto-redeploys in about a minute — no build step, no rebuild command needed.

**What's still a placeholder you should fill in yourself**, roughly in priority order:
- Lines marked `// EDIT ME` (turnaround times, revision counts, file formats per service) — these are guesses to keep the pricing section from looking empty. Swap in your real numbers.
- `reviews: []` — empty on purpose. Clients can submit one from the Reviews page, but nothing publishes automatically — add the ones you approve as `{ quote, name, role, rating }` objects (see "Approving a submitted review" below) and both the homepage teaser and the Reviews page pick it up automatically, average rating included. Never invent fake ones.
- `policies: {...}` (Terms of Service / Commission Policy / Privacy Policy / Refund Policy) — the text is generic boilerplate. Fine to publish short-term, but worth a real read-through (or a lawyer's) before you rely on it.
- `portfolio.overrides` — empty by default. If you want a specific cover or thumbnail to show its project type, tools used, or a short description in the detail popup, add an entry there — otherwise those fields just stay hidden, no fake info shown.
- Ads/PFPs galleries — still `count: 0, comingSoon: true`. Once you have real examples, drop the images in `assets/ads/`/`assets/pfps/` following the existing `ad-01.webp`, `ad-02.webp`... naming pattern, bump that category's `count`, and flip `comingSoon` to `false`.
- `business.stripe` — set to a **test-mode** Stripe Payment Link right now. Swap it for a live-mode link before you launch, or the "Card" button will only accept Stripe test cards. See "Setting up the Stripe 'Card' payment option" above.
- `beats: []` — empty on purpose. The Beats page shows a "coming soon" panel until you add your first entry. See "Uploading new media yourself" below.

Everything else — business name, email, Discord/social links, budget ranges, the Formspree form ID, FAQ answers — lives in the same file and is safe to tweak anytime the same way.

## Uploading new media yourself (no need to ask me)
Everything below can be done straight from **github.com** in your browser — no code editor, no terminal, no local setup. The short version for anything (a new cover art image, a new beat, anything): upload the file(s) through GitHub's web UI, add a few lines to `config.js` following the existing pattern, then commit — Vercel redeploys automatically in about a minute.

**To upload a file on GitHub:**
1. Go to your repo on github.com and open the folder you want (e.g. `assets/covers`).
2. Click **Add file → Upload files**.
3. Drag in your image/audio file(s), scroll down, and click **Commit changes**. That's it — the file now lives at that path on the live site.

### Adding a new portfolio piece (Cover Art / Thumbnails)
1. Upload the image to `assets/covers/` or `assets/thumbnails/` following the existing naming pattern (`cover-27.webp`, `thumb-33.webp`, etc. — next number in sequence, same file type as the others).
2. Open `assets/js/config.js`, find `portfolio.categories`, and bump that category's `count` by 1 (e.g. `count: 26` → `count: 27`).
3. Commit. The new piece appears in the grid automatically. (Optional: add a `portfolio.overrides` entry for it if you want a description/tools/client label to show in its detail popup — see the comment right above that section.)

### Adding a new beat
1. **Upload the audio file** (and an optional square cover image) to `assets/beats/` using the GitHub upload steps above. Create the `assets/beats/` folder the first time by uploading into a path that doesn't exist yet — GitHub creates it for you. MP3 is the safest format for browser playback.
2. **Create its Stripe Payment Link** (a fixed price, since each beat sells for one set amount):
   - In your Stripe Dashboard → Payment links → Create payment link.
   - Add a product (e.g. the beat's title) with a normal fixed price — **not** "Customer chooses price" (that's only for the deposit/Card option elsewhere on the site).
   - Finish creating it and copy the `https://buy.stripe.com/...` URL it gives you.
   - Remember: if your Stripe Dashboard is in Sandbox/Test mode, this link only takes test cards — switch to Live mode first if you want it to accept real payments (same as the note above for the Card payment option).
3. **Add the entry to `config.js`.** Find the `beats:` array (it has a commented-out example right above it showing every field) and add an object like:
   ```js
   {
     title: "Midnight Drive",
     bpm: 140,
     key: "F# Minor",
     tags: ["Trap", "Dark"],
     price: 12,
     audio: "assets/beats/midnight-drive.mp3",
     cover: "assets/beats/midnight-drive-cover.jpg",
     stripeLink: "https://buy.stripe.com/xxxxxxxx"
   },
   ```
   `bpm`, `key`, `tags`, and `cover` are all optional — leave any of them out and that part just doesn't show. Leaving `stripeLink` blank turns the card's button into "Ask on Discord" instead of "Buy," so it's safe to add a beat before its payment link exists. Standing default: all beats are listed at **$12** unless you decide otherwise for a specific one.
4. Commit. The "coming soon" panel on the Beats page disappears automatically the moment there's at least one entry, and your new beat shows up with a working preview player and Buy button.

### Hosting other producers' beats
You can list a beat made by another producer and automatically split each sale with them, using Stripe Connect — no code or backend needed, just an extra step or two per producer/beat on top of the normal "Adding a new beat" flow above. **On this site, you (Rowen) are always the merchant of record and you handle any refunds/disputes yourself**, same as every other sale — the producer never needs their own storefront or support process.

**One-time setup:**
1. In your Stripe Dashboard, go to Connect and register as a platform (a short profile form — what your business does). This only needs to happen once, ever.
2. Under Connect settings → branding, set your business name/icon/color so producer onboarding and checkout pages look consistent with your site.

**Onboarding a new producer (once per producer):**
1. Dashboard → Connect → Connected accounts → **+Create** → choose **Express** as the account type and their country.
2. Stripe generates a one-time onboarding link — send it to the producer privately (DM/email, never post it publicly). It expires in 90 days and only works once.
3. They complete Stripe's own onboarding (identity + bank account) directly with Stripe — you never see or handle that information yourself.
4. Once they're done, their account appears in your Connected accounts list and you're ready to list their beats.

**Listing one of their beats (per beat, same rhythm as "Adding a new beat" above):**
1. Upload the audio file the same way as usual.
2. Create the Payment Link the same way, but before finishing: under Advanced options → After payment, check **"Split the payment with a connected account"** and select that producer's account. Leave **"Make payment on behalf of selected account"** unchecked (this keeps your business as the merchant of record, matching the choice above).
3. Enter your cut as a flat dollar amount — the no-code version of Payment Links takes a fixed amount per sale rather than a live percentage, so do that math once against the beat's price (e.g. $6 on a $20 beat for a 30% cut).
4. Add the entry to `config.js` exactly as before, plus a `producer` field:
   ```js
   {
     title: "Neon Skyline",
     price: 20,
     audio: "assets/beats/neon-skyline.mp3",
     stripeLink: "https://buy.stripe.com/xxxxxxxx",
     producer: { name: "Jordan", link: "https://instagram.com/jordanbeats" }
   },
   ```
   `producer.link` is optional — leave it out and the card just shows plain text ("Produced by Jordan") instead of a clickable name.
5. Commit. The card shows "Produced by ___" under the title, and Stripe automatically routes the producer's share to them and keeps your cut on every sale — nothing else to track manually.

A couple of things worth keeping in mind: you're vouching for every producer you onboard (Stripe holds your platform responsible for vetting against fraud, and for covering a producer's balance if it ever goes negative from a refund after they've been paid out), and it's worth having a quick, even informal, understanding with each producer confirming the beat is actually theirs to sell and that they're okay with the revenue split before you list anything.

### Approving a submitted review
Reviews never publish themselves — when a client uses the form on the Reviews page (`#reviews`), it just sends you a message the same way a commission request does (labeled "New Review Submission" so it's easy to spot in your inbox, or in a separate Formspree form if you set `reviewsFormspreeId` — see the comment above that field in `config.js`). Nothing appears on the site until you add it yourself:
1. Read the submission and decide if you want to publish it. Skip this step entirely for any you don't.
2. Open `assets/js/config.js`, find the `reviews:` array (it has a commented-out example right above it showing every field), and add an object like:
   ```js
   {
     quote: "Rowen nailed the vibe on the first try — fast, easy to work with, worth every penny.",
     name: "Jordan",
     role: "Cover Art client",
     rating: 5
   },
   ```
   `role` and `avatar` are optional — leave either out and that part just doesn't show (no avatar image falls back to a plain initial-letter circle). `rating` is required, a whole number 1–5. Feel free to lightly tidy typos in what they wrote, but never change the substance of a review or invent one that didn't happen.
3. Commit. The new review shows up immediately on both the homepage teaser and the Reviews page, and the average rating banner on each recalculates automatically — no separate number to keep in sync.

If you'd rather I set any of this up directly — creating the Stripe payment link, wiring up a batch of beats at once, whatever — just send me the files/details in a message like always. This section is here for whenever you want to do it yourself instead.

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
- Beats: catalog page with a custom preview player (only one beat plays at a time) and a fixed-price "Buy" button per beat linking to that beat's own Stripe Payment Link; shows a "coming soon" panel automatically until `CFG.beats` has at least one entry.
- VSTs: placeholder "coming soon" page for now, matching the Ads/PFPs treatment — build out real content here whenever those exist.
- Reviews: homepage teaser and a dedicated Reviews page both show a prominent average rating banner and the full set of approved reviews; both auto-hide their rating/grid content until `CFG.reviews` has at least one entry. Clients can submit a review via a form on the Reviews page, but nothing publishes automatically — see "Approving a submitted review" above.
- Policies (Terms of Service, Commission Policy, Privacy Policy, Refund & Cancellation Policy) open as modals from the footer — placeholder text, flagged for legal review before publishing. Refund Policy now also covers beat purchases (instant digital downloads, generally non-refundable once sent).

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

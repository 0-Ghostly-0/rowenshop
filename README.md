# rowen.work

Monochrome graphic design studio site for Rowen — cover art, YouTube thumbnails, ads & PFPs. Built as one HTML file that feels like a multi-page site: navigating between Cover Art / Thumbnails / Ads / PFPs / Order swaps the visible page with a transition, and each has its own shareable URL (e.g. `rowen.work/#cover-art`), but it's all a single page load — no separate files or server routing needed.

## Files
- `index.html` — the entire site (HTML/CSS/JS, no build step needed)
- `assets/covers/` — the 26 cover art images shown in the Cover Art portfolio grid
- `assets/brand/` — logo, favicon, and background texture assets
- `favicon.ico`

## Pages
- **Home** (`#home`) — intro + links out to each service
- **Cover Art** (`#cover-art`) — full portfolio grid (26 real pieces)
- **Thumbnails** (`#thumbnails`) — full portfolio grid (31 real pieces)
- **Ads** (`#ads`) — "coming soon" panel until real examples are added
- **PFPs** (`#pfps`) — "coming soon" panel until real examples are added
- **Order** (`#order`) — shared request form; the "What do you need?" field auto-fills based on which service page you arrived from

## Status
- Order form: connected to Formspree (form ID `xojgvwbp`), sending submissions to rowenapichardo@gmail.com. Falls back to opening the visitor's email client if Formspree is ever unreachable.
- Contact: Discord is the featured/preferred channel throughout (nav, hero badge, footer, order page); email and CashApp/Venmo are secondary.
- Opening animation: a short logo intro plays once on every page load, then fades out (respects `prefers-reduced-motion`).
- Portfolio lightbox: clicking any cover art or thumbnail opens a bigger detail view. "Request Something Like This" closes the popup, jumps to the Order page with the right service pre-selected, and auto-attaches that image as a reference file. Picking more files afterward (drag-drop or the file picker) adds to the reference set instead of replacing it.

## Going live
Deploy to Vercel (see chat for the GitHub → Vercel walkthrough) and connect the `rowen.work` domain (already purchased).

## Adding real examples for Thumbnails / Ads / PFPs
Each of those pages currently shows a "coming soon" panel (search `class="coming-soon"` in `index.html`). To swap one out for a real portfolio grid once you have example work, follow the same pattern as the Cover Art page: add the images under `assets/`, then replace the `coming-soon` block with a `<div class="grid" id="grid-...">` and inject the images the same way the Cover Art grid does near the bottom of the `<script>` tag (search `portfolio data (real covers, inlined)`).

## If you ever want to swap the cover art
Open `index.html`, find `const covers = [...]` near the bottom of the `<script>` tag (search "portfolio data"), and replace entries with new image paths (or re-run the same process with a fresh batch of files).

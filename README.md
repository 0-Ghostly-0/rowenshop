# rowen.shop

Single-page monochrome portfolio + order site for Rowen's cover art business.

## Files
- `index.html` — the entire site (HTML/CSS/JS, no build step needed)
- `assets/covers/` — the 26 cover art images shown in the portfolio grid

## Status
- Portfolio: live with real covers.
- Order form: connected to Formspree (form ID `xojgvwbp`), sending submissions to rowenapichardo@gmail.com. Falls back to opening the visitor's email client if Formspree is ever unreachable.
- Contact: Discord is the featured/preferred channel throughout (nav, hero badge, footer, CTA band); email and CashApp/Venmo are secondary.

## Going live
Deploy to Vercel (see chat for the GitHub → Vercel walkthrough) and connect the `rowen.shop` domain once purchased.

## If you ever want to swap the cover art
Open `index.html`, find `const coverSrcs = [...]` near the bottom of the `<script>` tag, and replace entries with new image paths (or re-run the same process with a fresh batch of files).

# rowen.shop

Single-page monochrome portfolio + order site for Rowen's cover art business.

## Files
- `index.html` — the entire site (HTML/CSS/JS, no build step needed)

## Before going live
1. **Swap placeholder art** — the "Selected Work" grid (search for `const covers` in the `<script>` near the bottom of `index.html`) currently uses generated abstract placeholders. Replace with real cover art:
   - Easiest: change each card's art to `background-image:url('yourimage.jpg')` or add an `<img>` inside `.art`.
2. **Connect the order form** — the form currently falls back to opening the visitor's email client. To get real form submissions with attachments:
   1. Go to https://formspree.io and sign up with baytopiasurvival@gmail.com
   2. Create a new form, copy the Form ID
   3. In `index.html`, find `const FORMSPREE_ID = "YOUR_FORM_ID";` near the top of the `<script>` tag and paste your ID in
3. **Deploy to Vercel** and connect the `rowen.shop` domain (once purchased) — see chat for step-by-step instructions.

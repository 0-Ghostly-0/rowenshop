/* =========================================================
   ROWEN — SITE CONFIG
   ---------------------------------------------------------
   This is the ONE file you should need to touch for day-to-day
   updates: prices, turnaround times, FAQ answers, policy text,
   testimonials, social links, etc. Nothing here needs a build
   step — just edit and refresh.

   Anything marked "EDIT ME" is a placeholder value that was
   invented to keep the site looking finished. Replace it with
   your real numbers/policies before publishing.
   ========================================================= */

const SITE_CONFIG = {

  /* ---------- business identity ---------- */
  business: {
    name: "Rowen",
    tagline: "Graphic design for creators who mean it.",
    // Update this once the rowen.work domain is connected in Vercel.
    url: "https://rowen.work",
    email: "rowenapichardo@gmail.com",
    discordInvite: "https://discord.gg/dKVSJuT56u",
    instagram: "https://www.instagram.com/yoorowen/",
    tiktok: "https://www.tiktok.com/@yoorowen",
    cashapp: "https://cash.app/$rowenarts",
    venmo: "https://venmo.com/u/rowenarts",
    // Apple Pay has no shareable link — collected manually over Discord/email.
    // Stripe Payment Link — "Design Commission Payment," customer chooses
    // the amount at checkout (min $25, preset $50, max $1,000), collects a
    // "Project name or Discord username" field so you can match payments to
    // commissions.
    // ⚠ TEST MODE LINK — this was created in your Stripe sandbox, so it only
    // accepts Stripe's test cards (e.g. 4242 4242 4242 4242), not real money.
    // Before launch: switch your Stripe Dashboard to Live mode (top-left
    // toggle), recreate the same payment link there (or ask me to, once the
    // connection points at your live account), and paste the live
    // https://buy.stripe.com/... URL here in place of this test one.
    stripe: "https://buy.stripe.com/test_9B69AU3LN2sy1R2gHR0gw00",
    minBudget: 25
  },

  /* ---------- Formspree (order form backend) ----------
     Submissions currently post to https://formspree.io/f/xojgvwbp,
     which forwards to rowenapichardo@gmail.com. To change where
     submissions go, update the form at https://formspree.io/forms
     — no code changes needed here unless you swap to a new form. */
  formspreeId: "xojgvwbp",

  /* ---------- budget ranges (order form) ----------
     Shown as a required dropdown on the commission form instead of a
     free-text field. Every project has a hard $25 minimum (see
     business.minBudget above) — final pricing is always a custom quote,
     never a fixed rate, based on scope/complexity/deadline/revisions/use. */
  budgetRanges: ["$25–$49", "$50–$99", "$100–$199", "$200+", "Not sure yet"],

  /* ---------- what affects a quote ----------
     Rendered under the pricing section so visitors understand why
     there's no fixed rate card. Edit freely — keep it short. */
  pricingFactors: [
    "Project complexity & number of concepts",
    "Turnaround time you need",
    "Number of revisions requested",
    "Personal vs. commercial use",
    "Whether you need source/layered files"
  ],

  /* ---------- services ----------
     No fixed prices here on purpose — every project gets a custom quote
     based on the factors in "pricingFactors" above. "turnaround" /
     "revisions" / "formats" below are still EDIT ME placeholders for the
     baseline included with each service; swap in your real numbers. */
  services: [
    {
      key: "cover-art",
      name: "Cover Art",
      shortName: "Cover Art",
      blurb: "Albums, EPs, singles & visualizers — made from scratch.",
      turnaround: "3–7 days",       // EDIT ME
      revisions: "2 rounds included", // EDIT ME
      formats: "PNG + JPG, layered file on request", // EDIT ME
      commercialUse: "Included",     // EDIT ME
      sourceFiles: "Available on request (may include a fee)", // EDIT ME
      includes: [
        "Made from scratch to fit your genre/vibe",
        "2 rounds of revisions included",
        "Delivered print & streaming ready",
        "Commercial use included"
      ]
    },
    {
      key: "thumbnails",
      name: "YouTube Thumbnails",
      shortName: "Thumbnails",
      blurb: "High-contrast thumbnails built to get the click.",
      turnaround: "1–3 days", // EDIT ME
      revisions: "2 rounds included", // EDIT ME
      formats: "PNG/JPG at 1280×720", // EDIT ME
      commercialUse: "Included",
      sourceFiles: "Available on request (may include a fee)",
      includes: [
        "High-contrast, built to get the click",
        "2 rounds of revisions included",
        "Delivered at YouTube's exact spec",
        "Fast turnaround for time-sensitive uploads"
      ]
    },
    {
      key: "ads",
      name: "Advertisement Design",
      shortName: "Ads",
      blurb: "Scroll-stopping static & social ad creative.",
      turnaround: "3–5 days", // EDIT ME
      revisions: "2 rounds included", // EDIT ME
      formats: "PNG/JPG, sized to your platform(s)", // EDIT ME
      commercialUse: "Included",
      sourceFiles: "Available on request (may include a fee)",
      includes: [
        "Built for the platform you're running it on",
        "2 rounds of revisions included",
        "Multiple sizes available on request",
        "Commercial use included"
      ]
    },
    {
      key: "pfps",
      name: "Profile Pictures",
      shortName: "PFPs",
      blurb: "Custom profile pictures with actual personality.",
      turnaround: "1–3 days", // EDIT ME
      revisions: "1 round included", // EDIT ME
      formats: "PNG, square crop", // EDIT ME
      commercialUse: "Included",
      sourceFiles: "Available on request (may include a fee)",
      includes: [
        "Custom, made from scratch — not a filter",
        "1 round of revisions included",
        "Delivered square-cropped and ready to upload",
        "Commercial use included"
      ]
    }
  ],

  /* ---------- how it works (5-step deposit flow) ----------
     Keep this in sync with any payment copy elsewhere on the site —
     the site should never claim payment is optional until completion. */
  howItWorks: [
    {
      title: "Submit a brief",
      body: "Tell me what you want, drop reference links or files, or just say “creative freedom.”"
    },
    {
      title: "Get a quote & date",
      body: "I reply with a quote and an estimated completion date based on scope and your budget."
    },
    {
      title: "Pay the deposit",
      body: "A deposit is required to book the project and get it into the queue."
    },
    {
      title: "Review a watermarked preview",
      body: "You'll see a watermarked preview before final files are delivered, so you can request changes first."
    },
    {
      title: "Get your final files",
      body: "Once you approve the preview and the remaining balance is paid, final files are delivered ready to use."
    }
  ],

  /* ---------- FAQ ---------- */
  faq: [
    {
      q: "How does payment work?",
      a: "A deposit is required to book your project once you accept a quote. The remaining balance is due after you approve the watermarked preview and before final (unwatermarked) files are sent. This protects both of us — you see the direction before paying in full, and the deposit secures your spot in the queue."
    },
    {
      q: "How many revisions do I get?",
      a: "Every service includes at least one round of revisions (most include two — see the pricing section for specifics). Additional rounds beyond that can usually be added for a small fee, just ask."
    },
    {
      q: "How long will my project take?",
      a: "Turnaround varies by service and current queue — typical ranges are listed in the pricing section. Rush orders may be available for an added fee; ask on Discord before booking if your deadline is tight."
    },
    {
      q: "What's your refund & cancellation policy?",
      a: "If work hasn't started yet, deposits are refundable minus any processing fees. Once work has started, the deposit becomes non-refundable since it covers time already spent, but I'll always work with you to resolve an issue before it gets to that point. See the full Refund & Cancellation Policy for details."
    },
    {
      q: "Can I use the final files commercially?",
      a: "Yes — commercial use is included with every service by default unless stated otherwise in your quote. If you need an extended license or something unusual (resale, franchising, etc.), mention it in your brief so it can be quoted accordingly. One ask either way: if you make a post that's focused specifically on cover art I made for you (a reveal post, a print, a portfolio feature, etc.), please credit me as the designer — tagging @yoorowen or linking rowen.work is enough. Normal use of the art as your actual release cover across streaming platforms and social doesn't need a credit every time."
    },
    {
      q: "Do I get the source/layered files?",
      a: "Final exports (PNG/JPG etc.) are always included. Layered source files (e.g. PSD) are available on request and may include an additional fee — mention it in your brief if you think you'll need them."
    },
    {
      q: "Can I pay extra for a rush order?",
      a: "Often, yes — depending on the current queue. Ask on Discord with your deadline before submitting the form so it can be confirmed as possible before you book."
    },
    {
      q: "How are files delivered?",
      a: "Final files are delivered digitally — usually via Discord DM or email, whichever you prefer. Let me know your preferred contact method in the order form."
    }
  ],

  /* ---------- portfolio ----------
     Categories drive both the nav/filter tabs and how each grid is
     built. "count" images are auto-generated from "path" + a
     zero-padded number (e.g. assets/covers/cover-01.webp) — that
     matches how the images are already named, so nothing needs to
     be renamed. Categories with count:0 show the "coming soon"
     panel instead of a grid.

     "overrides" lets you attach real per-project detail (type,
     client/personal label, tools used, a short description, or a
     before/after comparison) to any specific piece without having
     to hand-write all 58 entries. Key format is "<category>-<NN>",
     e.g. "cover-art-01". Anything you don't fill in is simply
     hidden in the project detail view — nothing fake is shown. */
  portfolio: {
    categories: [
      { key: "cover-art", label: "Cover Art", shortLabel: "Cover Art", count: 26, path: "assets/covers/cover-", wide: false, comingSoon: false },
      { key: "thumbnails", label: "YouTube Thumbnails", shortLabel: "Thumbnails", count: 32, path: "assets/thumbnails/thumb-", wide: true, comingSoon: false },
      { key: "ads", label: "Advertisements", shortLabel: "Ads", count: 0, path: "assets/ads/ad-", wide: false, comingSoon: true },
      { key: "pfps", label: "Profile Pictures", shortLabel: "PFPs", count: 0, path: "assets/pfps/pfp-", wide: false, comingSoon: true }
    ],
    overrides: {
      // 'cover-art-01': {
      //   type: 'Album Cover',
      //   client: 'Personal project',      // or a real client/artist name, with permission
      //   tools: 'Photoshop, Illustrator',
      //   description: 'Short 1–2 sentence description of the brief.',
      //   before: 'assets/covers/cover-01-before.webp' // optional before/after
      // }
    }
  },

  /* ---------- testimonials ----------
     Leave this empty until you have real client testimonials —
     the section hides itself automatically when this array is empty.
     To add one: { quote, name, role (optional), avatar (optional
     image path). Never invent testimonials that didn't happen. */
  testimonials: [
    // { quote: "…", name: "…", role: "…", avatar: "assets/testimonials/example.webp" }
  ],

  /* ---------- policies ----------
     Placeholder legal-style text so the site doesn't feel unfinished.
     Everything here is generic boilerplate — have it reviewed
     (ideally by a lawyer familiar with your local laws) before you
     rely on it. Anything that needs your personal decision is
     flagged inline. */
  policies: {
    tos: {
      title: "Terms of Service",
      body: [
        { h: "Overview", p: "These Terms of Service (“Terms”) govern any commission or purchase made through rowen.work (“the Site”), operated by Rowen (“I”, “me”). By submitting a project request or making a payment, you agree to these Terms." },
        { h: "Commission scope", p: "Each project is scoped individually based on the brief you submit and the quote I send back. Work outside the agreed scope (extra concepts, unrelated revisions, additional deliverables) may be quoted separately." },
        { h: "Ownership & licensing", p: "Unless otherwise agreed in writing, you receive a license to use final delivered files for the purpose described in your brief, including commercial use. I retain the right to display finished work in my portfolio unless you request otherwise in your brief." },
        { h: "Credit for cover art", p: "Using a cover art piece as your actual album/single/EP cover across streaming platforms and social media doesn't require a credit each time. But if you make a post that's focused specifically on the cover art itself — a reveal post, a print, a portfolio or fan-art feature, etc. — please credit me as the designer, either by tagging/mentioning @yoorowen or linking rowen.work." },
        { h: "Client responsibilities", p: "You're responsible for making sure any reference material, logos, or text you provide doesn't infringe on someone else's rights. I reserve the right to decline projects that involve hate speech, harassment, or clearly illegal content." },
        { h: "Changes to these terms", p: "These Terms may be updated from time to time; the version in effect at the time you submit a request is the one that applies to that project." }
      ]
    },
    commission: {
      title: "Commission Policy",
      body: [
        { h: "Process", p: "Every commission follows the same flow: brief → quote & estimated date → deposit → watermarked preview → final files after approval and final payment. See the “How It Works” section on the home page for details." },
        { h: "Deposits", p: "A deposit is required before work begins. It secures your spot in the queue and goes toward the total cost of your project." },
        { h: "Revisions", p: "Each service includes a set number of revision rounds (listed in the pricing section). Revisions must relate to the original brief — new concepts requested after work has started may be quoted as a separate round." },
        { h: "Timelines", p: "Turnaround estimates are based on the current queue at the time of your quote and are not guaranteed delivery dates. I'll flag it as early as possible if something is going to run long." },
        { h: "Credit for cover art", p: "Using a cover art piece as your actual release cover across streaming platforms and social media doesn't need a credit every time. But if you make a post focused specifically on the cover art itself — a reveal post, a print, a portfolio feature, etc. — please credit me as the designer (tag/mention @yoorowen or link rowen.work)." },
        { h: "What I won't take on", p: "I reserve the right to decline any commission, including (but not limited to) hate symbols, explicit/NSFW content, plagiarized concepts, or anything that infringes on someone else's copyright or trademark." }
      ]
    },
    privacy: {
      title: "Privacy Policy",
      body: [
        { h: "What I collect", p: "When you submit the order form, I receive whatever you enter: your name, email, Discord username, project details, budget, deadline, reference links, and any files you upload. This is sent via Formspree (see below) directly to my email inbox." },
        { h: "How reference files are used", p: "Reference images or files you upload are used only to inform your commission. They are not shared publicly, sold, or reused for other clients' projects without your permission." },
        { h: "Third-party processors", p: "The order form is processed by Formspree (formspree.io), a third-party form backend. Their privacy policy governs how form submissions are transmitted and briefly stored on their end before forwarding to my inbox. No payment information is ever collected through the form or stored on this site — payment happens directly through CashApp, Venmo, Apple Pay, or card via Stripe, each governed by that provider's own privacy policy." },
        { h: "Data retention", p: "I keep project-related emails and files for as long as reasonably needed for the commission and basic recordkeeping, then delete them periodically. You can request deletion of your information at any time by emailing me." },
        { h: "Contact", p: "Questions about this policy can be sent to rowenapichardo@gmail.com." }
      ]
    },
    refund: {
      title: "Refund & Cancellation Policy",
      body: [
        { h: "Before work starts", p: "If you cancel before any work has begun, your deposit is refunded minus any non-recoverable processing fees (e.g. payment platform fees)." },
        { h: "After work starts", p: "Once work has started, the deposit becomes non-refundable, since it covers time already spent on your project. If you're unhappy with the direction, tell me — revisions within scope are included, and I'd rather fix it than keep a refund." },
        { h: "After final delivery", p: "Refunds are not offered after final (unwatermarked) files have been delivered and the remaining balance paid, except in cases where delivered work clearly does not match the agreed brief." },
        { h: "Cancellations by me", p: "If I need to cancel a project (illness, no longer able to take it on, etc.) before delivering final files, you'll receive a full refund of any amount paid." }
      ]
    }
  }
};

// Explicitly attach to window — top-level `const` in a classic <script> does
// NOT become a window property on its own, and main.js reads window.SITE_CONFIG.
window.SITE_CONFIG = SITE_CONFIG;

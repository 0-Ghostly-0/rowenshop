/* =========================================================
   ROWEN — SITE CONFIG
   ---------------------------------------------------------
   This is the ONE file you should need to touch for day-to-day
   updates: prices, turnaround times, FAQ answers, policy text,
   reviews, social links, etc. Nothing here needs a build
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
      a: "A deposit is required to book your project once you accept a quote. The remaining balance is due after you approve the watermarked preview and before final (unwatermarked) files are sent. This protects both of us — you see the direction before paying in full, and the deposit secures your spot in the queue. If you'd rather not wait for a quote, you're also welcome to pay in full or a deposit upfront using any of the payment options on the order page — just note your project name or Discord username so I can match it to your request."
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

  /* ---------- beats ----------
     Leave this empty until you have real beats to sell — the Beats
     page shows a friendly "coming soon" panel automatically when
     this array is empty, and switches to the real catalog the moment
     you add an entry. See the README section "Adding a new beat" for
     the full walkthrough (uploading the audio file + creating its
     Stripe payment link). Each entry:
       title       - beat name, shown on the card
       bpm         - optional, shown as a small tag (e.g. 140)
       key         - optional, shown as a small tag (e.g. "F# Minor")
       tags        - optional array of extra small tags (e.g. ["Trap","Dark"])
       price       - number, shown as "$12" — standing default: all
                     beats are listed at $12 unless you decide
                     otherwise for a specific one
       audio       - path to the preview/full audio file, e.g.
                     "assets/beats/midnight-drive.mp3"
       cover       - optional square artwork path; a default icon is
                     shown if you leave this out
       stripeLink  - fixed-price Stripe Payment Link for this exact
                     beat (NOT the customer-chooses-price one used for
                     deposits) — leave blank and the button becomes
                     "Ask on Discord" instead of "Buy" until it's set
       producer    - optional. Leave this out entirely for beats you
                     made yourself — nothing changes for those. For a
                     beat you're hosting on behalf of another producer,
                     set this to { name, link } (link is optional — their
                     Instagram/Discord/website) and the card credits them
                     as "Produced by ___" under the title. Their cut is
                     handled by Stripe when you create THEIR stripeLink
                     above (a Connect payment link that splits the sale
                     with their connected account) — see the README
                     section "Hosting other producers' beats" for the
                     full walkthrough. */
  beats: [
    // {
    //   title: "Midnight Drive",
    //   bpm: 140,
    //   key: "F# Minor",
    //   tags: ["Trap", "Dark"],
    //   price: 12,
    //   audio: "assets/beats/midnight-drive.mp3",
    //   cover: "assets/beats/midnight-drive-cover.jpg",
    //   stripeLink: "https://buy.stripe.com/xxxxxxxx",
    //   producer: { name: "Jordan", link: "https://instagram.com/jordanbeats" }
    // }
  ],

  /* ---------- VSTs (plugin store) ----------
     Leave this empty until you have a real plugin ready to sell — the
     VSTs page shows the "in development" teaser automatically while this
     array is empty, and switches to a real product catalog (with a cart
     + checkout) the moment you add an entry. See the README section
     "Selling a finished VST" for the full walkthrough, including the
     one-time setup (Vercel environment variables, creating the Stripe
     Price) you'll need to do first. Each entry:
       key         - short unique id, e.g. "vocal-polish" — used to match
                     this product to its Stripe Price server-side. Once
                     you've listed a product, don't change its key.
       name        - product name, shown as the card/detail title
       tagline     - one short line shown under the name
       description - a paragraph or two for the product detail view
       features    - array of short strings, shown as a feature list
       images      - array of screenshot/UI paths for the gallery (first
                     one is the card's cover image) — this is what makes
                     it "look like something," so use real screenshots or
                     renders, not stock art
       price       - number, shown as "$49" etc.
       priceId     - the Stripe Price ID (starts "price_...") for this
                     product's one-time purchase — created in the Stripe
                     Dashboard or ask me to create it once you're ready.
                     This is NOT secret, it's fine to expose in this file.
       badge       - optional small tag, e.g. "New" or "Best seller"
     Where does the actual installer file live? That's set up
     server-side (in api/_lib/catalog.js), matched to this same "key" —
     never in this file, since anything in config.js is publicly
     readable in the browser. See the README for exactly how to wire
     a new product's download up once it's ready. */
  vsts: [
    // {
    //   key: "vocal-polish",
    //   name: "Rowen Vocal Polish",
    //   tagline: "A vocal finishing plugin for artists who record at home and don't mix.",
    //   description: "Insert it on a vocal, turn up Polish, and it comes out cleaner, more controlled, brighter, and more finished. Five simple controls, ten factory presets, built for people who want a great-sounding vocal without learning to mix.",
    //   features: [
    //     "One main knob — Polish — plus Clarity, Smooth, Space & Width",
    //     "10 factory presets, from a light touch-up to the full treatment",
    //     "VST3, Windows — AU planned for a later phase"
    //   ],
    //   images: [
    //     "assets/vsts/vocal-polish/screenshot-1.webp",
    //     "assets/vsts/vocal-polish/screenshot-2.webp"
    //   ],
    //   price: 49,
    //   priceId: "price_xxxxxxxxxxxxx",
    //   badge: "New"
    // }
    {
      key: "fragment",
      name: "Rowen Fragment",
      tagline: "Turn any melody, vocal, or loop into something you've never heard before.",
      description: "Rowen Fragment takes whatever's playing — a melody, a vocal, a drum loop, a full sample — and continuously breaks it into small moving pieces you can reshape in real time. Six simple controls handle all the heavy lifting, so you get inspiring, unique textures fast without digging through a wall of technical settings.",
      features: [
        "Six creative controls — Fragment, Shift, Reverse, Scatter, Drift & Atmosphere",
        "Freeze any moment and keep transforming it from there",
        "One-click Randomize for instant new variations, with Undo",
        "30+ factory presets across Subtle, Melodic, Rhythmic, Vocal, Drum & Experimental categories",
        "VST3, Windows — AU planned for a later phase"
      ],
      images: [
        "assets/vsts/fragment/screenshot-1.webp",
        "assets/vsts/fragment/screenshot-2.webp"
      ],
      price: 25,
      priceId: "price_1TwkKsL5jHly4mj0dG4BHFxu",
      badge: "New"
    }
  ],

  /* ---------- reviews ----------
     Leave this empty until you have real reviews — the "What clients
     say" section on the homepage and the whole Reviews page hide/show
     automatically based on whether this array is empty. Customers can
     submit a review from the Reviews page (rating + quote + name),
     but nothing publishes automatically: submissions land in your
     inbox the same way commission requests do, and you add the ones
     you approve here yourself. See the README "Approving a submitted
     review" section for the full walkthrough. Never invent one that
     didn't happen.
       quote   - the review text
       name    - reviewer's name
       role    - optional, e.g. "Cover Art client" or a channel name
       rating  - required, a whole number 1–5
       avatar  - optional image path; falls back to an initial-letter
                 avatar if omitted */
  reviews: [
    // { quote: "…", name: "…", role: "…", rating: 5, avatar: "assets/reviews/example.webp" }
  ],

  /* ---------- review submissions (Reviews page form) ----------
     Leave blank to send review submissions to the SAME Formspree form
     as commission requests (formspreeId above) — they'll still be
     clearly labeled "New Review Submission" in your inbox so you can
     tell them apart. Set this to a second Formspree form's ID instead
     if you'd rather keep reviews in a completely separate inbox view. */
  reviewsFormspreeId: "",

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
        { h: "Eligibility", p: "By submitting a project request or making a payment, you confirm that you're legally able to enter into this agreement, or that you have permission from a parent or legal guardian to do so." },
        { h: "Commission scope", p: "Each project is scoped individually based on the brief you submit and the quote I send back. Work outside the agreed scope (extra concepts, unrelated revisions, additional deliverables) may be quoted separately." },
        { h: "Ownership & licensing", p: "Upon full payment, you receive an exclusive, worldwide license to use the final delivered files for the purpose described in your brief, including commercial use, unless otherwise agreed in writing. Beat purchases are licensed differently — see the Refund & Cancellation Policy. Some beats are made by other producers and hosted here with their permission; for those, I grant that license on the producer's behalf, and I'm still your point of contact for it. I retain the right to display finished work in my portfolio unless you request otherwise in your brief. (See the Commission Policy for cover art crediting guidelines.)" },
        { h: "Drafts & previews", p: "Preliminary concepts, drafts, sketches, and watermarked previews remain my intellectual property until full payment has been received. They aren't licensed for use, publication, or distribution until the final files have been delivered and paid for in full." },
        { h: "Client responsibilities", p: "You're responsible for making sure any reference material, logos, or text you provide doesn't infringe on someone else's rights. I reserve the right to decline projects that involve hate speech, harassment, or clearly illegal content." },
        { h: "Delays outside my control", p: "I'm not responsible for delays caused by circumstances outside my reasonable control — illness, internet or power outages, platform outages (Discord, Stripe, Formspree, etc.), natural disasters, and similar events. I'll let you know as soon as possible if something like this affects your timeline." },
        { h: "Governing law", p: "These Terms are governed by the laws of the Commonwealth of Pennsylvania, United States, and any disputes arising from them will be handled under that law." },
        { h: "Changes to these terms", p: "These Terms may be updated from time to time; the version in effect at the time you submit a request is the one that applies to that project." }
      ]
    },
    commission: {
      title: "Commission Policy",
      body: [
        { h: "Process", p: "Every commission follows the same flow: brief → quote & estimated date → deposit → watermarked preview → final files after approval and final payment. See the “How It Works” section on the home page for details." },
        { h: "Deposits", p: "A deposit is required before work begins. It secures your spot in the queue and goes toward the total cost of your project." },
        { h: "Final payment", p: "The remaining balance must be paid in full before final, unwatermarked files are delivered. Watermarked previews are provided for review in the meantime, but final files are only sent once payment is complete." },
        { h: "Revisions", p: "Each service includes a set number of revision rounds (listed in the pricing section). Revisions must relate to the original brief — new concepts requested after work has started may be quoted as a separate round." },
        { h: "Timelines", p: "Turnaround estimates are based on the current queue at the time of your quote and are not guaranteed delivery dates. I'll flag it as early as possible if something is going to run long, including for delays outside my control (see the Terms of Service)." },
        { h: "Credit for cover art", p: "Using a cover art piece as your actual album/single/EP cover across streaming platforms and social media doesn't require a credit each time. But if you make a post that's focused specifically on the cover art itself — a reveal post, a print, a portfolio or fan-art feature, etc. — please credit me as the designer, either by tagging/mentioning @yoorowen or linking rowen.work." },
        { h: "Beats from other producers", p: "Some beats in the catalog are made by other producers and hosted here with their permission, splitting each sale with them automatically. That doesn't change anything about buying one — I'm still the seller of record and your point of contact for any question, issue, or refund request, exactly like a beat I made myself. Any beat credited to another producer says so on its card." },
        { h: "What I won't take on", p: "I reserve the right to decline any commission, including (but not limited to) hate symbols, explicit/NSFW content, plagiarized concepts, or anything that infringes on someone else's copyright or trademark." }
      ]
    },
    privacy: {
      title: "Privacy Policy",
      body: [
        { h: "What I collect", p: "When you submit the order form, I receive whatever you enter: your name, email, Discord username, project details, budget, deadline, reference links, and any files you upload. If you submit a review, I receive your rating, review text, name, email, and optional project/channel name. Everything is sent via Formspree (see below) directly to my email inbox." },
        { h: "How reference files are used", p: "Reference images or files you upload are used only to inform your commission. They are not shared publicly, sold, or reused for other clients' projects without your permission." },
        { h: "Review submissions", p: "Reviews are moderated — submitting one doesn't publish it automatically. If I approve your review, your rating, review text, name, and optional project/channel name may be published on the site. Your email is never published; it's only used if I need to follow up with you." },
        { h: "Third-party processors", p: "The order form and review submissions are both processed by Formspree (formspree.io), a third-party form backend. Their privacy policy governs how submissions are transmitted and briefly stored on their end before forwarding to my inbox." },
        { h: "Payment processing", p: "Payments are made directly through Stripe, CashApp, Venmo, or Apple Pay — I never receive or store your card or bank details on this site. Stripe payments happen on Stripe's own secure checkout page, not on rowen.work; CashApp, Venmo, and Apple Pay payments happen directly in those apps. Each provider is governed by its own privacy policy." },
        { h: "Producer payouts", p: "For beats hosted here on behalf of another producer, Stripe automatically splits your payment between me and that producer. The producer completed their own identity and bank verification directly with Stripe to receive their share — I never see, collect, or store that information myself." },
        { h: "Cookies & third-party content", p: "rowen.work doesn't use cookies, analytics, or advertising trackers. The site loads fonts from Google Fonts, which may receive standard technical information (like your IP address) as part of serving those files — this is normal for any font hosted this way and isn't used to track you. If you follow a link to Discord, Instagram, TikTok, or a payment provider, that site's own cookies and privacy policy apply once you're there." },
        { h: "Data retention", p: "I keep project-related emails and files for up to 24 months after your project is completed, then delete them. You can request earlier deletion of your information at any time by emailing me." },
        { h: "Contact", p: "Questions about this policy can be sent to rowenapichardo@gmail.com." }
      ]
    },
    refund: {
      title: "Refund & Cancellation Policy",
      body: [
        { h: "Before work starts", p: "If you cancel before any work has begun, your deposit is refunded minus any non-recoverable processing fees (e.g. payment platform fees)." },
        { h: "After work starts", p: "Once work has started, the deposit becomes non-refundable, since it covers time already spent on your project. If you're unhappy with the direction, tell me — revisions within scope are included, and I'd rather fix it than keep a refund." },
        { h: "After final delivery", p: "Refunds are not offered after final (unwatermarked) files have been delivered and the remaining balance paid, except in cases where delivered work clearly does not match the agreed brief." },
        { h: "Cancellations by me", p: "If I need to cancel a project (illness, no longer able to take it on, etc.) before delivering final files, you'll receive a full refund of any amount paid." },
        { h: "Beats", p: "Beat purchases are instant digital downloads and are generally non-refundable once the files have been sent, except in cases of a clear technical issue (e.g. a corrupted or wrong file). Each beat is licensed for your use as described at checkout — it isn't sold exclusively unless we've separately agreed to that in writing. This applies the same way whether I made the beat myself or I'm hosting it on behalf of another producer — either way, refund requests go through me, not the producer." }
      ]
    }
  }
};

// Explicitly attach to window — top-level `const` in a classic <script> does
// NOT become a window property on its own, and main.js reads window.SITE_CONFIG.
window.SITE_CONFIG = SITE_CONFIG;

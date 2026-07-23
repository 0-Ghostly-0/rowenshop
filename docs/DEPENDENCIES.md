# Dependency & commercial licensing report

Identical footprint to Rowen Vocal Polish — no new dependencies were added for
Fragment, and the entire granular engine is original code.

| Name | Purpose | License | Commercial use | Attribution | Source |
|---|---|---|---|---|---|
| JUCE 8 (pinned 8.0.4) | Plugin framework, VST3 wrapper, GUI | Dual: AGPLv3 or JUCE commercial | ⚠ One commercial JUCE license covers both Rowen plugins — resolve before selling (juce.com/get-juce) | Splash screen until licensed (`JUCE_DISPLAY_SPLASH_SCREEN=1` set honestly) | github.com/juce-framework/JUCE |
| VST3 SDK (via JUCE) | Plugin format | Steinberg VST3 agreement (or GPLv3) | ✅ free with Steinberg terms; "VST" trademark notice required at release | Yes, at release | steinbergmedia.github.io/vst3_dev_portal |
| Space Grotesk / Poppins / JetBrains Mono | UI typography (Phase 6, shared with Vocal Polish) | SIL OFL 1.1 | ✅ | Ship OFL texts (already in the Vocal Polish assets) | Google Fonts / rsms.me / jetbrains.com/mono |
| pluginval | Validation, dev-only, never shipped | GPLv3 | ✅ (not distributed) | n/a | github.com/Tracktion/pluginval |
| Inno Setup 6 | Windows installer | Inno Setup License (free incl. commercial) | ✅ | n/a | jrsoftware.org |
| Rowen brand assets | Logo, texture (Phase 6) | Rowen's own property | ✅ | n/a | Rowen |

Original-work statement: the rolling buffer, seqlock visualization exchange,
grain voices/schedulers (Phase 3), modulators (Phase 4) and all interface code
are written from scratch for Rowen. The only algorithmic borrowing planned is
the public-domain Freeverb *topology* inside Atmosphere (Phase 4), implemented
in our own code — the same approach already shipped in Vocal Polish. Nothing
is derived from Autochroma or any other plugin's code, presets, graphics, or
branding.

Policy: any new dependency must be added to this table with its license
verified for closed-source commercial distribution before it is merged.

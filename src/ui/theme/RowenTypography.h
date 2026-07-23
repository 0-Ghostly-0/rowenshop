// Rowen typography — the site's font stack: Space Grotesk for headings,
// Inter for body text, JetBrains Mono for values/numbers.
//
// Font files are embedded from assets/fonts/ if present at build time (see
// CMakeLists). All three families are SIL OFL licensed — free for commercial
// embedding (docs/DEPENDENCIES.md). If a font file is missing the system
// sans-serif is used, so the build never breaks over a font.
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace rowen::theme
{

class Typography
{
public:
    static Typography& get()
    {
        static Typography instance;
        return instance;
    }

    // Space Grotesk, weight 500, letter-spacing -0.02em (site h1..h4 style).
    juce::Font heading (float px) const
    {
        auto f = fontFor (headingTypeface, px);
        return f.withExtraKerningFactor (-0.02f);
    }

    juce::Font body (float px) const   { return fontFor (bodyTypeface, px); }
    juce::Font mono (float px) const   { return fontFor (monoTypeface, px); }

private:
    Typography();

    juce::Font fontFor (const juce::Typeface::Ptr& tf, float px) const
    {
        if (tf != nullptr)
            return juce::Font (juce::FontOptions (tf).withHeight (px));
        return juce::Font (juce::FontOptions().withHeight (px));
    }

    juce::Typeface::Ptr headingTypeface, bodyTypeface, monoTypeface;
};

} // namespace rowen::theme

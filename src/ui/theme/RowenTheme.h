// Rowen design system — shared by ALL Rowen plugins, not just Vocal Polish.
// Every value here is taken directly from the rowenshop site stylesheet
// (assets/css/styles.css) so the plugin family and the website stay one brand.
#pragma once

#include <juce_graphics/juce_graphics.h>

namespace rowen::theme
{

// --- Colors (site :root custom properties) -----------------------------------
namespace color
{
    inline const juce::Colour black      { 0xff08080a }; // --black   (window bg)
    inline const juce::Colour black2     { 0xff0e0e11 }; // --black-2 (recessed areas)
    inline const juce::Colour card       { 0xff111114 }; // --card    (panels)
    inline const juce::Colour card2      { 0xff16161a }; // --card-2  (raised panels)
    inline const juce::Colour line       { juce::Colours::white.withAlpha (0.09f) }; // --line
    inline const juce::Colour lineStrong { juce::Colours::white.withAlpha (0.18f) }; // --line-strong
    inline const juce::Colour white      { 0xfff4f4f2 }; // --white   (primary text)
    inline const juce::Colour dim        { 0xff9a9aa0 }; // --dim     (secondary text)
    inline const juce::Colour dimmer     { 0xff7a7a82 }; // --dimmer  (tertiary text)
    inline const juce::Colour accent     { 0xffff2b83 }; // --accent  (Rowen magenta)
    inline const juce::Colour accentDim  { juce::Colour (0xffff2b83).withAlpha (0.35f) }; // --accent-dim
    inline const juce::Colour good       { 0xff3ddc84 }; // --good
    inline const juce::Colour bad        { 0xffff6161 }; // --bad
}

// --- Geometry ----------------------------------------------------------------
namespace metric
{
    constexpr float radius       = 14.0f;  // --radius (cards, panels)
    constexpr float radiusSmall  = 8.0f;   // inner elements
    constexpr float pillRadius   = 100.0f; // site buttons are pills
    constexpr float hairline     = 1.0f;   // border width everywhere
    constexpr int   spacingUnit  = 4;      // 4-based spacing grid (site uses 4/8/12/16/24/32)
}

// --- Motion (site --ease: cubic-bezier(.22,1,.36,1), fast-out settle) ---------
namespace motion
{
    constexpr int hoverMs = 250;  // matches the site's .25s hover transitions
    constexpr int panelMs = 350;

    // The site's cubic-bezier(.22,1,.36,1) evaluated for component animation.
    inline float ease (float t) noexcept
    {
        // Close approximation of the site curve: fast start, soft settle.
        const float u = 1.0f - t;
        return 1.0f - u * u * (1.0f - 0.36f * t);
    }
}

// --- Reusable glow (site: box-shadow: 0 0 30px 4px var(--accent-dim)) --------
inline void drawAccentGlow (juce::Graphics& g, juce::Rectangle<float> around, float intensity01)
{
    if (intensity01 <= 0.01f)
        return;
    const float spread = 10.0f * intensity01;
    for (int i = 3; i >= 1; --i)
    {
        const float a = 0.10f * intensity01 * float (i) / 3.0f;
        g.setColour (color::accent.withAlpha (a));
        g.drawEllipse (around.expanded (spread * float (4 - i) / 3.0f), spread * 0.8f);
    }
}

} // namespace rowen::theme

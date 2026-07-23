// Rowen toggle dot — the small section enable/bypass control. A lit magenta
// dot (with the site's accent glow) when on; a hollow hairline dot when off.
// Deliberately tiny and quiet so section bypasses never clutter the interface.
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../theme/RowenTheme.h"

namespace rowen::ui
{

class RowenToggleDot : public juce::Button
{
public:
    explicit RowenToggleDot (const juce::String& name) : juce::Button (name)
    {
        setClickingTogglesState (true); // required for ButtonAttachment behavior
        setMouseCursor (juce::MouseCursor::PointingHandCursor);
    }

    void paintButton (juce::Graphics& g, bool highlighted, bool) override
    {
        namespace c = rowen::theme::color;
        const auto b = getLocalBounds().toFloat();
        const float d = juce::jmin (b.getWidth(), b.getHeight()) * 0.5f;
        const auto dot = b.withSizeKeepingCentre (d, d);

        if (getToggleState())
        {
            rowen::theme::drawAccentGlow (g, dot, highlighted ? 1.0f : 0.7f);
            g.setColour (c::accent);
            g.fillEllipse (dot);
        }
        else
        {
            g.setColour (highlighted ? juce::Colours::white.withAlpha (0.35f) : c::lineStrong);
            g.drawEllipse (dot, 1.0f);
        }
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RowenToggleDot)
};

} // namespace rowen::ui

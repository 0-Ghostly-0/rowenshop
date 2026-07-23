// Rowen help panel — the in-plugin quick guide. Short, plain language, no
// producer jargon, no external website required.
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "RowenIconButton.h"
#include "../theme/RowenTheme.h"
#include "../theme/RowenTypography.h"

namespace rowen::ui
{

class HelpPanel : public juce::Component
{
public:
    HelpPanel()
    {
        setVisible (false);
        setAlwaysOnTop (true);
        closeButton.onClick = [this] { setVisible (false); };
        addAndMakeVisible (closeButton);
    }

    void paint (juce::Graphics& g) override
    {
        namespace c = rowen::theme::color;
        auto& type = rowen::theme::Typography::get();

        g.fillAll (juce::Colours::black.withAlpha (0.60f));

        g.setColour (c::card2);
        g.fillRoundedRectangle (panel, rowen::theme::metric::radius);
        g.setColour (c::lineStrong);
        g.drawRoundedRectangle (panel, rowen::theme::metric::radius, 1.0f);

        const float s = scale();
        auto inner = panel.getSmallestIntegerContainer().reduced (int (22 * s));
        inner.removeFromTop (int (4 * s));

        g.setColour (c::white);
        g.setFont (type.heading (19.0f * s));
        g.drawText ("How to use Fragment", inner.removeFromTop (int (28 * s)),
                    juce::Justification::centredLeft);
        inner.removeFromTop (int (10 * s));

        struct Row { const char* head; const char* text; };
        static constexpr Row rows[] = {
            { "FRAGMENT",   "How much of the sound gets broken into pieces. Low keeps it mostly intact; high turns it into a cloud of fragments." },
            { "SHIFT",      "Pitches the fragments up or down. Try big jumps for something unrecognizable, or small ones for texture." },
            { "REVERSE",    "How often fragments play backwards instead of forwards." },
            { "SCATTER",    "How far fragments reach into the recent past instead of staying near the newest audio." },
            { "DRIFT",      "Slow, wandering movement in pitch, position and pan — makes it feel alive instead of static." },
            { "ATMOSPHERE", "Adds a diffuse, spacious trail behind the fragments." },
            { "FREEZE",     "Locks the buffer so it stops capturing new audio and only replays what's already inside it." },
            { "RANDOMIZE",  "Nudges the main controls to a new tasteful combination — a fast way to find something new." },
            { "TIPS",       "Double-click any knob to reset it. Hold Shift while dragging for fine control. "
                            "The two Engines can be switched on or off independently. Watch the meters: the red light means too loud." },
        };

        for (const auto& row : rows)
        {
            const bool isTips = juce::String (row.head) == "TIPS";
            auto lineArea = inner.removeFromTop (isTips ? int (58 * s) : int (30 * s));
            auto headArea = lineArea.removeFromLeft (int (70 * s));
            g.setColour (c::accent);
            g.setFont (type.heading (12.0f * s));
            g.drawText (row.head, headArea, juce::Justification::topLeft);
            g.setColour (c::dim);
            g.setFont (type.body (12.5f * s));
            g.drawFittedText (row.text, lineArea, juce::Justification::topLeft, 3);
        }
    }

    void resized() override
    {
        const float s = scale();
        panel = juce::Rectangle<float> (juce::jmin (float (getWidth()) - 32.0f, 440.0f * s),
                                        juce::jmin (float (getHeight()) - 32.0f, 420.0f * s))
                    .withCentre (getLocalBounds().getCentre().toFloat());

        const int b = int (26 * s);
        closeButton.setBounds (int (panel.getRight()) - b - int (10 * s),
                               int (panel.getY()) + int (10 * s), b, b);
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        if (! panel.contains (e.position))
            setVisible (false);
    }

private:
    float scale() const { return juce::jmax (0.75f, float (getWidth()) / 560.0f); }

    RowenIconButton closeButton { "Close help", RowenIconButton::Icon::close };
    juce::Rectangle<float> panel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HelpPanel)
};

} // namespace rowen::ui

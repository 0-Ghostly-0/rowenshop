// Rowen look-and-feel — draws every stock JUCE widget class we use in the
// site's visual language: hairline borders, card surfaces, pill buttons,
// magenta accent, soft 250 ms hovers. Shared by all future Rowen plugins.
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace rowen::theme
{

class RowenLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    RowenLookAndFeel();

    // Knobs ------------------------------------------------------------------
    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider&) override;

    // Buttons ----------------------------------------------------------------
    void drawButtonBackground (juce::Graphics&, juce::Button&,
                               const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown) override;
    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override;

    // Combo box + popup menu -------------------------------------------------
    void drawComboBox (juce::Graphics&, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH,
                       juce::ComboBox&) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override;
    void drawPopupMenuBackground (juce::Graphics&, int width, int height) override;
    juce::Font getPopupMenuFont() override;

    // Labels / text ----------------------------------------------------------
    juce::Font getLabelFont (juce::Label&) override;

    // Tooltips ---------------------------------------------------------------
    void drawTooltip (juce::Graphics&, const juce::String& text, int width, int height) override;
    juce::Rectangle<int> getTooltipBounds (const juce::String& tipText,
                                           juce::Point<int> screenPos,
                                           juce::Rectangle<int> parentArea) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RowenLookAndFeel)
};

} // namespace rowen::theme

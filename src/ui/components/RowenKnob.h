// Rowen knob — a complete labeled control: name (Space Grotesk), custom rotary
// (drawn by RowenLookAndFeel), and live value readout (JetBrains Mono).
// Self-scaling: all fonts and geometry derive from the component's own size,
// so the resizable editor never produces blurry or misaligned controls.
//
// Interaction: drag to adjust, Shift-drag for fine control, double-click to
// reset to default, mouse-wheel supported, keyboard arrows when focused.
#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "../theme/RowenTheme.h"
#include "../theme/RowenTypography.h"

namespace rowen::ui
{

class RowenKnob : public juce::Component
{
public:
    using ValueFormatter = std::function<juce::String (double)>;

    RowenKnob (juce::AudioProcessorValueTreeState& apvts,
               const juce::String& paramID,
               const juce::String& displayName,
               const juce::String& tooltipText,
               ValueFormatter formatter)
        : name (displayName), format (std::move (formatter))
    {
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        slider.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f,
                                    juce::MathConstants<float>::pi * 2.75f, true);
        // Shift-drag = fine adjustment.
        slider.setVelocityModeParameters (0.6, 1, 0.0, true, juce::ModifierKeys::shiftModifier);
        slider.setWantsKeyboardFocus (true);
        slider.setTooltip (tooltipText);
        slider.setTitle (displayName);                    // screen-reader name
        slider.setDescription (tooltipText);              // screen-reader detail
        slider.onValueChange = [this] { repaint(); };
        addAndMakeVisible (slider);

        attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            apvts, paramID, slider);

        if (auto* p = apvts.getParameter (paramID))
            slider.setDoubleClickReturnValue (true,
                double (p->convertFrom0to1 (p->getDefaultValue())));

        setInterceptsMouseClicks (false, true);
    }

    void resized() override
    {
        auto area = getLocalBounds();
        labelH = juce::jmax (14, area.getHeight() / 7);
        valueH = juce::jmax (13, area.getHeight() / 8);
        area.removeFromTop (labelH);
        area.removeFromBottom (valueH);
        slider.setBounds (area);
    }

    void paint (juce::Graphics& g) override
    {
        namespace c = rowen::theme::color;
        auto& type = rowen::theme::Typography::get();

        auto area = getLocalBounds();
        const auto labelArea = area.removeFromTop (labelH);
        const auto valueArea = area.removeFromBottom (valueH);

        g.setColour (slider.isEnabled() ? c::dim : c::dimmer);
        g.setFont (type.heading (float (labelH) * 0.78f));
        g.drawText (name.toUpperCase(), labelArea, juce::Justification::centred);

        g.setColour (slider.isMouseOverOrDragging() ? c::white : c::dim);
        g.setFont (type.mono (float (valueH) * 0.80f));
        g.drawText (format (slider.getValue()), valueArea, juce::Justification::centred);
    }

    juce::Slider& getSlider() { return slider; }

    // Ready-made formatters for the two value styles Rowen plugins use.
    static ValueFormatter percentFormatter()
    {
        return [] (double v) { return juce::String (juce::roundToInt (v)) + "%"; };
    }
    static ValueFormatter decibelFormatter()
    {
        return [] (double v)
        {
            return (v >= 0.05 ? "+" : "") + juce::String (v, 1) + " dB";
        };
    }

private:
    juce::String name;
    ValueFormatter format;
    juce::Slider slider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    int labelH { 16 }, valueH { 14 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RowenKnob)
};

} // namespace rowen::ui

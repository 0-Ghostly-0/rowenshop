// Rowen modal — a minimal branded text-entry dialog drawn inside the plugin
// window (never a native OS dialog). Used for preset save/rename. Reusable.
// Return = confirm, Escape or backdrop click = cancel.
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../theme/RowenTheme.h"
#include "../theme/RowenTypography.h"

namespace rowen::ui
{

class RowenModal : public juce::Component
{
public:
    RowenModal()
    {
        setVisible (false);
        setAlwaysOnTop (true);

        title.setJustificationType (juce::Justification::centredLeft);
        addAndMakeVisible (title);

        input.setSelectAllWhenFocused (true);
        input.onReturnKey = [this] { confirm(); };
        input.onEscapeKey = [this] { dismiss(); };
        addAndMakeVisible (input);

        okButton.onClick = [this] { confirm(); };
        addAndMakeVisible (okButton);
        cancelButton.onClick = [this] { dismiss(); };
        addAndMakeVisible (cancelButton);
    }

    void show (const juce::String& titleText, const juce::String& initialText,
               std::function<void (const juce::String&)> onSubmitFn)
    {
        onSubmit = std::move (onSubmitFn);
        title.setText (titleText, juce::dontSendNotification);
        input.setText (initialText, juce::dontSendNotification);
        setVisible (true);
        toFront (true);
        input.grabKeyboardFocus();
    }

    void paint (juce::Graphics& g) override
    {
        namespace c = rowen::theme::color;
        g.fillAll (juce::Colours::black.withAlpha (0.60f)); // dim backdrop

        g.setColour (c::card2);
        g.fillRoundedRectangle (panel, rowen::theme::metric::radius);
        g.setColour (c::lineStrong);
        g.drawRoundedRectangle (panel, rowen::theme::metric::radius, 1.0f);
    }

    void resized() override
    {
        const float s = juce::jmax (0.75f, float (getWidth()) / 560.0f);
        const float pw = juce::jmin (float (getWidth()) - 40.0f, 340.0f * s);
        panel = juce::Rectangle<float> (pw, 150.0f * s).withCentre (getLocalBounds().getCentre().toFloat());

        auto inner = panel.getSmallestIntegerContainer().reduced (int (18 * s));
        title.setFont (rowen::theme::Typography::get().heading (17.0f * s));
        title.setColour (juce::Label::textColourId, rowen::theme::color::white);
        title.setBounds (inner.removeFromTop (int (26 * s)));

        inner.removeFromTop (int (8 * s));
        input.setFont (rowen::theme::Typography::get().body (15.0f * s));
        input.setBounds (inner.removeFromTop (int (32 * s)));

        inner.removeFromTop (int (12 * s));
        auto buttons = inner.removeFromTop (int (30 * s));
        okButton.setBounds (buttons.removeFromRight (int (80 * s)));
        buttons.removeFromRight (int (8 * s));
        cancelButton.setBounds (buttons.removeFromRight (int (80 * s)));
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        if (! panel.contains (e.position))
            dismiss();
    }

private:
    void confirm()
    {
        const auto text = input.getText();
        setVisible (false);
        if (onSubmit != nullptr)
            onSubmit (text);
    }

    void dismiss() { setVisible (false); }

    juce::Label title;
    juce::TextEditor input;
    juce::TextButton okButton { "Save" }, cancelButton { "Cancel" };
    juce::Rectangle<float> panel;
    std::function<void (const juce::String&)> onSubmit;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RowenModal)
};

} // namespace rowen::ui

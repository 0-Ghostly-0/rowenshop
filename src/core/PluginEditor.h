// Rowen Fragment — the branded interface.
// Layout: top bar (logo, name, category-grouped preset browser + favorites,
// undo/redo, help, bypass) -> a control strip (engine toggles, buffer
// length) -> the BufferDisplay centerpiece -> the Fragment knob as the
// dominant control -> the Shift/Reverse/Scatter/Drift/Atmosphere row -> the
// Movement/Mix/Output row flanked by meters and the Freeze/Randomize actions
// -> a quiet footer. Same design system as Vocal Polish (same theme,
// typography, knob and button language) with its own "experimental" visual
// personality carried entirely by BufferDisplay.
#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "PluginProcessor.h"
#include "../ui/components/BufferDisplay.h"
#include "../ui/components/FragmentFooter.h"
#include "../ui/components/FragmentTopBar.h"
#include "../ui/components/HelpPanel.h"
#include "../ui/components/LevelMeter.h"
#include "../ui/components/RowenKnob.h"
#include "../ui/components/RowenModal.h"
#include "../ui/components/RowenToggleDot.h"
#include "../ui/theme/RowenLookAndFeel.h"

namespace rowen
{

class FragmentEditor final : public juce::AudioProcessorEditor
{
public:
    explicit FragmentEditor (FragmentProcessor&);
    ~FragmentEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboAttachment  = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    struct EngineDot
    {
        EngineDot (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramID,
                   const juce::String& name, const juce::String& tip)
            : dot (name)
        {
            dot.setTooltip (tip);
            dot.setTitle (name);
            attachment = std::make_unique<ButtonAttachment> (apvts, paramID, dot);
        }

        ui::RowenToggleDot dot;
        std::unique_ptr<ButtonAttachment> attachment;
    };

    // A TextButton that also offers a right-click menu (Randomize strength),
    // without disturbing its normal left-click behavior.
    struct RandomizeButton final : public juce::TextButton
    {
        RandomizeButton() : juce::TextButton ("RANDOMIZE") {}
        std::function<void()> onRightClick;

        void mouseDown (const juce::MouseEvent& e) override
        {
            if (e.mods.isPopupMenu())
            {
                if (onRightClick) onRightClick();
                return;
            }
            juce::TextButton::mouseDown (e);
        }
    };

    enum class RandomStrength { subtle, balanced, extreme };

    void randomize (RandomStrength strength);
    void showRandomizeMenu();

    FragmentProcessor& processor;

    theme::RowenLookAndFeel lookAndFeel;
    juce::TooltipWindow tooltips { this, 650 };
    juce::Image texture;

    ui::FragmentTopBar topBar;
    ui::FragmentFooter footer;
    ui::BufferDisplay bufferDisplay;

    ui::RowenKnob fragmentKnob, shift, reverse, scatter, drift, atmosphere;
    ui::RowenKnob movement, mix, outGain, freeLength;

    EngineDot engineADot, engineBDot;

    juce::ComboBox bufferLenBox, shiftSnapBox;
    std::unique_ptr<ComboAttachment> bufferLenAttachment, shiftSnapAttachment;

    ui::RowenToggleDot extendedDot { "Extended range" };
    std::unique_ptr<ButtonAttachment> extendedAttachment;

    juce::TextButton freezeButton { "FREEZE" };
    std::unique_ptr<ButtonAttachment> freezeAttachment;

    RandomizeButton randomizeButton;

    ui::LevelMeter inMeter, outMeter;

    ui::HelpPanel helpPanel;
    ui::RowenModal modal;

    juce::Rectangle<int> engineLabelArea, bufferLabelArea;

    juce::Random rng;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FragmentEditor)
};

} // namespace rowen

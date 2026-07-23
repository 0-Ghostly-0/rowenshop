#include "ParameterLayout.h"
#include "ParameterIDs.h"

namespace rowen::fragparam
{

namespace
{
    using FloatParam  = juce::AudioParameterFloat;
    using BoolParam   = juce::AudioParameterBool;
    using ChoiceParam = juce::AudioParameterChoice;

    std::unique_ptr<FloatParam> percent (const char* id, const juce::String& name, float defaultValue)
    {
        return std::make_unique<FloatParam> (
            juce::ParameterID { id, kVersionHint }, name,
            juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), defaultValue,
            juce::AudioParameterFloatAttributes()
                .withLabel ("%")
                .withStringFromValueFunction ([] (float v, int) { return juce::String (juce::roundToInt (v)) + "%"; }));
    }

    std::unique_ptr<BoolParam> toggle (const char* id, const juce::String& name, bool defaultValue)
    {
        return std::make_unique<BoolParam> (juce::ParameterID { id, kVersionHint }, name, defaultValue);
    }
} // namespace

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // --- Main creative controls ----------------------------------------------
    layout.add (percent (id::fragment, "Fragment", 40.0f));

    // Full ±24 st range is allocated NOW so enabling Extended later never
    // changes the parameter contract; the UI simply limits its travel to ±12
    // unless shift_ext is on. Automation is written in semitones either way.
    layout.add (std::make_unique<FloatParam> (
        juce::ParameterID { id::shift, kVersionHint }, "Shift",
        juce::NormalisableRange<float> (-24.0f, 24.0f, 0.01f), 0.0f,
        juce::AudioParameterFloatAttributes()
            .withLabel ("st")
            .withStringFromValueFunction ([] (float v, int)
            {
                const auto rounded = std::round (v * 10.0f) / 10.0f;
                return (rounded > 0.04f ? "+" : "") + juce::String (rounded, 1) + " st";
            })));

    layout.add (std::make_unique<ChoiceParam> (
        juce::ParameterID { id::shiftSnap, kVersionHint }, "Shift Snap",
        juce::StringArray { "Semitones", "Fifths", "Octaves", "Off" }, 0));
    layout.add (toggle (id::shiftExt, "Extended Range", false));

    layout.add (percent (id::reverse,    "Reverse",    25.0f));
    layout.add (percent (id::scatter,    "Scatter",    30.0f));
    layout.add (percent (id::drift,      "Drift",      20.0f));
    layout.add (percent (id::atmosphere, "Atmosphere", 25.0f));
    layout.add (percent (id::movement,   "Movement",   35.0f));

    // --- Mix / output --------------------------------------------------------
    layout.add (percent (id::mix, "Mix", 40.0f));

    layout.add (std::make_unique<FloatParam> (
        juce::ParameterID { id::outGain, kVersionHint }, "Output",
        juce::NormalisableRange<float> (-24.0f, 6.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes()
            .withLabel ("dB")
            .withStringFromValueFunction ([] (float v, int)
            { return (v >= 0.05f ? "+" : "") + juce::String (v, 1) + " dB"; })));

    // --- Buffer --------------------------------------------------------------
    layout.add (std::make_unique<ChoiceParam> (
        juce::ParameterID { id::bufferLen, kVersionHint }, "Buffer Length",
        juce::StringArray { "1/4 Bar", "1/2 Bar", "1 Bar", "2 Bars", "4 Bars", "Free" }, 2));

    {
        juce::NormalisableRange<float> freeRange (250.0f, 8000.0f, 1.0f);
        freeRange.setSkewForCentre (1500.0f);
        layout.add (std::make_unique<FloatParam> (
            juce::ParameterID { id::bufferFree, kVersionHint }, "Free Length",
            freeRange, 2000.0f,
            juce::AudioParameterFloatAttributes()
                .withLabel ("ms")
                .withStringFromValueFunction ([] (float v, int)
                {
                    return v >= 1000.0f ? juce::String (v / 1000.0f, 2) + " s"
                                        : juce::String (juce::roundToInt (v)) + " ms";
                })));
    }

    layout.add (toggle (id::freeze, "Freeze", false));

    // --- Engines + global ----------------------------------------------------
    layout.add (toggle (id::engineA, "Engine A", true));
    layout.add (toggle (id::engineB, "Engine B", true));
    layout.add (toggle (id::bypass,  "Bypass",   false));

    // --- Advanced (hidden drawer; audible from Phase 3/4) --------------------
    layout.add (std::make_unique<FloatParam> (
        juce::ParameterID { id::densityBias, kVersionHint }, "Density Bias",
        juce::NormalisableRange<float> (-50.0f, 50.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel ("%")));

    layout.add (std::make_unique<FloatParam> (
        juce::ParameterID { id::grainMin, kVersionHint }, "Min Fragment",
        juce::NormalisableRange<float> (15.0f, 100.0f, 1.0f), 30.0f,
        juce::AudioParameterFloatAttributes().withLabel ("ms")));

    layout.add (std::make_unique<FloatParam> (
        juce::ParameterID { id::grainMax, kVersionHint }, "Max Fragment",
        juce::NormalisableRange<float> (100.0f, 600.0f, 1.0f), 400.0f,
        juce::AudioParameterFloatAttributes().withLabel ("ms")));

    layout.add (percent (id::posSpread, "Position Spread", 50.0f));
    layout.add (percent (id::panSpread, "Pan Spread",      60.0f));

    layout.add (std::make_unique<FloatParam> (
        juce::ParameterID { id::pitchVar, kVersionHint }, "Pitch Variation",
        juce::NormalisableRange<float> (0.0f, 100.0f, 1.0f), 20.0f,
        juce::AudioParameterFloatAttributes().withLabel ("ct")));

    layout.add (std::make_unique<FloatParam> (
        juce::ParameterID { id::feedback, kVersionHint }, "Feedback",
        juce::NormalisableRange<float> (0.0f, 60.0f, 0.1f), 20.0f,
        juce::AudioParameterFloatAttributes().withLabel ("%")));

    {
        juce::NormalisableRange<float> toneRange (500.0f, 8000.0f, 1.0f);
        toneRange.setSkewForCentre (2500.0f);
        layout.add (std::make_unique<FloatParam> (
            juce::ParameterID { id::fbTone, kVersionHint }, "Feedback Tone",
            toneRange, 3500.0f,
            juce::AudioParameterFloatAttributes().withLabel ("Hz")));
    }

    {
        juce::NormalisableRange<float> rateRange (0.05f, 8.0f, 0.01f);
        rateRange.setSkewForCentre (0.8f);
        layout.add (std::make_unique<FloatParam> (
            juce::ParameterID { id::motionRate, kVersionHint }, "Motion Rate",
            rateRange, 0.4f,
            juce::AudioParameterFloatAttributes().withLabel ("Hz")));
    }

    layout.add (std::make_unique<ChoiceParam> (
        juce::ParameterID { id::pulseDiv, kVersionHint }, "Pulse Rate",
        juce::StringArray { "1/32", "1/16", "1/8", "1/4", "1/2", "1 Bar", "2 Bars",
                            "1/16T", "1/8T", "1/4T", "1/16D", "1/8D", "1/4D" }, 2));

    return layout;
}

} // namespace rowen::fragparam

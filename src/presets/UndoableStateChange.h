// Rowen Fragment — turns a bulk parameter change (Randomize, loading a
// preset) into ONE undoable action instead of one undo step per parameter.
//
// Deliberately does NOT use AudioProcessorValueTreeState::replaceState() for
// perform()/undo(): a raw ValueTree swap doesn't reliably walk the normal
// host-notification path parameter-by-parameter, which can desync a DAW's own
// automation display. Instead this replays the same beginChangeGesture() /
// setValueNotifyingHost() / endChangeGesture() sequence normal interaction
// uses, just driven from a stored snapshot rather than a mouse drag.
#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_data_structures/juce_data_structures.h> // juce::UndoManager / juce::UndoableAction

namespace rowen::presets
{

class StateChangeAction final : public juce::UndoableAction
{
public:
    using Snapshot = std::vector<std::pair<juce::String, float>>; // paramID -> natural value

    StateChangeAction (juce::AudioProcessorValueTreeState& stateToChange,
                       Snapshot beforeValues, Snapshot afterValues)
        : apvts (stateToChange), before (std::move (beforeValues)), after (std::move (afterValues))
    {
    }

    bool perform() override { apply (after);  return true; }
    bool undo()    override { apply (before); return true; }

    int getSizeInUnits() override
    {
        return int ((before.size() + after.size()) * sizeof (Snapshot::value_type));
    }

    // Snapshots an explicit list of parameters (e.g. just what Randomize touches).
    static Snapshot snapshot (juce::AudioProcessorValueTreeState& apvts,
                              const std::vector<juce::String>& paramIDs)
    {
        Snapshot s;
        for (const auto& paramId : paramIDs)
            if (auto* p = apvts.getParameter (paramId))
                s.push_back ({ paramId, p->convertFrom0to1 (p->getValue()) });
        return s;
    }

    // Snapshots every registered parameter (e.g. before/after a preset load,
    // which sets every user-facing control at once). Bypass is intentionally
    // excluded -- presets and randomize must never touch it.
    static Snapshot snapshotAll (juce::AudioProcessorValueTreeState& apvts,
                                const juce::String& excludeParamID = {})
    {
        Snapshot s;
        for (auto* p : apvts.processor.getParameters())
        {
            if (auto* withId = dynamic_cast<juce::AudioProcessorParameterWithID*> (p))
            {
                if (withId->paramID == excludeParamID)
                    continue;
                if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*> (p))
                    s.push_back ({ withId->paramID, ranged->convertFrom0to1 (ranged->getValue()) });
            }
        }
        return s;
    }

private:
    void apply (const Snapshot& values)
    {
        for (const auto& [paramId, value] : values)
        {
            if (auto* p = apvts.getParameter (paramId))
            {
                p->beginChangeGesture();
                p->setValueNotifyingHost (p->convertTo0to1 (value));
                p->endChangeGesture();
            }
        }
    }

    juce::AudioProcessorValueTreeState& apvts;
    Snapshot before, after;
};

} // namespace rowen::presets

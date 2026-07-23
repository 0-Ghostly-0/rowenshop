#include "FactoryPresets.h"
#include "../params/ParameterIDs.h"

#include <algorithm>

namespace rowen::presets
{

using namespace rowen::fragparam;

namespace
{
    // buffer_len choice indices, matching ParameterLayout.cpp's ChoiceParam order.
    constexpr float kQuarterBar = 0.0f;
    constexpr float kHalfBar    = 1.0f;
    constexpr float kOneBar     = 2.0f;
    constexpr float kTwoBars    = 3.0f;
    constexpr float kFourBars   = 4.0f;

    // shift_snap choice indices.
    constexpr float kSnapSemitones = 0.0f;
    constexpr float kSnapFifths    = 1.0f;
    constexpr float kSnapOctaves   = 2.0f;
    constexpr float kSnapOff       = 3.0f;
}

// Every preset sets EVERY user-facing parameter except Output and Bypass (the
// spec explicitly excludes those from preset/randomize changes), so switching
// presets always produces a fully-defined, predictable state.
const std::vector<FactoryPreset>& getFactoryPresets()
{
    static const std::vector<FactoryPreset> presets = {

        // --- Subtle ----------------------------------------------------------
        { "Light Texture", "Subtle", "A faint grain over the source; mostly still itself.",
          { { id::fragment, 20 }, { id::shift, 0 }, { id::shiftSnap, kSnapOff }, { id::shiftExt, 0 },
            { id::reverse, 5 }, { id::scatter, 15 }, { id::drift, 15 }, { id::atmosphere, 15 },
            { id::movement, 20 }, { id::mix, 30 }, { id::bufferLen, kOneBar }, { id::bufferFree, 2000 },
            { id::freeze, 0 }, { id::engineA, 1 }, { id::engineB, 1 } } },

        { "Soft Movement", "Subtle", "A gentle wander in pitch and position, nothing dramatic.",
          { { id::fragment, 25 }, { id::shift, 0 }, { id::shiftSnap, kSnapOff }, { id::shiftExt, 0 },
            { id::reverse, 10 }, { id::scatter, 20 }, { id::drift, 30 }, { id::atmosphere, 20 },
            { id::movement, 40 }, { id::mix, 35 }, { id::bufferLen, kOneBar }, { id::bufferFree, 2000 },
            { id::freeze, 0 }, { id::engineA, 1 }, { id::engineB, 1 } } },

        { "Gentle Double", "Subtle", "A soft octave-up shadow that thickens the source.",
          { { id::fragment, 30 }, { id::shift, 12 }, { id::shiftSnap, kSnapOctaves }, { id::shiftExt, 0 },
            { id::reverse, 0 }, { id::scatter, 10 }, { id::drift, 10 }, { id::atmosphere, 10 },
            { id::movement, 15 }, { id::mix, 40 }, { id::bufferLen, kOneBar }, { id::bufferFree, 2000 },
            { id::freeze, 0 }, { id::engineA, 1 }, { id::engineB, 0 } } },

        { "Dust Around It", "Subtle", "A faint grainy halo sitting just behind the source.",
          { { id::fragment, 20 }, { id::shift, 0 }, { id::shiftSnap, kSnapOff }, { id::shiftExt, 0 },
            { id::reverse, 15 }, { id::scatter, 25 }, { id::drift, 20 }, { id::atmosphere, 30 },
            { id::movement, 20 }, { id::mix, 25 }, { id::bufferLen, kOneBar }, { id::bufferFree, 2000 },
            { id::freeze, 0 }, { id::engineA, 1 }, { id::engineB, 1 } } },

        { "Wide Detail", "Subtle", "Small fragments spread gently across the stereo field.",
          { { id::fragment, 25 }, { id::shift, 0 }, { id::shiftSnap, kSnapOff }, { id::shiftExt, 0 },
            { id::reverse, 10 }, { id::scatter, 20 }, { id::drift, 15 }, { id::atmosphere, 25 },
            { id::movement, 20 }, { id::mix, 30 }, { id::bufferLen, kOneBar }, { id::bufferFree, 2000 },
            { id::freeze, 0 }, { id::engineA, 1 }, { id::engineB, 1 } } },

        // --- Melodic -----------------------------------------------------------
        { "Broken Keys", "Melodic", "Fifth-snapped fragments that turn a phrase into a broken chord.",
          { { id::fragment, 45 }, { id::shift, 7 }, { id::shiftSnap, kSnapFifths }, { id::shiftExt, 0 },
            { id::reverse, 15 }, { id::scatter, 35 }, { id::drift, 20 }, { id::atmosphere, 25 },
            { id::movement, 25 }, { id::mix, 45 }, { id::bufferLen, kOneBar }, { id::bufferFree, 2000 },
            { id::freeze, 0 }, { id::engineA, 1 }, { id::engineB, 1 } } },

        { "Reverse Bloom", "Melodic", "Mostly-reversed fragments that swell in from nowhere.",
          { { id::fragment, 40 }, { id::shift, 0 }, { id::shiftSnap, kSnapOff }, { id::shiftExt, 0 },
            { id::reverse, 80 }, { id::scatter, 30 }, { id::drift, 25 }, { id::atmosphere, 40 },
            { id::movement, 30 }, { id::mix, 50 }, { id::bufferLen, kTwoBars }, { id::bufferFree, 2000 },
            { id::freeze, 0 }, { id::engineA, 1 }, { id::engineB, 1 } } },

        { "Crystal Pieces", "Melodic", "Octave-up glassy fragments scattered wide.",
          { { id::fragment, 55 }, { id::shift, 12 }, { id::shiftSnap, kSnapOctaves }, { id::shiftExt, 0 },
            { id::reverse, 10 }, { id::scatter, 40 }, { id::drift, 15 }, { id::atmosphere, 35 },
            { id::movement, 20 }, { id::mix, 45 }, { id::bufferLen, kOneBar }, { id::bufferFree, 2000 },
            { id::freeze, 0 }, { id::engineA, 1 }, { id::engineB, 1 } } },

        { "Falling Melody", "Melodic", "Downward-shifted fragments that seem to tumble.",
          { { id::fragment, 50 }, { id::shift, -5 }, { id::shiftSnap, kSnapOff }, { id::shiftExt, 0 },
            { id::reverse, 20 }, { id::scatter, 45 }, { id::drift, 30 }, { id::atmosphere, 30 },
            { id::movement, 35 }, { id::mix, 45 }, { id::bufferLen, kTwoBars }, { id::bufferFree, 2000 },
            { id::freeze, 0 }, { id::engineA, 1 }, { id::engineB, 1 } } },

        { "Floating Chords", "Melodic", "Slow, drifting fragments over a long pooled buffer.",
          { { id::fragment, 35 }, { id::shift, 0 }, { id::shiftSnap, kSnapOff }, { id::shiftExt, 0 },
            { id::reverse, 10 }, { id::scatter, 30 }, { id::drift, 40 }, { id::atmosphere, 45 },
            { id::movement, 45 }, { id::mix, 40 }, { id::bufferLen, kFourBars }, { id::bufferFree, 2000 },
            { id::freeze, 0 }, { id::engineA, 1 }, { id::engineB, 1 } } },

        { "Distant Memory", "Melodic", "Softly detuned fragments buried in a hazy atmosphere.",
          { { id::fragment, 30 }, { id::shift, -3 }, { id::shiftSnap, kSnapOff }, { id::shiftExt, 0 },
            { id::reverse, 20 }, { id::scatter, 35 }, { id::drift, 35 }, { id::atmosphere, 55 },
            { id::movement, 30 }, { id::mix, 35 }, { id::bufferLen, kFourBars }, { id::bufferFree, 2000 },
            { id::freeze, 0 }, { id::engineA, 1 }, { id::engineB, 1 } } },

        // --- Rhythmic ------------------------------------------------------------
        { "Quarter Chop", "Rhythmic", "Tight quarter-note-length chops, tempo-locked and dry.",
          { { id::fragment, 70 }, { id::shift, 0 }, { id::shiftSnap, kSnapOff }, { id::shiftExt, 0 },
            { id::reverse, 5 }, { id::scatter, 20 }, { id::drift, 5 }, { id::atmosphere, 10 },
            { id::movement, 10 }, { id::mix, 60 }, { id::bufferLen, kQuarterBar }, { id::bufferFree, 2000 },
            { id::freeze, 0 }, { id::engineA, 1 }, { id::engineB, 1 } } },

        { "Eighth Bounce", "Rhythmic", "Fast, bouncy eighth-feel chopping.",
          { { id::fragment, 75 }, { id::shift, 0 }, { id::shiftSnap, kSnapOff }, { id::shiftExt, 0 },
            { id::reverse, 10 }, { id::scatter, 15 }, { id::drift, 5 }, { id::atmosphere, 10 },
            { id::movement, 15 }, { id::mix, 55 }, { id::bufferLen, kQuarterBar }, { id::bufferFree, 2000 },
            { id::freeze, 0 }, { id::engineA, 1 }, { id::engineB, 1 } } },

        { "Triplet Scatter", "Rhythmic", "Off-beat, tripletty scatter across a half-bar window.",
          { { id::fragment, 65 }, { id::shift, 0 }, { id::shiftSnap, kSnapOff }, { id::shiftExt, 0 },
            { id::reverse, 15 }, { id::scatter, 40 }, { id::drift, 10 }, { id::atmosphere, 15 },
            { id::movement, 20 }, { id::mix, 55 }, { id::bufferLen, kHalfBar }, { id::bufferFree, 2000 },
            { id::freeze, 0 }, { id::engineA, 1 }, { id::engineB, 1 } } },

        { "Broken Rhythm", "Rhythmic", "Chops the groove apart and reassembles it slightly wrong.",
          { { id::fragment, 70 }, { id::shift, 0 }, { id::shiftSnap, kSnapOff }, { id::shiftExt, 0 },
            { id::reverse, 30 }, { id::scatter, 35 }, { id::drift, 15 }, { id::atmosphere, 15 },
            { id::movement, 20 }, { id::mix, 60 }, { id::bufferLen, kHalfBar }, { id::bufferFree, 2000 },
            { id::freeze, 0 }, { id::engineA, 1 }, { id::engineB, 1 } } },

        { "Pulse Cut", "Rhythmic", "Rapid, gated-feeling cuts with almost no ambience.",
          { { id::fragment, 80 }, { id::shift, 0 }, { id::shiftSnap, kSnapOff }, { id::shiftExt, 0 },
            { id::reverse, 5 }, { id::scatter, 10 }, { id::drift, 5 }, { id::atmosphere, 5 },
            { id::movement, 10 }, { id::mix, 65 }, { id::bufferLen, kQuarterBar }, { id::bufferFree, 2000 },
            { id::freeze, 0 }, { id::engineA, 1 }, { id::engineB, 1 } } },

        { "Off Grid", "Rhythmic", "Deliberately loose timing that fights the grid.",
          { { id::fragment, 60 }, { id::shift, 0 }, { id::shiftSnap, kSnapOff }, { id::shiftExt, 0 },
            { id::reverse, 25 }, { id::scatter, 50 }, { id::drift, 25 }, { id::atmosphere, 20 },
            { id::movement, 30 }, { id::mix, 55 }, { id::bufferLen, kHalfBar }, { id::bufferFree, 2000 },
            { id::freeze, 0 }, { id::engineA, 1 }, { id::engineB, 1 } } },

        // --- Vocal -----------------------------------------------------------
        { "Ghost Vocal", "Vocal", "Thins the voice into a faint, hovering presence.",
          { { id::fragment, 35 }, { id::shift, 0 }, { id::shiftSnap, kSnapOff }, { id::shiftExt, 0 },
            { id::reverse, 20 }, { id::scatter, 30 }, { id::drift, 25 }, { id::atmosphere, 50 },
            { id::movement, 25 }, { id::mix, 45 }, { id::bufferLen, kOneBar }, { id::bufferFree, 2000 },
            { id::freeze, 0 }, { id::engineA, 1 }, { id::engineB, 1 } } },

        { "Vocal Cloud", "Vocal", "Dissolves the words into a soft granular wash.",
          { { id::fragment, 40 }, { id::shift, 0 }, { id::shiftSnap, kSnapOff }, { id::shiftExt, 0 },
            { id::reverse, 15 }, { id::scatter, 35 }, { id::drift, 30 }, { id::atmosphere, 60 },
            { id::movement, 30 }, { id::mix, 50 }, { id::bufferLen, kTwoBars }, { id::bufferFree, 2000 },
            { id::freeze, 0 }, { id::engineA, 1 }, { id::engineB, 1 } } },

        { "Reverse Words", "Vocal", "Flips most fragments backward for an unintelligible, spoken-in-reverse feel.",
          { { id::fragment, 45 }, { id::shift, 0 }, { id::shiftSnap, kSnapOff }, { id::shiftExt, 0 },
            { id::reverse, 85 }, { id::scatter, 25 }, { id::drift, 15 }, { id::atmosphere, 25 },
            { id::movement, 20 }, { id::mix, 55 }, { id::bufferLen, kOneBar }, { id::bufferFree, 2000 },
            { id::freeze, 0 }, { id::engineA, 1 }, { id::engineB, 1 } } },

        { "Fragmented Hook", "Vocal", "Chopped, scattered pieces of a vocal hook.",
          { { id::fragment, 50 }, { id::shift, 0 }, { id::shiftSnap, kSnapOff }, { id::shiftExt, 0 },
            { id::reverse, 20 }, { id::scatter, 40 }, { id::drift, 20 }, { id::atmosphere, 30 },
            { id::movement, 25 }, { id::mix, 50 }, { id::bufferLen, kOneBar }, { id::bufferFree, 2000 },
            { id::freeze, 0 }, { id::engineA, 1 }, { id::engineB, 1 } } },

        { "Background Voices", "Vocal", "Pushes the voice back into a low, murmuring layer.",
          { { id::fragment, 30 }, { id::shift, -2 }, { id::shiftSnap, kSnapOff }, { id::shiftExt, 0 },
            { id::reverse, 15 }, { id::scatter, 30 }, { id::drift, 20 }, { id::atmosphere, 45 },
            { id::movement, 20 }, { id::mix, 30 }, { id::bufferLen, kTwoBars }, { id::bufferFree, 2000 },
            { id::freeze, 0 }, { id::engineA, 1 }, { id::engineB, 1 } } },

        // --- Drums -----------------------------------------------------------
        { "Loose Percussion", "Drums", "Percussion hits scattered into a looser pattern.",
          { { id::fragment, 55 }, { id::shift, 0 }, { id::shiftSnap, kSnapOff }, { id::shiftExt, 0 },
            { id::reverse, 10 }, { id::scatter, 25 }, { id::drift, 15 }, { id::atmosphere, 10 },
            { id::movement, 15 }, { id::mix, 45 }, { id::bufferLen, kQuarterBar }, { id::bufferFree, 2000 },
            { id::freeze, 0 }, { id::engineA, 1 }, { id::engineB, 0 } } },

        { "Drum Dust", "Drums", "A gritty, granular haze layered behind the beat.",
          { { id::fragment, 50 }, { id::shift, 0 }, { id::shiftSnap, kSnapOff }, { id::shiftExt, 0 },
            { id::reverse, 15 }, { id::scatter, 30 }, { id::drift, 20 }, { id::atmosphere, 35 },
            { id::movement, 20 }, { id::mix, 40 }, { id::bufferLen, kHalfBar }, { id::bufferFree, 2000 },
            { id::freeze, 0 }, { id::engineA, 1 }, { id::engineB, 1 } } },

        { "Reverse Hits", "Drums", "Drum hits flipped backward for risers and transitions.",
          { { id::fragment, 45 }, { id::shift, 0 }, { id::shiftSnap, kSnapOff }, { id::shiftExt, 0 },
            { id::reverse, 90 }, { id::scatter, 15 }, { id::drift, 10 }, { id::atmosphere, 10 },
            { id::movement, 10 }, { id::mix, 55 }, { id::bufferLen, kHalfBar }, { id::bufferFree, 2000 },
            { id::freeze, 0 }, { id::engineA, 1 }, { id::engineB, 1 } } },

        { "Metallic Loop", "Drums", "A clangy, pitched-up loop with a metallic edge.",
          { { id::fragment, 60 }, { id::shift, 5 }, { id::shiftSnap, kSnapOff }, { id::shiftExt, 0 },
            { id::reverse, 20 }, { id::scatter, 35 }, { id::drift, 15 }, { id::atmosphere, 15 },
            { id::movement, 20 }, { id::mix, 50 }, { id::bufferLen, kHalfBar }, { id::bufferFree, 2000 },
            { id::freeze, 0 }, { id::engineA, 1 }, { id::engineB, 1 } } },

        { "Broken Bounce", "Drums", "A bouncy, stumbling re-cut of the source rhythm.",
          { { id::fragment, 65 }, { id::shift, 0 }, { id::shiftSnap, kSnapOff }, { id::shiftExt, 0 },
            { id::reverse, 25 }, { id::scatter, 30 }, { id::drift, 20 }, { id::atmosphere, 15 },
            { id::movement, 20 }, { id::mix, 55 }, { id::bufferLen, kQuarterBar }, { id::bufferFree, 2000 },
            { id::freeze, 0 }, { id::engineA, 1 }, { id::engineB, 1 } } },

        // --- Experimental ------------------------------------------------------
        { "Data Collapse", "Experimental", "An octave-down, high-scatter breakdown of the source.",
          { { id::fragment, 85 }, { id::shift, -12 }, { id::shiftSnap, kSnapOctaves }, { id::shiftExt, 0 },
            { id::reverse, 50 }, { id::scatter, 70 }, { id::drift, 60 }, { id::atmosphere, 40 },
            { id::movement, 60 }, { id::mix, 70 }, { id::bufferLen, kTwoBars }, { id::bufferFree, 2000 },
            { id::freeze, 0 }, { id::engineA, 1 }, { id::engineB, 1 } } },

        { "Frozen Room", "Experimental", "A static, heavily atmospheric wash — built to Freeze into.",
          { { id::fragment, 40 }, { id::shift, 0 }, { id::shiftSnap, kSnapOff }, { id::shiftExt, 0 },
            { id::reverse, 10 }, { id::scatter, 20 }, { id::drift, 15 }, { id::atmosphere, 75 },
            { id::movement, 15 }, { id::mix, 55 }, { id::bufferLen, kFourBars }, { id::bufferFree, 2000 },
            { id::freeze, 0 }, { id::engineA, 1 }, { id::engineB, 1 } } },

        { "Falling Apart", "Experimental", "The source visibly disintegrates as it plays.",
          { { id::fragment, 75 }, { id::shift, -7 }, { id::shiftSnap, kSnapOff }, { id::shiftExt, 0 },
            { id::reverse, 40 }, { id::scatter, 65 }, { id::drift, 55 }, { id::atmosphere, 45 },
            { id::movement, 55 }, { id::mix, 65 }, { id::bufferLen, kTwoBars }, { id::bufferFree, 2000 },
            { id::freeze, 0 }, { id::engineA, 1 }, { id::engineB, 1 } } },

        { "Empty Signal", "Experimental", "A hollow, wandering texture that barely resembles the source.",
          { { id::fragment, 60 }, { id::shift, 0 }, { id::shiftSnap, kSnapOff }, { id::shiftExt, 0 },
            { id::reverse, 30 }, { id::scatter, 55 }, { id::drift, 70 }, { id::atmosphere, 60 },
            { id::movement, 65 }, { id::mix, 60 }, { id::bufferLen, kFourBars }, { id::bufferFree, 2000 },
            { id::freeze, 0 }, { id::engineA, 1 }, { id::engineB, 1 } } },

        { "Lost Transmission", "Experimental", "A degraded, drifting signal that keeps cutting out.",
          { { id::fragment, 70 }, { id::shift, -5 }, { id::shiftSnap, kSnapOff }, { id::shiftExt, 0 },
            { id::reverse, 45 }, { id::scatter, 60 }, { id::drift, 50 }, { id::atmosphere, 55 },
            { id::movement, 50 }, { id::mix, 60 }, { id::bufferLen, kTwoBars }, { id::bufferFree, 2000 },
            { id::freeze, 0 }, { id::engineA, 1 }, { id::engineB, 1 } } },

        { "Unstable Machine", "Experimental", "The most extreme setting: fast, wide, and unpredictable.",
          { { id::fragment, 90 }, { id::shift, 3 }, { id::shiftSnap, kSnapOff }, { id::shiftExt, 1 },
            { id::reverse, 55 }, { id::scatter, 80 }, { id::drift, 80 }, { id::atmosphere, 35 },
            { id::movement, 75 }, { id::mix, 70 }, { id::bufferLen, kHalfBar }, { id::bufferFree, 2000 },
            { id::freeze, 0 }, { id::engineA, 1 }, { id::engineB, 1 } } },
    };

    return presets;
}

const std::vector<juce::String>& getFactoryCategories()
{
    static const std::vector<juce::String> categories = [] {
        std::vector<juce::String> cats;
        for (const auto& p : getFactoryPresets())
        {
            const juce::String cat (p.category);
            if (std::find (cats.begin(), cats.end(), cat) == cats.end())
                cats.push_back (cat);
        }
        return cats;
    }();
    return categories;
}

} // namespace rowen::presets

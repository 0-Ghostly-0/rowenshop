// Rowen Fragment — factory presets. Pure parameter data, no file or UI
// dependencies, so both the processor's program list and the preset browser
// can share it. Every preset name here is original (see docs/DEPENDENCIES.md
// for the originality notes vs. plugins like Autochroma).
#pragma once

#include <juce_core/juce_core.h>
#include <vector>

namespace rowen::presets
{

struct FactoryPreset
{
    const char* name;
    const char* category;    // one of: Subtle, Melodic, Rhythmic, Vocal, Drums, Experimental
    const char* description;
    std::vector<std::pair<const char*, float>> values; // parameter ID -> natural value
};

const std::vector<FactoryPreset>& getFactoryPresets();

// Categories in display order, deduplicated, as they appear in getFactoryPresets().
const std::vector<juce::String>& getFactoryCategories();

} // namespace rowen::presets

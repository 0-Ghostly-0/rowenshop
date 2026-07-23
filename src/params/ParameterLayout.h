// Rowen Fragment — builds the AudioProcessorValueTreeState layout.
// Single source of truth for ranges, defaults and display formatting.
// The complete table lives in docs/PARAMETERS.md.
#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace rowen::fragparam
{

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

} // namespace rowen::fragparam

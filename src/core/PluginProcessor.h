// Rowen Fragment — the AudioProcessor. Thin by design: parameters live in
// src/params, the engine in src/dsp, visualization transfer in src/viz.
#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_data_structures/juce_data_structures.h> // juce::UndoManager

#include "../dsp/FragmentCore.h"
#include "../presets/PresetManager.h"

namespace rowen
{

class FragmentProcessor final : public juce::AudioProcessor
{
public:
    FragmentProcessor();
    ~FragmentProcessor() override = default;

    //==========================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using juce::AudioProcessor::processBlock;

    //==========================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Rowen Fragment"; }
    bool acceptsMidi() const override  { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 4.0; } // grains + atmosphere

    juce::AudioProcessorParameter* getBypassParameter() const override { return bypassParam; }

    //==========================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    //==========================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==========================================================================
    juce::AudioProcessorValueTreeState& getState() { return apvts; }
    frag::VizExchange& getVizExchange() { return core.getVizExchange(); }
    presets::PresetManager& getPresetManager() { return presetManager; }
    juce::UndoManager& getUndoManager() { return undoManager; }

    std::atomic<float> inputPeak  { 0.0f };
    std::atomic<float> outputPeak { 0.0f };

private:
    frag::CoreSettings makeSettings();
    frag::TempoInfo readTransport();

    juce::AudioProcessorValueTreeState apvts;
    frag::FragmentCore core;
    presets::PresetManager presetManager;
    juce::UndoManager undoManager;
    int currentProgram { 0 };

    // Cached raw parameter pointers (looked up once; audio thread reads only).
    std::atomic<float>* p (const char* paramID) { return apvts.getRawParameterValue (paramID); }

    std::atomic<float>* pFragment {}; std::atomic<float>* pShift {};
    std::atomic<float>* pShiftSnap {}; std::atomic<float>* pShiftExt {};
    std::atomic<float>* pReverse {}; std::atomic<float>* pScatter {};
    std::atomic<float>* pDrift {}; std::atomic<float>* pAtmosphere {};
    std::atomic<float>* pMovement {}; std::atomic<float>* pMix {};
    std::atomic<float>* pOutGain {}; std::atomic<float>* pBufferLen {};
    std::atomic<float>* pBufferFree {}; std::atomic<float>* pFreeze {};
    std::atomic<float>* pEngineA {}; std::atomic<float>* pEngineB {};
    std::atomic<float>* pBypass {};
    std::atomic<float>* pDensityBias {}; std::atomic<float>* pGrainMin {};
    std::atomic<float>* pGrainMax {}; std::atomic<float>* pPosSpread {};
    std::atomic<float>* pPanSpread {}; std::atomic<float>* pPitchVar {};
    std::atomic<float>* pFeedback {}; std::atomic<float>* pFbTone {};
    std::atomic<float>* pMotionRate {}; std::atomic<float>* pPulseDiv {};

    juce::AudioProcessorParameter* bypassParam { nullptr };
    uint32_t randomSeed { 20260722u };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FragmentProcessor)
};

} // namespace rowen

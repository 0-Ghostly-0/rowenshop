#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "../params/ParameterIDs.h"
#include "../params/ParameterLayout.h"

namespace rowen
{

using namespace rowen::fragparam;

FragmentProcessor::FragmentProcessor()
    : juce::AudioProcessor (BusesProperties()
                                .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                                .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout()),
      presetManager (apvts)
{
    pFragment   = p (id::fragment);   pShift      = p (id::shift);
    pShiftSnap  = p (id::shiftSnap);  pShiftExt   = p (id::shiftExt);
    pReverse    = p (id::reverse);    pScatter    = p (id::scatter);
    pDrift      = p (id::drift);      pAtmosphere = p (id::atmosphere);
    pMovement   = p (id::movement);   pMix        = p (id::mix);
    pOutGain    = p (id::outGain);    pBufferLen  = p (id::bufferLen);
    pBufferFree = p (id::bufferFree); pFreeze     = p (id::freeze);
    pEngineA    = p (id::engineA);    pEngineB    = p (id::engineB);
    pBypass     = p (id::bypass);
    pDensityBias = p (id::densityBias); pGrainMin = p (id::grainMin);
    pGrainMax    = p (id::grainMax);    pPosSpread = p (id::posSpread);
    pPanSpread   = p (id::panSpread);   pPitchVar = p (id::pitchVar);
    pFeedback    = p (id::feedback);    pFbTone   = p (id::fbTone);
    pMotionRate  = p (id::motionRate);  pPulseDiv = p (id::pulseDiv);

    bypassParam = apvts.getParameter (id::bypass);
}

//==============================================================================
void FragmentProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    core.prepare (sampleRate, samplesPerBlock, getTotalNumOutputChannels());
    core.setSettings (makeSettings());
    setLatencySamples (0);
}

void FragmentProcessor::releaseResources()
{
    core.reset();
}

bool FragmentProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto in  = layouts.getMainInputChannelSet();
    const auto out = layouts.getMainOutputChannelSet();
    const bool inOk  = in  == juce::AudioChannelSet::mono() || in  == juce::AudioChannelSet::stereo();
    const bool outOk = out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
    return inOk && outOk && (in == out || (in == juce::AudioChannelSet::mono()
                                           && out == juce::AudioChannelSet::stereo()));
}

//==============================================================================
frag::TempoInfo FragmentProcessor::readTransport()
{
    frag::TempoInfo t;
    if (auto* playHead = getPlayHead())
    {
        if (const auto pos = playHead->getPosition())
        {
            if (const auto bpm = pos->getBpm())          { t.bpm = *bpm; t.hasTempo = true; }
            if (const auto sig = pos->getTimeSignature())
            {
                t.timeSigNumerator = sig->numerator;
                t.timeSigDenominator = sig->denominator;
            }
            if (const auto ppq = pos->getPpqPosition())  t.ppqPosition = *ppq;
            t.playing = pos->getIsPlaying();
        }
    }
    return t;
}

frag::CoreSettings FragmentProcessor::makeSettings()
{
    frag::CoreSettings s;
    s.fragment       = pFragment->load()   * 0.01f;
    s.shiftSemitones = pShift->load();
    s.shiftSnap      = int (pShiftSnap->load());
    s.shiftExtended  = pShiftExt->load() > 0.5f;
    s.reverse        = pReverse->load()    * 0.01f;
    s.scatter        = pScatter->load()    * 0.01f;
    s.drift          = pDrift->load()      * 0.01f;
    s.atmosphere     = pAtmosphere->load() * 0.01f;
    s.movement       = pMovement->load()   * 0.01f;
    s.mix            = pMix->load()        * 0.01f;
    s.outGainDb      = pOutGain->load();
    s.bufferLength   = frag::BufferLength (int (pBufferLen->load()));
    s.bufferFreeMs   = pBufferFree->load();
    s.freeze         = pFreeze->load()  > 0.5f;
    s.engineA        = pEngineA->load() > 0.5f;
    s.engineB        = pEngineB->load() > 0.5f;
    s.bypassed       = pBypass->load()  > 0.5f;
    s.densityBias    = pDensityBias->load() * 0.01f;
    s.grainMinMs     = pGrainMin->load();
    s.grainMaxMs     = pGrainMax->load();
    s.posSpread      = pPosSpread->load() * 0.01f;
    s.panSpread      = pPanSpread->load() * 0.01f;
    s.pitchVarCents  = pPitchVar->load();
    s.feedback       = pFeedback->load() * 0.01f;
    s.fbToneHz       = pFbTone->load();
    s.motionRateHz   = pMotionRate->load();
    s.pulseDivIndex  = int (pPulseDiv->load());
    s.randomSeed     = randomSeed;
    s.tempo          = readTransport();
    return s;
}

void FragmentProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numIn  = getTotalNumInputChannels();
    const int numOut = getTotalNumOutputChannels();
    const int numSamples = buffer.getNumSamples();

    for (int ch = numIn; ch < numOut; ++ch)
        buffer.clear (ch, 0, numSamples);
    if (numIn == 1 && numOut == 2)
        buffer.copyFrom (1, 0, buffer, 0, 0, numSamples);

    core.setSettings (makeSettings());
    core.process (buffer.getArrayOfWritePointers(),
                  std::min (numOut, frag::RollingBuffer::kMaxChannels),
                  numSamples);

    if (core.getInputPeak() > inputPeak.load())   inputPeak.store (core.getInputPeak());
    if (core.getOutputPeak() > outputPeak.load()) outputPeak.store (core.getOutputPeak());
}

//==============================================================================
juce::AudioProcessorEditor* FragmentProcessor::createEditor()
{
    return new FragmentEditor (*this);
}

//==============================================================================
int FragmentProcessor::getNumPrograms()
{
    return presetManager.getNumFactoryPresets();
}

int FragmentProcessor::getCurrentProgram() { return currentProgram; }

void FragmentProcessor::setCurrentProgram (int index)
{
    if (index >= 0 && index < presetManager.getNumFactoryPresets())
    {
        currentProgram = index;
        presetManager.applyFactoryPreset (index);
    }
}

const juce::String FragmentProcessor::getProgramName (int index)
{
    return presetManager.getFactoryPresetName (index);
}

//==============================================================================
void FragmentProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty (prop::stateVersion, kStateVersion, nullptr);
    state.setProperty (prop::randomSeed, juce::int64 (randomSeed), nullptr);
    state.setProperty ("pluginVersion", JucePlugin_VersionString, nullptr);

    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void FragmentProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary (data, sizeInBytes);
    if (xml == nullptr || ! xml->hasTagName (apvts.state.getType()))
        return; // unknown or damaged state: keep current settings

    const auto incoming = juce::ValueTree::fromXml (*xml);
    randomSeed = uint32_t (juce::int64 (incoming.getProperty (prop::randomSeed,
                                                              juce::int64 (randomSeed))));
    apvts.replaceState (incoming);
}

} // namespace rowen

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new rowen::FragmentProcessor();
}

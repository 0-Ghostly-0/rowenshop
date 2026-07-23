#include "FragmentCore.h"

namespace rowen::frag
{

void FragmentCore::prepare (double sampleRate, int maxBlockSize, int numChannels)
{
    sr = sampleRate;
    preparedBlock = maxBlockSize;

    rolling.prepare (sampleRate, numChannels, kMaxCaptureSeconds);
    pool.reset();
    schedulerA.prepare (sampleRate, 0, engineAProfile());
    schedulerB.prepare (sampleRate, 1, engineBProfile());
    motion.prepare (sampleRate);
    atmosphereFx.prepare (sampleRate, maxBlockSize);

    wetL.assign (size_t (maxBlockSize), 0.0f);
    wetR.assign (size_t (maxBlockSize), 0.0f);

    outGain.prepare (sampleRate, 20.0f);
    outGain.setImmediate (1.0f);
    bypassMix.prepare (sampleRate, 40.0f);
    bypassMix.setImmediate (settings.bypassed ? 1.0f : 0.0f);
    mixAmount.prepare (sampleRate, 30.0f);
    mixAmount.setImmediate (settings.mix);
    engineGainA.prepare (sampleRate, 30.0f);
    engineGainA.setImmediate (settings.engineA ? 1.0f : 0.0f);
    engineGainB.prepare (sampleRate, 30.0f);
    engineGainB.setImmediate (settings.engineB ? 1.0f : 0.0f);

    vizInterval = std::max (256, int (sampleRate / 30.0));
    vizCountdown = 0;
    prepared = true;
    reset();
}

void FragmentCore::reset()
{
    rolling.reset();
    pool.reset();
    schedulerA.reset();
    schedulerB.reset();
    motion.reset();
    atmosphereFx.reset();
    inputPeak = outputPeak = 0.0f;
}

void FragmentCore::setSettings (const CoreSettings& s)
{
    settings = s;

    rolling.setFrozen (s.freeze);
    rolling.setTargetWindowSamples (
        TempoSync::targetWindowSamples (s.bufferLength, s.bufferFreeMs, s.tempo,
                                        sr, float (rolling.capacitySamples())));

    outGain.setTarget (dbToGain (clampf (s.outGainDb, -24.0f, 6.0f)));
    bypassMix.setTarget (s.bypassed ? 1.0f : 0.0f);
    mixAmount.setTarget (clampf (s.mix, 0.0f, 1.0f));
    engineGainA.setTarget (s.engineA ? 1.0f : 0.0f);
    engineGainB.setTarget (s.engineB ? 1.0f : 0.0f);

    motion.setRate (s.motionRateHz);
    atmosphereFx.setAmount (s.atmosphere, s.feedback, s.fbToneHz);
}

void FragmentCore::process (float* const* channels, int numChannels, int numSamples)
{
    if (! prepared || numSamples <= 0)
        return;

    numChannels = std::min (numChannels, RollingBuffer::kMaxChannels);
    numSamples  = std::min (numSamples, preparedBlock);

    // Input meter.
    float inPk = 0.0f;
    for (int c = 0; c < numChannels; ++c)
        for (int i = 0; i < numSamples; ++i)
            inPk = std::max (inPk, std::fabs (channels[c][i]));
    inputPeak = inPk;

    // Capture (continues while bypassed so the display stays alive).
    rolling.writeBlock (channels, numChannels, numSamples);

    // Fully bypassed and settled: dry passthrough, engines idle.
    if (! bypassMix.moving() && bypassMix.value() >= 1.0f)
    {
        outputPeak = inPk;
        vizCountdown -= numSamples;
        if (vizCountdown <= 0) { vizCountdown += vizInterval; publishViz(); }
        return;
    }

    // --- Modulation + schedule new grains ------------------------------------
    motion.advance (numSamples);
    const auto motionNow = motion.current();

    auto trigger = [this] (const FragmentVoice& v) { pool.add (v); };
    if (settings.engineA || engineGainA.value() > 0.001f)
        schedulerA.advance (numSamples, settings, rolling, motionNow, trigger);
    if (settings.engineB || engineGainB.value() > 0.001f)
        schedulerB.advance (numSamples, settings, rolling, motionNow, trigger);

    // --- Render the wet signal ----------------------------------------------
    std::fill (wetL.begin(), wetL.begin() + numSamples, 0.0f);
    std::fill (wetR.begin(), wetR.begin() + numSamples, 0.0f);

    const float gainA = engineGainA.skip (numSamples);
    const float gainB = engineGainB.skip (numSamples);
    pool.renderAll (rolling, wetL.data(), wetR.data(), numSamples, numChannels, gainA, gainB);

    // --- Atmosphere: diffusion + HF softening, wet path only ----------------
    if (atmosphereFx.isActive())
        atmosphereFx.process (wetL.data(), wetR.data(), numSamples);

    // --- Equal-power mix, output gain, bypass crossfade, safety, meter ------
    float outPk = 0.0f;
    for (int i = 0; i < numSamples; ++i)
    {
        const float m = mixAmount.next();
        const float dryGain = std::cos (m * kPi * 0.5f);
        const float wetGain = std::sin (m * kPi * 0.5f);
        const float g = outGain.next();
        const float b = bypassMix.next();

        for (int c = 0; c < numChannels; ++c)
        {
            const float dry = channels[c][i];
            const float wet = numChannels > 1 ? (c == 0 ? wetL[size_t (i)] : wetR[size_t (i)])
                                              : 0.707f * (wetL[size_t (i)] + wetR[size_t (i)]);
            const float processed = softLimit ((dry * dryGain + wet * wetGain) * g);
            const float out = processed + b * (dry - processed);
            channels[c][i] = out;
            outPk = std::max (outPk, std::fabs (out));
        }
    }
    outputPeak = outPk;

    // --- Viz ----------------------------------------------------------------
    vizCountdown -= numSamples;
    if (vizCountdown <= 0)
    {
        vizCountdown += vizInterval;
        publishViz();
    }
}

void FragmentCore::publishViz()
{
    VizData d;
    rolling.fillPeaks (d.peaks, VizData::kBins);
    d.writePos01 = 1.0f;
    d.windowSeconds = rolling.usableWindow() / float (sr);
    d.inputPeak = inputPeak;
    d.outputPeak = outputPeak;
    d.frozen = rolling.isFrozen() ? 1 : 0;
    d.playing = settings.tempo.playing ? 1 : 0;

    // Grain markers: the first kMaxMarkers active voices.
    const double head = double (rolling.totalWritten());
    const float window = std::max (1.0f, rolling.usableWindow());
    int m = 0;
    for (const auto& v : pool.all())
    {
        if (! v.active) continue;
        auto& marker = d.markers[m];
        const float dist = float (head - v.position);
        marker.position01 = clampf (1.0f - dist / window, 0.0f, 1.0f);
        marker.age01 = clampf (v.age / v.lengthSamples, 0.0f, 1.0f);
        marker.direction = char (v.direction);
        marker.engine = (unsigned char) v.engine;
        marker.active = 1;
        if (++m >= VizData::kMaxMarkers) break;
    }

    viz.publish (d);
}

} // namespace rowen::frag

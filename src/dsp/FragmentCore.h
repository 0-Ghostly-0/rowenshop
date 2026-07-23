// Rowen Fragment — the engine facade the processor talks to.
// Phase 3: rolling capture + freeze + the two grain engines. The signal path
// is: capture -> Engine A/B schedulers -> voice pool renders wet -> equal-
// power dry/wet mix -> output gain -> soft safety -> meters + viz.
// Phase 4 adds Drift/Movement modulation and the Atmosphere block on the wet
// path without changing this interface.
#pragma once

#include "AtmosphereFx.h"
#include "CoreSettings.h"
#include "DspUtil.h"
#include "GrainScheduler.h"
#include "MotionLfo.h"
#include "RollingBuffer.h"
#include "VoicePool.h"
#include "../viz/VizSnapshot.h"

namespace rowen::frag
{

class FragmentCore
{
public:
    void prepare (double sampleRate, int maxBlockSize, int numChannels);
    void reset();

    void setSettings (const CoreSettings& s);
    void process (float* const* channels, int numChannels, int numSamples);

    VizExchange& getVizExchange() { return viz; }

    float getInputPeak()  const { return inputPeak; }
    float getOutputPeak() const { return outputPeak; }
    int   getActiveVoices() const { return pool.activeCount(); }

    const RollingBuffer& buffer() const { return rolling; }

    static constexpr double kMaxCaptureSeconds = 16.0;

private:
    void publishViz();
    static float softLimit (float x) noexcept
    {
        constexpr float t = 0.891f; // knee at -1 dBFS
        const float a = std::fabs (x);
        if (a <= t) return x;
        const float shaped = t + (1.0f - t) * std::tanh ((a - t) / (1.0f - t));
        return x > 0.0f ? shaped : -shaped;
    }

    RollingBuffer rolling;
    VoicePool pool;
    GrainScheduler schedulerA, schedulerB;
    MotionLfo motion;
    AtmosphereFx atmosphereFx;
    VizExchange viz;
    CoreSettings settings;

    std::vector<float> wetL, wetR; // preallocated wet accumulation buffers

    LinearSmoother outGain, bypassMix, mixAmount, engineGainA, engineGainB;
    double sr { 48000.0 };
    int preparedBlock { 0 };
    int vizCountdown { 0 };
    int vizInterval { 1600 };
    float inputPeak { 0.0f }, outputPeak { 0.0f };
    bool prepared { false };
};

} // namespace rowen::frag

// Rowen Fragment — the Motion modulator behind Drift.
// Two detuned sines combined into four CORRELATED outputs (pitch, position,
// pan, tone). Correlation is the point: everything breathes together like a
// worn machine, instead of independent random wobble on every parameter.
#pragma once

#include "DspUtil.h"

namespace rowen::frag
{

class MotionLfo
{
public:
    struct Values
    {
        float pitch { 0.0f };   // -1..1
        float position { 0.0f };
        float pan { 0.0f };
        float tone { 0.0f };
    };

    void prepare (double sampleRate)
    {
        sr = sampleRate;
        phaseA = 0.13f;
        phaseB = 0.71f; // different start: outputs related but not identical
    }

    void reset() { phaseA = 0.13f; phaseB = 0.71f; }

    void setRate (float hz) { rateHz = clampf (hz, 0.02f, 12.0f); }

    void advance (int numSamples)
    {
        const float incA = rateHz / float (sr);
        const float incB = incA * 0.618f; // golden-ratio detune: never repeats exactly
        phaseA += incA * float (numSamples);
        phaseB += incB * float (numSamples);
        phaseA -= std::floor (phaseA);
        phaseB -= std::floor (phaseB);
    }

    Values current() const
    {
        const float a = std::sin (2.0f * kPi * phaseA);
        const float b = std::sin (2.0f * kPi * phaseB + 1.7f);
        Values v;
        v.pitch    = a;
        v.position = 0.6f * a + 0.4f * b;
        v.pan      = b;
        v.tone     = 0.5f * (a + b);
        return v;
    }

private:
    double sr { 48000.0 };
    float rateHz { 0.4f };
    float phaseA { 0.0f }, phaseB { 0.0f };
};

} // namespace rowen::frag

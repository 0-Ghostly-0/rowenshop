// Rowen Fragment — one grain voice. A plain struct with a tight render loop:
// no allocation, no virtuals, no branches beyond what the audio needs.
//
// Positions live on the RollingBuffer's monotonic totalWritten() timeline, so
// a triggered voice replays a FIXED piece of captured audio while the write
// head moves on (or holds, when frozen). The scheduler guarantees the region
// it hands out stays clear of the write head and inside the window.
#pragma once

#include "DspUtil.h"
#include "RollingBuffer.h"

namespace rowen::frag
{

struct FragmentVoice
{
    bool  active { false };
    int   engine { 0 };          // 0 = A, 1 = B
    int   direction { 1 };       // +1 forward, -1 reversed
    double position { 0.0 };     // absolute (totalWritten timeline)
    float speed { 1.0f };        // playback ratio (pitch via varispeed)
    float lengthSamples { 0.0f };
    float age { 0.0f };          // samples rendered so far
    float gain { 0.0f };
    float panL { 0.707f }, panR { 0.707f };

    // Hann window value for the current age.
    float envelope() const noexcept
    {
        const float t = age / lengthSamples;
        return 0.5f * (1.0f - std::cos (2.0f * kPi * t));
    }

    // Accumulate up to numSamples into wetL/wetR. engineGain is the smoothed
    // per-engine enable gain for this block.
    void render (const RollingBuffer& rb, float* wetL, float* wetR,
                 int numSamples, int numChannels, float engineGain) noexcept
    {
        if (! active || engineGain <= 0.0001f)
        {
            if (active && engineGain <= 0.0001f && age > 0.0f)
                advanceSilently (numSamples); // muted engine: keep time moving
            return;
        }

        const int rightSource = numChannels > 1 ? 1 : 0;

        for (int i = 0; i < numSamples; ++i)
        {
            const float env = envelope() * gain * engineGain;
            const float sL = rb.readAbsolute (0, position);
            const float sR = rb.readAbsolute (rightSource, position);

            wetL[i] += sL * env * panL;
            wetR[i] += sR * env * panR;

            position += double (direction) * double (speed);
            if ((age += 1.0f) >= lengthSamples)
            {
                active = false;
                return;
            }
        }
    }

    void advanceSilently (int numSamples) noexcept
    {
        position += double (direction) * double (speed) * double (numSamples);
        if ((age += float (numSamples)) >= lengthSamples)
            active = false;
    }
};

} // namespace rowen::frag

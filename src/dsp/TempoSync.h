// Rowen Fragment — tempo/transport information and buffer-length math.
// Host data is advisory: every value is validated and clamped before use, and
// missing information falls back to safe defaults (120 BPM, 4/4).
#pragma once

#include "DspUtil.h"

namespace rowen::frag
{

struct TempoInfo
{
    double bpm { 120.0 };
    int timeSigNumerator { 4 };
    int timeSigDenominator { 4 };
    bool playing { false };
    double ppqPosition { 0.0 };
    bool hasTempo { false }; // host actually reported a tempo
};

// Buffer length choices (parameter order is frozen — see ParameterIDs.h).
enum class BufferLength : int
{
    quarterBar = 0,
    halfBar,
    oneBar,
    twoBars,
    fourBars,
    freeTime
};

class TempoSync
{
public:
    static double sanitizedBpm (const TempoInfo& t) noexcept
    {
        const double bpm = t.hasTempo ? t.bpm : 120.0;
        return bpm > 20.0 && bpm < 999.0 ? bpm : 120.0;
    }

    static double barSeconds (const TempoInfo& t) noexcept
    {
        const double bpm = sanitizedBpm (t);
        const int num = t.timeSigNumerator   > 0 ? t.timeSigNumerator   : 4;
        const int den = t.timeSigDenominator > 0 ? t.timeSigDenominator : 4;
        return double (num) * (60.0 / bpm) * (4.0 / double (den));
    }

    // Target capture window in samples for the current settings, clamped to
    // what the preallocated buffer can hold.
    static float targetWindowSamples (BufferLength mode, float freeMs,
                                      const TempoInfo& tempo,
                                      double sampleRate, float maxSamples) noexcept
    {
        double seconds = 0.0;
        switch (mode)
        {
            case BufferLength::quarterBar: seconds = barSeconds (tempo) * 0.25; break;
            case BufferLength::halfBar:    seconds = barSeconds (tempo) * 0.5;  break;
            case BufferLength::oneBar:     seconds = barSeconds (tempo) * 1.0;  break;
            case BufferLength::twoBars:    seconds = barSeconds (tempo) * 2.0;  break;
            case BufferLength::fourBars:   seconds = barSeconds (tempo) * 4.0;  break;
            case BufferLength::freeTime:   seconds = double (clampf (freeMs, 250.0f, 8000.0f)) * 0.001; break;
        }

        const float samples = float (seconds * sampleRate);
        return clampf (samples, float (sampleRate) * 0.1f, maxSamples * 0.98f);
    }
};

} // namespace rowen::frag

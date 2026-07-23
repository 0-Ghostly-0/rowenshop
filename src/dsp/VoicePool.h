// Rowen Fragment — fixed-size grain voice pool. Preallocated, allocation-free,
// with a safe stealing strategy: when full, the quietest voice already past
// its envelope peak is recycled (its fade-out is replaced by the new grain's
// fade-in, so stealing is inaudible in practice).
#pragma once

#include "FragmentVoice.h"
#include <array>

namespace rowen::frag
{

class VoicePool
{
public:
    static constexpr int kMaxVoices = 48;
    static constexpr int kMaxPerEngine[2] = { 16, 32 }; // A stable, B textural

    void reset()
    {
        for (auto& v : voices) v.active = false;
    }

    // Returns false only if the engine is at its cap AND no voice is safely
    // stealable — the grain is simply skipped (never a glitch).
    bool add (const FragmentVoice& newVoice)
    {
        int engineCount = 0;
        FragmentVoice* freeSlot = nullptr;
        FragmentVoice* steal = nullptr;
        float quietest = 1.0e9f;

        for (auto& v : voices)
        {
            if (! v.active)
            {
                if (freeSlot == nullptr) freeSlot = &v;
                continue;
            }
            if (v.engine == newVoice.engine)
            {
                ++engineCount;
                if (v.age > v.lengthSamples * 0.5f) // past envelope peak
                {
                    const float loudness = v.envelope() * v.gain;
                    if (loudness < quietest) { quietest = loudness; steal = &v; }
                }
            }
        }

        if (engineCount >= kMaxPerEngine[newVoice.engine & 1])
        {
            if (steal == nullptr) return false;
            *steal = newVoice;
            return true;
        }

        if (freeSlot != nullptr) { *freeSlot = newVoice; return true; }
        if (steal != nullptr)    { *steal = newVoice;    return true; }
        return false;
    }

    void renderAll (const RollingBuffer& rb, float* wetL, float* wetR,
                    int numSamples, int numChannels,
                    float engineGainA, float engineGainB)
    {
        for (auto& v : voices)
            if (v.active)
                v.render (rb, wetL, wetR, numSamples, numChannels,
                          v.engine == 0 ? engineGainA : engineGainB);
    }

    int activeCount() const
    {
        int n = 0;
        for (const auto& v : voices) n += v.active ? 1 : 0;
        return n;
    }

    const std::array<FragmentVoice, kMaxVoices>& all() const { return voices; }

private:
    std::array<FragmentVoice, kMaxVoices> voices {};
};

} // namespace rowen::frag

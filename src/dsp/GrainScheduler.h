// Rowen Fragment — per-engine grain scheduling.
//
// Two instances run side by side with different profiles:
//   Engine A — stable: longer fragments, gentler density, reads close to the
//              newest audio; keeps the source recognizable.
//   Engine B — textural: shorter fragments, higher density, wider position /
//              pitch / pan variation; adds life around A.
//
// Determinism: every grain's random values come from a counter-keyed xorshift
// (seed ^ grainIndex ^ engineId), and triggering counts absolute samples — so
// a saved project reproduces its character regardless of host block size.
//
// Safety: trigger-time math guarantees the whole grain path stays inside
// [head - window, head - guard] for its entire life, including up-pitched
// forward grains that chase the moving write head.
#pragma once

#include "CoreSettings.h"
#include "DspUtil.h"
#include "FragmentVoice.h"
#include "MotionLfo.h"

namespace rowen::frag
{

struct EngineProfile
{
    float lengthAtLow, lengthAtHigh;    // ms at Fragment = 0 -> 1
    float densityAtLow, densityAtHigh;  // grains/sec
    float positionWeight;               // how far Scatter reaches into the buffer
    float reverseWeight;                // scales the Reverse probability
    float pitchVarWeight;               // scales cent variation
    float panWeight;                    // scales pan spread
    float levelDb;                      // engine mix level
};

inline EngineProfile engineAProfile() { return { 380.0f, 110.0f, 1.5f,  6.0f, 0.45f, 0.90f, 0.5f, 0.6f,  0.0f }; }
inline EngineProfile engineBProfile() { return { 170.0f,  45.0f, 2.5f, 12.0f, 1.00f, 1.15f, 1.5f, 1.0f, -4.5f }; }

//==============================================================================
class GrainScheduler
{
public:
    void prepare (double sampleRate, int engineIndex, const EngineProfile& engineProfile)
    {
        sr = sampleRate;
        engineId = engineIndex;
        profile = engineProfile;
        samplesToNext = 0.0;
        grainCounter = 0;
    }

    void reset()
    {
        samplesToNext = 0.0;
        grainCounter = 0;
    }

    // Called once per block; triggers zero or more grains into the pool via
    // tryTrigger(). Advance is in samples.
    template <typename TriggerFn>
    void advance (int numSamples, const CoreSettings& s, const RollingBuffer& rb,
                  const MotionLfo::Values& motion, TriggerFn&& trigger)
    {
        const float density = currentDensity (s);
        if (density <= 0.001f)
        {
            samplesToNext = 0.0; // retrigger immediately once density returns
            return;
        }

        double remaining = double (numSamples);
        while (remaining > 0.0)
        {
            if (samplesToNext <= 0.0)
            {
                FragmentVoice v;
                if (makeGrain (s, rb, motion, v))
                    trigger (v);

                // Next interval: base rate with seeded jitter (more at high
                // Fragment/Scatter, never fully random).
                Xorshift rng (grainSeed (s.randomSeed, grainCounter, 0x51ed));
                const float jitterAmt = 0.10f + 0.25f * s.fragment + 0.20f * s.scatter;
                const float jitter = 1.0f + jitterAmt * rng.nextBipolar();
                samplesToNext = double (sr) / double (density) * double (clampf (jitter, 0.4f, 1.8f));
                ++grainCounter;
            }

            const double step = std::min (remaining, samplesToNext);
            samplesToNext -= step;
            remaining -= step;
        }
    }

    float expectedOverlap (const CoreSettings& s) const
    {
        return currentDensity (s) * currentLengthMs (s) * 0.001f;
    }

private:
    float currentLengthMs (const CoreSettings& s) const
    {
        float ms = profile.lengthAtLow + (profile.lengthAtHigh - profile.lengthAtLow) * s.fragment;
        return clampf (ms, s.grainMinMs, s.grainMaxMs);
    }

    float currentDensity (const CoreSettings& s) const
    {
        const float d = profile.densityAtLow + (profile.densityAtHigh - profile.densityAtLow) * s.fragment;
        return d * (1.0f + s.densityBias);
    }

    static uint32_t grainSeed (uint32_t base, uint32_t counter, uint32_t salt)
    {
        return (base ^ (counter * 2654435761u) ^ (salt * 0x9E3779B9u)) | 1u;
    }

    static float snapSemitones (float st, int mode)
    {
        switch (mode)
        {
            case 0: return std::round (st);                    // semitones
            case 1: return std::round (st / 7.0f) * 7.0f;      // fifths
            case 2: return std::round (st / 12.0f) * 12.0f;    // octaves
            default: return st;                                // off
        }
    }

    bool makeGrain (const CoreSettings& s, const RollingBuffer& rb,
                    const MotionLfo::Values& motion, FragmentVoice& v)
    {
        // Drift: smooth, correlated movement sampled at trigger time. Movement
        // scales the overall modulation depth (Phase 4 adds the Pulse source).
        const float driftDepth = s.drift * (0.30f + 0.70f * s.movement);

        Xorshift rng (grainSeed (s.randomSeed, grainCounter, uint32_t (0xA11CE + engineId * 7919)));

        const float window = rb.usableWindow();
        const float guard  = float (rb.guardSamples());
        if (window <= guard * 6.0f)
            return false; // buffer not filled enough yet

        // --- length ---------------------------------------------------------
        float length = currentLengthMs (s) * 0.001f * float (sr);
        length *= 1.0f + 0.20f * rng.nextBipolar();
        length = clampf (length,
                         s.grainMinMs * 0.001f * float (sr),
                         std::min (s.grainMaxMs * 0.001f * float (sr), window * 0.45f));
        if (length < 32.0f)
            return false;

        // --- pitch ----------------------------------------------------------
        const float uiLimit = s.shiftExtended ? 24.0f : 12.0f;
        float st = snapSemitones (clampf (s.shiftSemitones, -uiLimit, uiLimit), s.shiftSnap);
        const float centRange = s.pitchVarCents * profile.pitchVarWeight * (0.3f + 0.7f * s.scatter);
        st += (centRange * rng.nextBipolar()) * 0.01f;
        st += driftDepth * motion.pitch * 0.9f; // Drift: up to ~1 st of slow warp
        v.speed = clampf (std::pow (2.0f, st / 12.0f), 0.24f, 4.05f);

        // --- direction ------------------------------------------------------
        const float revProb = clampf (s.reverse * profile.reverseWeight, 0.0f, 1.0f);
        v.direction = rng.next01() < revProb ? -1 : 1;

        // --- start position (the safety math) -------------------------------
        // Forward grains consume length*speed samples of buffer travel while
        // the head advances length samples; reversed grains walk away from
        // the head into older audio.
        const float margin = 16.0f;
        float minStart, maxStart;
        if (v.direction > 0)
        {
            minStart = guard + margin + std::max (0.0f, length * (v.speed - 1.0f));
            maxStart = window - margin - length * v.speed;
        }
        else
        {
            minStart = guard + margin;
            maxStart = window - margin - length * v.speed;
        }
        if (maxStart <= minStart)
        {
            // Window too small for this grain at this speed: shorten it.
            length = clampf ((window - guard - 3.0f * margin) / std::max (1.0f, v.speed * 1.5f),
                             32.0f, length);
            maxStart = std::max (minStart + 1.0f, window - margin - length * v.speed);
            if (maxStart <= minStart)
                return false;
        }

        // Scatter shapes WHERE in [minStart, maxStart] we read: 0 hugs the
        // newest audio, 1 roams the whole window. Squared response keeps low
        // values stable, and the engine's positionWeight scales the reach.
        const float reach = clampf (s.scatter * profile.positionWeight
                                        * (0.35f + 0.65f * s.posSpread * 2.0f), 0.0f, 1.0f);
        float depth01 = reach * rng.next01() * rng.next01();
        depth01 = clampf (depth01 + driftDepth * motion.position * 0.15f, 0.0f, 1.0f);
        const float startDist = minStart + (maxStart - minStart) * depth01;

        v.position = double (rb.totalWritten()) - double (startDist);
        v.lengthSamples = length;
        v.age = 0.0f;

        // --- pan (equal power) ---------------------------------------------
        const float panAmt = s.panSpread * profile.panWeight * (0.25f + 0.75f * s.scatter);
        const float pan = clampf (panAmt * rng.nextBipolar()
                                      + driftDepth * motion.pan * 0.5f, -1.0f, 1.0f);
        const float angle = (pan + 1.0f) * 0.25f * kPi;
        v.panL = std::cos (angle);
        v.panR = std::sin (angle);

        // --- gain: keep the wet level steady as overlap grows ---------------
        const float overlap = std::max (1.0f, expectedOverlap (s));
        v.gain = dbToGain (profile.levelDb) / std::sqrt (overlap);
        v.gain *= 0.9f + 0.2f * rng.next01();

        v.engine = engineId;
        v.active = true;
        return true;
    }

    double sr { 48000.0 };
    int engineId { 0 };
    EngineProfile profile {};
    double samplesToNext { 0.0 };
    uint32_t grainCounter { 0 };
};

} // namespace rowen::frag

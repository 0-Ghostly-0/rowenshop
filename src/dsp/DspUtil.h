// Rowen Fragment — small framework-free DSP utilities.
// Same architecture rule as Rowen Vocal Polish: nothing in src/dsp includes
// JUCE, so the whole engine is unit-testable standalone and reusable across
// Rowen products.
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace rowen::frag
{

constexpr float kPi = 3.14159265358979323846f;

inline float dbToGain (float dB) noexcept { return std::pow (10.0f, dB * 0.05f); }
inline float gainToDb (float g)  noexcept { return 20.0f * std::log10 (std::max (g, 1.0e-9f)); }
inline float undenorm (float x)  noexcept { return std::fabs (x) < 1.0e-20f ? 0.0f : x; }
inline float clampf (float v, float lo, float hi) noexcept { return std::min (std::max (v, lo), hi); }

//==============================================================================
// Linear per-sample smoother (identical contract to the Vocal Polish one).
class LinearSmoother
{
public:
    void prepare (double sampleRate, float rampMs) noexcept
    {
        stepsTotal = std::max (1, int (sampleRate * rampMs * 0.001));
        setImmediate (target);
    }

    void setImmediate (float v) noexcept { current = target = v; stepsLeft = 0; step = 0.0f; }

    void setTarget (float v) noexcept
    {
        if (v == target) return;
        target = v;
        stepsLeft = stepsTotal;
        step = (target - current) / float (stepsTotal);
    }

    float next() noexcept
    {
        if (stepsLeft > 0)
        {
            current += step;
            if (--stepsLeft == 0) current = target;
        }
        return current;
    }

    float skip (int n) noexcept
    {
        if (stepsLeft > 0)
        {
            const int k = std::min (stepsLeft, n);
            current += step * float (k);
            stepsLeft -= k;
            if (stepsLeft == 0) current = target;
        }
        return current;
    }

    float value()  const noexcept { return current; }
    bool  moving() const noexcept { return stepsLeft > 0; }

private:
    float current { 0.0f }, target { 0.0f }, step { 0.0f };
    int stepsLeft { 0 }, stepsTotal { 1 };
};

//==============================================================================
// One-pole smoother for slowly gliding values (buffer window length).
class OnePoleGlide
{
public:
    void prepare (double sampleRate, float tauMs) noexcept
    {
        coef = std::exp (-1.0f / (float (sampleRate) * tauMs * 0.001f));
    }

    void setImmediate (float v) noexcept { current = target = v; }
    void setTarget (float v) noexcept { target = v; }

    float nextBlock (int numSamples) noexcept
    {
        const float c = std::pow (coef, float (numSamples));
        current = target + (current - target) * c;
        if (std::fabs (current - target) < 1.0e-3f) current = target;
        return current;
    }

    float value() const noexcept { return current; }

private:
    float current { 0.0f }, target { 0.0f }, coef { 0.999f };
};

//==============================================================================
// Deterministic seedable PRNG (xorshift32) — audio-safe, allocation-free.
class Xorshift
{
public:
    explicit Xorshift (uint32_t seed = 0x9E3779B9u) { setSeed (seed); }

    void setSeed (uint32_t seed) noexcept { state = seed == 0 ? 0x9E3779B9u : seed; }

    uint32_t nextU32() noexcept
    {
        uint32_t x = state;
        x ^= x << 13; x ^= x >> 17; x ^= x << 5;
        return state = x;
    }

    float next01() noexcept { return float (nextU32() >> 8) * (1.0f / 16777216.0f); }
    float nextBipolar() noexcept { return next01() * 2.0f - 1.0f; }

private:
    uint32_t state;
};

} // namespace rowen::frag

// Rowen Fragment — the Atmosphere block. Operates ONLY on the wet fragment
// signal: short diffusion (Freeverb-topology combs + allpasses, public-domain
// structure, our code — same approach as Vocal Polish's Space), plus
// high-frequency softening. The internal feedback amount is influenced by
// Atmosphere and the hidden `feedback` parameter, hard-capped for stability.
#pragma once

#include "DspUtil.h"
#include <vector>

namespace rowen::frag
{

class AtmosphereFx
{
public:
    void prepare (double sampleRate, int maxBlock)
    {
        sr = sampleRate;
        const float scale = float (sampleRate / 44100.0);
        static constexpr int combTunings[kCombs] = { 1187, 1361, 1523, 1699 };
        static constexpr int apTunings[kAllpasses] = { 225, 556 };
        static constexpr int spread = 19;

        for (int c = 0; c < 2; ++c)
        {
            const int off = c == 0 ? 0 : spread;
            for (int i = 0; i < kCombs; ++i)
            {
                combBuf[c][i].assign (size_t (int (float (combTunings[i] + off) * scale)), 0.0f);
                combIdx[c][i] = 0; combStore[c][i] = 0.0f;
            }
            for (int i = 0; i < kAllpasses; ++i)
            {
                apBuf[c][i].assign (size_t (int (float (apTunings[i] + off) * scale)), 0.0f);
                apIdx[c][i] = 0;
            }
            softState[c] = 0.0f;
        }

        send.prepare (sampleRate, 60.0f);
        send.setImmediate (0.0f);
        (void) maxBlock;
        setAmount (0.0f, 0.2f, 3500.0f);
    }

    void reset()
    {
        for (int c = 0; c < 2; ++c)
        {
            for (int i = 0; i < kCombs; ++i)
            { std::fill (combBuf[c][i].begin(), combBuf[c][i].end(), 0.0f); combStore[c][i] = 0.0f; }
            for (int i = 0; i < kAllpasses; ++i)
                std::fill (apBuf[c][i].begin(), apBuf[c][i].end(), 0.0f);
            softState[c] = 0.0f;
        }
    }

    void setAmount (float atmosphere01, float feedbackParam01, float toneHz)
    {
        amount = clampf (atmosphere01, 0.0f, 1.0f);
        // Perceptual taper on the send; decay grows with Atmosphere and the
        // hidden feedback parameter, hard-capped well below instability.
        send.setTarget (std::pow (amount, 1.5f));
        combFeedback = clampf (0.55f + 0.28f * amount + 0.25f * feedbackParam01, 0.0f, 0.88f);
        damp = clampf (1.0f - toneHz / 9000.0f, 0.15f, 0.85f);

        // HF softening of the whole wet path: 12 kHz -> 6.5 kHz with Atmosphere.
        const float cutoff = 12000.0f - 5500.0f * amount;
        softCoef = 1.0f - std::exp (-2.0f * kPi * cutoff / float (sr));
    }

    // In-place on the wet buffers.
    void process (float* wetL, float* wetR, int numSamples)
    {
        float* wet[2] = { wetL, wetR };

        for (int i = 0; i < numSamples; ++i)
        {
            const float s = send.next();
            const float input = (wetL[i] + wetR[i]) * 0.5f * 0.030f; // network input scaling

            for (int c = 0; c < 2; ++c)
            {
                float acc = 0.0f;
                for (int k = 0; k < kCombs; ++k)
                {
                    auto& buf = combBuf[c][k];
                    size_t& idx = combIdx[c][k];
                    const float out = buf[idx];
                    combStore[c][k] = undenorm (out * (1.0f - damp) + combStore[c][k] * damp);
                    buf[idx] = undenorm (input + combStore[c][k] * combFeedback);
                    if (++idx >= buf.size()) idx = 0;
                    acc += out;
                }
                for (int k = 0; k < kAllpasses; ++k)
                {
                    auto& buf = apBuf[c][k];
                    size_t& idx = apIdx[c][k];
                    const float bufOut = buf[idx];
                    buf[idx] = undenorm (acc + bufOut * 0.5f);
                    if (++idx >= buf.size()) idx = 0;
                    acc = bufOut - acc;
                }

                // Blend diffusion in, then soften highs across the whole wet.
                float v = wet[c][i] * (1.0f - 0.40f * s) + acc * (1.35f * s);
                softState[c] += softCoef * (v - softState[c]);
                wet[c][i] = undenorm (softState[c]);
            }
        }
    }

    bool isActive() const { return amount > 0.001f || send.value() > 0.001f || send.moving(); }

private:
    static constexpr int kCombs = 4;
    static constexpr int kAllpasses = 2;

    double sr { 48000.0 };
    std::vector<float> combBuf[2][kCombs];
    size_t combIdx[2][kCombs] {};
    float combStore[2][kCombs] {};
    std::vector<float> apBuf[2][kAllpasses];
    size_t apIdx[2][kAllpasses] {};
    float softState[2] {};
    float amount { 0.0f }, combFeedback { 0.6f }, damp { 0.5f }, softCoef { 1.0f };
    LinearSmoother send;
};

} // namespace rowen::frag

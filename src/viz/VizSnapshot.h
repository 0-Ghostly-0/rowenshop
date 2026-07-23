// Rowen Fragment — lock-free processor→UI visualization transfer.
// A classic seqlock: the audio thread publishes a small plain struct; the UI
// thread copies it and retries on the (rare) torn read. No locks, no
// allocation, no waiting on either side.
#pragma once

#include <atomic>
#include <cstring>

namespace rowen::frag
{

struct GrainMarker
{
    float position01 { 0.0f };   // where in the window (0 = oldest, 1 = newest)
    float age01 { 0.0f };        // envelope progress
    signed char direction { 1 }; // +1 forward, -1 reversed
    unsigned char engine { 0 };  // 0 = A, 1 = B
    unsigned char active { 0 };
};

struct VizData
{
    static constexpr int kBins = 256;
    static constexpr int kMaxMarkers = 16;

    float peaks[kBins] {};
    GrainMarker markers[kMaxMarkers] {};
    float writePos01 { 1.0f };   // display-space position of the newest audio
    float windowSeconds { 1.0f };
    float inputPeak { 0.0f };
    float outputPeak { 0.0f };
    unsigned char frozen { 0 };
    unsigned char playing { 0 };
};

class VizExchange
{
public:
    // Audio thread. Never blocks.
    void publish (const VizData& d) noexcept
    {
        seq.fetch_add (1, std::memory_order_acq_rel);      // -> odd: writing
        std::memcpy (&slot, &d, sizeof (VizData));
        seq.fetch_add (1, std::memory_order_acq_rel);      // -> even: stable
    }

    // UI thread. Returns false if no consistent snapshot could be taken
    // (caller just keeps the previous frame).
    bool read (VizData& out) const noexcept
    {
        for (int attempt = 0; attempt < 4; ++attempt)
        {
            const unsigned s1 = seq.load (std::memory_order_acquire);
            if (s1 & 1u)
                continue;
            std::memcpy (&out, &slot, sizeof (VizData));
            const unsigned s2 = seq.load (std::memory_order_acquire);
            if (s1 == s2)
                return true;
        }
        return false;
    }

private:
    std::atomic<unsigned> seq { 0 };
    VizData slot;
};

} // namespace rowen::frag

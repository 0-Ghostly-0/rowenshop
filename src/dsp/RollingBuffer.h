// Rowen Fragment — the rolling capture buffer. The heart of the plugin.
//
// Design (see docs/BUFFER.md for the full explanation):
// - One circular buffer per channel, preallocated in prepare() for the WORST
//   case (16 s), never resized afterwards. Nothing here allocates on the
//   audio thread.
// - The "window" (how far back readers may look) is what tempo sync changes.
//   Changing it never touches audio data — it only re-scopes readers — and the
//   value glides over ~80 ms so grain positions never jump.
// - Freeze is a ramped WRITE MIX, not a switch: freezing blends the input into
//   the existing content over ~50 ms and then stops the write head; unfreezing
//   ramps writing back in from the stopped position. Both directions leave a
//   smooth seam in the data, so readers crossing it never click.
// - Readers must stay at least guardSamples() behind the write head; that
//   guard is what makes buffer wrap inaudible.
#pragma once

#include "DspUtil.h"

namespace rowen::frag
{

class RollingBuffer
{
public:
    static constexpr int kMaxChannels = 2;

    void prepare (double sampleRate, int numChannels, double maxSeconds)
    {
        sr = sampleRate;
        channels = clampi (numChannels, 1, kMaxChannels);
        capacity = std::max (1024, int (sampleRate * maxSeconds));

        for (int c = 0; c < kMaxChannels; ++c)
            data[c].assign (size_t (capacity), 0.0f);

        writePos = 0;
        written = 0;
        totalWrittenCount = 0;
        guard = std::max (16, int (sampleRate * 0.030)); // 30 ms guard zone

        writeMix.prepare (sampleRate, 50.0f); // freeze crossfade
        writeMix.setImmediate (1.0f);
        window.prepare (sampleRate, 80.0f);   // window-length glide
        window.setImmediate (float (sampleRate)); // 1 s until told otherwise
        frozen = false;
    }

    void reset()
    {
        for (int c = 0; c < kMaxChannels; ++c)
            std::fill (data[c].begin(), data[c].end(), 0.0f);
        writePos = 0;
        written = 0;
        writeMix.setImmediate (frozen ? 0.0f : 1.0f);
    }

    //==========================================================================
    void setTargetWindowSamples (float samples)
    {
        window.setTarget (clampf (samples, float (guard * 4), float (capacity) * 0.98f));
    }

    void setFrozen (bool shouldFreeze)
    {
        frozen = shouldFreeze;
        writeMix.setTarget (shouldFreeze ? 0.0f : 1.0f);
    }

    bool isFrozen() const noexcept { return frozen; }
    bool isFullyFrozen() const noexcept { return frozen && ! writeMix.moving() && writeMix.value() <= 0.0001f; }

    //==========================================================================
    // Capture one block. Advances the window glide once per call.
    void writeBlock (const float* const* input, int numCh, int numSamples)
    {
        window.nextBlock (numSamples);
        const int chansIn = clampi (numCh, 1, channels);

        for (int i = 0; i < numSamples; ++i)
        {
            const float mix = writeMix.next();
            if (mix <= 0.0001f)
                continue; // fully frozen: head stopped, data untouched

            for (int c = 0; c < channels; ++c)
            {
                const float in = input[c < chansIn ? c : chansIn - 1][i];
                float& slot = data[c][size_t (writePos)];
                slot = undenorm (in * mix + slot * (1.0f - mix));
            }
            if (++writePos >= capacity) writePos = 0;
            if (written < capacity) ++written;
            ++totalWrittenCount;
        }
    }

    //==========================================================================
    // Absolute-position read for grain voices. Positions are expressed on the
    // monotonically increasing totalWritten() timeline (which pauses while
    // frozen), so a triggered grain reads a FIXED piece of audio even though
    // the write head keeps moving. Callers (the scheduler) guarantee the
    // region stays inside [head - window, head - guard]; indices are still
    // wrapped safely here regardless.
    float readAbsolute (int ch, double absolutePosition) const noexcept
    {
        const int c = clampi (ch, 0, channels - 1);
        double ring = std::fmod (absolutePosition, double (capacity));
        if (ring < 0.0) ring += double (capacity);

        const int i1 = int (ring);
        const float frac = float (ring - double (i1));
        const int i0 = wrap (i1 - 1), i2 = wrap (i1 + 1), i3 = wrap (i1 + 2);

        const float y0 = data[c][size_t (i0)], y1 = data[c][size_t (i1)];
        const float y2 = data[c][size_t (i2)], y3 = data[c][size_t (i3)];
        const float a0 = y3 - y2 - y0 + y1;
        const float a1 = y0 - y1 - a0;
        const float a2 = y2 - y0;
        return ((a0 * frac + a1) * frac + a2) * frac + y1;
    }

    // Monotonic sample counter of everything ever written (pauses while
    // frozen). The grain voices' position timeline.
    uint64_t totalWritten() const noexcept { return totalWrittenCount; }

    //==========================================================================
    // Interpolated read at a fractional offset BEHIND the write head.
    // Offsets are clamped into [guard, window] — readers can neither touch the
    // write head nor fall off the back of the captured window.
    float readAt (int ch, float offsetBehindHead) const noexcept
    {
        const int c = clampi (ch, 0, channels - 1);
        const float maxBack = usableWindow();
        const float off = clampf (offsetBehindHead, float (guard), maxBack);

        float pos = float (writePos) - off;
        while (pos < 0.0f) pos += float (capacity);

        const int i1 = int (pos);
        const float frac = pos - float (i1);
        const int i0 = wrap (i1 - 1), i2 = wrap (i1 + 1), i3 = wrap (i1 + 2);

        // 4-point cubic (Catmull-Rom) interpolation.
        const float y0 = data[c][size_t (i0)], y1 = data[c][size_t (i1)];
        const float y2 = data[c][size_t (i2)], y3 = data[c][size_t (i3)];
        const float a0 = y3 - y2 - y0 + y1;
        const float a1 = y0 - y1 - a0;
        const float a2 = y2 - y0;
        return ((a0 * frac + a1) * frac + a2) * frac + y1;
    }

    //==========================================================================
    // Fast peak scan for the visualization (bins across the usable window).
    // Cost: bins * samplesPerBin reads; called at viz rate, not per block.
    void fillPeaks (float* peaks, int bins, int samplesPerBin = 6) const noexcept
    {
        const float win = usableWindow();
        for (int b = 0; b < bins; ++b)
        {
            // bin 0 = oldest audio, last bin = newest (just behind the head)
            const float offStart = win - (win - float (guard)) * (float (b) / float (bins));
            float peak = 0.0f;
            for (int s = 0; s < samplesPerBin; ++s)
            {
                const float off = offStart - (win / float (bins)) * (float (s) / float (samplesPerBin));
                float pos = float (writePos) - clampf (off, float (guard), win);
                while (pos < 0.0f) pos += float (capacity);
                for (int c = 0; c < channels; ++c)
                    peak = std::max (peak, std::fabs (data[c][size_t (int (pos))]));
            }
            peaks[b] = peak;
        }
    }

    //==========================================================================
    float usableWindow() const noexcept
    {
        const float filled = float (std::min (written, capacity));
        return std::min ({ window.value(), filled, float (capacity) * 0.98f });
    }

    int   guardSamples() const noexcept { return guard; }
    int   numChannels()  const noexcept { return channels; }
    int   capacitySamples() const noexcept { return capacity; }
    float windowTargetProgress() const noexcept { return window.value(); }
    double sampleRate() const noexcept { return sr; }

private:
    static int clampi (int v, int lo, int hi) noexcept { return std::min (std::max (v, lo), hi); }
    int wrap (int i) const noexcept
    {
        if (i < 0) return i + capacity;
        if (i >= capacity) return i - capacity;
        return i;
    }

    std::vector<float> data[kMaxChannels];
    double sr { 48000.0 };
    int channels { 2 }, capacity { 0 }, writePos { 0 }, written { 0 }, guard { 1440 };
    uint64_t totalWrittenCount { 0 };
    LinearSmoother writeMix;
    OnePoleGlide window;
    bool frozen { false };
};

} // namespace rowen::frag

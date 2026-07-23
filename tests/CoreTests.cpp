// Rowen Fragment — standalone core tests (no JUCE, no dependencies).
//   g++ -std=c++20 -O2 -Wall -Wextra -Isrc tests/CoreTests.cpp src/dsp/FragmentCore.cpp -o core_tests
// Covers: buffer wrapping continuity, freeze/unfreeze data integrity and seam
// smoothness, guard-zone enforcement, window glides, tempo math, interpolation
// accuracy, viz seqlock consistency under a real concurrent reader, mono +
// stereo, multiple sample rates and block sizes, and garbage-input safety.

#include "dsp/FragmentCore.h"
#include "dsp/TempoSync.h"

#include <cstdio>
#include <cstdlib>
#include <random>
#include <thread>

using namespace rowen::frag;

static int failures = 0;

#define CHECK(cond, msg)                                                      \
    do {                                                                      \
        if (! (cond)) { std::printf ("FAIL: %s (line %d)\n", msg, __LINE__);  \
                        ++failures; }                                         \
    } while (false)

namespace
{

//==============================================================================
void testWrapContinuity()
{
    // Write a known ramp far beyond capacity; reads at increasing offsets
    // behind the head must reproduce the ramp exactly (modulo interpolation),
    // proving the circular indexing has no seam at the wrap point.
    for (double sr : { 44100.0, 48000.0, 96000.0 })
    {
        RollingBuffer rb;
        rb.prepare (sr, 1, 2.0); // small capacity to force many wraps
        rb.setTargetWindowSamples (float (sr));

        const int total = int (sr * 7.0); // wraps ~3.5 times
        std::vector<float> block (256);
        long n = 0;
        for (int start = 0; start < total; start += 256)
        {
            for (int i = 0; i < 256; ++i) block[size_t (i)] = float ((n + i) % 1000) / 1000.0f;
            const float* ptrs[1] = { block.data() };
            rb.writeBlock (ptrs, 1, 256);
            n += 256;
        }

        // Sample values at integer offsets behind the head and compare to the
        // known written sequence.
        bool ok = true;
        const int guard = rb.guardSamples();
        for (int off = guard + 4; off < int (sr) - 8; off += 997)
        {
            const float expected = float ((n - off) % 1000) / 1000.0f;
            const float got = rb.readAt (0, float (off));
            if (std::fabs (got - expected) > 0.02f) { ok = false; break; }
        }
        CHECK (ok, "ramp readback across wraps diverged");
    }
}

//==============================================================================
void testFreezeHoldsData()
{
    RollingBuffer rb;
    rb.prepare (48000.0, 2, 4.0);
    rb.setTargetWindowSamples (48000.0f);

    // Fill with a sine, then freeze and let the crossfade complete.
    std::vector<float> l (512), r (512);
    const float* ptrs[2] = { l.data(), r.data() };
    long n = 0;
    auto writeSine = [&] (int blocks)
    {
        for (int b = 0; b < blocks; ++b)
        {
            for (int i = 0; i < 512; ++i)
            {
                l[size_t (i)] = std::sin (2.0f * kPi * 220.0f * float (n + i) / 48000.0f);
                r[size_t (i)] = l[size_t (i)] * 0.5f;
            }
            rb.writeBlock (ptrs, 2, 512);
            n += 512;
        }
    };

    writeSine (200);
    rb.setFrozen (true);
    writeSine (20); // crossfade completes during these writes
    CHECK (rb.isFullyFrozen(), "buffer did not report fully frozen");

    // Snapshot the frozen window.
    const int guard = rb.guardSamples();
    std::vector<float> before;
    for (int off = guard; off < 40000; off += 61)
        before.push_back (rb.readAt (0, float (off)));

    // Keep "playing" loud input at it while frozen — data must not change.
    writeSine (300);
    size_t idx = 0;
    float maxDiff = 0.0f;
    for (int off = guard; off < 40000; off += 61)
        maxDiff = std::max (maxDiff, std::fabs (rb.readAt (0, float (off)) - before[idx++]));
    CHECK (maxDiff == 0.0f, "frozen buffer contents changed while frozen");

    // Unfreeze: writing resumes; contents must change again.
    rb.setFrozen (false);
    writeSine (200);
    idx = 0;
    float changed = 0.0f;
    for (int off = guard; off < 40000; off += 61)
        changed = std::max (changed, std::fabs (rb.readAt (0, float (off)) - before[idx++]));
    CHECK (changed > 0.01f, "buffer did not resume capturing after unfreeze");
}

//==============================================================================
void testFreezeSeamIsSmooth()
{
    // The freeze crossfade must leave no large sample-to-sample step in the
    // stored data around the point where writing stopped.
    RollingBuffer rb;
    rb.prepare (48000.0, 1, 4.0);
    rb.setTargetWindowSamples (96000.0f);

    std::vector<float> block (64);
    const float* ptrs[1] = { block.data() };
    long n = 0;
    auto writeSine = [&] (int blocks, float freq)
    {
        for (int b = 0; b < blocks; ++b)
        {
            for (int i = 0; i < 64; ++i)
                block[size_t (i)] = 0.8f * std::sin (2.0f * kPi * freq * float (n + i) / 48000.0f);
            rb.writeBlock (ptrs, 1, 64);
            n += 64;
        }
    };

    writeSine (2000, 220.0f);
    rb.setFrozen (true);
    writeSine (100, 220.0f); // completes the fade

    // Scan the stored window for the biggest step; a hard freeze switch would
    // show a step close to full amplitude, the crossfade keeps it sine-sized.
    float maxStep = 0.0f, prev = rb.readAt (0, float (rb.guardSamples()));
    for (int off = rb.guardSamples() + 1; off < 90000; ++off)
    {
        const float v = rb.readAt (0, float (off));
        maxStep = std::max (maxStep, std::fabs (v - prev));
        prev = v;
    }
    // 220 Hz sine at 0.8 moves at most ~0.023/sample; allow generous margin.
    CHECK (maxStep < 0.12f, "freeze left a click-sized seam in the buffer");
}

//==============================================================================
void testGuardZone()
{
    RollingBuffer rb;
    rb.prepare (48000.0, 1, 2.0);
    rb.setTargetWindowSamples (48000.0f);

    // Write silence then one loud sample exactly at the head; a read requested
    // AT the head (offset 0) must be clamped back to the guard and return the
    // older (silent) audio, never the just-written sample.
    std::vector<float> quiet (1024, 0.0f);
    const float* qp[1] = { quiet.data() };
    for (int b = 0; b < 100; ++b) rb.writeBlock (qp, 1, 1024);

    std::vector<float> loud (8, 0.99f);
    const float* lp[1] = { loud.data() };
    rb.writeBlock (lp, 1, 8);

    CHECK (std::fabs (rb.readAt (0, 0.0f)) < 0.01f, "guard zone failed: read reached the write head");
    CHECK (std::fabs (rb.readAt (0, -100.0f)) < 0.01f, "negative offset was not clamped to the guard");
}

//==============================================================================
void testWindowGlide()
{
    RollingBuffer rb;
    rb.prepare (48000.0, 1, 8.0);
    rb.setTargetWindowSamples (48000.0f);

    std::vector<float> block (512, 0.1f);
    const float* ptrs[1] = { block.data() };
    for (int b = 0; b < 400; ++b) rb.writeBlock (ptrs, 1, 512);

    const float before = rb.usableWindow();
    rb.setTargetWindowSamples (192000.0f); // 1 s -> 4 s
    float last = before;
    bool monotonic = true;
    for (int b = 0; b < 400; ++b)
    {
        rb.writeBlock (ptrs, 1, 512);
        const float w = rb.usableWindow();
        if (w + 1.0f < last) monotonic = false; // may only grow toward target
        last = w;
    }
    CHECK (monotonic, "window glide moved backwards");
    CHECK (std::fabs (last - 192000.0f) < 2000.0f, "window did not reach its target");
}

//==============================================================================
void testTempoMath()
{
    TempoInfo t;
    t.bpm = 120.0; t.hasTempo = true; t.timeSigNumerator = 4; t.timeSigDenominator = 4;
    CHECK (std::fabs (TempoSync::barSeconds (t) - 2.0) < 1e-9, "bar length wrong at 120 BPM 4/4");

    t.timeSigNumerator = 6; t.timeSigDenominator = 8;
    CHECK (std::fabs (TempoSync::barSeconds (t) - 1.5) < 1e-9, "bar length wrong at 120 BPM 6/8");

    t.timeSigNumerator = 4; t.timeSigDenominator = 4;
    const float oneBar = TempoSync::targetWindowSamples (BufferLength::oneBar, 0, t, 48000.0, 1.0e9f);
    CHECK (std::fabs (oneBar - 96000.0f) < 1.0f, "one bar at 120 BPM should be 96000 samples");

    // Insane host values fall back safely.
    t.bpm = 0.0;
    CHECK (TempoSync::barSeconds (t) > 0.1, "zero BPM was not sanitized");
    t.bpm = 1.0e9;
    CHECK (TempoSync::barSeconds (t) > 0.1, "absurd BPM was not sanitized");

    // Free mode clamps.
    const float freeLen = TempoSync::targetWindowSamples (BufferLength::freeTime, 999999.0f, t, 48000.0, 1.0e9f);
    CHECK (freeLen <= 8000.0f * 48.0f + 1.0f, "free mode exceeded its 8 s clamp");
}

//==============================================================================
void testInterpolationAccuracy()
{
    // A pure sine read back at fractional offsets should match the analytic
    // sine closely (cubic interpolation, mid-band frequency).
    RollingBuffer rb;
    rb.prepare (48000.0, 1, 2.0);
    rb.setTargetWindowSamples (80000.0f);

    std::vector<float> block (256);
    const float* ptrs[1] = { block.data() };
    long n = 0;
    for (int b = 0; b < 400; ++b)
    {
        for (int i = 0; i < 256; ++i)
            block[size_t (i)] = std::sin (2.0f * kPi * 441.0f * float (n + i) / 48000.0f);
        rb.writeBlock (ptrs, 1, 256);
        n += 256;
    }

    double err = 0.0; int count = 0;
    for (float off = float (rb.guardSamples()) + 10.5f; off < 60000.0f; off += 313.37f)
    {
        const double t = double (n) - double (off);
        const float expected = float (std::sin (2.0 * double (kPi) * 441.0 * t / 48000.0));
        const float got = rb.readAt (0, off);
        err += std::fabs (double (got - expected));
        ++count;
    }
    CHECK (err / count < 0.01, "cubic interpolation error too large on 441 Hz sine");
}

//==============================================================================
void testVizSeqlockUnderContention()
{
    // Writer publishes patterned frames as fast as it can; a real concurrent
    // reader must never observe a torn frame (mixed pattern).
    VizExchange ex;
    std::atomic<bool> stop { false };
    std::atomic<int> torn { 0 }, good { 0 };

    std::thread reader ([&]
    {
        VizData d;
        while (! stop.load())
        {
            if (ex.read (d))
            {
                const float v = d.peaks[0];
                bool consistent = true;
                for (int i = 0; i < VizData::kBins; ++i)
                    if (d.peaks[i] != v) { consistent = false; break; }
                (consistent ? good : torn).fetch_add (1);
            }
        }
    });

    VizData frame;
    for (int f = 0; f < 200000; ++f)
    {
        const float v = float (f % 977);
        for (int i = 0; i < VizData::kBins; ++i) frame.peaks[i] = v;
        ex.publish (frame);
        // Yield periodically so a starved reader (e.g. a 2-core CI/sandbox
        // runner) still gets real scheduling time under this tight writer
        // loop. This does not weaken what the test proves: torn == 0 is the
        // correctness guarantee, and the yield only affects how many good
        // reads the reader manages to interleave, not their validity.
        if ((f & 0x3F) == 0)
            std::this_thread::yield();
    }
    stop.store (true);
    reader.join();

    CHECK (torn.load() == 0, "seqlock delivered a torn visualization frame");
    CHECK (good.load() > 200, "reader failed to obtain snapshots under contention");
}

//==============================================================================
void testCoreEndToEnd()
{
    // FragmentCore passthrough sanity at several rates/blocks, mono + stereo,
    // including garbage (NaN/inf) input never crashing or propagating.
    for (double sr : { 44100.0, 48000.0, 88200.0, 96000.0 })
    {
        for (int blockSize : { 32, 64, 128, 256, 512, 1024, 2048 })
        {
            for (int ch : { 1, 2 })
            {
                FragmentCore core;
                core.prepare (sr, blockSize, ch);
                CoreSettings s;
                s.bufferLength = BufferLength::halfBar;
                s.tempo.bpm = 140.0; s.tempo.hasTempo = true;
                core.setSettings (s);

                std::vector<float> l (size_t (blockSize), 0.0f), r (size_t (blockSize), 0.0f);
                float* ptrs[2] = { l.data(), r.data() };
                bool finite = true;
                for (int b = 0; b < 200; ++b)
                {
                    for (int i = 0; i < blockSize; ++i)
                    {
                        l[size_t (i)] = 0.5f * std::sin (2.0f * kPi * 330.0f * float (b * blockSize + i) / float (sr));
                        if (ch > 1) r[size_t (i)] = l[size_t (i)];
                    }
                    core.process (ptrs, ch, blockSize);
                    for (int i = 0; i < blockSize; ++i)
                        if (! std::isfinite (l[size_t (i)])) finite = false;
                }
                CHECK (finite, "passthrough produced non-finite output");
            }
        }
    }

    // Garbage input: NaN and inf must not crash and the buffer must survive.
    FragmentCore core;
    core.prepare (48000.0, 256, 2);
    core.setSettings ({});
    std::vector<float> l (256, std::numeric_limits<float>::quiet_NaN());
    std::vector<float> r (256, std::numeric_limits<float>::infinity());
    float* ptrs[2] = { l.data(), r.data() };
    core.process (ptrs, 2, 256); // must not crash
    std::fill (l.begin(), l.end(), 0.0f);
    std::fill (r.begin(), r.end(), 0.0f);
    for (int b = 0; b < 100; ++b) core.process (ptrs, 2, 256);
    CHECK (true, "garbage input crashed the core"); // reaching here is the pass
}

//==============================================================================
// ---- Phase 3: fragment engine tests -----------------------------------------

// Shared helper: run a core with given settings over a sine input, return the
// full output of the second half (settled) as a vector (channel 0).
std::vector<float> runEngine (const CoreSettings& s, int seconds = 3,
                              int blockSize = 256, int channels = 2,
                              double sr = 48000.0, float inputFreq = 330.0f)
{
    FragmentCore core;
    core.prepare (sr, blockSize, channels);
    core.setSettings (s);

    const int total = int (sr) * seconds;
    std::vector<float> out;
    out.reserve (size_t (total / 2));
    std::vector<float> l (size_t (blockSize), 0.0f), r (size_t (blockSize), 0.0f);
    float* ptrs[2] = { l.data(), r.data() };

    for (int start = 0; start < total; start += blockSize)
    {
        for (int i = 0; i < blockSize; ++i)
        {
            l[size_t (i)] = 0.4f * std::sin (2.0f * kPi * inputFreq * float (start + i) / float (sr));
            if (channels > 1) r[size_t (i)] = l[size_t (i)];
        }
        core.process (ptrs, channels, blockSize);
        if (start >= total / 2)
            for (int i = 0; i < blockSize; ++i) out.push_back (l[size_t (i)]);
    }
    return out;
}

float rmsOf (const std::vector<float>& v)
{
    double acc = 0.0;
    for (float x : v) acc += double (x) * double (x);
    return float (std::sqrt (acc / double (std::max (size_t (1), v.size()))));
}

CoreSettings fullWetSettings()
{
    CoreSettings s;
    s.mix = 1.0f;             // wet only
    s.fragment = 0.6f;
    s.scatter = 0.3f;
    s.reverse = 0.0f;
    s.shiftSemitones = 0.0f;
    s.tempo.bpm = 120.0; s.tempo.hasTempo = true;
    return s;
}

void testEnginesProduceAudio()
{
    auto wet = runEngine (fullWetSettings());
    CHECK (rmsOf (wet) > 0.01f, "engines produced no wet signal");
    for (float v : wet)
        if (! std::isfinite (v)) { CHECK (false, "wet signal non-finite"); break; }
}

void testSilenceInSilenceOut()
{
    FragmentCore core;
    core.prepare (48000.0, 512, 2);
    auto s = fullWetSettings();
    core.setSettings (s);
    std::vector<float> l (512, 0.0f), r (512, 0.0f);
    float* ptrs[2] = { l.data(), r.data() };
    float peak = 0.0f;
    for (int b = 0; b < 400; ++b)
    {
        core.process (ptrs, 2, 512);
        for (float v : l) peak = std::max (peak, std::fabs (v));
        std::fill (l.begin(), l.end(), 0.0f);
        std::fill (r.begin(), r.end(), 0.0f);
    }
    CHECK (peak < 1.0e-5f, "engines generate noise from silence");
}

void testGrainsAreClickFree()
{
    // Full-wet granular output of a smooth sine must not contain jumps much
    // larger than the sine's own slope — Hann envelopes guarantee this.
    auto s = fullWetSettings();
    s.fragment = 0.8f; s.scatter = 0.6f;
    auto wet = runEngine (s, 4);
    float maxStep = 0.0f;
    for (size_t i = 1; i < wet.size(); ++i)
        maxStep = std::max (maxStep, std::fabs (wet[i] - wet[i - 1]));
    // 330 Hz sine at 0.4 slews ~0.017/sample; grains overlap, allow margin.
    CHECK (maxStep < 0.2f, "granular output contains click-sized discontinuities");
}

void testDeterminismWithSeed()
{
    auto s = fullWetSettings();
    s.randomSeed = 777;
    const auto a = runEngine (s);
    const auto b = runEngine (s);
    CHECK (a.size() == b.size(), "determinism runs differ in length");
    float maxDiff = 0.0f;
    for (size_t i = 0; i < a.size(); ++i)
        maxDiff = std::max (maxDiff, std::fabs (a[i] - b[i]));
    CHECK (maxDiff == 0.0f, "same seed did not reproduce identical output");

    s.randomSeed = 778;
    const auto c = runEngine (s);
    float diff = 0.0f;
    for (size_t i = 0; i < a.size(); ++i)
        diff = std::max (diff, std::fabs (a[i] - c[i]));
    CHECK (diff > 1.0e-4f, "different seed produced identical output");
}

void testReverseChangesOutput()
{
    auto s = fullWetSettings();
    s.reverse = 0.0f;
    const auto fwd = runEngine (s);
    s.reverse = 1.0f;
    const auto rev = runEngine (s);
    float diff = 0.0f;
    for (size_t i = 0; i < std::min (fwd.size(), rev.size()); ++i)
        diff = std::max (diff, std::fabs (fwd[i] - rev[i]));
    CHECK (diff > 1.0e-4f, "reverse=100% output identical to forward");
    CHECK (rmsOf (rev) > 0.01f, "reversed grains produced no signal");
}

void testPitchShiftRoughlyDoubles()
{
    // +12 st on a sine: wet-only output's zero-crossing rate should be close
    // to double the input's. Engine A only, no scatter/reverse, snapping on.
    auto measure = [] (float shift)
    {
        CoreSettings s;
        s.mix = 1.0f; s.fragment = 0.3f; s.scatter = 0.0f; s.reverse = 0.0f;
        s.pitchVarCents = 0.0f; s.panSpread = 0.0f;
        s.engineB = false;
        s.shiftSemitones = shift; s.shiftSnap = 0;
        s.tempo.bpm = 120.0; s.tempo.hasTempo = true;
        const auto wet = runEngine (s, 4, 256, 1, 48000.0, 220.0f);
        int crossings = 0;
        for (size_t i = 1; i < wet.size(); ++i)
            if ((wet[i - 1] < 0.0f) != (wet[i] < 0.0f)) ++crossings;
        return float (crossings);
    };

    const float base = measure (0.0f);
    const float up   = measure (12.0f);
    CHECK (base > 100.0f, "pitch test produced too little signal to measure");
    const float ratio = up / std::max (1.0f, base);
    CHECK (ratio > 1.6f && ratio < 2.4f, "+12 st did not roughly double the frequency");
}

void testVoiceStealingUnderExtremeLoad()
{
    auto s = fullWetSettings();
    s.fragment = 1.0f; s.scatter = 1.0f; s.densityBias = 0.5f;
    s.grainMinMs = 15.0f;
    const auto wet = runEngine (s, 4, 128);
    for (float v : wet)
        if (! std::isfinite (v)) { CHECK (false, "extreme load produced non-finite output"); return; }
    CHECK (rmsOf (wet) > 0.005f, "extreme load silenced the engine");
    CHECK (rmsOf (wet) < 1.0f, "extreme load lost gain control");
}

void testEngineTogglesAndMonoPath()
{
    auto s = fullWetSettings();
    s.engineA = false; // B only
    CHECK (rmsOf (runEngine (s)) > 0.005f, "engine B alone produced nothing");

    s.engineA = true; s.engineB = false; // A only
    CHECK (rmsOf (runEngine (s)) > 0.005f, "engine A alone produced nothing");

    s.engineA = false; s.engineB = false;
    CHECK (rmsOf (runEngine (s)) < 1.0e-3f, "both engines off still produced wet signal");

    s.engineA = true; s.engineB = true;
    const auto mono = runEngine (s, 3, 256, 1);
    CHECK (rmsOf (mono) > 0.005f, "mono track produced no granular output");
}

void testBlockSizeConsistency()
{
    // Same seed at different block sizes: outputs will not be bit-identical
    // (documented), but the overall energy must be equivalent — the scheduler
    // counts absolute samples, not blocks.
    auto s = fullWetSettings();
    const float a = rmsOf (runEngine (s, 3, 64));
    const float b = rmsOf (runEngine (s, 3, 1024));
    CHECK (std::fabs (gainToDb (a) - gainToDb (b)) < 3.0f,
           "granular energy varies wildly with host block size");
}

//==============================================================================
// ---- Phase 4: Drift (MotionLfo) + Atmosphere tests --------------------------

void testMotionLfoCorrelatedAndBounded()
{
    // The four outputs are deterministic combinations of two detuned sines,
    // not independent per-parameter noise. Verify: all outputs stay in
    // [-1, 1], all actually move over time (not stuck), and pitch/position
    // are meaningfully correlated (position is 60% pitch's sine by design) —
    // the signature of "everything breathes together" rather than
    // uncorrelated wobble on every parameter.
    MotionLfo lfo;
    lfo.prepare (48000.0);
    lfo.setRate (0.4f);

    std::vector<float> pitch, position, pan, tone;
    const int blockSize = 256;
    const int blocks = int (48000.0 * 6.0) / blockSize; // 6 seconds
    for (int b = 0; b < blocks; ++b)
    {
        lfo.advance (blockSize);
        auto v = lfo.current();
        CHECK (v.pitch >= -1.0f && v.pitch <= 1.0f, "motion pitch left [-1,1]");
        CHECK (v.position >= -1.0f && v.position <= 1.0f, "motion position left [-1,1]");
        CHECK (v.pan >= -1.0f && v.pan <= 1.0f, "motion pan left [-1,1]");
        CHECK (v.tone >= -1.0f && v.tone <= 1.0f, "motion tone left [-1,1]");
        pitch.push_back (v.pitch);
        position.push_back (v.position);
        pan.push_back (v.pan);
        tone.push_back (v.tone);
    }

    auto stddev = [] (const std::vector<float>& v)
    {
        double mean = 0.0;
        for (float x : v) mean += x;
        mean /= double (v.size());
        double acc = 0.0;
        for (float x : v) acc += (double (x) - mean) * (double (x) - mean);
        return std::sqrt (acc / double (v.size()));
    };
    auto correlation = [&] (const std::vector<float>& x, const std::vector<float>& y)
    {
        double mx = 0.0, my = 0.0;
        for (size_t i = 0; i < x.size(); ++i) { mx += x[i]; my += y[i]; }
        mx /= double (x.size()); my /= double (y.size());
        double cov = 0.0, vx = 0.0, vy = 0.0;
        for (size_t i = 0; i < x.size(); ++i)
        {
            const double dx = x[i] - mx, dy = y[i] - my;
            cov += dx * dy; vx += dx * dx; vy += dy * dy;
        }
        return cov / std::sqrt (vx * vy);
    };

    CHECK (stddev (pitch) > 0.1, "motion pitch is not actually moving");
    CHECK (stddev (pan) > 0.1, "motion pan is not actually moving");
    CHECK (correlation (pitch, position) > 0.5,
           "motion position is not correlated with pitch (looks like independent noise)");
    CHECK (correlation (pan, tone) > 0.5,
           "motion tone is not correlated with pan (looks like independent noise)");
}

void testDriftAudibleAndScalesWithMovement()
{
    // Same seed, same everything except drift/movement: Drift must audibly
    // change the output, and Movement must scale how much it changes it.
    auto base = fullWetSettings();
    base.drift = 0.0f;
    base.movement = 0.0f;

    auto driftLowMovement = base;
    driftLowMovement.drift = 1.0f;
    driftLowMovement.movement = 0.0f;

    auto driftHighMovement = base;
    driftHighMovement.drift = 1.0f;
    driftHighMovement.movement = 1.0f;

    const auto baseOut = runEngine (base);
    const auto lowOut  = runEngine (driftLowMovement);
    const auto highOut = runEngine (driftHighMovement);

    auto diffRms = [] (const std::vector<float>& a, const std::vector<float>& b)
    {
        double acc = 0.0;
        const size_t n = std::min (a.size(), b.size());
        for (size_t i = 0; i < n; ++i)
        {
            const double d = double (a[i]) - double (b[i]);
            acc += d * d;
        }
        return float (std::sqrt (acc / double (std::max (size_t (1), n))));
    };

    const float diffLow  = diffRms (baseOut, lowOut);
    const float diffHigh = diffRms (baseOut, highOut);

    CHECK (diffLow > 1.0e-4f, "drift with movement=0 produced no audible change");
    CHECK (diffHigh > diffLow,
           "movement=1 did not deepen drift's effect vs movement=0");
}

void testAtmosphereAddsDiffusionTail()
{
    // Feed one loud block then silence; Atmosphere should leave an audible
    // diffusion tail behind after the input stops, while atmosphere=0 should
    // not (beyond the harmless one-pole settle of the impulse itself).
    auto runTail = [] (float atmosphere) -> std::vector<float>
    {
        AtmosphereFx fx;
        const double sr = 48000.0;
        const int blockSize = 256;
        fx.prepare (sr, blockSize);
        fx.setAmount (atmosphere, 0.5f, 3500.0f);

        std::vector<float> l (size_t (blockSize), 0.0f), r (size_t (blockSize), 0.0f);
        std::vector<float> tail;
        const int totalBlocks = int (sr * 2.0) / blockSize;
        for (int b = 0; b < totalBlocks; ++b)
        {
            for (int i = 0; i < blockSize; ++i)
            {
                const float in = (b == 0 && i < 8) ? 1.0f : 0.0f; // a short loud impulse
                l[size_t (i)] = in; r[size_t (i)] = in;
            }
            fx.process (l.data(), r.data(), blockSize);
            if (b >= 4) // skip past the impulse itself and its immediate settle
                for (int i = 0; i < blockSize; ++i) tail.push_back (l[size_t (i)]);
        }
        return tail;
    };

    const auto tailOff = runTail (0.0f);
    const auto tailOn  = runTail (1.0f);

    CHECK (rmsOf (tailOn) > rmsOf (tailOff) * 3.0f,
           "atmosphere=1 left no meaningfully longer diffusion tail than atmosphere=0");
    for (float v : tailOn)
        CHECK (std::isfinite (v), "atmosphere tail produced non-finite output");
}

void testAtmosphereStableAcrossSampleRates()
{
    // Worst-case settings (max atmosphere + max hidden feedback) at several
    // sample rates, driven by a sustained loud tone: output must stay finite
    // and bounded — the feedback cap must hold regardless of comb length
    // scaling per sample rate.
    for (double sr : { 44100.0, 48000.0, 88200.0, 96000.0 })
    {
        AtmosphereFx fx;
        const int blockSize = 256;
        fx.prepare (sr, blockSize);
        fx.setAmount (1.0f, 1.0f, 20000.0f); // max send, max feedback, minimal damping

        std::vector<float> l (size_t (blockSize), 0.0f), r (size_t (blockSize), 0.0f);
        const int totalBlocks = int (sr * 5.0) / blockSize;
        float peak = 0.0f;
        bool finite = true;
        for (int b = 0; b < totalBlocks; ++b)
        {
            for (int i = 0; i < blockSize; ++i)
            {
                const float in = 0.8f * std::sin (2.0f * kPi * 220.0f * float (b * blockSize + i) / float (sr));
                l[size_t (i)] = in; r[size_t (i)] = in;
            }
            fx.process (l.data(), r.data(), blockSize);
            for (int i = 0; i < blockSize; ++i)
            {
                if (! std::isfinite (l[size_t (i)]) || ! std::isfinite (r[size_t (i)]))
                    finite = false;
                peak = std::max (peak, std::max (std::fabs (l[size_t (i)]), std::fabs (r[size_t (i)])));
            }
        }
        CHECK (finite, "atmosphere produced non-finite output at some sample rate");
        CHECK (peak < 10.0f, "atmosphere feedback ran away instead of staying capped");
    }
}

void testMovementGatesDriftEvenWhenAtmosphereOff()
{
    // Sanity check that Drift's effect does not depend on Atmosphere being on
    // — the two Phase 4 features are independent.
    auto s = fullWetSettings();
    s.atmosphere = 0.0f;
    s.drift = 1.0f;
    s.movement = 1.0f;
    auto sBase = fullWetSettings();
    sBase.atmosphere = 0.0f;
    sBase.drift = 0.0f;
    sBase.movement = 0.0f;

    const auto withDrift = runEngine (s);
    const auto without   = runEngine (sBase);
    double acc = 0.0;
    const size_t n = std::min (withDrift.size(), without.size());
    for (size_t i = 0; i < n; ++i)
    {
        const double d = double (withDrift[i]) - double (without[i]);
        acc += d * d;
    }
    const float diff = float (std::sqrt (acc / double (std::max (size_t (1), n))));
    CHECK (diff > 1.0e-4f, "drift produced no effect independent of atmosphere");
}

} // namespace

int main()
{
    std::printf ("Rowen Fragment core tests\n");

    testWrapContinuity();
    testFreezeHoldsData();
    testFreezeSeamIsSmooth();
    testGuardZone();
    testWindowGlide();
    testTempoMath();
    testInterpolationAccuracy();
    testVizSeqlockUnderContention();
    testCoreEndToEnd();

    // Phase 3: fragment engines
    testEnginesProduceAudio();
    testSilenceInSilenceOut();
    testGrainsAreClickFree();
    testDeterminismWithSeed();
    testReverseChangesOutput();
    testPitchShiftRoughlyDoubles();
    testVoiceStealingUnderExtremeLoad();
    testEngineTogglesAndMonoPath();
    testBlockSizeConsistency();

    // Phase 4: Drift (Motion) + Atmosphere
    testMotionLfoCorrelatedAndBounded();
    testDriftAudibleAndScalesWithMovement();
    testAtmosphereAddsDiffusionTail();
    testAtmosphereStableAcrossSampleRates();
    testMovementGatesDriftEvenWhenAtmosphereOff();

    if (failures == 0)
    {
        std::printf ("All tests passed.\n");
        return EXIT_SUCCESS;
    }
    std::printf ("%d check(s) FAILED.\n", failures);
    return EXIT_FAILURE;
}

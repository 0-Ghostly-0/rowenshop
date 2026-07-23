// Rowen Fragment — plain settings struct handed from the parameter layer to
// the engine once per block. Framework-free. Phase 2 uses the buffer-related
// fields; the creative fields are already defined so parameter IDs, state and
// automation are frozen from the first build (see docs/PARAMETERS.md).
#pragma once

#include "TempoSync.h"

namespace rowen::frag
{

struct CoreSettings
{
    // Creative controls, normalised 0..1 (wired to the engines in Phase 3/4)
    float fragment  { 0.40f };
    float shiftSemitones { 0.0f };  // -12..+12 (or -24..+24 extended)
    int   shiftSnap { 0 };          // 0 semitones, 1 fifths, 2 octaves, 3 off
    bool  shiftExtended { false };
    float reverse   { 0.25f };
    float scatter   { 0.30f };
    float drift     { 0.20f };
    float atmosphere{ 0.25f };
    float movement  { 0.35f };

    // Mix / output
    float mix       { 0.40f };      // 0..1, equal-power
    float outGainDb { 0.0f };

    // Buffer
    BufferLength bufferLength { BufferLength::oneBar };
    float bufferFreeMs { 2000.0f };
    bool  freeze { false };

    // Engines + global
    bool engineA { true };
    bool engineB { true };
    bool bypassed { false };

    // Advanced (hidden drawer; defaults are the tuned voice)
    float densityBias  { 0.0f };     // -0.5..+0.5 (fraction)
    float grainMinMs   { 30.0f };
    float grainMaxMs   { 400.0f };
    float posSpread    { 0.50f };    // 0..1
    float panSpread    { 0.60f };    // 0..1
    float pitchVarCents{ 20.0f };    // 0..100
    float feedback     { 0.20f };    // 0..0.6 (Phase 4)
    float fbToneHz     { 3500.0f };  // (Phase 4)
    float motionRateHz { 0.4f };     // (Phase 4)
    int   pulseDivIndex{ 2 };        // (Phase 4)

    // Reproducibility
    uint32_t randomSeed { 20260722u };

    // Host state (filled by the processor every block)
    TempoInfo tempo;
};

} // namespace rowen::frag

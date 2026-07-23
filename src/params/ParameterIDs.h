// Rowen Fragment — stable parameter identifiers.
//
// ⚠ COMPATIBILITY CONTRACT (same rule as Vocal Polish): these IDs are written
// into every saved DAW session and preset. NEVER rename or reuse one after
// release. The FULL set — including controls that only become audible in
// Phase 3/4 — is frozen now so sessions saved with any build stay valid.
#pragma once

namespace rowen::fragparam
{

inline constexpr int kVersionHint  = 1;
inline constexpr int kStateVersion = 1;

namespace id
{
    // Main creative controls
    inline constexpr const char* fragment   = "fragment";
    inline constexpr const char* shift      = "shift";        // semitones, ±24 allocated
    inline constexpr const char* shiftSnap  = "shift_snap";   // Semitones/Fifths/Octaves/Off
    inline constexpr const char* shiftExt   = "shift_ext";    // UI range ±12 -> ±24
    inline constexpr const char* reverse    = "reverse";
    inline constexpr const char* scatter    = "scatter";
    inline constexpr const char* drift      = "drift";
    inline constexpr const char* atmosphere = "atmosphere";
    inline constexpr const char* movement   = "movement";

    // Mix / output
    inline constexpr const char* mix        = "mix";
    inline constexpr const char* outGain    = "out_gain";

    // Buffer
    inline constexpr const char* bufferLen  = "buffer_len";
    inline constexpr const char* bufferFree = "buffer_free";
    inline constexpr const char* freeze     = "freeze";

    // Engines + global
    inline constexpr const char* engineA    = "engine_a";
    inline constexpr const char* engineB    = "engine_b";
    inline constexpr const char* bypass     = "bypass";

    // Advanced (hidden drawer; wired in Phase 3/4)
    inline constexpr const char* densityBias = "density_bias";
    inline constexpr const char* grainMin    = "grain_min";
    inline constexpr const char* grainMax    = "grain_max";
    inline constexpr const char* posSpread   = "pos_spread";
    inline constexpr const char* panSpread   = "pan_spread";
    inline constexpr const char* pitchVar    = "pitch_var";
    inline constexpr const char* feedback    = "feedback";
    inline constexpr const char* fbTone      = "fb_tone";
    inline constexpr const char* motionRate  = "motion_rate";
    inline constexpr const char* pulseDiv    = "pulse_div";
} // namespace id

// Non-parameter state properties (saved in the APVTS tree, not automatable):
namespace prop
{
    inline constexpr const char* randomSeed   = "randomSeed";
    inline constexpr const char* stateVersion = "stateVersion";
}

} // namespace rowen::fragparam

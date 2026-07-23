# Rowen Fragment

*Turn Any Sound Into Something New.*

A real-time creative effect that continuously captures the last stretch of
audio on the track and rebuilds it as moving fragments — repeated, reversed,
re-pitched, scattered, and pushed into atmosphere. The second plugin in the
Rowen line, built on the same design system, engine architecture, and build
pipeline as Rowen Vocal Polish.

**Version:** 0.5.0 (Phase 5 — presets, favorites, and undo/redo)
**Format:** VST3 (64-bit Windows first; code is cross-platform, AU later)

---

## Current status — read this first

| Layer | State |
|---|---|
| Rolling buffer core (`src/dsp/`, `src/viz/`) | ✅ Complete and **machine-verified**: circular capture with wrap-safe reads, click-free freeze crossfades, guard zone, tempo-synced window with 80 ms glide, cubic interpolation (tested against analytic sine), lock-free viz exchange (clean under ThreadSanitizer contention testing), GCC 13 + ASan/UBSan/TSan all green. |
| Parameters (`src/params/`) | ✅ **Complete frozen set** — all 27 parameters, so session compatibility holds from the first build. |
| Fragment engines (`src/dsp/`) | ✅ **Machine-verified**: 48-voice preallocated pool (A≤16 stable / B≤32 textural), seeded deterministic schedulers, Hann envelopes (click-tested), reverse playback, varispeed pitch with snap (verified +12 st ≈ 2× frequency), scatter with head-safe position math, equal-power pan and dry/wet, voice stealing under extreme load, mono path. ASan/UBSan/TSan clean. |
| Drift + Atmosphere (`src/dsp/MotionLfo.h`, `src/dsp/AtmosphereFx.h`) | ✅ **Machine-verified**: Drift is a two-sine correlated modulator (pitch/position/pan move together, not independent noise) whose depth is scaled by Movement; Atmosphere is a Freeverb-topology diffusion + HF-softening block on the wet path only, with a hard-capped feedback (≤0.88) proven stable across 44.1–96 kHz under sustained worst-case input. |
| Branded interface (`src/core/PluginEditor.*`, `src/ui/`) | ✅ Written and API-reviewed: full Rowen theme/typography/look-and-feel reused verbatim from Vocal Polish (same fonts: Space Grotesk, Poppins, JetBrains Mono), plus the BufferDisplay centerpiece — a fragmented block waveform with animated grain-spark markers. Every control is exposed: Fragment, Shift (+ snap/extended), Reverse, Scatter, Drift, Atmosphere, Movement, Mix, Output, Freeze, Randomize, engine A/B toggles, buffer length. |
| Presets, favorites, undo (`src/presets/`) | ✅ Written and API-reviewed, new in Phase 5: 33 factory presets across 6 categories (Subtle, Melodic, Rhythmic, Vocal, Drums, Experimental), a category-grouped + Favorites-shortcut preset browser, user preset save/rename/delete, and a single undoable action for both Randomize and preset loads (Randomize also has a right-click Subtle/Balanced/Extreme strength menu). **Not yet compiled against JUCE** — first compile happens on the repo's CI, same as always. |

## What works in this build (once compiled)

Every main control is audible and on-screen: Fragment, Shift (with
snap/extended range), Reverse, Scatter, Drift, Atmosphere, Movement, Mix,
Output, Freeze, Randomize, buffer length, and the two engine toggles. Set Mix
high, play anything, and the plugin rebuilds it from moving fragments; turn up
Drift and Movement together to hear it wander, add Atmosphere for a diffuse
tail, and Freeze to turn the last bar into a playable texture. The top bar's
preset browser groups 33 factory presets by category, with a Favorites
section for quick access (click the star to favorite/unfavorite whatever's
selected); save, rename, or delete your own presets from the "..." menu.
Randomize (bottom-right) nudges Fragment/Shift/Reverse/Scatter/Drift/
Atmosphere to a new combination — right-click it for Subtle/Balanced/Extreme
strength. Undo/redo (top bar) reverts the last Randomize or preset load as a
single action.

## Getting the VST3

Identical pipeline to Vocal Polish: push this folder to a GitHub repo with the
included `.github/workflows/build-windows.yml`, and the Actions tab produces
both a `Rowen-Fragment-VST3` artifact and a `Rowen-Fragment-Installer`
artifact (double-clickable Setup.exe). Local builds: `build.bat` (needs
Visual Studio 2022 + CMake). Core tests without JUCE:
`cmake -B bt -DROWEN_DSP_TESTS_ONLY=ON && cmake --build bt && ctest --test-dir bt`.

## Project layout

```
src/core/     Thin AudioProcessor / the branded editor (PluginProcessor,
              PluginEditor — preset browser wiring, Randomize + undo/redo)
src/dsp/      Framework-free engine: DspUtil, TempoSync, RollingBuffer,
              CoreSettings, FragmentCore, GrainScheduler/FragmentVoice/
              VoicePool, MotionLfo (Drift), AtmosphereFx
src/params/   Frozen parameter IDs + APVTS layout
src/viz/      Lock-free processor->UI snapshot (seqlock)
src/ui/       Rowen theme + typography + look-and-feel (shared with Vocal
              Polish) plus Fragment's own components: BufferDisplay,
              FragmentTopBar (preset browser/favorites/undo/help/bypass),
              FragmentFooter, RowenModal (save/rename text entry)
src/presets/  ✅ Phase 5 — FactoryPresets (33 presets/6 categories),
              PresetManager (factory + user presets, favorites),
              UndoableStateChange (single-action undo for Randomize/preset
              load, host-automation-safe)
src/licensing/ Phase 6 (copied from Vocal Polish)
tests/        Standalone core test suite
docs/         PARAMETERS, BUFFER, TESTING, DEPENDENCIES, BUILD_WINDOWS
installer/    Inno Setup script (same pipeline as Vocal Polish)
assets/       Brand images + fonts (same files as Vocal Polish)
```

## Documentation

`docs/PARAMETERS.md` — every parameter with ranges and DSP mapping ·
`docs/BUFFER.md` — how the rolling buffer, freeze, and guard zone work ·
`docs/TESTING.md` — automated coverage + the manual DAW checklist ·
`docs/DEPENDENCIES.md` — commercial licensing report ·
`docs/BUILD_WINDOWS.md` — build guide.

© Rowen. All rights reserved.

# Testing

## Automated core tests (run on every build, no JUCE needed)

`tests/CoreTests.cpp` — passing on GCC 13 / Linux x64; clean under
AddressSanitizer, UndefinedBehaviorSanitizer, **and ThreadSanitizer**.

| Test | Proves |
|---|---|
| `testWrapContinuity` | Reads reproduce a known signal across multiple buffer wraps at 44.1/48/96 kHz — no seam at the wrap point |
| `testFreezeHoldsData` | Fully-frozen contents are bit-identical regardless of incoming audio; capture resumes after unfreeze |
| `testFreezeSeamIsSmooth` | The freeze crossfade leaves no click-sized step in the stored data |
| `testGuardZone` | Reads at/behind the write head are clamped into the guard — the wrap-click defense |
| `testWindowGlide` | Window length changes glide monotonically to target (tempo changes can't jump grain positions) |
| `testTempoMath` | Bar math at 4/4 and 6/8, insane-BPM sanitization, Free-mode clamps |
| `testInterpolationAccuracy` | Cubic reads match an analytic sine (mean error < 0.01) |
| `testVizSeqlockUnderContention` | 200k published frames against a live reader thread: zero torn snapshots |
| `testCoreEndToEnd` | 4 sample rates × 7 block sizes × mono/stereo passthrough finite; NaN/inf input cannot crash or poison the core |

### Phase 3 — fragment engine tests

| Test | Proves |
|---|---|
| `testEnginesProduceAudio` | Grains generate a finite wet signal from live input |
| `testSilenceInSilenceOut` | No self-noise: silence in → silence out at full wet |
| `testGrainsAreClickFree` | Hann envelopes keep full-wet granular output free of click-sized steps |
| `testDeterminismWithSeed` | Same seed → bit-identical runs; different seed → different output |
| `testReverseChangesOutput` | Reverse 100% audibly differs from forward and still produces signal |
| `testPitchShiftRoughlyDoubles` | +12 st measurably doubles a sine's frequency (zero-crossing count) |
| `testVoiceStealingUnderExtremeLoad` | Max density + min grain size: finite, bounded, never silent |
| `testEngineTogglesAndMonoPath` | A-only / B-only / both-off behave; mono tracks get full granular output |
| `testBlockSizeConsistency` | Wet energy equivalent at 64 vs 1024-sample blocks (absolute-sample scheduling) |

Documented behavior, not a bug: different host block sizes produce the same
*character and level* but not bit-identical renders; bit-exactness holds for a
fixed block size (see `testDeterminismWithSeed`).

### Phase 4 — Drift (Motion) + Atmosphere tests

| Test | Proves |
|---|---|
| `testMotionLfoCorrelatedAndBounded` | All four Motion outputs stay in [-1, 1], actually move over time, and are meaningfully correlated with each other (position with pitch, tone with pan) — not independent per-parameter noise |
| `testDriftAudibleAndScalesWithMovement` | Same seed, Drift on vs off produces an audible difference; Movement=1 deepens that difference vs Movement=0 |
| `testAtmosphereAddsDiffusionTail` | A short impulse leaves a measurable diffusion tail with Atmosphere on and effectively none with it off |
| `testAtmosphereStableAcrossSampleRates` | Worst-case Atmosphere + feedback settings stay finite and bounded (peak < 10) under sustained loud input at 44.1/48/88.2/96 kHz |
| `testMovementGatesDriftEvenWhenAtmosphereOff` | Drift's effect is independent of Atmosphere being engaged |

Run: `ctest --test-dir build -C Release` or the `ROWEN_DSP_TESTS_ONLY=ON`
configuration for a JUCE-free build.

### Phase 5 — presets, favorites, undo/redo

**No automated tests exist for this layer.** `FactoryPresets`,
`PresetManager`, `UndoableStateChange`, and `FragmentTopBar` are all
JUCE-dependent (APVTS, `juce::UndoManager`, `juce::Component`), so they fall
outside the framework-free `src/dsp` layer this sandbox can compile with
plain g++ — there is no local JUCE install and no network access to fetch
one here. They were written by hand against Vocal Polish's proven equivalent
code, and checked by static review only: constructor signatures and member
init order cross-referenced against JUCE headers used elsewhere in this repo,
braces/parens counted programmatically to catch mismatches, and the factory
preset data verified by script to contain exactly 33 presets × 15 parameters
each (495 value assignments, matching the intended table). None of it has
been compiled or run. First real verification happens on the repo's GitHub
Actions CI (build + ctest) and then in a DAW — treat Phase 5 as "should work,
not yet proven" until that comes back, the same status Phase 4's branded UI
carried before this update.

Manual checks once it compiles, beyond the DAW checklist below:

- [ ] Preset browser groups factory presets under category headings (Subtle,
      Melodic, Rhythmic, Vocal, Drums, Experimental) and a Favorites section
      appears once something is favorited
- [ ] Prev/next step through the list and skip section headings
- [ ] Star toggles favorite status and its icon (outline ↔ filled)
- [ ] Save prompts for a name (RowenModal) and adds it under USER
- [ ] Rename/Delete from the "..." menu work only on user presets
- [ ] Loading a preset does not change Output level or engage Bypass
- [ ] Undo after a preset load or Randomize reverts every touched parameter
      in one step; redo reapplies it; DAW automation lane for each parameter
      still shows the change (no automation desync)
- [ ] Randomize right-click menu offers Subtle/Balanced/Extreme and each
      produces a visibly different amount of change

## Manual DAW checklist

- [ ] Scans and loads (pluginval strictness 8 first)
- [ ] Audio passes through unchanged on mono and stereo tracks
- [ ] BufferDisplay shows the fragmented waveform and grain markers while playing
- [ ] Buffer length follows DAW tempo (1 bar at 120 BPM ≈ 2 s of display)
- [ ] Changing tempo mid-playback glides, no clicks
- [ ] Free mode length control behaves (0.25–8 s)
- [ ] Freeze: display freezes, no click on engage/release; automates from the DAW
- [ ] Randomize produces a new, tasteful combination each click
- [ ] Drift + Movement audibly wander pitch/position/pan; Atmosphere adds a diffuse tail
- [ ] Fonts (Space Grotesk / Poppins / JetBrains Mono) and logo render correctly
- [ ] Transport stop/start and loop jumps cause no artifacts
- [ ] Bypass crossfades cleanly
- [ ] Save/reopen project: settings restored
- [ ] Sample-rate and buffer-size changes survive
- [ ] Multiple instances independent

Phase 5's own manual checklist is above, under "Phase 5 — presets, favorites,
undo/redo". Phase 6+ still needs to extend this list further (CPU profiling,
automation storms, licensing).

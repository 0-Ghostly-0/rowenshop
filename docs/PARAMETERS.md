# Parameter reference

The complete set is frozen from the first build — including controls that only
become audible in Phase 3/4 — so any session saved with any Fragment build
stays valid forever. Never rename or reuse an ID after release.

All continuous parameters are smoothed in the DSP; booleans/choices are
crossfaded or glided. All are host-automatable except where noted.

## Main controls

| Parameter | ID | Min | Max | Default | Unit | Curve | DSP mapping (phase it becomes audible) |
|---|---|---|---|---|---|---|---|
| Fragment | `fragment` | 0 | 100 | 40 | % | linear | Macro: grain length 400→45 ms, density 2→14 grains/s/engine, overlap 1.2→3.5×, trigger jitter 0→35% (P3/P4) |
| Shift | `shift` | −24 | +24 | 0 | st | linear, center detent | Voice pitch ratio 2^(st/12). UI travels ±12 unless Extended; the ±24 range is allocated now so the contract never changes (P3) |
| Shift Snap | `shift_snap` | Semitones / Fifths / Octaves / Off | Semitones | — | stepped | Quantizes the shift target before smoothing (P3) |
| Extended Range | `shift_ext` | off/on | off | — | stepped | Widens the Shift knob's UI travel to ±24 (exposed in the UI now) |
| Reverse | `reverse` | 0 | 100 | 25 | % | linear | Per-grain backward probability; Engine B weighted 1.3× (P3) |
| Scatter | `scatter` | 0 | 100 | 30 | % | linear | Read-offset window across the buffer, timing jitter, pan spread, ±30 ct pitch variation (P3/P4) |
| Drift | `drift` | 0 | 100 | 20 | % | linear | Motion-LFO depth into pitch/position/pan/tone — correlated movement (P4) |
| Atmosphere | `atmosphere` | 0 | 100 | 25 | % | linear | Diffusion/reverb send (^1.5 taper), HF softening, spread, internal feedback 0→0.45 (P4) |
| Movement | `movement` | 0 | 100 | 35 | % | linear | Global depth for the Motion + Pulse modulators (P4) |
| Mix | `mix` | 0 | 100 | 40 | % | linear knob, equal-power law | dry·cos / wet·sin blend (P3) |
| Output | `out_gain` | −24 | +6 | 0 | dB | linear | Post gain + soft ceiling (active now) |

## Buffer & global

| Parameter | ID | Values | Default | Notes |
|---|---|---|---|---|
| Buffer Length | `buffer_len` | ¼ / ½ / 1 / 2 / 4 bars / Free | 1 Bar | Tempo-synced capture window (active now) |
| Free Length | `buffer_free` | 250–8000 ms | 2000 | Log taper; used in Free mode (active now) |
| Freeze | `freeze` | off/on | off | 50 ms write-mix crossfades; automatable (active now) |
| Engine A / B | `engine_a` / `engine_b` | off/on | on | Engine mutes with voice fade-out (P3) |
| Bypass | `bypass` | off/on | off | 40 ms crossfade; reported via `getBypassParameter()` (active now) |

## Advanced drawer (hidden; all DSP-wired, not exposed in the main UI)

| Parameter | ID | Range | Default |
|---|---|---|---|
| Density Bias | `density_bias` | −50…+50 % | 0 |
| Min Fragment | `grain_min` | 15–100 ms | 30 |
| Max Fragment | `grain_max` | 100–600 ms | 400 |
| Position Spread | `pos_spread` | 0–100 % | 50 |
| Pan Spread | `pan_spread` | 0–100 % | 60 |
| Pitch Variation | `pitch_var` | 0–100 ct | 20 |
| Feedback | `feedback` | 0–60 % | 20 |
| Feedback Tone | `fb_tone` | 500–8000 Hz (log) | 3500 |
| Motion Rate | `motion_rate` | 0.05–8 Hz (log) | 0.4 |
| Pulse Rate | `pulse_div` | 1/32…2 bars + triplets/dotted | 1/8 |

## Non-parameter state (saved, not automatable)

`randomSeed` (reproducible grain character per project/preset),
`stateVersion` (currently 1), `pluginVersion`. Missing parameters on load keep
defaults; unknown ones from newer versions are ignored — same compatibility
policy as Vocal Polish.

## Presets (Phase 5)

Factory presets set exactly 15 of the parameters above — the six main macros
(`fragment`, `shift`, `reverse`, `scatter`, `drift`, `atmosphere`), their
supporting controls (`shift_snap`, `shift_ext`, `movement`, `mix`,
`buffer_len`, `buffer_free`, `freeze`, `engine_a`, `engine_b`) — and
deliberately leave `out_gain` and `bypass` untouched, so recalling a preset
never changes the current output trim or drops the plugin into bypass. User
presets saved from the UI capture the same 15-parameter set for consistency.
No new parameter IDs were introduced for presets; the frozen set above is
unaffected by Phase 5.

Loading any preset (factory or user) and the Randomize action both go through
`rowen::presets::StateChangeAction`, a single `juce::UndoableAction` that
snapshots before/after values and replays them via each parameter's normal
`beginChangeGesture`/`setValueNotifyingHost`/`endChangeGesture` path (never a
raw `ValueTree` swap), so DAW automation lanes stay in sync and one undo
reverts the whole preset/randomize action as a unit.

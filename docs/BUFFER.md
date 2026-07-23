# The rolling buffer

`src/dsp/RollingBuffer.h` — the heart of Fragment. Everything here was chosen
so the audio thread never allocates, never locks, and never clicks.

## Capture model

One circular buffer per channel, preallocated in `prepare()` for the worst
case (16 s — 4 bars at 60 BPM plus headroom) at the session sample rate.
~12 MB stereo at 96 kHz, allocated once, never resized. The write head wraps
continuously; "how much history exists" and "how much may be used" are
different things:

- **Capacity** — fixed allocation, never changes after prepare.
- **Window** — how far back readers may look. This is what the Buffer Length
  control and tempo sync change. Changing it never touches audio data; the
  value *glides* over ~80 ms (`OnePoleGlide`) so fragment positions shift
  smoothly instead of jumping on a tempo change or length switch.
- **Guard zone** — 30 ms directly behind the write head that readers can
  never enter. This is the entire wrap-click strategy: a reader can never
  observe the seam where new audio is currently overwriting old audio.

## Freeze

Freeze is a **ramped write-mix**, not a switch:

- Freezing: over 50 ms, each written sample blends from `input` toward
  `existing content` (`slot = in·mix + slot·(1−mix)`), then the write head
  stops. The blend leaves a smooth seam in the data (verified by test: max
  sample-to-sample step stays sine-sized, never click-sized).
- While frozen: the head is stationary, so reader offsets are stable — the
  frozen sound does not slowly rotate — and contents are bit-identical no
  matter what audio keeps arriving (verified by test).
- Unfreezing: the mix ramps back up from the stopped position, fading live
  input over the old material — no wall-of-new-audio transition.

Freeze is a normal automatable parameter; toggle vs. momentary is a UI
behavior (Phase 6) on top of the same parameter.

## Tempo sync

`TempoSync` converts Buffer Length (¼/½/1/2/4 bars from BPM + time signature,
or Free 0.25–8 s) into a target window in samples, clamped to capacity. Host
data is advisory: missing tempo falls back to 120 BPM 4/4, absurd values
(0 BPM, 10⁹ BPM) are sanitized — all covered by tests. Transport stop simply
stops callbacks in most hosts; the buffer holds its contents and resumes
capturing when audio flows again. Transport jumps need no special handling:
old audio is naturally overwritten.

## Reading

`readAt(channel, offsetBehindHead)` — fractional offsets, 4-point Catmull-Rom
cubic interpolation (mean error < 0.01 vs. an analytic 441 Hz sine in tests),
offsets clamped into [guard, window]. This is the exact API the Phase 3 grain
voices consume.

## Visualization path

The audio thread downsamples the window into 256 peak bins (~30 Hz, a few
thousand reads per publish — negligible) plus state flags, and publishes
through a **seqlock** (`src/viz/VizSnapshot.h`): writer increments a sequence
counter around a plain memcpy; the UI copies and retries on a torn read. No
locks, no allocation, no waiting on either side. Torture-tested with a real
concurrent reader thread (200k frames, zero torn reads) and clean under
ThreadSanitizer.

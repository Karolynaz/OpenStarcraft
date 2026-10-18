# Refinery and gas economy milestone — 2026-10-19

The user-console audio benchmark passed (see HARDWARE_AUDIO_VALIDATION.md), so
this milestone extends the same private 64x64 map with native Refinery/gas
mechanics. Factory/tanks remain the next feature; gas is not yet spent by the
current test roster.

## Behavior

The application starts Gas AI vs AI (scenario 5). Both deterministic test bots
use seven SCVs, build a Refinery after establishing their two Barracks, and
reserve one worker for gas. They continue producing/attacking with Marines;
the existing 40-live/pending-unit budget still applies. The original Marines
AI vs AI fixture (scenario 3, six SCVs) remains available in START for comparison.
The bots are adapter test controllers, not Blizzard AI scripts.

Player vs AI supports manual Refinery building in SCV Y → Refinery (100).
Its placement footprint snaps to a nearby neutral geyser. A selected SCV's A on
an own completed Refinery now harvests gas rather than selecting the building.
The native core handles entry/hiding, cargo, trips back to the Command Center,
construction timing, costs, refunds and depleted-resource behavior. No test
resources, spawns, health adjustments or instant completions are used.

Temporary hiding exposed an existing bookmark issue. Recall still omits hidden
units from selection, but now keeps live hidden members in the saved group.
Dead/invalid/wrong-owner IDs are pruned as before. Thus an SCV reappearing from
a refinery can be recalled by its original bookmark. UI reports a temporarily
unavailable group separately from a successful recall.

Eight START menu rows use matching 12-pixel drawing/touch spacing. The basic
build menu has three rows. Original controller chords and native action batch
limits remain in force.

## Startup timing and telemetry

Both user-console baselines have one two-second stall immediately after their
first CSV sample. The logs cannot prove the cause; initial SD log commit is a
candidate. The new build flushes the header while loading and completes the
first GPU present before starting the simulation clock. Normal-play scheduling
debt is retained. CSV appends red_gas, blue_gas and csv_flush_us; the latter is
the preceding record's fflush duration, or the initial header flush in the
first sample. This distinguishes measured write latency from simulation cost.
A further console run is needed to confirm the startup improvement.

## Host/native asset checks

- Existing core, scheduler/debt/pause, input routing, controls, invalid-map tests
  pass. The original full Marines battle retains its frame-13200 blue win and
  36/33 losses.
- The public-adapter gas test mines its own funds, rejects unaffordable and
  off-geyser construction, verifies snapped placement, the 100-mineral charge,
  75-mineral cancellation refund, restoration of the neutral geyser, native
  completion and prevention of a second Refinery on an occupied geyser.
- A contextual A on the completed own Refinery retains SCV selection and
  delivers 120 real gas. The test observes hiding and reappearance and verifies
  the same saved bookmark works afterward.
- The complete gas AI battle ends in a blue win at native frame 11016, losses
  36/18, peak 97 objects. Both sides collect gas. The host summary at 11257
  includes its final winner check plus 240 aftermath frames. Observer order
  isolation and manual-player/AI separation also pass.
- The combined gameplay/baseline/gas/AI audio exporter passes AddressSanitizer
  without a sound-event overflow. LeakSanitizer is disabled on this macOS host.
- Graphics profile: 444 tiles, 38 image types, 871 original frames, 2613
  team/frame keys, 1187 unique rectangles. It still fits eight 512x512 RGBA5551
  pages: 4 MiB. Every terrain/sprite/team/alpha texel and GPU layout is verified.
- Audio profile: 31 complete original effects, 1,878,338 normal-heap PCM bytes,
  longest effect 126,976 bytes (within each 128 KiB DSP buffer). The three music
  files are unchanged. Exact decoded-PCM and audio ownership/lifetime tests pass.
- Final ELF remains ARMv6K/VFPv2; printf/CSV formats pass -Werror=format.

Reproduce:

```
make -C nintendo3ds test HOST_CXX=clang++
nintendo3ds/build/host-sim 'data files' nintendo3ds/local-data/3ds/openbw/maps/probe.scx gas
make -C nintendo3ds test-graphics HOST_CXX=clang++
make -C nintendo3ds prepare-audio HOST_CXX=clang++
make -C nintendo3ds test-audio HOST_CXX=clang++
make -C nintendo3ds DEVKITPRO=/Users/karolynaz/devkitpro -j2
```

## Final native build smoke test and SD delivery

Azahar ran the final executable in verified original-3DS mode, CPU 100%,
through frame 6546 (272 one-second samples, roughly 4.5 minutes), including
Refinery construction, gas entry/return trips on both sides, Marine attacks,
damage and losses. The benchmark is deliberately a partial match; full outcome
coverage above is from host/ASAN tests. The audio backend had already completed
the previous full user-console and emulator matches and is unchanged here.

The captured summary is gas-summary.json, with the last 120 samples in
gas-samples.csv and the full private CSV in local-data/gas-captures. The summary
selects the last run, excluding earlier development runs in the appended CSV.
All seven running artifact hashes were checked against gas-artifacts.json.

Mean 59.830 FPS / 23.997 TPS; maximum tick 2957 us; peak 96 objects in this
capture; normal heap 26,648,392 bytes and linear allocation 7,502,848 bytes.
Both sides had delivered gas (recorded maxima 56/48). Zero backlog, missing
images, missing effects, sound-event drops, music underruns, I/O errors or DSP
dropped frames. There were six intentional eight-voice-limit rejections.
Maximum recorded prior CSV flush was 227 us in emulation; that cannot establish
actual SD-card write latency on the console.

Native menu checks verified all eight START rows fit, Player vs AI loads, A
selects an SCV, Y opens three build rows, D-pad/A chooses Refinery and shows
its invalid red footprint on ordinary terrain, and B cancels. The emulator's
touch injection while minimized did not activate a row, so it is not counted
as a successful touch check; drawing/hit-test row spacing both use 12 pixels.
The physical L/R chords still need console ergonomics testing.

The executable, graphics.bin, effects.bin, audio manifest and README were
written to the mounted Nintendo SD, preserving its old audio-only build in
private local-data/hardware-captures/2026-10-19-before-gas. All seven final
release content hashes (including unchanged map/music) were read back and
verified. The user audio log remains on SD and its original copy is preserved
privately. Console gas/stability and the startup logging improvement remain
the next real-hardware test.


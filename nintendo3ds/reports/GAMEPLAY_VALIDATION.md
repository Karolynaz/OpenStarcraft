# Limited Terran gameplay — 2026-10-16

This milestone connects the physical controls to OpenBW's original action
system. It supports SCV mining, SCV/Marine training, Supply Depot and Barracks
construction, production cancellation/refunds, contextual attacks, attack-move
and Stop. The core costs, supply, movement, gathering, damage and build times
are unchanged. The opponent slot remains human/idle; there is no AI.

## Controls and presentation

Circle Pad moves the cursor and scrolls at screen edges. A selects an own unit
using its actual collision box plus six pixels of aiming tolerance. B issues
OpenBW's default contextual order (including minerals, enemies, repair/resume
construction and building rally points). X attack-moves; Y stops while running.
D-pad Up trains SCV at Command Center or Marine at Barracks. Right previews a
Barracks, Down previews a Supply Depot. Aim its tile-snapped footprint, press B
to place, A to cancel. Green means the selected builder, costs, visibility,
terrain and occupancy checks passed; red means placement is unavailable.
Resources/occupancy may change while the SCV travels; the core rechecks before
charging for construction. Left cancels placement, a selected incomplete
building, or the final queued unit. SELECT pauses. While paused, X toggles CSV
and Y advances one frame. L reloads, R chooses the next scenario, START exits.
Touch remains restricted to camera movement inside the bottom minimap.

The top HUD shows minerals, gas, supply, selected unit name/HP and queue length.
A health bar marks the selected unit. The default two-unit scenario now places
the enemy at (512,128), in the same reachable test area as the own marine.
R then L loads the Terran starting bases with resources and four SCVs per side.

## Asset preparation

The exporter runs the public adapter's gameplay regression sequence and records
all image types seen across the map, then the idle scenarios. It exports every
frame of those types, including direction changes. The packer now uses
MaxRects best-short-side placement instead of shelf rows: the expanded profile
fits the existing **eight-page / 4 MiB GPU texture limit**. Normal/linear heap
reservations remain 36/8 MiB; total requested heap stays 44 MiB.

Palette-remap-only effects (glow, cloak and warp) are deliberately omitted.
Their pixels encode blend-table indices, and treating them as ordinary palette
colors produced opaque red artifacts around mining SCVs. Normal sprites and
basic shadows remain animated; this does not change simulation behavior.

Current private package: 504 terrain tiles, 40 image types, 851 original
animation frames, 2553 team/frame records, 1204 deduplicated rectangles,
8 atlas pages, 4194304 texture bytes, 4243324 file bytes. No game assets or
screenshots containing game art are committed. Copy both graphics.bin and the
updated .3dsx from local-data/3ds/openbw to SD /3ds/openbw/.

## Reproducible host checks

```
make -C nintendo3ds test HOST_CXX=clang++
python3 nintendo3ds/tests/assets.py --data "data files" \
  --map "/Applications/StarCraft/Maps/(2)The Small Divide.scm"
make -C nintendo3ds test-graphics HOST_CXX=clang++
make -C nintendo3ds DEVKITPRO=/Users/karolynaz/devkitpro -j2
```

Passed core/scheduler/controller/preflight tests, all four 2400-tick private
scenarios, and the public-command integration test. The integration test:

- Selects the edge of a Command Center; trains, cancels/refunds and retrains SCV.
- Orders the four starting workers to mine. After 2400 ticks there are five
  workers and 400 minerals, with no resource/build-time/health cheats.
- Rejects off-map construction without charging; constructs Depot and verifies
  eight additional supply; constructs Barracks and produces a Marine.
- Rejects Marine construction, issues attack-move and Stop, targets the enemy,
  then observes actual damage and death plus 240 aftermath ticks. This also
  captures images spawned by death scripts after the live unit is removed.

AddressSanitizer checked gathering, construction, training and combat through
unit death, including reloading from a live base to the combat scenario. macOS does not support LeakSanitizer in this
runtime, so the run uses ASAN_OPTIONS=detect_leaks=0; leak detection is not claimed.
The graphics test checks every terrain/sprite/team/alpha texel, map signature,
lookup bounds, rectangle non-overlap and transparent gutters. The final ELF
passes ARMv6K/VFPv2 validation with no Thumb-2 or NEON.

## Emulator evidence

Emuautotest/Azahar was temporarily set to original-3DS mode at 100% CPU clock.
Physical-button injection verified mining with returned cargo, additional SCV
production, a completed Supply Depot (supply 10 to 18), construction of Barracks
and Marine training. Green/red placement and command error messages were
visually checked. After rebooting the final build, mining again increased minerals from 50 to
82 with zero missing sprites and without the opaque glow artifacts. The private
`local-data/gameplay-captures/mining-final.png` captures that final executable.
The final binary also accepted B on the enemy Marine: the selected Marine
survived with 4/40 HP and the live count dropped from two to one. This check
exposed a late death-script image missing from the initial profile; the exporter
regression now continues 240 ticks after a kill to include it. Repeating the
B attack with the expanded package showed one surviving unit at 4/40 HP, zero
missing image records, and frame 1186 after the full death aftermath. The private
`local-data/gameplay-captures/combat-final.png` records that final check.
Detailed captured metrics and final artifact hashes are recorded below.

These are emulator results, not original-console performance measurements.
There is no opponent AI, fog rendering, audio playback, full roster, networking,
or enforced gameplay unit cap. The atlas remains a fixed profile for the
selected map; battle effects outside that profile can be missing. Rendering
still caps 512 visible image layers and 128 unit/minimap entries, rather than
changing simulation timing or discarding scheduled frames. Physical original
3DS profiling and a practical unit budget remain necessary before AI expansion.

The 402 logged samples cover frames 413..10027 during mining, SCV production,
Depot/Barracks construction and Marine training: mean 59.83 FPS, mean 23.85 TPS
(including pauses), maximum tick time 1047 us, 39 objects, zero retained tick
debt and zero missing sprite records. Normal allocated heap peaked at 24759880
bytes (23.61 MiB); used linear memory at 6355968 bytes (6.06 MiB). This does not
measure load-time peaks or include code/stack/framebuffers. The capture used
the same command code before the final omission of unsupported glow effects.
`gameplay-summary.json` summarizes it; `gameplay-samples.csv` keeps the last 120
samples. Earlier renderer measurements remain in RENDERER_VALIDATION.md.

Final build: 857676-byte openbw-3ds.3dsx, SHA-256
`1eb97a4e4acdd7062e2b69d27ab83c0c31ce1ca5168a7780088c4e38468a5cfe`.
Graphics SHA-256:
`b308f04d9d83b16a9b61f7a1a027a35e1fc1fb38782159d9d58e96e0a05a4d53`.

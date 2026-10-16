# Terran AI observer and controller scheme — 2026-10-16

The default launch now automatically runs Terran vs Terran with both players
controlled by a small deterministic test bot, with full-map rendering and no
fog. No input is needed to start. The camera follows bases, attacking armies
and engagements; Circle Pad/minimap interrupts following for eight seconds.
START provides camera/scenario/logging/restart/exit options; SELECT pauses.
The CSV recorder starts automatically at /3ds/openbw/logs/ai-benchmark.csv.
The observer pauses at victory, defined here as destruction of all buildings.

## Controller behavior and bounds

The isolated adapter controller in include/battle_ai.h updates every twelve
native simulation frames. It gathers minerals, grows to six SCVs, builds Supply
Depots and two Barracks, trains Marines, and sends attack groups of at least
four. Existing native attack/movement/combat logic resolves the fights. A
bounded eight-candidate building search leaves clearance around structures,
and another worker can resume an abandoned foundation. The bot knows the enemy
base location; this is a transparent performance fixture, not campaign AI or
an implementation of aiscript.bin/bwscript.bin.

Orders use OpenBW action APIs, normal resources/supply/build times, with no
resource grants or instant completion. Each side limits production to forty
live/pending units and twenty-four Marines. These are controller budgets, not
native engine pool changes or unit deletion. The two slots remain occupied
human controllers at the core level; only the adapter issues their AI orders.

The private fixture is now (4)Blood Bath, 64x64, with only slots 0 and 1 active.
Its remaining start resources remain neutral. The earlier Small Divide map
has disconnected ground start areas, so Marines could not meet. AI ground
modes explicitly reject unconnected starts. The original private map is backed
up as local-data/3ds/openbw/maps/probe-small-divide.scx; current probe.scx and
its graphics package must be copied together.

Normal/linear heap reservations remain 36/8 MiB, with 256 KiB stack. The device
unit/minimap buffer increases from 128 to 192 to cover both controller budgets
and neutral resources; visible image layers remain capped at 512. No simulation
frames are dropped, and the 24 Hz scheduler is unchanged.

## Input

See ../CONTROLS.md for the complete mapping. A acts/selects/confirms, B cancels.
X selects military in view, L+X all own workers, Y primary/build, L+Y secondary
menu. R+D-pad saves four groups; a direction recalls, double-tap within 350 ms
centers the camera. L/R alone no longer reload/cycle scenarios. Native orders
for up to 64 selected units are issued in batches of at most twelve. Bookmarks
use 32-bit generation-checked IDs, remove dead entries, and clear on reload.

Bottom menus accept D-pad/A/B and touch rows. Touch outside menus is restricted
to the minimap; it never controls the top pointer or gives a game order.
Observer game orders are rejected so performance captures cannot be disturbed.
Player vs AI is selectable from START. Basic SCV menus support Depot/Barracks;
advanced construction is a visible placeholder. A native Siege/Unsiege dispatch
route is prepared, but tanks/Factory/gas/technology/graphics are not delivered.

## Host verification

```
make -C nintendo3ds test HOST_CXX=clang++
python3 nintendo3ds/tests/assets.py --data "data files" \
  --map "/Applications/StarCraft/Maps/(4)Blood Bath.scm" \
  --output nintendo3ds/reports/host-ai-scenarios.csv
make -C nintendo3ds test-graphics HOST_CXX=clang++
make -C nintendo3ds DEVKITPRO=/Users/karolynaz/devkitpro -j2
```

The host suite verifies scheduler debt/pause behavior, preflight rejection,
pointer/minimap behavior, modifier-only keys, A/B, menu navigation, group
store/recall and double-tap timing. Private-asset integration covers economy,
training/refunds, construction/supply, thirteen Marines selected and ordered in
native batches, worker/group bookmarks, reload cleanup and combat/death.

The complete AI match is checked for building/training/waves on both sides,
actual damage and losses, production budget, a winner, refusal of observer
orders, pruning a destroyed building bookmark, and separation of manual red
from blue AI after reloading Player vs AI. Earlier regular and AddressSanitizer
runs reached frame 13441 including 240 aftermath ticks, blue victory, peak 94
objects, and losses 36/33. Host results do not predict original ARM11 speed.
AddressSanitizer uses detect_leaks=0 because this macOS runtime does not support
LeakSanitizer. The disconnected Small Divide fixture is also checked for an
explicit load error.

The exporter observes all image types in the gameplay and complete AI tests,
including post-death images. The current package uses 444 terrain tiles,
34 image types, 851 animation frames, 2553 team/frame records, 1171 distinct
rectangles and eight atlas pages (4 MiB textures, 4242724 total file bytes).
The texel test checks every packed terrain/sprite/team/alpha value, lookup/map
signature/bounds, rectangle overlap and gutters. No Blizzard assets are tracked.

## Emulator and final artifacts

The original-3DS emulator mode and normal clock are used temporarily, with the
user's model preferences restored afterwards. The complete initial emulator match reached blue victory at frame 13200.
The release build is checked again after strengthening bookmark IDs and
expanding the unit view buffer. Results and hashes follow below. No real original
Nintendo 3DS measurements are available yet. The fixed roster/terrain profile,
missing game audio, campaigns, save/load and advanced units remain limitations.

Native testing verifies automatic launch, normal-clock original-model mode,
camera following and its menu toggle, mining/building/training, waves, actual
combat damage, building destruction, victory pause and CSV output. A bottom
menu row was activated through an emulated touchscreen event and the matching
camera option changed. The controller routing/engine tests cover L/R chords,
four bookmarks, double-tap centering and thirteen-unit batched commands. The
available single-button emulator injector does not reliably preserve chords;
the native UI tool also rejects two non-modifier keys in a single chord. Those
combined physical-button paths still need a real-device ergonomics check.
Temporary test input/model preferences are restored after emulator shutdown.

The release executable also completed the entire emulator match at frame
13200, blue victory and losses 36/33, matching the host controller outcome.
The 549 active samples (frames 24..13183) average
59.83 FPS and 23.94 TPS. Maximum observed tick time was
2186 us, peak 94 objects, with zero retained tick debt
and zero missing image records. Normal allocated heap peaked at
24684336 bytes (23.54 MiB); linear use at
6355968 bytes (6.06 MiB). These exclude
code/stack/framebuffers and do not measure loading peaks. Zero-TPS/terminal
samples are excluded from the displayed averages; partial pause/resume
intervals remain included (hence the overall 23.94 TPS). Combat alone averages
24.00 TPS across 334 samples; the full private capture is
local-data/ai-captures/final-benchmark.csv. ai-summary.json includes economy
and combat aggregates; ai-samples.csv preserves the final 120 active rows.
The private battle-final.png and victory-final.png capture the actual release
build. These are emulator measurements, not proof of original-console speed.

Release artifacts (private SD directory local-data/3ds/openbw):

- openbw-3ds.3dsx: 879152 bytes; SHA-256 `dbaa5d2a251f34e8ef81806a775f127a0835c133a10993a4831258cb6e002b4b`.

- graphics.bin: 4242724 bytes; SHA-256 `ed71bd4cb1120b31641ca0c3cd1bad5955916d7859bdce019898e35e13656457`.

- maps/probe.scx: 31048 bytes; SHA-256 `00f955a8d5817cc5194cef4649e690ca892758a1f375cdb75c63e63a8489fdfe`.

After the final match, START switched successfully into Player vs AI. A selected
the focused SCV, Y opened its basic build menu, touch on Supply Depot opened
the placement preview, B cancelled it, and Circle Pad + A on minerals issued
a gathering order. B then cleared selection without cancelling the existing
worker order. These checks use the updated physical A/B mapping and bottom
menu touch handling rather than a desktop mouse backend.

# Protoss milestone — 2026-10-19

Protoss are implemented as an additional standard multiplayer race profile,
using the original OpenBW simulation and private original art/audio. Zerg are
not yet integrated into these menus/scenarios. No copyrighted assets are
committed. The previously delivered Terran SD build is preserved separately.

## Scope and native integration tests

- All sixteen Probe buildings: Nexus, Pylon, Assimilator, Gateway, Forge,
  Photon Cannon, Shield Battery, Cybernetics Core, Robotics Facility, Stargate,
  Citadel of Adun, Robotics Support Bay, Fleet Beacon, Templar Archives,
  Observatory and Arbiter Tribunal.
- Twelve standard trainable army/worker types, plus Carrier Interceptors,
  Reaver Scarabs, and High/Dark Templar merges into Archon/Dark Archon.
- All seven research technologies and nineteen upgrade types; the five
  weapon/armor/shield upgrades reach all three levels.
- Native Hallucination, Psionic Storm, Feedback, Mind Control, Maelstrom,
  Stasis Field, Disruption Web, Recall and Shield Battery recharge commands.
- Native Shuttle loading/unloading and fighter capacity/parent attachment.

`tests/protoss_build.h` constructs every building through public Probe orders,
first rejecting an unpowered Gateway and then expanding psi with ordinary
Pylons. Assimilator placement uses an actual neutral geyser. Warp construction
uses PlaceProtossBuilding, never Terran SCV construction. Native costs,
prerequisites, psi, supply, energy and durations remain active.

`tests/protoss.h` manufactures all fourteen products, researches every tech
with cancel/refund/restart, completes all upgrade levels and rejects maximum
upgrades, fills Carrier/Reaver ammunition to their upgraded limits and rejects
excess, merges both Templar types in clear terrain, loads/unloads a Shuttle,
recharges a damaged Zealot, creates two hallucinations, kills an energized
Ghost with Feedback, transfers a Battlecruiser with Mind Control and exercises
the other targeted powers. Enemy unit-targeted spells are tested after
revealing their target with native sight. The no-fog observer presentation
does not bypass cloaking/detection or combat sight rules.

## Prepared scenarios

The current local build defaults to **All Protoss battle** (scenario 8).
START → Scenarios lists Protoss battle/lab first, then all previous Terran and
small economy scenarios. Eight rows fit the existing lower-screen menu.

Each side has 78 troops across fourteen types (including both Archons),
200 supply, eight Interceptors per Carrier and ten Scarabs per Reaver, plus
native Nexus/Pylon supply providers. The stress fixture explicitly prepares
resources, research and fighter-capacity upgrades. It is a small deterministic
performance controller, not Blizzard campaign AI. The existing 30-second
loss threshold and 180-second simulation/active real-time cap apply.

The host battle test passes: peak 351 native objects, frame 720, blue wins,
remaining resource-value scores 4400/8227, losses 62/41. Fighters are excluded
from army scoring so launching/reloading ammunition cannot change the result.
The manual **Protoss laboratory** (scenario 9) starts with completed prerequisite
buildings, legal local Pylons and 10,000 of each resource; research is initially
unresearched. A damaged friendly Zealot and stationary enemy spell targets
are explicit lab fixtures. This is not an ordinary opening economy opponent.

## Graphics, audio and memory

The shared resident-GRP GPU cache handles Protoss assets without a second
static atlas. The same flat arena terrain package is reused. Shield values
are exposed and selected units get a separate blue shield bar. Warp-in bodies
use a translucent approximation; destination-dependent palette distortion and
full original warp-texture compositing are still unfinished, as are the older
renderer limitations described in TERRAN_VALIDATION.md.

The Proto asset exporter captures 199 image types / 7,721 frames. ASan tests
compare 6,995,265 pixels with independently exported GRP/palette data, then
11,195,327 cache/swizzle pixels during the moving battle, with no cache
miss failure or overflow. The older Terran cache reference/combat tests also
pass (3,545,808 / 8,722,438 pixels). Scheduler/input/core/map recovery, audio
buffer lifetime and CSV tests pass; the final ELF remains ARMv6K/VFPv2.

Protoss uses its own sound bank, avoiding simultaneous residency of both
races' samples. The final bank contains 202 complete PCM8 effects/voices,
8,368,683 PCM bytes, maximum 119,040 bytes per effect; three complete original
Protoss music tracks stream as stereo PCM16. All packaged PCM matches FFmpeg
conversion of the private WAVs. Audio load supports race-specific music names
and resets the old bank/channel stream when changing races. The bank is close
to its 8 MiB bound; do not append other races' complete samples to it.

## Original-model emulator measurement

See protoss-emulator-summary.json and its per-second samples. The final
202-effect build completed the 30-second battle with mean 52.09 FPS and about
24 TPS (the partial final sample is retained in the 23.78 TPS mean), within unchanged 36 MiB normal / 8 MiB linear reservations. Sampled
heap peak 34,176,104 bytes; linear 7,789,568 bytes. No backlog, missing image
layers, cache overflows, missing effect IDs, queued sound-event drops, music
underruns, audio I/O/DSP errors or CSV errors/drops were recorded.

Eight effect voices plus music are bounded: 3,973 effect requests did not
obtain a voice in the dense battle; 160 effects played. This is limited
simultaneous SFX mixing, not interrupted music. This emulator test uses DSP
HLE with its existing empty firmware placeholder; it does not validate real
DSP firmware or original-console performance. The user's actual SD/DSP was
not mounted for this overnight run. The final bank includes all building voice IDs.

## Reload memory correction

Native testing exposed std::bad_alloc when switching from battle to lab with
the old resident PCM bank still allocated during core MPQ initialization.
Reload now joins the sound worker, releases PCM capacity and sprite scratch,
then loads the new core before the selected race sound bank and GPU cache.
The corrected build loaded Protoss laboratory from battle successfully; A
selected the Nexus and Y queued a Probe with the native 50-mineral charge.
Audio ASan tests cover explicit unload, zero remaining DSP allocations, and
race-specific music reload. Both sprite reference/cache tests still pass.
The emulator also switched from Protoss lab to Terran lab and back to Protoss
lab, with music/effects ready and no allocation error after either reload.
Emulator model settings were restored after the test.

## Delivery / next work

The new local executable and complete audio/protoss folder must accompany the
existing three MPQs, both graphics files, maps/arena.chk and earlier music.
The SD had already been removed; this new race build has not been copied to it.
The tested earlier Terran SD release remains the on-card version. Original
console performance and full controller ergonomics remain unmeasured.

Next race is Zerg: creep, Drone/building/unit morphs, Larvae, burrowing, Overlord
transport and spell/upgrade menus require distinct native action routes. The
05:27 continuation reads NIGHT-CONTINUE.md and proceeds from this milestone.

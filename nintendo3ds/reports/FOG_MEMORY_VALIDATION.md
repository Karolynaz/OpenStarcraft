# Bounded fog memories and creep correction (2026-10-20)

Player vision now retains a frozen last-seen picture of scouted enemy buildings,
mineral fields and geysers, plus their minimap markers. Mobile enemies still
vanish immediately. Remembered objects cannot expose current HP or become live
selection/hover targets. Re-observing an empty footprint clears its memory;
unseen destruction or changes do not. Snapshot image records own no native
pointers, so recycled unit slots cannot turn an old memory into a new unit.

Memory is bounded to 64 static objects with 16 image layers each and a 4096-byte
creep grid. These records use ordinary heap, not additional linear textures.
Visible creep updates its saved grid; explored tiles retain the last seen state.
Toggling observer vision does not populate memories from hidden simulation data.
Reload clears the records. The observer path skips memory updates.

Fog texture filtering uses hardware linear interpolation between tile-center
alpha values to soften the former hard 32-pixel edges. Viewport and minimap
share the existing two 64x64 masks; no larger fog allocation or CPU screen mask.
Local native sounds require current player sight; own-unit/global UI sounds stay
audible. Frozen fog images do not produce live enemy sound information.

A separate creep bug was exposed by the new ordinary-map Zerg economy scene:
variant selection used all 16 CV5 entries, including unused black entries
13..15. OpenBW's existing UI chooses common variants 0..5 and rare 6..12.
The port now follows that valid range and roughly four-percent rare frequency,
using a stable visual-only seed independent of simulation RNG.

## Checks

ASan fog test scouts an enemy Nexus (400 frames), retreats until explored fog
returns (75 frames), verifies the enemy is no longer a live unit/hover target,
compares frozen images across advancing simulation frames, revisits to replace
the memory with current sight, and draws remembered images through the bounded
GPU-cache fake. It also checks 4096 uploaded fog values, shared viewport/minimap
texture, Scanner Sweep reveal/expiry, real energy and disabled power state.

The native presentation-state fixture destroys a Nexus while hidden and removes
creep there. The last-seen image, marker and creep survive; revealing the empty
footprint then clears all three. Reset removes old records. Across 4096 cells,
the visual selector produces 3931 common and 165 rare choices, all 13 valid
variants represented and no invalid variants.

Original-model / CPU100 / DSP HLE emulator verifies the corrected continuous
creep at the player Hatchery, hidden viewport/minimap with fog ON, and the
full terrain after the START fog toggle. The Hatchery → Larva → Y path opens
all eight choices without spending minerals. A paid Drone morph completes.
The added host contextual-scout test moves that new Drone from (1755,1882)
to (1571,1453) over 120 frames toward the enemy base at (288,1808).
Fogged observer bookmarks retain IDs without revealing hidden centers; turning
fog off restores their recall. Placement preview uses native visibility-aware
checks rather than leaking hidden obstacles. Mines show remaining charges
instead of a misleading zero-energy label.

All race construction, production, research, power and stress suites pass.
Frame-720 arena scores remain Terran 475/575, Protoss 4400/8227, Zerg 2050/4000.
All three GPU-cache/reference suites and animated cursor tests pass, including
75 full composed viewports. The Zerg composition changes intentionally because
its creep now excludes invalid variants. Reload passes 28 loads under ASan.
ARMv6K / VFPv2 validation passes, without Thumb-2/NEON or heap increases.

Exact manually tested executable:
3b97834496dc71eda0353c8240681acd7923446b4e0ac4e38cd8ed80c4003221.
Matching ELF and private logs: local-data/fog-memory-2026-10-20/.
Private release zerg-economy-fog-memory-2026-10-20.zip contains 25 files,
344072362 bytes, SHA256
80a756f2462e33ea497da6daecae2d459fbbfeb9367c453f0b38b6fc76032b0b.
Owned MPQ manifests, ZIP CRC and packaged executable identity were verified.
Emulator was exited normally, stopped, and original model preferences restored.
No new build from this stage has been written to the physical SD card.

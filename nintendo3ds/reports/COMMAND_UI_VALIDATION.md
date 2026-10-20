# Original command icons and bottom catalogue (2026-10-20)

Production, research, upgrade and power menus use the user's original
unit/cmdbtns/cmdicons.grp (390 frames, 36x34 canvas). Unit frame IDs, technology
and upgrade icon fields, and native orders.dat highlight fields provide the
mappings. Orders without highlight art fall back to their native technology
icon; Archon/Dark Archon merge icons use their separate technology fields.
The existing [BWAPI order documentation](https://bwapi.github.io/namespace_b_w_a_p_i_1_1_orders.html)
was checked as background; the implementation reads the purchased DAT metadata.

The icon inks are indices 0..15. unit/cmdbtns/ticon.pcx contains palette-index
remap rows, not a display palette: its own PCX palette has placeholder magenta.
The enabled and disabled rows are resolved through the current race console's
original PCX palette. Small button backgrounds come from that race's original
game/tconsole.pcx, pconsole.pcx or zconsole.pcx. GRPs, remaps and button pixels
load once per map, with a text fallback for incomplete loose-asset installations.
No additional external asset files or linear textures are required.

Catalogue choices occupy two columns and four rows on the bottom screen, with
original icons, names/costs, disabled colors, selection highlight and a full
selected-option caption. D-pad left/right cycles choices; up/down follows the
same column and skips missing cells. A confirms and B cancels. Open catalogue
touch uses the exact same bounds; other menus and minimap touch retain their
existing layout. Commands refresh their availability before activation.

UI textures share the bounded world atlas. Uploads retain the current frame's
pin age so top-screen textures referenced by queued GPU draws cannot be evicted
while the bottom screen draws. No extra linear GPU allocation is introduced.

## Checks

An independent bounded Python PCX decoder resolves a private OpenBW GRP-index /
coverage reference for all three console palettes. AddressSanitizer compares
2,046,792 RGBA5551 pixels across all 390 frames, both availability states, all
three races and the button crops. It checks 294 native catalogue icon mappings,
then 75 moving battle viewports and 1200 UI draws, asserting that every queued
world texture region retains its pixels. All fake GPU allocations are released.

Exhaustive one-to-eight-choice grid checks cover all D-pad directions, inverse
navigation, odd/incomplete rows, cell edges and touches outside the catalogue.
Common input/control/scheduler/map-rejection checks, fog and the 28-load cross-
race ASan reload test pass. Full Protoss and Zerg economy ASan matches retain
their previous outcomes and every native sound ID remains banked.

Original-model / CPU100 / DSP HLE emulator checks the Zerg Hatchery/Larva and
Protoss Gateway grids, original enabled/disabled icon colors and D-pad movement.
Choosing RIGHT then DOWN in the eight-choice Larva menu selects Hydralisk;
A pays 75 minerals and 25 gas and the native Hydralisk finishes. The final Terran Barracks grid also queues a 50-mineral Marine, which finishes.
These are emulator checks, not physical-console performance claims.

Citro2D retains its current texture pointer between draws (see
[its source](https://github.com/devkitPro/citro2d/blob/master/source/base.c)).
A new UI upload now explicitly binds the uploaded page, preventing a stale
same-texture draw from sampling another page. A focused 390-icon pressure test
crosses atlas pages with zero stale bindings; the prior forced-first-page
implementation fails that same regression. The complete pixel/world-pin suite
passes with the fix.

Prototype executable b20267ca... checked Zerg and Protoss workflows; metadata
candidate 28e9c1c3... checked the final Terran workflow. The final binding-fixed
executable and matching ELF are preserved in local-data/original-ui-2026-10-20/.
Its exact SHA256 is
a68d01274e0a27eddb5f8fbb5255e13dc6e5fbead31a091f1446aff1f1a7e773.
Across the manual sessions, peak normal heap is 34,069,384 bytes and linear
7,822,336 bytes. Listed graphics/cache/missing-sound/event/music/I/O/DSP/CSV
errors are all zero. ARMv6K / VFPv2 validation passes, with 36/8 MiB heaps.

The binding-fixed candidate was rechecked in the eight-cell Larva menu. The
subsequent source-identified final release completed the native Zerg stress
battle at frame 720 with blue score 2050/4000, losses 130/121, no missing
sprites/cache overflow or music/I/O/DSP/CSV errors. Its exact executable SHA is
28bf8de7a3c426230ff627dd1c53e295dff590553eb5193a7890e646383e97d8 and source
marker src-ef07ed30b780ef84. Every new CSV row has 53 matching columns and that
marker; the load journal and visible diagnostics agree. The CSV drain-on-exit
and catalogue navigation tests pass. Emulator stopped and preferences restored.

Verified private release: original-command-ui-2026-10-20.zip, 25 files,
344078553 bytes, SHA256
b038692015b945d25f89dd9dfd40c3e277c2f99ad8e32f9a8696da094c54bc78.
Owned assets/ZIP CRC/executable identity checked. It is prepared locally; the
physical SD still has the earlier cursor build and was not overwritten.

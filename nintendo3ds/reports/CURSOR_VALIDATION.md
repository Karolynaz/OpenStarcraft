# Original cursor milestone — 2026-10-19

Replaced the top-screen crosshair with the owned original `cursor/*.grp` art:
arrow; green/yellow/red hover rings; green/yellow/red command targets; eight
edge-scroll directions. The loader also supports the original time/drag assets.
There are 17 cursor kinds, 74 original frames. No new copyrighted asset files
are exported or added to the release: existing private MPQs supply the GRPs.

Frames preserve their 128x128 native canvas offsets around hotspot (64,64).
Animation advances every 100 ms, independently of simulation/pause, and restarts
when the cursor kind changes. Timing is a port UI choice, not a claim of exact
original cursor timing. Ownership colors distinguish own/enemy/neutral objects;
ability targets use the same colors. These do not promise spell validity.
Building placement uses existing native placement validation for green/red.
Actual stick pressure at an edge selects the matching scroll direction.
Menus use the original arrow; Circle Pad, A/B, modifiers and touch are unchanged.

Cursor rendering uses two alternating 64x64 RGBA5551 textures (16 KiB linear)
and fixed 8 KiB decode scratch, independent of the sprite atlas working set.
Only frame/kind changes upload pixels; frames perform no SD reads or heap
allocation. Cursor pixels use the tileset palette without player recoloring.
Cursors draw after the top HUD and before switching to the bottom target.
Fallback crosshair remains available when graphics/core is unavailable.
Existing 36/8 MiB heaps and original-3DS CPU mode remain unchanged.

## Tests

`make test-cursors test-reload HOST_CXX=clang++` passes under AddressSanitizer.
An independent decoder reads original MPQ GRP records and checks every frame,
color, coverage pixel, offset, animation wrap and the full 64x64 swizzled upload:
17 kinds / 74 frames / 303104 texture pixels. Contextual hover/target colors,
eight edge directions, neutral stick and non-mutating cursor queries pass.
All allocations are released. 24 cross-map/race reloads pass.

All three sprite-cache/reference suites remain unchanged in their battle pixel
fingerprints; 75 complete composition comparisons pass, including the earlier
glow/shadow/Templar corrections. Common pacing/scheduler/input/control/map
validation tests pass. ARM11 validation passes ARMv6K / VFPv2, no Thumb-2/NEON.

## Private local release

Executable SHA-256:
`39607cd8f58702873c6d4e5c87a4c7f9497e34450863682bc8e953ac3c9df5cb`.
Matching ELF/candidate and raw emulator logs are retained privately under
`local-data/cursors-2026-10-19/`.

`local-data/releases/original-cursors-2026-10-19.zip`: 25 files, 344057384 bytes,
SHA-256 `0b01387e0070b0ab7a932246c2cafcfa16a79793653e17aa26433ca7d0f82494`.
Owned input hashes, ZIP CRC and packaged executable verified. Includes prior
Protoss economy and rendering corrections. Existing ZIPs are preserved.
Physical SD is unchanged during this cursor milestone.

## Final executable emulator smoke check

Original-model / CPU100 / DSP HLE: observed original enemy red hover ring and
menu arrow in the Zerg stress scene, then reloaded Protoss laboratory. Own Nexus
hover ring aligned with A selection; Y queued a native Probe for 50 minerals.
The produced Probe was selected with Circle Pad/A, its Y build menu opened,
and a Pylon placement showed red over an overlap and green on clear ground.
B restored the ordinary arrow. Original Nexus glow and normal HUD colors remained
intact. Exited via START > Exit; emulator stopped, original preferences restored.

Across 138 active laboratory samples, mean 59.836 FPS /
23.996 TPS. Maximum observed heap 33,704,288 bytes,
linear allocation 7,805,952 bytes. Music/effects enabled, all listed
missing-graphics/cache/audio/I/O/DSP/CSV errors zero. This is a workflow smoke
check, not a complete large-battle comparison or physical-console measurement.
Summary: `cursors-emulator.json`; private raw CSV and load journal retained.

## Subsequent physical SD delivery — 2026-10-20

On the user's explicit request, the identified SD was updated to this exact
39607cd... cursor executable. All 25 read-back hashes passed; only executable
and README changed. 15 existing DSP/log/dump files were backed up and preserved.
Card synced and safely unmounted; receipt and returned prior-console evidence
remain private in `local-data/cursors-2026-10-19/sd-delivery-2026-10-20/`.
Further fog/ability work is a separate local stage, not part of this SD delivery.

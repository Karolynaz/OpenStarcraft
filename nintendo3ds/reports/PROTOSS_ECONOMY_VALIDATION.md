# Protoss economy skirmishes — local continuation after SD performance delivery

The user is testing performance binary1660ed... on the SD. This subsequent
feature is developed separately and has not been written to that physical card.

## Playable modes

Append scenario12 (Protoss AI vs AI) and13 (Protoss player vs AI), accessible via
START → Scenarios → More scenarios. Existing scenario IDs0–11 and the default
Zerg stress test remain unchanged. These modes use the existing ordinary map
and its matching graphics, not the prepared arena or rich laboratory fixtures.
Race and arena classification is explicit so appending Protoss modes cannot
accidentally select Zerg data/creep or the arena's terrain signature.

Both players start with one Nexus, four Probes and50 minerals. The bounded
opening bot gathers minerals, grows to eight Probes, builds Pylons and up to two
Gateways, harvests gas through an Assimilator, builds Cybernetics Core, and
produces Zealots with a Dragoon component. Native Probe warp orders, psi checks,
prerequisites, costs, queue/build times, food and combat rules remain in force.
The bot's controller budget is40 units per side; the native pool and manual
player limits are unchanged. It launches/reinforces attack waves toward enemy
buildings and wins when the opponent has no surviving buildings.

This is a basic opening opponent. Higher-tech planning, research strategy,
expansion, clever micro and Zerg economy AI are later work. Manual production,
research and abilities retain their existing full catalogues. This is not
Blizzard campaign AI bytecode compatibility or a campaign implementation.

## Checked native simulation and asset coverage

Complete Protoss AI battle passed: frame11761 including240 post-victory death
script frames, peak100 objects, blue winner, losses34/10. Both sides gathered
gas and completed native Dragoons; both constructed, trained and launched waves.
Initial-state assertions reject free Pylons/army or altered starting resources.
Observer controls cannot issue orders to AI armies; dead bookmark pruning passes.
Player/AI assertions confirm the human side stays idle with four workers/50
minerals while the opponent builds its economy. A paid human Nexus Probe costs
50 minerals and completes normally.

AddressSanitizer repeated the complete match with122 moving viewport draws on
the ordinary map and9348 native sound events. No missing graphics or cache
overflow; every event ID exists in the current202-effect Protoss bank. No audio
asset replacement is needed. Future Protoss audio export also exercises this
new economy path. Existing source/reference data remains private.

Terran AI and gas AI regressions retain their old outcomes: respectively
frame13441/peak94/losses36/33 and frame11257/peak97/losses36/18.
Full Protoss construction/production/tech/power and Zerg construction/production/
tech/power suites pass; their arena outcomes remain4400/8227 and2050/4000 at
frame720. Common scheduler, input, controls, map rejection/recovery and a24-load
cross-race arena↔economy map ASan reload test pass. ARM11 build validation passes
ARMv6K / VFPv2 with no Thumb-2 or NEON, retaining36/8 MiB heaps.

## Emulator and package

Original-model / CPU100 / DSP HLE emulator completed the entire new AI match:
frame 11520, blue winner, losses 34/10, matching the host outcome. Across 478
active samples, mean 59.833 FPS / 23.998 TPS (minimum 59.73 FPS). Peak normal heap
33,804,928 bytes; peak linear allocation 7,789,568 bytes. Music transitioned from
track 1 to track 2. Missing images, cache overflows, missing sounds, music
underruns, audio I/O, DSP and CSV errors/drops were all zero.

Navigated START > Scenarios > More scenarios using buttons. In the player/AI
mode, A selected the human Nexus and Y queued a native Probe: minerals fell
50 to 0 and the fifth worker completed (observed frame 5198). The opponent
independently reached eight workers and attacked; the human workers received no
automated harvesting orders. Exited via START > Exit. Emulator stopped and its
original model preferences restored. Summary: `protoss-economy-emulator.json`;
raw CSV and load journal remain private under `local-data/protoss-economy-2026-10-19/`.

Native executable SHA-256:
`fee7bb7dd4a421a0d77300b1fc43302027c92230f0521fcd1c84e0480f5b8311`.
Matching ELF and candidate are retained privately. The new local release is
`local-data/releases/protoss-economy-2026-10-19.zip`: 25 files, 344,050,417 bytes,
CRC, owned MPQ hashes and packaged executable checked. ZIP SHA-256:
`df6dbf596f635ee460267f405e24d76f361c89257203f9f3b7070432ff888040`.

This feature has not been copied to the physical SD. The user is testing the
separate performance executable `1660ed...`, safely delivered earlier this turn;
its matching ELF and SD receipt remain under `local-data/performance-2026-10-19/`.
No new physical-console measurements are claimed.

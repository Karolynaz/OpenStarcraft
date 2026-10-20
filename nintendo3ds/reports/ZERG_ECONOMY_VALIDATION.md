# Zerg economy skirmishes (2026-10-20)

Adds scenario 14 (Zerg AI vs AI) and 15 (Zerg player vs AI) under
START → Scenarios → More scenarios. They use the ordinary map, its matching
terrain graphics and the existing private Zerg audio bank. The stress-test and
laboratory scenarios retain their IDs, data and default behavior.

The opening starts with a native Hatchery, four Drones, one Overlord and 50
minerals. The controller gathers resources, morphs Drones from Larvae, grows
Overlord supply, consumes Drones through native building orders, and builds a
Spawning Pool, Extractor and Hydralisk Den. It produces paired Zerglings and
Hydralisks and sends attack waves. Costs, supply checks, prerequisites, creep,
Larva spawning, morph timing and combat remain the original engine's rules.
Placement attempts are limited to eight per twelve simulation frames. The
40-object controller budget reserves room for Larva replenishment and the
second Zergling from an egg. No units are deleted to enforce that budget.

This is an opening opponent, with no expansion or higher-tech strategy yet.
The human side retains the full manual catalogue. The player scenario defaults
to native fog of war; observer mode defaults to full-map visibility.

## Host checks

The full deterministic match, including 240 post-victory script frames, finishes
at frame 12049, red winner, peak 106 total objects and losses 23/46. Both sides
harvest gas, complete Hydralisks, construct, morph and attack. Initial-state
assertions reject free combat units, extra buildings or resources. Observer
orders are rejected; player and AI ownership remain separate. A human Larva
morph pays 50 minerals and completes the fifth Drone.

AddressSanitizer covers the complete battle, 125 moving viewports on the ordinary
map and 5999 native audio events. All requested sound IDs exist in the current
Zerg bank; no missing sprites, cache overflow, event overflow or texture leaks.
The cross-race/lab/economy ASan reload test passes 28 loads, including both new
Zerg modes and their fog/ownership/map classifications.

Original-model / CPU100 / DSP HLE emulator completed the entire match at
frame 11796, red winner, losses 23/46 and peak 106 objects, matching the host.
491 active CSV samples average 59.8327 FPS / 23.9541 TPS. Peak normal heap
31,977,400 bytes; linear 7,822,336 bytes; zero backlog, missing sprites, cache
overflows, missing sounds, event overflow, music gaps, audio I/O, DSP or CSV
errors. The eight-effect-voice budget deliberately declined 474 overlapping
requests; the stream continued and music changed from track one to two.
Raw CSV/settings/matching ELF are private under local-data/zerg-economy-2026-10-20.
Exact full-match executable: a77c505872d33dab61c9e58f77d3b432ae7ddbf85c50775466796c7952638681.
The following fog-memory build also corrects the creep squares noticed here.
Its manual Zerg check confirms Y opens the Larva catalogue without spending,
A pays the selected Drone's cost, and the fifth Drone finishes natively.
ARMv6K / VFPv2 / no Thumb-2 or NEON validation passes; heaps remain 36/8 MiB.
This development stage has not been written to the user's physical SD card.

# Nine-stage campaign milestone — 2026-10-26

Source `src-b8b7a90a3285870e`. Adds original Egression (scenario23) and Into the
Flames (24); the first seven IDs remain stable. Original3DS heaps remain36MiB /
8MiB. This is not an all-campaign completion claim.

Original Egression wins at frame6460 using native worker pickup, Hunter Killer
rescue/escort and Chrysalis delivery to the original beacon. Save/replay while
holding the actual item matches. All six campaign artefact types independently
pass pickup, Bring counting, death drop, second-race pickup and explicit drop.
No artificial objective counters or altered original maps are used.

Original Into the Flames wins at24517 using paid economy/combat,15-minute timer,
original rear-force creation, live Fenix and Cerebrate death. Completed replay
matches. The bottom clock displays the countdown when the map sets one.
A real legacy CHK bug was found: Create Unit count0 creates ONE, not zero. Fixed
both ordinary creation and creation with properties. Format reference:
https://docs.scmjs.dev/triggers/create-unit/

Nine startup/save/replay/48-frame continuation and17248-frame idle tests PASS.
All nine original required-force/hero-loss fixtures PASS.139 original dialogue
WAVs (117 prior +8 Zerg2 +14 Protoss2) stream byte-exact through bounded mixer,
zero I/O errors. Trigger arithmetic/creation/paid originalAI tests and ARM11
architecture PASS. New maps' unit response banks remain private.

Original-model CPU100 Azahar:
- Protoss2 briefing played8, missing0/IO0; native map and T-14:21 clock visible.
  Audio-ready normal30,995,856 bytes, linear free123,904.
- Native reconstructed Egression held-item save, a real A movement and Save Game
  succeeded. The native saved frame1916 replays on host: Drone434,123 HP40,
  Chrysalis216 held, selected;120 additional host ticks PASS. Native audio-ready
  normal31,141,856 bytes, linear free123,904. This is real native save evidence,
  unlike the earlier raced larva-save attempt.
- Emulator stopped; original model/profile/both original save slots restored.
  Physical SD untouched. No physical-console performance/full-win claim.

Creation correction changes genuine reinforcement waves in earlier maps. Fresh
host victories pass for Wasteland, Zerg1 (21972), Protoss1 (23544), BW Protoss1
(10771), BW Zerg1 (13725), Terran2, Zerg2 and Protoss2. Protoss1 test now protects
Fenix and restores Pylon power instead of stalling an unpowered Gateway.
IMPORTANT: the prior BW Terran1 autoplay strategy loses against the correctly
created reinforcements. Its old-source37012 victory must not be attributed to
this build. No new runtime exception is reported in that match; tactics and a
fresh complete victory still need work. Preserve the previous seven-stage ZIP.

A private60-map compatibility audit found24 startup successes,26 load rejections
and10 early tick rejections; it is NOT a victory audit. Remaining issues include
14 large maps, missing AI/trigger operations, doodad/trap states and map loaders.
Private raw report: local-data/campaign-all-audit/runtime-before.csv.

Private incremental package: local-data/releases/campaign-nine-update-2026-10-26.zip
(requires the previous six-campaign asset package). Receipt under private
local-data/campaign-six-validation; no purchased content is tracked or uploaded.

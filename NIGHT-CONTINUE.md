# Overnight continuation — 2026-10-19 Europe/Vilnius

User explicitly authorized further development of other races (Protoss first,
then Zerg), unattended overnight, with a 05:27 continuation for five hours.
No subagents requested. Retain original 3DS 36 MiB normal / 8 MiB linear heaps
and existing Circle Pad/A/B, X/L+X, Y/L+Y and R+D-pad controls.

## Scheduling and sleep

- One-time same-thread heartbeat `starcraft-3ds-t-sinys-05-27`, active for
  2026-10-19 05:27 local time, requests useful development until 10:27.
- Current account five-hour window resets 05:27:56; if a scheduled run starts
  before that and is limited, wait until 05:28 once. Do not bypass limits,
  purchase credits, change models/accounts or make empty self-ping loops.
- `caffeinate -i` PID 98053 prevents idle system sleep until 10:27. Verified
  with `pmset -g assertions`. Display sleep is allowed; no permanent power
  setting changed. Do not kill this user's authorized overnight assertion.

## Starting point

`60e256c` is the tested full Terran milestone; clean checkout before new work.
Full Terran code/host/native tests and emulation passed; details in
`nintendo3ds/reports/TERRAN_VALIDATION.md`. The SD was updated, hash-verified
and unmounted. Do not claim newer work is on SD unless it is mounted and
actually updated again. New race work must preserve that tested Terran path.

SDK `/Users/karolynaz/devkitpro`. Build:
`make -C nintendo3ds DEVKITPRO=/Users/karolynaz/devkitpro -j2`.
Host `HOST_CXX=clang++`. Private owned MPQs in ignored `data files/`.
Staged private data in `nintendo3ds/local-data/3ds/openbw/`.
Map `maps/arena.chk` is flat 64×64; `graphics-arena.bin` holds terrain only.
Resident GRPs feed the GPU sprite cache; no need a static all-frames atlas.
The 202-effect Terran PCM8 pack is already 7.8 MB; adding all races to one
resident effect bank will exceed memory. Use race-specific sound banks or
another bounded approach, retaining full-length music and original sounds.

## Implementation sequence

1. Add complete standard Protoss catalogue: sixteen buildings, trainable units,
   Carrier Interceptors, Reaver Scarabs, Archon/Dark Archon merge, seven
   research techs, weapon/armor/shield upgrades and all special upgrades.
2. Generalize adapter recipes/menu/supply/race handling without changing native
   costs/prerequisites, Probe warp construction or psi requirements.
3. Add manual Protoss lab and prepared Protoss battle, bounded scoring/time.
4. Integrate native Protoss abilities, shield information, and native fighter
   production/merge actions. Verify public commands with private-data tests.
5. Prepare bounded original sound/music bank and render coverage; validate ARM
   architecture, heaps, and emulator behavior. No false hardware-performance
   claims (the full Terran emulator battle is 45 FPS mean / min 12 / ~24 TPS).
6. Checkpoint tested stages with local commits and update this file. Continue
   Zerg if time/usage permit; document exact unfinished parts candidly.

## Checked milestones as of 07:40

Protoss committed as407e02c. Zerg integration now complete for all standard
unit/building recipes, native research/upgrades, Queen/Defiler powers, burrow,
Overlord transport, cancellation/refunds and paired Nydus construction/transit.
All three race host suites pass. Zerg battle has144 troops +25 Overlords and
Hatchery per side, native200 supply providers; peak342 objects, frame720,
blue wins scores2050/4000, losses130/121. No campaign or ordinary Proto/Zerg
opening economy AI is claimed. ARM11 build checks pass.

ASan reference/cache tests pass: creep48576 pixels, Zerg body18426789,
movingcache16053103/2361 misses, no overflow; earlier races unchanged. Single
Engine18 cross-race battle/lab reloads pass. Scheduler/input/map recovery,
MPQ ADPCM, audio lifetime/race unload and asyncCSV tests pass.
Zerg bank192 full effects6433618 PCM8 bytes +3 full original tracks, PCM exactly
matches FFmpeg. Other race banks separate. Original36/8 MiB heaps unchanged.

Final Zerg emulator battle:48.184 FPS mean,17.38 min,~23.98 TPS; heap32312768,
linear7789568 bytes; no backlog/missing/cache/audio-stream/CSV errors.
Voice budget:207 effects played,2984 no-voice requests,13 off-view culled.
First tick sample123.49 ms, latermax35.49 ms. Dense graphics FPS remains slow.
Original-model/CPU100/DSP HLE only; no actual console evidence.

Native manual Zerg lab A/Y produced Drone. Zerg battle→Proto lab→Zerg lab→Proto
lab passed. One earlier navigation attempt ended emulation/RPC, cause unknown;
explicit-row repeats passed. Emulator stopped and original settings restored
from local-data/zerg-emulator-original/model.json. DSP placeholder unchanged.

Private ZIP ready:local-data/releases/all-races-2026-10-19.zip,25 files,~344 MB.
All CRC/owned-input hashes and built/staged/emulator/ZIP binary hashes match.
Physical SD absent, not updated. Preserve previous Terran/Proto checkpoints.
Detailed reports:ZERG_VALIDATION.md, all-races-artifacts.json, zerg-emulator-*.

All-race milestone committed as03c6de7. A subsequent 1024x512 paired texture
cache experiment stayed within the same4 MiB GPU budget and passed all three
race pixel/ASan/native checks, but measured46.684 FPS versus48.184 baseline.
It was reverted; reports/render-cache-trial.json records the result. No
unverified experiment replaced the private SD ZIP. Native build restored and verified equal to the checked7f452542... release
and private ZIP. Emulator copy restored to that same checked binary; model
settings restored and emulator stopped.

All requested race integration/test stages are complete. Next practical step
is the user's original-console test with this package; hardware/DSP firmware
and physical controls remain unmeasured because the SD is absent. Campaign AI
and a pixel-identical renderer are later work. No empty wake loops/paid-credit
changes. Temporary caffeinate assertion expires10:27; emulator settings restored.


## Returned console SD: scenario loading crashes, evening 2026-10-19

Two new Luma dumps identify the SDK DSP thread pushing onto unmapped stack.
The initial exception is unproven. A real uncaught Audio::load bad_alloc path
was reproduced and fixed: effects now use <=64 KiB blocks, partial failure
cleans up, and streaming music paths do not allocate C++ heap. Load graphics
scratch before segmented audio; the reverse order was rejected after a caught
graphics bad_alloc in a repeated Terran load. Keep original 36/8 MiB heaps.
Automatic per-stage closed journal: logs/scenario-load.log, build reload-safe-v2.
ASan audio/failure/boundary tests, three PCM-bank equality checks, common host
regressions and ARM11 build passed. Final original-model/CPU100/HLE emulator
boot plus eight cross-race reloads all had graphics and audio ready.
Detailed evidence: nintendo3ds/reports/SCENARIO_RELOAD_FIX.md and
reload-fix-emulator.json. Raw dumps, old/new matching ELF and executables are
private under local-data/console-crashes-2026-10-19/.
Actual console CSV active battle FPS is ~12.5–12.7 Zerg, ~19.9 Protoss,
~10.7 Terran (TPS ~19.8 in Terran); lab ~59.8 FPS. Rendering optimization
remains, and emulator performance must not replace these hardware results.
Final executable SHA 2117f7153527fc98ec2a233055ed8dcc31c8c9c107674add62c11b6fd9001b81.
Updated SD disk4s1, all25 file hashes checked; only exe/README changed. DSP,
Luma dumps and CSV unchanged. Prior ZIP retained, new private reload-fix ZIP
prepared and CRC/input/exe checked. Await user's console retest for crash proof.
Emulator stopped and original model settings restored.
SD synced and disk4 safely unmounted after final delivery.


## Performance update after loading fix — 2026-10-19

Implemented dual-VBlank-aware pacing (60 FPS cap retained), opaque texture
batching with depth/alpha masks and ordered translucent effects, and precomputed
team/light colors for GRP decoding. Standard 36/8 MiB heaps/CPU retained.
New CSV phase timers and optional summarizer fields support next hardware test.
All three ASan cache/pixel references and 75 complete viewport software
composition comparisons pass; pixel fingerprints unchanged after palette/RLE
optimization. Frame pacing tests, common host regressions, 18 cross-race engine
loads, audio allocation-failure test and ARM11 build pass.
Matched original-model/CPU100/HLE emulator full battles, baseline→optimized:
Terran44.36→53.79 FPS, Protoss51.27→59.72, Zerg47.64→57.75; all ~24TPS,
identical native outcomes, no missing sprites/cache/music/I/O/CSV errors.
Earlier independent Zerg baseline47.76 agrees. Native A/Y Nexus→Probe and
health/shield UI overlays tested in Proto lab after all three battles.
These are emulator results: actual console gains remain unmeasured.
Reports: nintendo3ds/reports/PERFORMANCE_VALIDATION.md and performance-2026-10-19.json.
Private raw measurements, analysis script and exact baseline/final binaries/ELF
in local-data/performance-2026-10-19/. Emulator stopped, original settings
restored, emulator copy restored to final performance binary.
Final SHA1660ed166b3ed5c9334269c994e3dfea1fe33a5fc9c0aed43624d7f2f2e4e34f.
Private all-races-performance-2026-10-19.zip prepared/CRC/owned/hash checked;
older reload-fix ZIP preserved. No mounted SD, so no performance SD update.
Next useful step is a physical SD test and inspection of new CSV phase timings.

Performance package now delivered on next user insertion: disk4s1 UUID verified,
25 read-back hashes pass,2 files changed,15 DSP/log/dump files preserved.
SD synced/unmounted successfully. User testing fixed1660ed... performance build.
Further development must not overwrite that test identity or claim new work
is already on SD. Next local stage: basic Protoss economy skirmishes (AI/AI and
player/AI), normal starting base/resources and native costs/psi/production.


## Protoss normal-economy milestone — 2026-10-19

Added scenarios12/13: Protoss AI vs AI and player vs AI, under START > Scenarios
> More scenarios. Both start with native Nexus +4 Probes +50 minerals. Shared
bounded opening controller mines, builds Pylons, up to2 Gateways, Assimilator,
Cybernetics Core, trains workers/Zealots/Dragoons and sends waves. Native costs,
psi, supply and build times;40-object bot budget, manual limits unchanged.
Building-elimination victory; prepared all-race stress/lab fixtures unchanged.
Basic strategy only: high-tech/research/expansion planning and Zerg economy AI
remain next work. Full manual catalogues still work. Not Blizzard campaign AI.

Host full match, both-side gas/Dragoon coverage, paid manual Probe and ownership
assertions pass. ASan full match:122 viewport draws,9348 banked native sounds;
24 cross-map/race reloads and Terran/gas/Protoss/Zerg regressions pass. ARM11
build valid original3DS36/8MiB. Native emulator whole match frame11520,blue,
losses34/10:478 active samples59.833FPS/23.998TPS, maxheap33,804,928 and
linear7,789,568bytes; music track1→2, no graphics/audio/I/O/CSV errors.
Manual emulator A/Y Nexus purchase spends50M and completes fifth Probe while
blue independently builds economy and attacks. Exited normally, emulator stopped,
original preferences restored. Details PROTOSS_ECONOMY_VALIDATION.md and JSON.
Final executable fee7bb7dd4a421a0d77300b1fc43302027c92230f0521fcd1c84e0480f5b8311.
Exact ELF/candidate and raw logs in local-data/protoss-economy-2026-10-19/.
Local private protoss-economy-2026-10-19.zip checked25files/344050417bytes,
SHAdf6dbf596f635ee460267f405e24d76f361c89257203f9f3b7070432ff888040.
Canonical local staging now contains this feature; OLD performance ZIP unchanged.
SD is unmounted and still performance1660ed build, NOT fee7 feature. Do not
update while user tests; preserve SD test identity and use performance matching
ELF for any returned Luma dump. Next physical logs can quantify real FPS gains;
next game milestone is bounded Zerg economy AI/native morph opening.


## Rendering correction from physical screenshots — 2026-10-19

User reported colored/translucent duplicate silhouettes, dark Nexus glow discs
and solid-looking echoes. Shadow cache frames now contain black coverage;
fire/glow uses additive RGB approximation of the original remap rows. trans50
keeps source/team colors and half opacity. Native High Templar echo sprite449
retains native frames/position with half opacity (cloaked112 retained). No fake
units/trails or simulation changes. Ordered translucent blending restores alpha
before UI; opaque texture batching and 36/8MiB heaps remain unchanged.
All3 ASan cache tests pass:35,970,868 cached pixels,75 complete composition
comparisons, independent original palette/GRP references,234 moving Templar
afterimages. Original3DS ARMv6K/VFPv2 build passes.
Executable fd6a1cb0c9cd1412eb9b74d395948fae0fd2916f0721ca550024505fef767b60;
matching ELF/candidate/rawlogs in local-data/effects-2026-10-19/.
New local private effects-fix-2026-10-19.zip checked25files/344051472bytes,
SHA466dc305ff184b9a645585a54deae14bf9cefd0cb198208dae1642efa110edec.
Includes previous Protoss economy milestone. Existing performance/protoss ZIPs
retained. Physical SD still untouched1660ed performance build while user tests.
Original destination-indexed color blending is still approximated in RGB;
warp/cloak/full palette composition remain later work. Report EFFECTS_VALIDATION.md.

Final effects binary emulator originalmodel/CPU100/HLE: Protoss stress complete
frame720,blue,scores4400/8227,losses62/41; 29 final-boot active samples
59.684FPS/23.997TPS, no graphics/audio/I/O/CSV errors.
Reloaded Proto lab, visually verified Nexus glow/shadow, normal UI; exited
START > Exit. Emulator stopped and original preferences restored. Summary
nintendo3ds/reports/effects-emulator.json. SD unchanged during this stage.

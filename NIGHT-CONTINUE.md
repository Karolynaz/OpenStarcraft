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

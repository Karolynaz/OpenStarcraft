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

## Progress at 07:10 continuation

Protoss catalogue, native production/research/upgrades, all powers, merge,
fighter ammunition, transport, prepared battle and lab are implemented.
Host tests and Terran regression pass. ASan sprite reference tests pass for
both races. Separate 202-effect Protoss sound bank (8,368,683 PCM bytes) and
three full original music tracks validated against FFmpeg. ARM11 build passes.
Original-model emulator battle measured 52.09 FPS mean / about 24 TPS; full
results and limitations are in nintendo3ds/reports/PROTOSS_VALIDATION.md.

Native reload initially failed std::bad_alloc. Audio::unload now releases old
PCM bank capacity and Graphics::clear releases CPU scratch before temporary
core initialization. Fixed build successfully loaded Protoss lab from battle;
A selected Nexus and Y queued Probe (50 minerals charged, queue 1). The emulator round-trip Protoss → Terran → Protoss lab passed. Audio unload/race music ASan test
and both sprite cache tests pass after the fix. Emulator model config restored; fixed binary staged and hashed in
reports/protoss-artifacts.json. Checkpoint Protoss, then continue Zerg.

Then implement Zerg via original native Larva/unit/building morph actions,
creep placement, research/upgrades, powers and transport. No Zerg completion
is claimed. No new build copied to SD (card absent). Keep 36/8 MiB heaps.
Emulator stopped and original model keys restored from
local-data/protoss-emulator-original/model.json after testing. CPU100, DSP HLE.
The existing DSP file is empty; do not claim console/DSP firmware validation.

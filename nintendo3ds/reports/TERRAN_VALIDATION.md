# Full Terran build — 2026-10-19

This is a standard multiplayer Terran implementation/test profile. Campaign
heroes, campaign presentation/AI, saving and multiplayer networking are not
part of this milestone. Original simulation and original privately owned
assets remain in use; no Blizzard assets are committed.

## Functionality

- Twelve SCV buildings and all six legal add-ons, including mutually exclusive
  Command Center and Science Facility add-ons.
- Fourteen production products: SCV, Marine, Firebat, Medic, Ghost, Vulture,
  Siege Tank, Goliath, Wraith, Valkyrie, Dropship, Science Vessel, Battlecruiser,
  Nuclear Missile. Siege mode, native turrets and Spider Mines are represented.
- Eleven research technologies and fifteen upgrade types, including all three
  levels of the six weapon/armor upgrades. Original prices, prerequisites,
  energy, supply and completion times apply.
- Siege/unsiege, Stim, cloaking/decloaking, healing, Restoration, Optical Flare,
  Lockdown, Matrix, EMP, Irradiate, Yamato, Scanner Sweep, mines, transport,
  Bunker entry/unload, nuclear strikes and building lift/land command routes.
- Controller production/build/ability menus; groups expanded to 192 IDs with
  native commands still batched twelve at a time.

Terran lab provides completed prerequisite buildings and 10,000 of each
resource, with technology initially unresearched. Player vs AI retains its
normal opening; its small opponent controller still produces Marines rather
than autonomously traversing the full Terran tree.

All Terran battle is a separate prepared symmetric fixture on a flat 64×64
arena made from the user's original buildable/walkable tile. Each side starts
with 114 troops across thirteen types plus one missile, 200 supply including
the missile. Native supply buildings and a legal silo are present. Its small
controller uses tanks, mines, transport, spells and nuclear strikes.

Army elimination ends immediately. After 30 simulation seconds, loss of 65%
of initial army resource value also ends the battle. Otherwise the hard limit
is 180 simulation seconds or 180 active real seconds. Remaining army
mineral+gas value decides the result; buildings, missiles and mines are
excluded. Paused time does not count. This avoids a prolonged base hunt.

## Verification

`prepare-audio` and `prepare-graphics` run private-data integration scenarios:
all twelve buildings and all six add-ons constructed through public commands;
all fourteen products manufactured; all eleven techs researched with
cancel/refund/restart; all upgrade levels completed and maximum levels
rejected. Tests also exercise siege, Stim, both cloaks, eight targeted energy
abilities, Medic healing, Dropship and Bunker load/unload, lift/land, mine
placement and detonation of a normally manufactured missile.

The deterministic stress integration test passes at frame 720: 228 initial
troops, peak 333 native objects (including subunits/effects), blue wins,
remaining scores 475/575. A separate test verifies the 180-second real-time
cap even without advancing the simulation. Original economy, gas, refund,
selection, combat, scheduler, input, map rejection and recovery tests pass.

AddressSanitizer tests pass for audio buffer lifetime/PCM8, missing DSP, EOF,
pause/reload, asynchronous CSV queue ordering/overflow/drain/error handling,
and the GPU sprite-cache model. The cache test compares 3,545,808 pixels to
original GRP/palette data and 8,722,438 cached/swizzled pixels while camera and
combat advance. No cache overflow. Terrain/minimap texels and signatures pass
for both graphics packages. Final ELF is ARMv6K/VFPv2 without Thumb-2/NEON.

MPQ extraction exposed two inherited sector-length bugs: a full final sector
was treated as empty, and the removed compression-prefix byte remained in the
decoder input length. ADPCM output bounds also subtracted in the wrong
unsigned direction. Synthetic partial/full-sector and odd-buffer tests now
pass under AddressSanitizer. The prepared package contains 202 complete
22050 Hz signed PCM8 effects/voices (7,809,806 PCM bytes), plus the three
complete original stereo PCM16 music tracks. All samples match independent
FFmpeg conversion of the extracted WAVs.

## Original-model emulator result

Emuautotest/Azahar 2126.2, `is_new_3ds=false` and CPU 100%. This is an emulator
measurement, not an original-console performance result. See
`terran-emulator-summary.json`.

- Full 30-second battle completes and pauses at frame 720, blue wins.
- Mean 45.28 FPS; observed one-second samples 11.61–59.89 FPS. The dense
  combat interval is slow. Mean TPS is 23.99 excluding the partial final
  second (23.72 retaining that 15.96 TPS final sample). Maximum backlog one.
- Peak sampled objects 332; normal heap 33,685,256 bytes; linear 7,789,568
  bytes, within unchanged 36 MiB / 8 MiB reservations.
- Zero missing image layers, cache overflows, missing effect IDs, queued
  sound-event drops, music underruns, audio I/O errors, DSP drops, CSV queue
  drops or CSV errors.
- Eight effect voices plus one stereo music channel are intentionally bounded.
  2,352 requests did not obtain an effect voice during the mass battle; this
  is priority/budget culling, not a music interruption. 149 effects played.
- The 121-effect preliminary build had almost identical mean FPS (45.14),
  but missed 742 sound requests. The complete 202-effect package resolves
  those missing IDs; it does not resolve the dense-combat simulation cost.

Manual emulator smoke testing loads Terran lab, selects its Command Center,
opens the production/add-on menu with Y, starts SCV production, charges the
native price and completes the queue. All remaining gameplay paths have the
private-data integration evidence above; physical controller ergonomics and
full-battle performance still require the user's console run.

## Memory/render/log changes

The old static sprite atlas cannot hold all Terran frames within original-3DS
linear memory. Graphics format OB3DGFX2 stores terrain/minimap only. Original
GRPs already resident for iscript provide pixels to a bounded, pinned-frame
LRU GPU cache (at most eight total 512×512 RGBA5551 pages / 4 MiB). Sprites
are not read from SD during combat. Palette glow is approximated against
black with alpha; full destination-dependent palette compositing/distortion
is unfinished. Cloaked/burrowed observer images use faint alpha while native
detection rules remain active. Terrain is initially explored for both slots.

The imported user-console gas log measured 58.47 FPS / 24.03 TPS, with zero
stream errors, but also a 2,063,879 µs CSV flush on the main thread. The new
64-row asynchronous CSV queue moves SD writes/flushes off the render thread;
Exit drains it. Emulator background flush max was 1 ms. Hardware SD contention
and the larger battle must be measured again; the earlier console result does
not establish this version's performance.

## Console procedure

The updated SD needs the executable, BOTH graphics files, maps/arena.chk and
new audio/effects.bin together; keep the existing MPQs, maps/probe.scx, three
music files and console DSP firmware. Launch and let the automatic battle
finish. Use START → Exit before removing the card, then return the CSV. Watch
FPS/TPS during dense combat, Audio ON, Gaps 0 / IO 0. The result intentionally
arrives quickly; surviving bases need not be destroyed.

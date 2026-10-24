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


## Original cursor milestone — 2026-10-19

Replaced crosshair with native cursor/*.grp art loaded from owned MPQs (no new
package assets): arrow, own/neutral/enemy hover rings, colored command targets,
8 edge-scroll arrows. Time/drag art supported by loader, not new input modes.
17 kinds/74 frames. Native128x128 canvas offsets/hotspot64 retained. Cursor
animation100ms UI choice independent of24TPS. Menuarrow; A/B/CirclePad/minimap
mapping unchanged. Target ownership color does not guarantee spell validity;
build placement uses native validity. Two64x64 RGBA5551 textures add16KiB linear,
fixed8KiB scratch; no SD reads/allocation during frame rendering. 36/8MiB retained.
ASan independent original GRP/palette/hotspot/wrap/swizzle test passes303104
texturepixels, contextual/edge controls and no simulation sideeffects. 24reloads,
all3cache tests/75composition viewports, common controls/pacing/map tests pass.
ARM11 original3DS validation passes.
Final SHA39607cd8f58702873c6d4e5c87a4c7f9497e34450863682bc8e953ac3c9df5cb,
exactELF/candidate/logs in local-data/cursors-2026-10-19/.
Private original-cursors-2026-10-19.zip checked25files/344057384bytes,
SHA0b01387e0070b0ab7a932246c2cafcfa16a79793653e17aa26433ca7d0f82494.
Includes prior Proto economy and effects fixes; previousZIPs preserved.
Physical SD unchanged, still1660ed performance build unless user reinserts and
requests upload. Details CURSOR_VALIDATION.md. Remaining original UI frames,
command icons, portraits/title/menu are separate future milestones.

Final cursor executable emulator originalmodel/CPU100/HLE smoke: Zerg scene red
hover/menuarrow, Proto lab own Nexusgreenhover A/YpaidProbe, CirclePad/Aselects
newProbe, Ybuildmenu, redinvalid/greenvalidPylon target, B restoresarrow. Normal
Exit, emulator stopped and original preferences restored. Lab 138active samples
59.836FPS/23.996TPS, maxheap33704288,linear7805952;
no listed graphics/audio/I/O/CSV errors. Not a full battle or hardware benchmark.
See cursors-emulator.json. Physical SD unchanged.


## SD delivery on explicit user request — 2026-10-20

Verified mounted disk4s1 UUID7DFD8F37-8701-3051-A125-45E84C4C885E, installed
original-cursors-2026-10-19.zip:25 files read-back hash checked,2 changed.
15 DSP/log/Luma files preserved, evidence copied privately. Synced and safely
unmounted disk4; user told card can be removed/tested. Physical SD NOW runs
39607cd8f58702873c6d4e5c87a4c7f9497e34450863682bc8e953ac3c9df5cb,
not older1660ed. Matching ELF remains cursors-2026-10-19/candidate.elf; use that
for future dumps. Receipt/raw prior-console logs:
local-data/cursors-2026-10-19/sd-delivery-2026-10-20/. Prior1660 logs retained
there, not yet systematically analyzed; do not confuse them with new cursor run.
User next requested fog of war and ability UI improvements; subsequent local
work must not be claimed to be on the current SD until a later delivery.


## Fog / ability UI milestone — 2026-10-20

Default native fog/exploration for scenarios2/4/13. Prepared labs/stress/AI
observers remain explored and fog-off; START system row4 toggles Fog, Exit now
row5 (six system rows). Native tile states black/gray/visible feed double64x64
RGBA4 texture16KiB shared by battlefield/minimap, onequad/view, no SDread/frame.
Enemy units/images/minimap/hover/targeting respect native visibility/detection;
generic owner0 effects also require native sprite visibility. Fixed spritepool
scratch2500bytes only initialized when fogon. 36/8MiB unchanged. No building/
creep lastseen ghosts or smooth fogedges yet; intentionally documented MVP.

Unit Yfirstpower; L+Ysecond ifexactly2 else fullmenu. Worker/building menus remain.
Energy labels/checks consider matching selected casters; disabled options cannot
enter aiming. Rejected A keeps targetmode, Bcancels, selection changes cancel.
Scanner Sweep UI checks fp8::integer(50) (fp8 literals are raw units!). Native
vision/energy/duration unchanged. ASan fogtest scouts enemy400ticks, withdraw75,
all4096fogtexels/states and minimap match; ScanSweepreveals1tick, energyexhaustion
UI disabled, finalremainingduration174ticks. Own/scouted sprites remain visible.
24crossloads/common controls/cursors/all3cache75composition tests pass. Allrace
production/tech/powers/stress suites pass; native manual commands and Proto AI
11761/34:10 outcome unchanged. Original3DS ARMv6K/VFPv2 passes.

Manual emulator candidate3d23...: playerFogON/OFF, blackunknown viewport/minimap,
grayexplored terrain, Comsat CirclePad/A selection, Ytargetnamed50energy, Bcancel,
Acastnativeenergy, Ydisabledbelow50. Mean~59.83FPS/24TPS in152player/214lab samples,
no listed errors. Final1edf... tightereffectfilter: fullZerg battle720/2050:4000/
loss130:121 unchanged,29active samples57.371FPS/23.989TPS; Proto13reload ownNexus/
workers visible and unknown black. NormalExit; emulator stopped/preferences
restored. These are emulator checks, not new physical-console measurements.
Reports FOG_ABILITIES_VALIDATION.md and fog-abilities-emulator.json.

Final executable1edfb1a2c630115fb9d5af396e7bed9fdc26eaa1c702819a469ea1aa366a8eef;
exactELFs/candidates/rawlogs/tests local-data/fog-abilities-2026-10-20/.
Private fog-abilities-2026-10-20.zip verified25files/344061886bytes,
SHA3c378a7630fe2385440598785df56bc73f3fcae1f0ff9631661f32f39cdd302a.
Canonical staging/emulator now finalfogbuild, but PHYSICAL SD REMAINS39607cd...
cursor release from earlier delivery. Do not use fogELF for that card's dumps.
SD was safely unmounted; no secondwrite during user testing. Await next SD
insertion/request for newfog release. Earlier cursor/effects/economy ZIPs retained.
One-time Oct19 automation was paused on its stale Oct20 trigger; do not restart.


## 2026-10-20 user-authorized five-hour work session (no automatic extension)

Started 04:45:13 UTC / 07:45:13 Vilnius; requested window ends 09:45:13 UTC /
12:45:13 Vilnius. User explicitly says no automatic continuation afterward.
Old automation remains PAUSED. Work in this turn without adding automations.

Completed Zerg economy modes 14/15 on ordinary map, existing Zerg bank: native
Hatchery/four Drones/one Overlord/50 minerals, Larva morphs, Overlords, Pool,
Extractor, Den, paired Zerglings/Hydralisks and bounded 40-object opening AI.
Host full match frame12049 including240 post-victory frames, peak106, red winner,
losses23/46. Original3DS CPU100 DSP HLE emulator matches winner/losses at11796,
491 active samples59.8327FPS/23.9541TPS, heap31,977,400 / linear7,822,336 bytes,
zero backlog/cache/missing/music/IO/DSP/CSV errors; 474 intentional effect-voice
budget declines. Full-match executablea77c5058... and matchingELF/rawlogs at
local-data/zerg-economy-2026-10-20/. Protoss/Terran AI outcomes unchanged.

Added bounded last-seen building/resource images and minimap markers (64x16
layers), 4096-byte creep memory, no native pointers. Unseen death keepsmemory;
revisiting empty footprint removesit. Mobile enemies/hover/currentHPstayhidden.
Observer bookmarkrecall and autocamera respectfog; placement preview avoids
hiddenobstacleknowledge. Local soundssightfiltered. Fog hardwarelinearfilter
softens32px boundariesusingexistingmasktextures. Correctedblackcreep squares:
validCV5 variants0..12, common0..5/rare6..12 peroriginalOpenBWUI insteadall16.
LarvaYnowopensproductionmenu;L+Yremaining9thchoice,notgenericarmyorders.
Mineslabelremainingcharges. Allracecatalogues/stressscoresunchanged,75complete
cachecompositionviewports/cursors,28reloadASan,fog/scout/radar/bookmark tests pass.
NativeARMv6K/VFPv2/noThumb2orNEON,36/8MiB unchanged. Manualoriginal3DSemulator
verifiescreepfix,FogON/OFF,Hatchery→LarvaY8choices,paidfifthDroneproduction.
A manuallongscoutattemptaimedatimpassablebottommapedge,notanenginefailure:
hostnativecontextscoutofthenewpaidDronecorrectlymoves(1755,1882)→(1571,1453)
in120frames towardactualenemybase(288,1808). OriginalspawnsareBOTTOM corners.

Final executable3b97834496dc71eda0353c8240681acd7923446b4e0ac4e38cd8ed80c4003221,
ELF/rawlogs local-data/fog-memory-2026-10-20/. Localprivatepackage
zerg-economy-fog-memory-2026-10-20.zip:25files,344072362bytes,SHA
80a756f2462e33ea497da6daecae2d459fbbfeb9367c453f0b38b6fc76032b0b.
Canonicalstagingandemulatorhave3b978...; physicalSDSTILL39607cd...cursorrelease,
unmounted,no newinsertion/uploadrequest. Do not usecurrentELFforphysicaldumps.
Emulatorexited/stopped/preferencesrestored. ReportsZERG_ECONOMY_VALIDATION.md,
FOG_MEMORY_VALIDATION.md,zerg-economy-emulator.json.

Next withinwindow: originalcommandiconsandbetterbottomcatalogue. Inspected
privateoriginalsunderlocal-data/original-ui-2026-10-20/: unit/cmdbtns/cmdicons.grp
390frames,36x34canvas; unit/cmdbtns/ticon.pcx96x1indexremap,NOTitsmagentaPCXpalette.
CorrecticoncolorsremapGRPindices0..15throughfirst16ticonpixelsintoconsolePCX
palette. Second16aregray/disabled. game/tconsole.pcx,pconsole,zconsole are
640x480originalHUDpanels. glue/palmm/backgnd.pcxandrez/gluMain.binexist.
Temporary /tmp/sc3ds-ui-extract.cpp usesOpenBWGRPdecode; privatecontactsheet
icons-remapped.pngshowscorrectgoldicons. Commoncommands228+needverifiedmapping;
uniticonframe=unitid; research/upgrade/abilityiconsfromnativeDATiconfields.
Do not inventiconsorgeneratefakeBlizzardassets. ExistingpurchasedMPQsprivate.


## Final checked work in the current quota window — 2026-10-20

Quota clarification: the earlier 12:45 wall-clock estimate treated the user
message as a fresh timer. The account reports that the CURRENT five-hour window
resets at 07:27:34 UTC / 10:27:34 Vilnius. At the planning check 89% was used.
Do not start another window, use reset credits, or create automatic continuation.
The old automation remains paused. No empty wake/ping loops were created.

Added original command art: cmdicons.grp, native unit/tech/upgrade/order icon
metadata, original ticon index remaps into race console PCX palettes, and small
race button backgrounds. Catalogue is two columns/four rows with icons, costs,
disabled state, caption and shared D-pad/touch bounds. Native shortcuts retained.
Assets load once; incomplete loose assets retain text. The world atlas is reused
without extra linear allocation. The UI keeps the world's current pin age.
A focused all-390-icon pressure test models Citro2D's cached texture binding:
old forced-first-page uploads fail; binding the uploaded page passes. Pixel
oracle: 2,046,792 pixels, 294 catalogue mappings, 75 battle viewports/1200 UI draws,
no overwritten queued world regions or leaked allocations. Grid/CSV/common tests,
fog, 28 ASan reloads and Protoss/Zerg full economy regressions pass.

Emulator prototypes b20267ca... and28e9c1c3... verified Zerg, Protoss and Terran
menus. RIGHT then DOWN in Larva grid chose Hydralisk, paid75/25 and completed;
Terran Marine paid50 and completed. Final binding candidate a68d0127... rechecked
Larva grid. Then final source-identified release completed Zerg stress720,
blue2050/4000, losses130/121, zero graphics/cache/missing/music/IO/DSP/CSV errors.
Original3DS/CPU100/DSPHLE, linear7,822,336 bytes; manual-session heap peak34,069,384.
These are emulator checks, not physical performance claims.

Physical audit: returned CSV is IDENTICAL to the pre-performance-delivery CSV,
179509 bytes, SHAa091c027100469b072bb5a95e4c8e5f1b8a5cb2d1dc56388baa90ced4c47ba91.
It has no new render-phase headers. Do NOT attribute its old FPS to1660ed or
current builds; no new physical performance session is established by that file.
Added build/source_id.h generation from native source inputs, excluding assets,
absolute paths and documentation. Marker visible in diagnostics, load journal,
every CSV row; summarizer handles old unidentified rows. Every new row has53
matching fields. Source marker src-ef07ed30b780ef84. Exact executable SHA
28bf8de7a3c426230ff627dd1c53e295dff590553eb5193a7890e646383e97d8; ELF/rawlogs
local-data/build-identity-2026-10-20/. UI prototype ELFs/logs retained separately
under local-data/original-ui-2026-10-20/. Full Zerg economy a77c... remains there
in its own stage, not the current executable. No source/MPQ data published.

Canonical staging and emulator now source-identified28bf... release.
Private original-command-ui-2026-10-20.zip:25 files,344078553 bytes,SHA
b038692015b945d25f89dd9dfd40c3e277c2f99ad8e32f9a8696da094c54bc78.
Replaces an undelivered UI prototype ZIP; its old manifest is marked retired.
Prior Zerg/fog/cursor releases retained. PHYSICAL SD STILL39607cd... cursor
release, unmounted; use its matching cursor ELF for physical dumps. No second
SD write without a new insertion/request. Emulator stopped and original model
preferences restored. Reports COMMAND_UI_VALIDATION.md, command-ui-emulator.json,
PHYSICAL_LOG_AUDIT.md and source-identity-emulator.json.

Next useful user-driven work: physical test of this release, original selection
circles/HUD/title menus, then higher-tech/research/expansion AI and mixed races.
Current opening AI is mirror-race only; campaign missions/movies remain separate.
Do not continue these automatically after this window.

## 2026-10-22 — batched user UI work, test build complete

Implemented the explicitly collected SCV alignment correction plus title /
campaign / Custom Game frontend and supplied Figma node27:30 in-game HUD.
See nintendo3ds/reports/TITLE_HUD_VALIDATION.md and frontend-hud-emulator.json.
Final native src-61642441ae705794, 3dsx SHA256
ed1de87bcc44ede195e4518a183be91a076bfaf1ba7e46fc5d4ebce0acc9afc8.
Local private packages title-hud-2026-10-22.zip and
 title-hud-update-2026-10-22.zip under nintendo3ds/local-data/releases.
Emulator original-model menu flow, reboot persistence, Terran (intermediate)
and final Zerg manual openings/catalogues validated; touch injection limitation
remains, so physical touchscreen needs user test. SD not written this turn.
Map/speed/type selections are currently frontend-only by user instruction;
Play chooses mirror-race economy scenario4/13/15 from the player's race.
No automatic continuation was created or resumed. Await test feedback.

## 2026-10-23 — Custom settings layout and real two-computer opening

User collected changes, then explicitly said 'viskas padaryk'; implemented all
and the follow-up separate Cancel/OK panels + slide-in animation. Backgrounds
are private user PNGs. Native selected-map metadata/preview (up to256 tiles)
on top; five-row settings + separate buttons below. Random/Off for two AIs,
selected-map slot cap, all higher slots closed. Native three-side mixed-race
opening controller is real; actual selected-map/type simulation is still the
previously agreed test-scenario placeholder. Mixed enemy-specific audio banks
are not combined yet. See CUSTOM_SETTINGS_VALIDATION.md for tests/limitations.
Final src-5c4330564beb133b, executable SHA256
3d92656211bdfd545c23c3e61c497e73c5c2597e43e9ccdbb0d6087e37fad6d7.
Packages custom-settings-2026-10-23.zip and custom-settings-update-2026-10-23.zip
in local-data/releases. Exact ELF and raw emulator evidence in
local-data/custom-settings-2026-10-23. No physical SD write this stage.

## 2026-10-23 — original resource HUD icons

Top-right resource text replaced by original14x14 game/icons.grp art and green
counts, racial gas/supply variants, single measured/right-aligned row. See
RESOURCE_HUD_VALIDATION.md. Source src-361f99ef011a0937, 3dsx SHA256
4c8965b888df236e7b47fa8bd1191d33269c177d5b44545d5ddfa2aed84e55df.
Packages resource-hud-2026-10-23.zip / resource-hud-update-2026-10-23.zip under
local-data/releases. No new loose asset required. All three race mappings and
4116 original resource pixels tested; extended command/GPU tests pass; original
model emulator opening and10000-resource laboratory checked, normal exit and
config restore. No physical SD write this stage.

## 2026-10-23 — selected-unit information and native production queue

Implemented the user's bottom selection information panel, original wireframes,
five queue places/progress and exact touch cancellation through native actions.
Includes research/upgrade/morph/add-on/construction information and cancellation.
Rapid head-cancel guard prevents a native pre-tick unrefunded queue removal.
See SELECTION_PANEL_VALIDATION.md for final host/native tests and limitations.
Final src-867dc906d2e0277e, 3dsx SHA256
820ddedd45b3eb5a5be1d655703f094bcd3399a11f94fdb4a3f297d3adffb3f5.
Private selection-panel-2026-10-23.zip and selection-panel-update-2026-10-23.zip
under local-data/releases; matching ELF/logs in local-data/selection-panel-2026-10-23.
Final original-model emulator idle/active Command Center views verified; normal
Exit and model preference restored. Physical touch still needs user check;
wire damage colors approximate overall health. No physical SD write this stage.

## 2026-10-23 — actual selected maps, results/return flow, expanded melee AI

User explicitly authorized plan items2,3,4. Play now uses the selected map,
native starts and real chosen races/Off slots. Sparse starts remapped; maps
32..128 supported, terrain/minimap generated on-device into map-terrain.bin,
496-slot streamed GPU terrain cache; 64/128 fog masks. UMS explicitly rejected
with a retained-settings error, not silently replaced. Melee/FFA supported.
Victory/Defeat and Surrender result -> Play Custom return and second-match load
are real. AI gas/technology/upgrades and mixed ground/air production extended
for all3 races, budget56; expansion/transport/full spells remain outside this
stage. Full details/test results: reports/MELEE_VALIDATION.md.
Final src-bb5cf4a33b8d25fc, 3dsx SHA256
6e994bceb7ca2b53897ab671a451669940606f908be245b526cdf0702808a49f.
Private melee-2026-10-23.zip and melee-update-2026-10-23.zip under local-data/
releases, matching ELF/evidence in local-data/melee-2026-10-23. Three36k-frame
economy tests, 600-tile cache/385560 pixel checks and selection/fog/frontend
regressions pass. Final original-model emulator: UMS error, 128map+audio,
Surrender/Defeat/return, second64map+pan+Exit pass. Preferences restored and
temporary emu map removed. Physical SD not updated. No automatic continuation.

## 2026-10-24 — campaign cursor/animations/motion/readability

Combined both user messages: green menu cursor, smaller original animated
figures, larger race/episode labels and4 large146x28 buttons in2 rows. Complete
13 SMK cycles/319 frames now stream through seven double-buffered128 surfaces;
new required ui/animations.bin,448KiB linear; static menu reduced8->4 pages.
Idle+hover overlays preserved. Campaign/custom menus animate out/in380ms per
phase, gated input; Cancel/B reverses, keeps custom selections. Read
reports/MENU_MOTION_VALIDATION.md for test details and physical-test limits.
Final src-248f76a3f5e2d2be, 3dsx SHA256
e1de4471761fb3df334f7aaf747b8370c9346ec85ed69c18b3b334a5a3bf6856.
ASan full-cycle/GPU-texel and frontend tests pass, native original-model menu
roundtrip and subsequent game+audio+Exit checked. Private menu-motion-2026-10-24
full/update ZIPs in local-data/releases, matchingELF/evidence in local-data/
menu-motion-2026-10-24. Update now has6 files: MUST include animations.bin and
matching menu/manifest. Emulator model/profile restored. No physical SD write.

## 2026-10-24 — Play Custom larger text and metal supports

User-requested Play Custom window updated: larger top metadata and bottom
field/button fonts, fixed-size text fitting with two/six-line title/description
limits and ellipsis. Original pSlots/pListMap2/pInfo panel beams/corners and
PalNl Cancel metal mounts for the two footer buttons. Draw/input coordinates
move together: rows60/85/110/141/167; footer x18/178,y196,124x28. Atlas stays4
pages, animation bank unchanged. Read reports/CUSTOM_METAL_VALIDATION.md.
Final src-d4e7d4454ac23955; 3dsx SHA256
da14ffe71c43177f8f65153563863f329f833f1480b94de5e7df5866c5c86192.
Private custom-metal-2026-10-24.zip and custom-metal-update-2026-10-24.zip
under local-data/releases; matching ELF/evidence in local-data/custom-metal-
2026-10-24. ASan frontend/ARM checks pass; original-model emulator normal and
long-title/description previews, Cancel/B return and Exit checked. Test map
removed/model restored. No physical SD write. No gameplay/AI change.

## 2026-10-24 — in-game larger-font physical-test build

User asked to enlarge game fonts now and refine fit later after their review.
Changed only main/selection drawing: larger resources/clock/menu/captions,
selected unit titles/stats/HP/shields/production/queue numbers. Removed title
auto-shrink; work names keep one-line ellipsis. Minor baselines/portrait HP
backing adjusted; HUD layout/touch targets/game logic retained. Some long
system rows reach right edge, intentionally left for fit feedback.
Final src-0e959a2c62a058b2, 3dsx SHA256
5a50edc938fa71a3d0b7161e6e0e4992faccbe74040e63709dbd5c792dae1e23.
Private ingame-fonts-2026-10-24.zip and ingame-fonts-update-2026-10-24.zip
under local-data/releases; matching ELF/evidence in local-data/ingame-fonts-
2026-10-24. ARM check passes. Original-model emulator CC idle/training, menu,
rich Protoss lab/Nexus shields and Exit checked; preference restored.
Standard opening audio ready, but mid-process Protoss lab reload audio-off
with heap-memory guard. Recorded, not attributed to this font-only change;
see reports/INGAME_FONTS_VALIDATION.md. No physical SD write this stage.

## 2026-10-24 — console AI difficulty, owned map catalogue and bounded audio

User authorized Easy/Medium, more maps and useful further work. Original AI
script files exist in owned data, but this checkout has no interpreter; BWAPI
PC DLLs cannot be directly loaded on3DS. Extended existing native-action AI,
no external bot code imported. Easy8workers/14army/6min grace; Medium14/24/3min,
persistent sixth menu row. Medium second base, stable main anchor, refinery
spacing deadlock fix, incomplete-worker accounting, Zerg air roster room,
detector follow, last-seen building memory and ground sweep. New five-entry
map browser;87 owned original ground-connected maps32..128 (27SC/60BW), all
87 mixed-race load/terrain/240-tick ASan tests pass. All-race shared577-effect
bank NEW audio/effects-all.bin (22MB disk), bounded4MiB16KiB cache + background
SFX IO after music refills fixes whole-bank reload pressure. PCM byte tests for
all3 banks+merged bank, zero-cache allocation fallback/reset/unload/read-failure
and nine36k native economy/expansion fixtures pass. Three-race/two-AI native
match ends at11833 frames, peak147units, zero sound-event drops. Read report
AI_MAP_AUDIO_VALIDATION.md for exact native results/limitations and packages.
Final src-35e9cceb44d81d8f, 3dsx SHA256
be0596b848d32eac3910dd209f4e5d1e236228cb7ee9dbcd582a5af4561ceeb1.
Private easy-medium-maps-2026-10-24.zip and easy-medium-maps-update-2026-10-24.zip
in local-data/releases; update includes96 files, ALL87maps, UI and NEW shared
audio bank. Matching ELF/source/logs/screenshots/receipts in local-data/
ai-map-stage-2026-10-24. No physical SD write or scheduled continuation.
Final emulator CPU100/original model: seven audio-ready initializations with
4MiB cache; no missing sprites/sounds, music gaps or audio IO errors. Mixed
match median59.83FPS/23.93TPS; all-unit Zerg/Protoss/Terran stress medians near60,
but Zerg min39.41FPS and Terran min20.89FPS under heavy CPU/upload work. These
dips remain; not a claimed physical performance improvement. Stress results,
B return retaining settings, normal Exit, model/profile restore verified.
Exact source35e9cceb and all package entries/hashes rechecked. User should test
Easy/Medium balance and physical SD/audio/frame timing next. No automation.

## 2026-10-24 — accumulated title/custom/pause feedback

User said "padaryk atnaujinimus": implemented Game + Play Custom/Load Saved
three-button title, metal supports, text-only version hover; padded continuous
custom top backing/map/text, Players below name, complete bottom backing and
removed Create settings heading. New bottom Game Menu pauses/dims BOTH screens,
Circle Pad/D-pad/A/B and shared touch rectangles; Options holds test scenarios,
camera/CSV/reload/fog/exit. End Game confirms/returns to custom. Save/Load are
explicit unsupported notices, no fake serialization. World input/camera gated,
scheduler reset; B nested keeps pause/root resumes. Read nintendo3ds/reports/
MENU_PAUSE_VALIDATION.md. ASan frontend/pause navigation/layout and native ARM
checks PASS. Original-model emulator title/custom/hover/pause/Options/B/resume/
EndGame checked; frozen top image unchanged after X/Y/CirclePad; CSV TPS0 with
zero paused backlog, resumed23.93TPS. Final build src-795c51f4f52748fa separately
loaded/resumed/paused/exited, no sprite misses or audio IO errors. 3DSX SHA256
6b68f1f70b76d120a867b1ffdcc1b99b8da841fc1b7a88e91410775318907610.
Private menu-pause-{update-,}2026-10-24.zip in local-data/releases; evidence/
ELF under local-data/menu-pause-stage-2026-10-24. Emulator model/profile restored;
no physical SD write, publication or automation. Existing battle dips remain.

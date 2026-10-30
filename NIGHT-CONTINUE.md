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

## 2026-10-25 — shared title controls and centered captions

User authorized queued UI changes. Load left, Play Custom right, Options below
center in shared title frame, separate left/right/bottom native metal mounts.
Options still opens game-version dialog. Custom labels x35; Cancel/OK centered
in both axes using measured glyph dimensions. Pause menu/submenu captions also
centered; removed > marker (border/yellow still select). Native source
src-394b0d7f82ecef48; 3DSX SHA256
f007365608e39d3d083d05a2d1f12e14c3b975cfcc50c5372a383bb8ec622e75.
ASan frontend, ARM checks, original-model emulator all3title controls/custom/
load-gameplay/pause/resume/end-return passed. Physical test still needed; no
save implementation. Private menu-centering-{update-,}2026-10-25.zip under
local-data/releases; screenshots/ELF/source under local-data/menu-centering-
2026-10-25. Atlas remains4pages after removing unused entries. Model/profile
restored. No physical SD write. See reports/MENU_CENTERING_VALIDATION.md.

## 2026-10-25 — tactical AI and title observer scenarios

User requested scouts/adaptation, transport/islands, complex powers and four
observer fixtures including1Medium vs2alliedEasy, all3races. Implemented bounded
BattleTactics: reserved scout/transport/cargo roles, sight-limited air counters,
load/fly/unload/retry/return with caster cargo priority, selected native combat
spells and paid caster tech/production. Kept42Easy/56Medium budgets, reserve
headroom to avoid low-tier armies blocking tech. Per-side difficulty, native
alliances and team victory; observer fogOFF, rotating2/3base + action camera,
Scout/Cast/Drop HUD counters. Title AI tests has3islandMedium mirrors and
TerranMedium vs ProtossEasy+ZergEasy. Owned maps/AI/{islands,three-islands}.scm
plus observer-maps.json required by packages. Observer End Game returns title.
Read nintendo3ds/reports/AI_TACTICS_VALIDATION.md for scope/results/limitations.
ASan6economy regressions,3native transports,6native energy/cast tests, hidden
knowledge,4observer configurations/cameras, team death/victory allPASS. Three
60k ordinary-resource island economies build support and drop troops. Full
mixed match ended65736frames, winner2Easyteam, peak189units; casts56/0/16 and
drops49/12/20. Synthetic tech/energy grants exist only in isolated tests.
Final src-65651930a6229cc5; binary SHA256
3ae9399e998ef95624ba44115551d8179bbefeef350b420b6d08be3c1331f171.
Original-model CPU100 emulator booted all4scenarios; no sprite misses/audio IO/
music gaps; median59.83FPS/23.93TPS, early-run dips to28.94FPS recorded. Not a
full-battle physical performance claim. Preferences/profile restored. Private
ai-tactics-{update-,}2026-10-25.zip in local-data/releases; ELF/source/logs/screens
under local-data/ai-tactics-2026-10-25. No physical SD write/publication/automation.
Still limited: straight transport routes, basic air counters, no complete spell
roster or general strategy coordinator. Protoss may correctly skip unsafe storms.

## 2026-10-25 — persistent native Save / Load

Implemented build-specific replay-backed disk saves (embedded map + bounded
streamed input journal), native deterministic reconstruction and state digest.
One slot plus checked temporary write/previous .bak; title Load and START
Save/Load functional. Stores camera/autocamera; reconstructs AI/fog/units/
resources/research/queues/bookmarks. New commands after load can be re-saved.
Progress and B cancellation; wrong-build/corrupt saves rejected pre-mutation.
Reload current after restore uses extracted map. Exact-build compatibility,
long-game replay latency,12h/1M-event/2MiB-map limits; no crash autosave.
See nintendo3ds/reports/SAVE_LOAD_VALIDATION.md and CONTROLS.md.

Final src-a3274172bb26d242, 3DSX SHA256
b5ddee234af6abbae02ecf34cedcf31b3c2939a1b4d0a32a29d749fd88a36e77.
ASan save tests: queue/cancel/groups/fog/camera, continued frame checks, appended
commands/re-save, backup, corrupt/truncated/foreign files, early/mid cancel;
20,000-frame mixed AI reconstruction+720frames exact continuation; wall-clock
stress journal. Frontend regression and ARMv6K/VFPv2 checks PASS. Final original
model CPU100 emulator: new custom save, in-game load, fresh process/title Load
continued to frame49 from saved27; no missing sprites/music underruns/audio IO.
Earlier iteration also tested saved521->continued579. Heap max30,699,736bytes
in final short samples. Preferences/profile restored. No physical SD write.
Private save-load-{update-,}2026-10-25.zip in local-data/releases; binary/evidence
under local-data/save-load-2026-10-25. Archives exclude runtime saves.

## 2026-10-25 — first original Terran campaign mission

Wasteland (Original -> Terran Episode I) playable. Owned StarCraft.mpq nested
scenario extracted with campaign-export into private campaign/terran01.chk;
human slot1->0 normalization includes restrictions/forces/masks/triggers.
Private four-page text briefing and native objectives/subtitles/camera/waits;
original map units/resources/technology restrictions,96px Raynor rescue,
original victory/defeat. No custom BattleAI. Mission enemies guard natively;
this map has no build-AI script. Unsupported campaign trigger opcodes reject.
Core fixes: FORC disk0->native1 IDs; Wait retains action index; elapsedseconds;
Accumulate; specific-unit count uses requested player; Move Location uses
destination center; passive rescue initialization no longer aborts.
Campaign loader restores SIDE after generic open-slot race randomization.

ASan full paid playthrough: escort/rescue, mine, construct Barracks, train to
10Marines, victory frame7773. Factory forbidden. Fixture Raynor/total-loss
defeat branches PASS; campaign save/load+120exactframes PASS; owner swap
involution PASS. Frontend briefing/data/error/back/begin tests, prior savegame,
custom/mixed AI,4observers and fogmemory regressions PASS. ARMv6K/VFPv2 PASS.
Original-model CPU100 emulator: correct road/initialunits/supply10/18/resource
icons, native campaign save/load, cross-architecture replay of the full winning
host journal, Victory and progress.cfg, final Campaign menu returns to title.
Source src-88f350761cef9368, 3DSX SHA256
6333672f2be8bf887855c7c148bb786bc0d46b168bbe38b4caf15008f22797df.
Preferences/profile restored; no physical SD installation/publication.
Private releases campaign-1-{update-,}2026-10-25.zip include only initial
campaign assets, exclude runtime saves/progress. Evidence in local-data/
campaign-2026-10-25; see reports/CAMPAIGN_1_VALIDATION.md and CONTROLS.md.

Limits: first mission only; speech/portraits/movies and exact cinematic unit
freeze deferred (subtitles/waits present). Later campaign AI/opcodes/missions
still needed. Completion recorded, no mission2unlock. Saves build-specific.

## 2026-10-25 — Raynor speech and responses

User completed Wasteland and reported no mission/selection speech. Added
private campaign-audio-export + prepare_campaign_audio.py; merge_audio keeps
optional campaign bank.24owned hero Marine/Vulture responses +5TRIG WAVs.
Transmission7 and PlayWAV8 now emit reserved1200+STR sound IDs; once per
transmission before Wait. Shared606effects/23,204,191PCMbytes, fixed4MiB cache/
8voices; IDtable extended2300. Packaging checks key hero/mission IDs.
ASan full paid mission (frame7649), explicit1357/1532/1604 scripted and423/427
hero select/ack, savegame regressions allPASS. All606effects byte-exact atDSP
double (413streamed),29ownedWAV PCM exact, audio pause/memory/cancel testsPASS.
Original3DS model CPU100 emulator real rescue + Raynor select/move produced
3->5playbacks, missing/drop/IO/gaps0,59.83FPS/23.93TPS shortsample; heap30,741,760.
No human listening or physical performance claim. Model/profile restored.
Final src-29f5fa8e6b3d1c6f, binary SHA256
55ea5a4b256fb7b10c7d7e60f14eed07df6a7329bfb01de11b278e8fc126dad8.
Private raynor-audio-{update-,}2026-10-25.zip under local-data/releases; evidence
under local-data/raynor-audio-2026-10-25. No physicalSD/publication. Briefing
narration/movies stillpending; active speech not restored midway onLoad.


## 2026-10-25 — HUD/control v2

User supplied protoss_updated_UI.png and Figma node28:30. Imported exact320×240
skin; fitted minimap80×100; clock/MENU, original static48×48portdata portraits,
larger selection/production panel,4+5grid and rounded key badges. Private110
portraits convertedfromownedidleSMKs; existing4MiB atlas reused on demand.
World A/context+empty A drag, Bcancel, Selectallviewport, R+Xarmy/R+Yworkers,
L+Dpadstore/R+Dpadrecall/doublecenter, STARTpause. World X/Y/LX/LYpowers,
LRfifth/page; workerX/Ybasic/advanced. R+Apatrol,R+Bstop,L+Aattackmove.
Catalogue fixed X,Y,LA,LB/LX,LY,RX,RY,LR; Dpad+A/touch. Mixedpowersdedup and
Stim/Siege/Nukepriority; nativeeligibilityfilterspreservegroup. Ten-second
attackminimapmarker jump. NoncampaignUMStriggerssuppressed: priorcampaign
integrationhadresetlabresources50onfirsttick; paidqueue/missiontestsnowPASS.

ASan nativeHUDmixedpowers/patrol/stop/selection/portraits/journalPASS;
input/4+5geometry/aspectmaptestsPASS; selectionqueue/refunds/research/morph,
custommixedAI, existingcampaign+savegameregressionsPASS. ARMv6K/VFPv2PASS.
Final originalmodelCPU100 emulator43unitProtoslab61activesecond samples:
last59.85FPS23.94TPS, heap30,505,224 linear8,084,480, miss/overflow0.
VisualX/Dpad/Aplace/Bcancel/Selectgroup/STARTdimPASS. TouchRPCdidnotdeliver
pressesevenunchangedtitle; GUI fallbacknoWindowsAvailable; hostgeometryPASS,
physicaltouchconfirmationpending. Emulatorprofile/model/previoussaverestored.

Final src-66443a1419587c1c, 3DSX SHA256
8c7fdff68230ee5dede07c422789efabfd40e19a0c2e432e258b84def57e0cc9.
Private hud-controls-v2-{update-,}2026-10-25.zip; noSDwrite/publication.
See reports/HUD_CONTROLS_V2_VALIDATION.md and rewritten CONTROLS.md. Saves
remainexactsourcebuildspecific. Portraitsstaticbyrequest, onlyattackalerts
currentlyimplemented. Fullcampaign/AIobjectivesfromearlierhistoryunchanged.


## 2026-10-25 — Readable key badges from Diablo3Ds reference

User reported smeared tiny command labels. Read their local Diablo3Ds HUD:
23×23 exported Figma badges are blitted at native output pixels after palette
conversion. Copied generic X/Y reference art; generated A/B and35×23combo
capsules with14pxArial. Committed PNGs need no runtime font; all draw1:1 at
integer positions. Packed13sprites into unused existing HUD columns324+;
still256KiB GPU texture, background unchanged. Full badge touch height included.
Tool prepare_hud now installs badges; package checks13badge manifest field.

ARMv6K/VFPv2, ASan HUD/replay, 4+5geometry and badge pixel/reference/background
checks PASS. OriginalmodelCPU100 emulator reviewed world and build catalogue;
letters sharp/centered. Emulator preferences/profile/save restored; no SD write.
Final src-2fa7384bbd9ea1e1, 3DSX SHA256
5753cdd7f990f0de5496177078a52e5c56d53e0e313e9cd56c224b49f0d862d8.
Private readable-key-badges-{update-,}2026-10-25.zip, 212file22.68MB update.
See reports/KEY_BADGES_VALIDATION.md, local-data/key-badges-2026-10-25 evidence.

## 2026-10-26 — 10px Figma selected-unit panel

User requested Figma node31:59, nominal10px type, existing colors,64px Probe
wireframe; upgrades remain text. Read full Figma context/screenshot. Background
matches current supplied PNG exactly. Figma wireframe56 overridden by user64.
New native Roboto Bold10 raster font in unused HUD columns (117glyphs; still
256KiB GPU page). Name/stats x94,156/174+, wire17,153 at1:1, HP then shields
side by side y219. Existing colors kept. Native armor/shield/weapon upgrade
levels exposed as text. Production head94,174 and four24px queue cells134+
share touch geometry; all existing native cancellation tests passed.

ARMv6K/VFPv2, ASan selection/native cancellation/all races, HUD mixed powers/
portrait/replay and atlas bounds/background tests PASS. OriginalmodelCPU100
emulator Probe and idle/producing Nexus reviewed; layout fits. Model/profile/
previous save restored, no physical SD write. Actual10px font has7–8px visible
ink, matching typographic Figma size rather than the old misleading C2D scale.

Final src-957604597e5b9ddf. Binary SHA256
be2ee5a0def4cb3465baafa82c3bab026957adec679c10707eb26e0e2e5c959a.
Private selection-panel-10px-{update-,}2026-10-26.zip;22.69MB update. Evidence
in local-data/panel10-2026-10-26, report SELECTION_PANEL_10PX_VALIDATION.md.
Wireframe palette still previous approximation; not changed in this task.

## Active 2026-10-26 overnight request — six campaign openings

User authorized continuing campaign work now and one automatic restart at
2026-10-26 03:35 Europe/Vilnius. Existing paused heartbeat
starcraft-3ds-t-sinys-05-27 updated to ACTIVE, name StarCraft3DSkampanijos03:35,
once-only COUNT1 at03:35. No later rescheduling/reset-credit/purchase permission.
At00:34 current5hwindow23%used; reset advertised03:30. Goal: first Terran,
Zerg,Protoss in original andBW (six missions); if done+tested, continue missions
or real fixes. No physicalSD write unless separately asked.

Audit: all six owned maps extracted privately under local-data/campaign-six-
audit. Keys terran01,zerg01,protoss01,expprotoss01,expterran01,expzerg01.
Maps64–128tiles. Human owners vary1/3/7/2/5 etc; old normalize onlyswaps0/1.
Additional triggers needed: switches11/13, score21/27, deaths15, properties11,
AI16, ping28, invincibility43, give48, HP49, shields51, move39, alliance57.
Original VM scripts reachable opcodes audited via PyMS primary format source:
Zer1/Pro1/TB1A build/research/attack waves (~25opcode subset), PB1A/PB1B
mostlycapture/upgrade, EnBk enterbunker. Private bytecode+disassembly in audit.
PyMS references downloaded only to/tmp/sc3ds-{AIBIN,TRG}-reference.py; do not
execute/copy their implementation. Native OpenBW has no full AI VM yet.
Need replace mission1-only frontend/audio/globalHUD assumptions, implement
native trigger subset faithfully, actual original script-driven AI, per-map
speech IDs/banks, six-start/progression/win-loss tests+emulator/memory checks.

### 2026-10-26 01:40 checkpoint in progress

Six menu entrypoints/normalized original maps, additional CHK semantics and
bounded original-script AI now implemented. New headers campaign_catalogue.h,
campaign_triggers.h,campaign_ai.h. All six maps/banks staged privately in
local-data/3ds/openbw/campaign. Export/merge/verify tools support six keys.
Per-map banks avoid CHK string ID collisions; long voices use reusable128KiB
chunks, same256KiB long voice +4MiB cache. All58 mission WAVs byte-exact PASS.
Native src-57f63c0be9bfa4b8 ARMv6K PASS. Candidate full package being prepared.

SC T1 paid victory7649 and audio/rescue/save/defeat tests PASS. SCZ1 paid
victory22344 and SCP1 paid victory21393 on current source PASS. Protoss revealed
important depot rescue bug: rescuing Nexus must transfer entire rescuable base;
fixed against primary GPTP behavior (reference/tmp/sc3ds-rescuable-reference.cpp).
Corrected native Exactly comparison raw10 (old code2 was wrong) and resource
Subtract9. Dedicated native trigger contracts PASS before adding AI fixture.
All6 save/replay+48framecontinuation PASS, 5 idle runs17248frames; BWZ1 idle
loses2251 after Exactly fix. BW fixed-army naive route tests lose (not proof of
bug); BW3 normal victory paths remain to test and diagnose.

Emulator originalmodel CPU100 reviewed SCZ1 and BW3 openings at src8d48, largest
heap31.85MB/linearfree123904, all loaded world/audio. Stopped and prefs/profile
restored. Latest57f63c binary final emu pass outstanding. No physicalSDwrite.

CURRENT active tools may finish independently: host campaign-triggers-test
build /tmp/sc3ds-trigger-build.log; paid AI fixture failed CC placement using
trigger_create_unit, now switched to loader.create_initial_unit, rerun then
validate paid Zer1 sparse town. Also package_release running output
/tmp/sc3ds-six-package.log. Tests current passing logs:
/tmp/sc3ds-{six-checkpoint-extended,zerg01-current-playthrough,
protoss01-current-playthrough,six-dialogue,six-audio-memory}.log.
BWroute diagnostic tests /tmp/sc3ds-{expprotoss,expzerg}-route.log lose.
Narrow code/report checkpoint still to commit. Read reportCAMPAIGN_SIX_VALIDATION
for precise claims. Do NOT call six missions fully end-to-end verified yet.

### Checkpoint artifact status
Sparse-town original Zer1 AI PASS: Barracks1, Marines3, native spending600
minerals while mining continued (ending4280 is higher than4000 because mining,
not free production). Complete native trigger contract PASS. All58 actual
mission dialogue samples byte-exact; fragmented4MiB audio tests PASS.
Current binary SHA256211ea89158a63bbecafe2927b5fb699623c4907c30b4083bca074b6c4ace111e.
Private full candidate254files537279751bytes SHAe8e5e23cc7b1b924dcde09b851cb63c63f8733c1e59f32d6e4ff0baa94fb166b.
Private update139files188018144bytes SHA36bc42571e732d85f24de9ab36b2545841f6680be7760a3bd59f6241da044c53.
Paths local-data/releases/campaign-six-candidate[-update]-2026-10-26.zip.
Both are candidates: BW normal victory paths remain unverified. Their fixed
army diagnostics currently lose; improve actual routing/tactics, then inspect
original triggers if still wrong. Do not change maps just to claim normal wins.
Final current-source SCProtoss emulator startup is in progress; MUST stop and
restore config/profile from campaign-six-validation backups before leaving.

### 03:35 scheduled continuation received (2026-10-26)

New5hwindow confirmed0%used. Do NOT create another automation. Continue until
~08:35 or useful authorized work done. Important discovery since checkpoint:
root native UNIT loader ignored initial invincibility/burrow/cloak flags. This
broke BWZerg1 protected hives and ambushes. Implemented valid_flags16 invincible,
2 burrow and1 cloak in bwgame.h. Native ARMv6K and six ASan startup/replay/extended
runs PASS at src-754ddd9a610d794a. BWZerg now survives17248 idle frames, rather
than losing2251. New test campaign_initial_flags.cpp confirms original7hives
invincible and88enemy units burrowed. Compiled/run directly, no Make target yet.
Current bwgame.h + initial-flags test still uncommitted (save checkpoint next).
BWnaive actual routes still lose: expprotoss hero killed2609, expzerg army dies
2451. Need tactical route improvements and then original victory triggers.

Emulator57f63 SCProtoss startup completed, screenshot/log in validationfolder.
Emulator STOPPED and original config/profile restored. No emu still running.
Existing candidate ZIPs contain PRE-initial-flags57f63 build; must regenerate
before delivering final test package. Report remains historical until updated.

### CURRENT checkpoint — all six original victories verified, 04:25 continuation

Source src-1882ab71e63794f3, native ARMv6K/VFPv2 PASS. Current core adds
MRGN ground/air mask correction (bits0..2 ground,3..5 air; original was swapped),
and UNIT serial links for initial addons and Nydus pairs. These were actual
campaign blockers: BW Terran initial Factory could not train tanks; BW Zerg
last hive is across an impassable cliff and requires preplaced Nydus transport.
Contracts: six addon pairs and native paid tank queue PASS; seven invulnerable
hives,88 burrowed enemies,eight reciprocal Nydus entrances PASS.

All6 NORMAL original victories now ASan PASS using public player orders:
SC T1 frame7649; SCZ1 22344; SCP1 21393; BWP1 10672; BWT1 37012;
BWZ1 13725 (seven hive rescues, native Nydus transport, original final attack).
Original maps/triggers/stats unchanged; navigation fog disabled for repeatability.
Six EMPTY-HUMAN test fixtures also execute original defeat triggers PASS
(frame2,2,2,50,2,2). T1 also has explicit hero-death fixture and response voices.
All6 startup+save/replay48framecontinuation+17248 idle frames PASS on1882.
Trigger contracts including ground-vs-air masks PASS; Zer1 sparse-town fixture
trained3 Marines+Barracks, spent600 minerals. Use arena.chk for this fixture.
Added air fixture must be removed before AI town or it shoots test buildings!

Emulator currently RUNNING at src1882 BWP1 (scenario19), originalmodel/CPU100.
World/audio load PASS; heap31853576,linearfree123904. Pause bothscreens dim and
bottom menu PASS. Prior0c70 BWZ1 world/audio PASS heap31743272. Evidenceprivate
local-data/campaign-six-validation. MUST stop emulator and restore original
config/profile from this folder before final. Physical SD untouched.

Native/tests/core need current local checkpoint commit; reports/package old
candidate files remain stale57f63 and MUST regenerate after final validation.
User authorized further useful work; briefings currently TEXT ONLY, in-mission
58 dialogue WAVs and hero speech supported. No movies/animated briefing portraits.
Do not reschedule automation; 03:35 heartbeat already fired, newwindowends08:35.

### FINAL delivered milestone — 2026-10-26, source src-aece41334bc62432

All SIX opening victories verified with public paid/player orders and unchanged
original maps/triggers/stats: SC T1 7649, Z1 22344, P1 21393, BW P1 10672,
T1 37012, Z1 13725. ASan all6 original defeat fixtures + startup/save/replay/
48-framecontinuation/17248 idle frames PASS. Navigation fog disabled in the
normal win tests; this is not six full manual console playthroughs.

Also added original BRIEFING VOICES: all97 dialogue WAVs (39 briefing+58 in-game)
byte-exact through bounded mixer, IO0. Briefing paragraphs are paginated using
actual font widths, with continuous voice across text continuation pages. Scope
releases briefing4MiB cache before mission allocations. Corrected selected hero
names: Raynor/Fenix/Zeratul/Duran/Hunter Killer/Dark Templar. Frontend pagination/
voice metadata ASan PASS; obsolete preflight limit expectation updated to128,
real oversized/duplicate/truncated/missing-asset recovery PASS.

Final ARMv6K/VFPv2 native PASS. Original-model emulator finalAECE restores BWZ1
11233-frame checkpoint, A/context Nydus transfer, native save replayed on ASan
with digest equality: selectedHunter type53 at1352,3664 frame13374 PASS. Final
screenshot shows Hunter Killer label. Priorf31 Terran briefing4voices missing0
IO0 and world load PASS; largest opening heap31853576, linearfree123904.

EMULATOR STOPPED. Original model settings/profile and both save slots restored
byte-exact. Physical SD untouched. No further automation created/rescheduled.

FINAL PRIVATE PACKAGES (supersede earlier candidates):
local-data/releases/campaign-six-tested-update-2026-10-26.zip
145files200540403bytes SHA256645acc04ec5edc283dcc37d88616170d8077d8fddbcd5588e657cbeebf665641
Requires already-installed base MPQs/graphics/race music; merge3ds toSDroot.
local-data/releases/campaign-six-tested-2026-10-26.zip
260files549802010bytes SHA256472f416940d22c906d4ed9fdbd68fcf5ea4e2c17b10c24ee8a5fc42ed2d147e6
Complete private owned-data package. Neither includes profile/progress/saves.
ZIPCRC/file hashes/executable source identity PASS. Receipts/screenshots/logs
local-data/campaign-six-validation. Read reports/CAMPAIGN_SIX_VALIDATION.md.

Limits: six openings only; no further mission progression, movies or animated
briefing portraits. AI is audited original opening-script subset, not full
StarCraft computer AI. Future missions need opcode/trigger audit. Saves match
exact source build. No physical-device performance claim. User can now test6.

### ACTIVE NEW REQUEST — all campaign missions, 2026-10-26 ~07:40

User explicitly expanded scope to ALL original+BW missions and authorized TWO
new scheduled continuations after08:35 and after the next5h window. Supersedes
earlier "no new automation" restriction. Existing heartbeat updated to08:40
(one-timeCOUNT1), new starcraft-kampanijos-13-45 created for13:45(COUNT1), both
Europe/Vilnius today2026-10-26. Next08:40 workuntil~13:40, last13:45until18:45.
Do not create a third continuation or change account/model/buy/reset credits.

THREE UI REQUESTS DONE and tested at src-d3916d80e599ff23:
permanent mission title strip removed; doubleA same generation-aware nativeID
within350ms selects own same-type visible units, including larvae; all loading
screens black with green centered Loading below. New Engine.select_similar()
journalopcode16 and Engine.own_unit_at() reader. First A drag-select preserved.
ASan3larvae selection+save/replay+48framesPASS; all6startup/replay/17248idlePASS;
native ARMv6K PASS. Emulator originalmodel realdoubleA showed3larva icons. HostASan separately
verified newgroupjournal+replay3. Native save read raced GUI save and read prior
state selected0; DO NOT claim native-to-host3 from that log. Loading screenshot
exactblack/greenPASS.
Emulator STOPPED and originalprofile/model/slots RESTORED. No physicalSDwrite.
Private binary-only UI update local-data/releases/campaign-ui-update-2026-10-26.zip
requires previous six-campaign package. Receipt in campaign-six-validation.
Read reports/CAMPAIGN_UI_NEXT.md. Current UI changes need checkpoint commit.

ALL-MISSION WORK NOW: tools/campaign_inventory.cpp extracts owned raw+normalized
numbered/alternate maps to ignored local-data/campaign-all-audit. Found60 records,
14largerthan128limit. Canonical Terran skips cut maps07 and10: retailstoryuses
01..06,08,09,11,12. Don't assume numerical successor. BW triggerSetNextScenario
references branches05a/b(Terran),04a..f(Zerg),Bonus.scx(secret). Actual60inventory
has04a/b only, noBonus; needprobe otherarchivepaths orlistfile. Static AI audit
/tmp/sc3ds-audit-all-ai.py (read-only format reference/tmp/sc3ds-AIBIN-reference.py;
never import/execute its implementation) decoded all176referencedAIScript entries,
no missing orparseerrors. JSONinventory/opcodesprivate in campaign-all-audit.
Reachable newopcodes include9/10 waits,31suicide,32/33 alliances,37rescue,38DT,
45nukerate,61guard,64/65call/return,72multirun,75define_max,76train,78wait_train,
87/88vision,93/94branches,107..112specialactions. CampaignAIcurrentlyfirst6subset.
Large maps up to192x192,128x256 still rejectedbyvalidate_map; memory workneeded
under original3DSlimits. New conditions countdown1/kills5/least7/17/never23 and
newactions countdown14/nextscenario41/deaths45/doodad42 etc needauditedsupport.
Do not merelyignore unsupported gameplay actions and claim all missions work.

## 08:40 continuation — seven-stage milestone

Fresh08:40 user heartbeat received; work continues until~13:40. One remaining
13:45 continuation already exists; do not schedule more. Latest scope remains
ALL campaigns, not just six openings. No subagents authorized.

Source src-5d5ba6a3954a4350 adds Backwater Station as appended scenario22;
first6 IDs preserved. Generic exports now use mission keys, per-map sound merger
accepts --missions. Frontend episode cards resume first uncompleted available
stage; briefing arrows/D-pad allow replay. Original Terran2 fully won in ASan via
paid economy/rescue/combat at16967 with Raynor alive and12 transmission requests;
completed save replay matches. Seven startup/save/replay/17248 idle PASS. Seven
loss triggers PASS; T2 fixture must remove rescuable Raynor, NOT just human force.
117 original dialogue WAVs byte-exact through mixer PASS, including20 T2 voices.
Timer condition/action/pause/resume, Never and SetDeaths now supported; timer
fixture victory126 and active timer replay PASS; arithmetic edge tests PASS.
Frontend resume/replay/profile tests and ARM11 architecture PASS.

Original-model CPU100 Azahar T2 startup/menu/briefing/Loading/live world PASS,
no title strip. Audio ready30,971,408 normal, linear free173,056; briefing
played4/missing0/IO0; idle~59.8FPS/~23.9TPS emulator only. Not a native full-win
or physical-console performance claim. Initial virtual-SD prep review rejection
was resolved with NEW08:40 user instruction explicitly requiring emulator and
restricting only physical SD; narrow fresh review approved. Emulator stopped,
original model/profile/both slots restored. Physical SD untouched.
Report nintendo3ds/reports/CAMPAIGN_SECOND_TERRAN.md. Private screenshots/logs in
local-data/campaign-six-validation. Latest incremental private package
local-data/releases/campaign-seven-update-2026-10-26.zip (requires previous six
asset package); regenerate notes/receipt if status/source changes. Earlier UI-only
D391 package retained. All campaign work still substantial, do not claim finished.

## Continued 08:40 window — nine-stage working milestone (~10:00 local)

Current source src-b8b7a90a3285870e adds original Zerg2 Egression23 / Protoss2 Into
Flames24. Generic exports already support keys. Artefact pickup/drop was missing
in core; now six campaign item types supported using worker.powerup + carrying
flags, hidden actual item, native Bring counts and drop on death. Neutral and
carried artefact Move Location searches fixed. UnitView exposes carried item and
disabled state. Countdown shown instead of elapsed bottom clock when active.
Legacy Create Unit count0 must create ONE; corrected both11 and44, checked via
primary editor runtime-reference docs and original map Fenix spawn. This changes
real reinforcement waves, so old-source victories cannot be blindly reused.

Egression actual original rescue/escort/pickup/beacon wins6460, held-item replay
PASS. Into Flames paid economy/combat/timer/reinforcements/Fenix wins24517 with
completed replay PASS. Nine startup/save/replay/17248 idle +nine loss fixtures
PASS.139 dialogue WAVs byte-exact streaming PASS; 6 artefact lifecycle ASan tests,
trigger creation/arithmetic/paid townAI and ARM11 architecture PASS.
Original-model emu Protoss2 startup/briefing8/missing0/IO0 and timer PASS; Zerg2
held checkpoint loaded, actual A move, native Save Game, host replay of native
save frame1916 Drone434,123 HP40 carrying216 selected,120 continuation ticks PASS.
Evidence private campaign-six-validation. Emulator STOPPED and original model/
profile/both original slots restored. Physical SD untouched. No screenshot or
host victory alone is a native full-win/physical-console claim.

Fresh first-stage victories with corrected creation: T1, Z1 21972, P1 23544,
BWP1 10771, BWZ1 13725; T2 +new Z2/P2 also PASS. BW TERRAN1 OLD AUTOPLAY LOSES
against real newly created reinforcements; old37012 success is NOT valid forB8.
No runtime exception, but a fresh complete win/tactics remains. Trial tactics in
/tmp/sc3ds-bwt-tactics-trial.cpp; original tracked test restored. Do not hide this
remaining test limitation or call all9 fully victory verified.

Report CAMPAIGN_NINE_VALIDATION.md. New private9-update package to be generated;
previous7 package preserved. Core/host object /tmp/sc3ds-nine-engine.o isB8;
/tmp/sc3ds-native-chrysalis-check.log is actual native save evidence.

New tracked tools/campaign_runtime_audit.cpp audited60 normalized owned maps
using generic campaign scenario and1200 ticks; NOT a progression/victory test.
Private runtime-before.csv:24 startup,26 load-rejected,10 tick-rejected.14 map
size guards,8 missing AI opcode,4 missing condition,3 missing action,3 disabled
thingy loader,2 AI thread capacity,2 data_reader bounds. This gives next priorities.
No source/native build running at this handoff; preserve36/8MiB. Existing13:45
follow-up remains; no more scheduling. Work continues now in08:40–13:40 window.

## Large geometry infrastructure verified — source774686bb25688432

Dimensions up to256 peraxis; FogMemory actual vector/stride avoids128-overflow.
Large fog texture256², seven atlas pages instead of8, and2MiB SFX cache for
area>16384; original36/8MiB heap limits unchanged. Small maps keep4MiB/cache layout.
ASan original owned192²/128x256 NO-TRIGGER TEMP fixtures load/tick/save/terrain and
far-corner graphics/full65536 fogtexels/minimap cleanup PASS.139 originaldialogue
WAVs byte-exact with2MiB cache PASS; audio allocation-failure/fallback tests PASS.
Nine menu/startup/save/replay/continuation/17248 idle regressions PASS. ARM11 PASS.
Native original-model CPU100 geometry probes:192²411units audio-ready29,858,496
normal/451,584linearfree;128x256444units29,926,360/451,584. GPU textures3,948,544.
Tall-map native~3min idle59.6FPS/~24TPS, no missing/cache/audio IO/music errors.
NOT mission victory/script support and NOT console performance proof.256²custom
not hardware validated. Emulator STOPPED, originalmodel/profile/both saves restored.
PhysicalSD untouched. Report CAMPAIGN_LARGE_MAPS.md; private evidence/binary/ELF
archived in local-data/campaign-large-validation.9-updateB8 ZIP remains unchanged.
New runtime-large.csv60maps:25startup/20loadreject/15tickreject;14sizeguards gone.
Still missing AIops, conditions/actions, disabledthingy and2unusedcutmap formats.
Current next work: broaden originalAI/triggers. Source774host object
/tmp/sc3ds-large-engine.o; no active emulator/native build at this checkpoint.

## 2026-10-26 ~11:15 Vilnius checkpoint (08:40 continuation still active)
New source src-785cc29d3d8dcef5: combat Kill tracking/UI + larger bounded originalAI
VM + Terran3 DesperateAlliance, ten catalogue entries. Details/report:
nintendo3ds/reports/CAMPAIGN_TEN_VALIDATION.md. All ten startup/save/replay/48tick/
17248idle PASS. ASan VM real paid construction/vision/callstack tests PASS;
combat allied/self/hallucination/turret/cargo/Broodlings tests PASS.
Fresh original wins: T1=7649,Z1=21972,P1=23544,BWP1=10771,BWZ1=13725,
T2=17215,Z2=6460,P2=24517,T3=43442. T3 original30min evacuation,7 transmissions,
paid defense/repair; completed replay identical. BW T1 bot still loses46605,
NOT current victory verified. Don't substitute old source's win.
Native origmodel T3 menu/briefing4missing0IO0/blackLoading/countdown/live/no-title
checked. Normal31,252,032,linearfree123,904; idle~59.8FPS/23.9TPS. Emulator only.
Emulator STOPPED, originalmodel/profile/both saves byte-exact restored; no physicalSD.
Private package campaign-ten-update-2026-10-26.zip (requires fullsixassets),
receipt/logs/screenshots/ELF in local-data/campaign-ten-validation. PreviousB8nine
package preserved. Hostobject /tmp/sc3ds-ten-engine.o, all fresh tests /tmp/sc3ds-ten-*.
Broader60record compatibility40startup/15loadreject/5tickreject afternewVM;
private runtime-vm.csv. Still NOTallmissionsverified. Next: native menu Dpad calls
are missing even though Frontend::direction has stage support; wire actual main
input. MoreAIops61guards/105randomseed and door/trigger actions needed later.
No more automations; userauthorized13:45 continuation already exists.

## 2026-10-26 14:03 Vilnius checkpoint; final13:45 window active to18:45
Commit12 source src-01da4c40a7f6daed: adds SC Zerg3/Protoss3 + ADPCM output bounds.
Fresh11original victories onexactsource: T1=7649,Z1=21972,P1=23544,BWP1=10771,
BWZ1=13725,T2=17215,Z2=6460,P2=24517,T3=43442,Z3=21026,P3=38399.
Z3paidMutaattack + Chrysalisprotect; P3paidScouts clear elevated colonies.
Completed replay bothmatches. BW T1 still NOT current full-victory verified.
All12active save/replay/48continuation/17248idle PASS. All167dialogueWAVs mixer
byte-exact2MiB IO0/unloadclean. ADPCM ASan+UBSan bounds/stereo-tail PASS;28new
speechWAVs independentStormLibbyteexact. Publicreference built only/tmp, no code
copied into product. Temporarycmake /tmp/sc3ds-cmake-runtime, referenceCLI
/tmp/sc3ds-storm-extract (backslashMPQpaths), sourceclone/tmp/sc3ds-stormlib-reference.
Native01D origmodel P3/Z3 menu/briefing/blackLoading/live/no-top-title checked;
normal31,288,888/30,971,264 respectively,linearfree123,904. No nativefullwinsclaim.
Emulator STOPPED, originalmodel/profile/bothslotssame restored; no physicalSD.
IMPORTANT CORRECTION topreviousNIGHT: main alreadycalled Frontend::direction
laterinloop. Myduplicate caused doublestagechanges onintermediate797build;
nativecheckcaughtit, removedbefore01D. Finalmaininputunchanged; nativeDpadLEFT
opening->P3/Z3 passes. Intermediate797/bc builds are NOT packages to use.
Currentpackage campaign-twelve-update-2026-10-26.zip, requires previousfullsix;
private logs/screenshots/ELF/3DSX inlocal-data/campaign-twelve-validation.
Currenthostobject /tmp/sc3ds-twelve-final-engine.o, tests/logs/tmp/sc3ds-final-*.
Read report nintendo3ds/reports/CAMPAIGN_TWELVE_VALIDATION.md.
Latest13:45heartbeat arrivedduringwork, authorizesuntil18:45; no more automations.
Next: missing latercampaign triggeractions/doorstates/AIguards/branchprogression;
notallmissionsdone. Preserve package while extending backend, keepclaims honest.

## 2026-10-26 ~14:15 branch checkpoint
Source b081b7ef2b56e4c4 adds SetNextScenario41 -> runtime/string/digest -> atomic
per-episode next=key progress routing, preserving future04c-f/Bonuskeys. Native
buildARM11PASS, but nativebranchprogressNOTyetchecked; no replacementpackage.
HostASan synthetic41timerwin/active savereplay, filenamevalidation/routes/reload/
failedwrite rollbackPASS. Frontendand12startup/save/replay/48/17248PASS. Broader
runtime-routes.csv43startup/6loadreject/11tickreject; nextmissingAI61/94/105,
4doormaploadrejects+2unusedcutmapformats. ReportCAMPAIGN_BRANCH_ROUTES.md.
Currenthostobject/tmp/sc3ds-routes-engine.o; test/tmp/sc3ds-campaign-routes,
/tmp/sc3ds-routes-openings. OutcurrentlyB081native, stage/localpackage still01D
(the tested12release). Emulator STOPPED/restored since12stage, no physicalSD.
Next implementboundedoriginalAIguardrequests/op105seed/op94enemyresourcejump,
then door/trapstate; don'tsilentlyskipfunctionalops. Lastwindowuntil18:45active.

## 2026-10-26 14:29 AI guard checkpoint
Source d77a6bf557d8ccee adds bounded128paidguardrequests/three-death retirement,
83/94own/enemyresource branches,93enemyowns,105seed,digest andinvalidracebounds.
ASan VM PASS incl2realpaidMarine replacements50+50 andno third replacement;
queuecounts preventendlessduplicateorders. Fixture originallyraceNone5 caused
wrong-supplyreads/autodepot; fixedfixturetoTerran andaddedruntimeboundscheck.
All12 startup/save/replay/48/17248PASS, nativeARM11buildPASS, NO nativeruntime
check/packageforthisstage. ReportCAMPAIGN_AI_GUARDS.md. Hostobj/tmp/sc3ds-guards-engine.o.
Broader runtime-guards.csv51startup/6loadreject/3tickreject; tickerrorsAI45nukerate
(BWT5a),92guardall(BWT8/BWZ6). Fourdoormaploadrejects(T4,Z5,P6,BWT7)and2unusedcutmaps.
NEXT implementnative doors/traps. PublicGPTPprimaryreferencecloned read-onlyunder
/tmp/sc3ds-gptp-reference, hooks/orders/doodad_orders.cpp hasoriginalordersemantics.
Do NOT copy implementation; usefacts forindependentOpenBWstate functions.
Doors waitiscript order_signal1; Disable animation opens, Enablecloses; nocollide
setonlyafteropenanim. HiddenGun collapses/cloaks/nocollide, searches15ticktarget,
Reveal specialstate1 thenqueuedattack thenHide specialstate2 thenHiddenGun.
Existingnativeflags disabled0x400/nocollide0x200000, methodsqueue_order_back,
find_acquire_random_target,order_done,sprite_run_anim. THG2flag0x80 currentlythrows.
Latestnative-testedpackage12source01Dpreserved. Emulatoroff/restored. Windowto18:45.

## 2026-10-26 ~16:40 installation checkpoint
Source95f9442b1bb571c4: doors/traps/iscript/state42, disabled originalconditions,
commandmost/least, THG2owner normalization, richerorderdigest. HostASan door4cycles/
collision/traprealweapon/reveal/hidePASS, extremaPASS, routesdisabledBringloc0PASS.
All12startup/savereplay/48/17248PASS. Fresh11victories: T1 7649,T2 17215,T3 43442,
Z1 21972,Z2 6460,Z3 21026,P1 23544,P2 24517,P3 38399,BWP1 10771,BWZ1 13605.
BWT1 loses46605, still no victoryclaim. Full logs /tmp/sc3ds-doorpaid-*.log.
Native95F originalmodel BWZ1brief12missing0IO0, blackLoading/live/notopstripPASS;
normal31754464 linearfree123904. EmulatorSTOPPED/restoredprofile/model/bothslots;
no physicalSD. ReportCAMPAIGN_DOODADS_VALIDATION.md. Last13:45windowto18:45active.
Broader64inventoryrecords59startup/3AI45or92tickreject/2unusedcutmapreject;1200ticks
isnotcompletion. NewTHGnormalizationchanged11of12stagedmaps, old01DZIPpreserved.
T4trial4 all12teleportedfirstpad butroute976,192stuckaround2272,1137; no menuT4yet.
Temporarytrial /tmp/sc3ds-terran04-trial.cpp, terrainmapguide.png helpsfindroute.
CurrentASanobj/tmp/sc3ds-doors-ready-engine.o. Nativeout95F; staged3DSXstill01D.
Latestprivatepackage still campaign-twelve-update-2026-10-26.zip (01D), not95F.

## 2026-10-26 15:55 Vilnius Terran4 checkpoint (clock correction)
Earlier~16:40 checkpoint timestamp was one hour ahead; UTCclock13:37 was15:37local.
Currentwindowstillto18:45. Sourcee811e7f7f211049c enables13thmission SC Terran4
scenario28, originalJacobsInstallation. Publiccontrols originalwin27129, Raynoralive,
teleport/22voices/completedsavereplayPASS. Route976,192fromstartthen2448,2016afterwarp
(the3552,2496padisoptionalcameraroomdeadend, not mainroute). Testcampaign_terran_fourth.
All13startup/menu/savereplay/48/17248PASS. NativeoriginalmodelT4DpadLEFTchapter,
brief4missing0IO0/worldstartPASS normal31522480 linearfree123904. Emulatorstopped/
restored; physicalSDuntouched.17T4WAVs byteexactindependentStormLib. Mixerall13running.
Hostobj/tmp/sc3ds-thirteen-engine.o, logs/tmp/sc3ds-thirteen-*.log.
Nativebuilddone/sourcefingerprintstableE811. ProtoP4triallosesstartingarmy1800ticks
withdirectattackmarch; Tassadar/Raynorstillaliveallied. Needsrealstorm/cohesionroute.
T4package saved privately campaign-thirteen-update-2026-10-26.zip122616894bytes
SHA2563d4c21a0939c1b95c5ca4a048b3de058d269fecb11f4cbcceef8747169876562.
All184originaldialogueWAVs (167+17) mixerbyteexact2MiB/IO0/unloadcleanPASS.
Z4prototypeinitially"wins"2101beforehatching: rejectedascoverage! CampaignAI.attack
choosesnearesthostileAI(RaynorCC) insteadhuman0, DukeAI destroys plotCCprematurely.
NowfixingAIwavehumanprioritywhenhostile, fallbackothersifhumanallied/inactive.
Thiswillrequirefreshregressions; do notcountZ4asvalidatedorofferinyet.

## 2026-10-26 19:45 Vilnius final checkpoint
Finalsource src-d309c707e49eb3cc adds SC Protoss4 scenario29,14cataloguemissions.
ASan publiccontrols paidcolony/army, rescuedTassadar+Raynor, originalambush/beacon
victory48321; bothheroesalive,21speechevents,completedsavereplaydigestidentical.
Cleanedtestsame48321. Testcampaign_protoss_fourth.cpp, hostobj/tmp/sc3ds-fourteen-engine.o.
All14startup/menu/active save/replay/48/17248PASS;205originaldialogueWAVs mixerbyteexact
2MiB IO0/unloadclean;21P4WAVs independentStormLibbyteexact. NativeARM11PASS;
originalmodelP4DpadLEFT/brief3missing0IO0/blackLoading/live/notopstripPASS,
normal31347744 linearfree123904. EmulatorSTOPPED/restoredmodel/profile/bothslots;
physicalSDuntouched. Evidence local-data/campaign-fourteen-validation andreport
CAMPAIGN_FOURTEEN_VALIDATION.md. Staged3DSX/latestpackage14sourceD309.
IMPORTANTCORRECTION: earlierclaimedZ4AInearesttargetcausewasnotestablished.
Trialhuman-priorityAIchange didNOTfixZ4 (idleCC2disappearsaroundf721, Chrysalis1,
Kerrigan0,timer571, victory2101), so thatchangeanditsVMtestwereREVERTED.
NoAIsemanticchangeafterD77exceptdoorstage. Z4notenabledandNOTvalidated.
NextinvestigateZ4CC2deathcause/forcealliances/guards; do notcountwin2101ascoverage.
P4bruteforcearmyfailed; finalpaidstrategyvalidandpassesonrevertedAIbackend.
Remainingallmissionsgoalunfinished:BW T1autoplayerloses46605; AI45nuke_rate/92guard_all
unsupported;59of64inventoriedbranch/cutrecordsreach1200ticks,NOTcompletionproof.
Latest13:45windowwaslastscheduledcontinuation; userlaterexplicitlysaid"tęsk".
No furtherautomationscreated. Stopafterfinalprivatepackage/report/localcommit.

Final14package nintendo3ds/local-data/releases/campaign-fourteen-update-2026-10-26.zip bytes141115051 SHA2561c0e8f75b7a02d896fad4af2d0c9682b89781936f2bb174c3067dea4203f5a97; ZIPCRC/hash/14maps/8banks/exact3DSX/no-profile-progress-savesPASS.

## 2026-10-26 resumed at user's explicit request: AI / Z4 / BW T1 completed
Finalsource src-5ba4136efbd5aa80. UserexplicitlyaskedforlaterAIactions,Z4progressbug,
BWT1victorycheck. No newautomation and no physicalSDwrites.
RootZ4causeCONFIRMED: no defaultcross-Forcecomputeralliance; Duke1 damagesRaynorCC2
fromf120,kills717, falsewin2101. Enginecampaigncallback nowalliesoriginalOWNR5pairs
BEFOREconvertingcontrollerstooccupied. Explicit57stilloverrides; syntheticf60CC0
hurt/CC2full, original3secenemyactionthenCC2hurt+active savereplayPASS.
NineaddedAIops45/62/89/92/98/102/107/109/110; corrected97kill_thread marks killable
threadsanddoesnotstopunmarkedcaller.64threads/16stack/128guards/160economyretained.
Regionmask5000bytes/visiblethreats/idlearmy/activealliedhelp. Nuclear nativeorders,
real launchtimer, paid200/200normalmissileproduction. Explicitretail107isonefree
scenario-missilegrant(empty silo,noqueuedmissile/no duplicate), NOTnormalAIcheat.
ASan tests campaign_ai_later/campaign_ai_vm/campaign_alliances allPASS, nativeARM11PASS.
FreshALL15originalhostvictories onexactfinalsource: T1/2/3/4=7649/17215/43442/27129;
Z1/2/3/4=21972/6460/21026/78808; P1/2/3/4=23544/24517/31703/129293;
BWP1/T1/Z1=10771/48606/13605. BWT1nowDuranalive+completedreplayexact. Z4hatch14477,
changedobjectives, livingKerrigan,paidMutaattack14voices+completedreplayexact.
EarlierZ4transportexperimentsdidNOTprovelanding; finaltestusesairfleet. No transport
coverageclaimfromZ4. Coretactics transport hadseparateearliertests.
P4initialregressiondriverlostwithcorrectCPUalliances (Zerg3/7notfightingnow);
newpaidcannonpriority+hero-preservationdriverwins129293 withoutweakeningAI.
All15 menu/startup/active save/replay/48/17248PASS;222dialogueWAVs mixerbyteexact2MiB
IO0/unloadclean,17newZ4WAVs independentStormLibbyteexact. Nativeoriginalmodel final5ba:
Z4DpadLEFT/brief2missing0IO0/Loading/world/countdown/noTopTitlePASS,normal31079712,
linearfree123904. EmulatorSTOPPED/restoredmodel/profile/bothslots; physicalSDuntouched.
Privateevidence local-data/campaign-fifteen-validation; hostobj/tmp/sc3ds-ai-final2-engine.o.
Hostlogs/tmp/sc3ds-ai2-paid-*.log, /tmp/sc3ds-ai2-zerg04.log, /tmp/sc3ds-ai-final2-later.log.
Broaderaudit64records62startup1200ticks;58reach6000. LatermissingAI103(BWP3),108(BWT5a),
111(BWZ6),38(SCZ8); unusedSCT7/T10formatreject. Auditsarenotfullmissioncoverage.
Toolcampaign_runtime_audit nowoptional limit(arg3),default1200,max36000.
Report CAMPAIGN_AI_PROGRESSION_VALIDATION.md. Prepare/verifyprivate15updateZIP next,
localcommitallverifiedwork,thenfinalsummary. No further scheduledworkcreated.

Final15package nintendo3ds/local-data/releases/campaign-fifteen-update-2026-10-26.zip bytes160270635 SHA256c0cff31c73c8419d4f9fa3e0ba4cc53b2924f6d05c79fcf892a9050bd8609b9a; ZIPCRC/hash/15maps/9banks/exact5ba3DSX/no-profile-progress-savesPASS. Finalreportandallsource/testsreadyforlocalcommit.

## Latest requested UI update: dual-screen campaign briefing
Source src-83ddecfdb581c29c. Objectives/dialogue pages moved to top screen,
.65 font (previously .46), 348px width / eight rows shared with pagination.
Bottom has mission title/chapter navigation/page counter/Back/Next/Begin only.
No new assets required; original speaker portraits/briefing-room video not added.
ASan frontend PASS (fixed stale Terran4 navigation fixture), native ARM11 PASS;
original-model emulator verified objectives/speech/A/B/chapter navigation.
Emulator STOPPED/config byte-restored, profile/saves/physical SD untouched.
Report BRIEFING_DUAL_SCREEN.md + briefing-dual-screen.png. Out/staged3DSX current;
older fifteen-mission ZIP remains5ba, do not call it current UI build.
No campaign victory reruns for this UI-only update. Earlier timer inset/larger
in-game objective text/empty-selection icons request was explicitly held by user
and remains pending; this request authorized only briefing layout changes.

## 2026-10-27 completed campaign presentation sequence
Source src-d8212ce708a062eb. All15 offered chapters now have original establishing
shots/mission-location labels, green scrolling history below (original introductions
where present, authored short summaries for9laterchapters), A->briefing. Original
race room and flipped rightmost portrait frame below, unflipped original static
speaker face; large Start, Cancel; no lower dialogue/objective boxes. Upper shot
20%black dim and dialogue panel; original MBRF speech timings/slots/voices drive
auto advance; completion Y replay prompt. Y restarts cached audio. Start/Afocused
or touchStart -> bottomblackLoading, topshot+originalobjectives retained.
ASan all15 presentation state tests and existing frontend PASS; nativeARM11 PASS.
Originalmodel emulator naturalcomplete/prompt, Yreplay(aftertemporarymatching
emulatorkeyboardbinding), Start/loading/world startup verified. FinalsourceT2
core/graphics/audio ready; heap30989208,linearfree107520 with64KiBshot retained.
Intermediateaudioallocationfailure FIXED by freeing shot BEFOREterrain/atlas,
reloading afterndspallocation. Clear afterwarming; budgets36MiB/8MiB unchanged.
64KiB256x128photo mapped400x240 preserves crop/aspect. Room released beforeload.
63newprivate resources from40originalpresentationassets+MBRF+existingportraits.
Tools campaign_presentation_export/prepare_campaign_presentation/
package_campaign_presentation; trackedpathlist reproducible byteexact. Fullpackage
script now verifies presentationmanifest. Privateupdate
nintendo3ds/local-data/releases/campaign-presentation-update-2026-10-27.zip
1172459bytes SHA2560d741b8bc6bbc44fcb03c4fdbf57da7934e124b92bb8809cc2a8c9bdeb0f3d81.
CRC/resourcehashes/exactexe PASS; requires existingfullcampaigninstall, no saves,
profiles/progress. Out/staged3DSX current. EmulatorSTOPPED/settings/profile/progress/
saves byte-restored; physicalSD untouched. ReportCAMPAIGN_PRESENTATION_VALIDATION.md,
UIcaptures inreports, private loadlogevidence campaign-presentation/emulator-load.log.
No fullmissionvictory reruns forpresentation-onlychange. EarlierheldHUDtimer/font/
emptyselectionicon requests remain pending; campaignAI limitations unchanged.

## 2026-10-27 original lower briefing layout correction
Source src-2dc0b1b024abc416. User requested full original lower scene at320x240,
with only speech text omitted. Prepare tool now reads owned glurdy{t,p,z}.bin
positions/visibility/movie offsets, composes original PCX/SMKstills at640x480,
then uniformly halves. All4portraitframes, emptycentraltextframe, nativeobjective
window and Start/Replay/Cancel restored. Originalnormalmissions hide tutorialonly
controls. Objectives again shown below per latest request. No mirrored/movedparts.
New private .brief-cast sidecars retain all4 original speaker-slot assignments
throughpagination; runtime renders unmirrored portraits and nativeactivehighlight.
Original campaign metal decorations restored around upperdialog andloadingobjectives.
Room+4highlights shareone512x256page; retainedshot64KiB+frame32KiB, releasedbefore
terrain and reuploadedadterndsp. ASanall15cast/state/replay/hitboxes+frontendPASS,
79resourcehashes/3native320x240banks/nativeARM11PASS. Originalmodel Z4 room/portraits/
upperframe/loading/liveworld/audio-ready verified:heap31081384,linearfree25600
with96KiBtemporaryart. Notfullmissionvictoryrerun. NativeUI movies/portraits static.
New private overlay campaign-original-briefing-update-2026-10-27.zip,1145476bytes,
SHA2567b0547def339e021bcadccfe738b94e8bc5bd5115916a2b6a8b36782f3728020.
CRC/79hashes/exact3DSX PASS; oldpresentationZIP remains earlier layout.
Report ORIGINAL_BRIEFING_LAYOUT.md. Finish Terranvisualcheck, stopemulator/restore
config/profile/progress/saves from /tmp/sc3ds-presentation-emulator-backup, commit.
Original-layout final: Terran2visualcheckPASS, original4slots/Raynor+Adjutant,
emptycentraltextframe/nativebuttons/objectives/uppermetalframe. Bcancel native;
briefaudio played5/missing0/IO0. EmulatorSTOPPED/config/profile/progress/saves
byte-restored; physicalSDuntouched. Evidenceoriginal-layout-emulator.log inprivate
campaign-presentationfolder; 3original-briefing*.png reports. Readyforlocalcommit.


## 2026-10-28 UI/briefing animation/five-slot checkpoint
Source src-edc95d348573a5a9. Latest authorized batch complete: story arrows/bar
removed; touch/stick/D-pad vertical scroll and horizontal Cancel/Briefing focus;
original lower briefing empty objective/text frames; centered replay; original
race ornaments; talking/idle portraits and Start/room animation; loading objective
padding; no lower gameplay objectives, black upper pause objectives; five independent
Save/Load slots in-game and title Load with measured mission/time/date rows.
RTC timestamp checksummed in reserved header words (no journal format change).
Only prior UI-only 2dc0/d821 builds additionally accepted, still exact replay check.
ASan frontend, all15 presentation tests, real-engine five-slot exact replays,
existing save/mixed-AI20k+720 continuation/stress regressions PASS. ARM11 PASS.
128 animated sequences/1484 frames/112 manifest resources verified. Originalmodel
native Protoss1 story/room/goldframe/replay/portrait+crystal motion/pause/five slots
verified. Saved Slot4 with correct Oct28 date and loaded successfully to liveworld.
Budgets36MiB/8MiB unchanged; startup audio-ready heap31090928,linearfree25600.
Emulator stopped; config/profile/progress/saves byte-restored. PhysicalSD untouched.
Private overlay campaign-ui-slots-animations-update-2026-10-28.zip,5384456bytes,
SHA256e56b6965803511ec50bd8cb2f1bf270d99f2ea38d5ac7eb9ec5a64225e8a2280.
ZIPCRC/112hashes/exactexePASS; use over existing full private install. Report
nintendo3ds/reports/OCT28_UI_SLOTS_ANIMATIONS.md. No fullmissionvictory reruns;
laterAI limitations and earlier held timer/font/emptyselectionicon requests remain.


## 2026-10-28 later-campaign work, pause near account limit
Final source src-f1bb1c604dd5ed45, nativeARM11PASS. NewAI38move_dt/103Web/104Recall/
108scenario create_unit; own eligible caster/real native effects/energy, ordinary
andheroDT move, one-shot boundedscenario grant. No new persistentAI state/allocation.
HeroGhost aliases expose SarahKerrigan/Duran/InfestedDuran/Stukov abilities. Latest
edcUI saves also accepted only with CRC/journal/exact-reconstruction checks.
ASan cinematics/laterAI(intendedarena)/five slots+edclegacy/herooptions PASS.
Broadaudit64records61no-runtime-rejectionat6000,60at36000 ororiginalmapend; NOTwins.
Remaining111allies_watch BWZ6,101junkyard_dog BWT8(later),2unusedcutT7/T10 format.
No additional catalogue missions enabled yet;15previous victories remain baseline.
Terran5 privatepilot rescuedKerrigan,Raynorsafe,cloakedK reaches2786,500 butnoownCC/
no win. Pilot assumptionmaybewrong: location1 setsSwitch4, mustinspectoriginal
trigger/Forceprogression. EarlierdirectattacklostRaynor2164, NOTengineproof.
Allrawlogs/experimentalpilot andnative3DSX inlocal-data/campaign-next-validation.
Hostobj/tmp/sc3ds-cinematic-final-engine.o. Source/report CAMPAIGN_NEXT_CHECKPOINT.md.
Internalprivateoverlay campaign-backend-checkpoint-2026-10-28.zip5385246bytes SHA256
109378e2927fc2509b710873944232b41d6f7f8753aec240f848bce65d4242c1; CRC/112hash/exePASS.
No emulator run this backend checkpoint, noSDwrites. LatestemutestedUI ZIP unchanged.
Userrequested automaticcontinuations: starcraft-kampanij-t-sinys-02-25 and
starcraft-kampanij-t-sinys-07-30, one-shotCOUNT1 Oct29 Europe/Vilnius. Actualreset
02:20:23. First shouldverifyrealresetsAt/adjustexistingsecondifneeded, deleteown
one-shotafterrun; secondlastscheduled, noadditionalautoruns. Stopnearcurrentlimit
94%used, notgoalcomplete. Next:111/101, realT5progression/victory, catalogue+asset
closure expansion onlyafteroriginalmap/publiccontrolvictory+defeat/saveproof.


## 2026-10-29 02:25 continuation: sixteen-mission verified checkpoint
Finalsource77ee612af31c2ed4 (b641 backend, final280px dialogue inset only).
AppendedSC T5Revolution scenario31: originalmap/MBRF/estt05/story/hero responses/
17WAVs/animatedbrief/campaignprogress. Rootcause neutralparticipant3Force3 never
scheduledRscu; triggerload now includesneutral7+occupied/computergame, excludes
passiverescue3/inactive/closed. Overbroadincludepassive trial BROKE T4teleports,
REJECTED; finalgatingrestoresT4win27129. NativeRescuePassive orders nowNeutralidle;
CampaignRuntime doesproximityrescue. Neutralaggressioncallback usesboundedadapter
withnativeattackorders, fixesBWP1 laterupstreamcomputerAIerror, win10765.
AI47scoped clearcommitted and101bounded deterministicreachable wandering, original
locationbounds inheritedchildthreads/digest. Arrays1700IDs+ownerbytes;36MiB/8MiB.
Simulationstatechanged: removedoldUIcrossbuildaliases; older saves unavailable,
metadata/files preserved. Existingcampaignprofile/progress compatible.
All16original publiccontrol/paidvictoriesPASS onb641; final77eeT5win29112 heroesalive,
16voices, exactcompletedreplay. FinalBW P1/Z1 completedreplay10765/13605PASS.
All16defeatfixturesPASS; final16menu/story/brief/startup/active replay+48/17248PASS.
ASan neutraldirect/Force/delayedcolony/closed-inactive-passiveexclusion/hostile
retaliation+replay, scopedAI47/101 movement/owner/RNG, five slots PASS. Audio239
WAVs2MiBbyteexactIO0/cleanunload,17T5WAVsindependentStormLibexact. NativeARM11PASS.
Originalmodel final77eeT5story/animatedbrief/paddedframe/loading/live/pause/native
Slot4save+loadPASS; heap31206248,linearfree25600 with96KiBtemporaryart. Emulator
STOPPED/config/profile/progress/saves byte-restored; nophysicalSD.
Overlay campaign-sixteen-update-2026-10-29.zip23700961bytes SHA256
fa1833033b823fa9e327e158c80884df4f6158a83f0334680b9267c7e36546a9,119hashes/newT5assets/
CRC/exact3DSXPASS. Stageexe current. ReportCAMPAIGN_SIXTEEN_VALIDATION.md; private
rawlogs/receipts/emubackups inlocal-data/campaign-sixteen-validation. Hostfinalobj
/tmp/sc3ds-sixteen-final-engine.o. PythonPillow bundledruntime path fromdependencies,
plainpython3 lacksPIL. Toolsverify/package now catalogue-driven; originalesttexts
cp1252decode. Furthergoalincomplete:111allies_watch BWZ6, newlatermissiontests.
Fresh6000audit61/64runtime no-rejection, notwins; unusedcutT7/T10stillreject;
longerauditnew101 onBWT8pending. Existing07:30one-shot stillcorrectreset07:25:27;
delete02:25one-shotwhenendingthisturn, noextrascheduledcontinuationsauthorized.

## 2026-10-29 continuation: seventeenth mission verified
Source25f92e80eb908e7b: appendBW Protoss2DunesofShakuras scenario32, map/MBRF/
originalestp02x story/location, animatedbrief/cast,18originalWAVs/heroresponses.
No backend changes since16. Paid publiccontrol originalwin35681, NW384,512 colony,
4DT rescue/objectivechange/Nexussurvives/voices>=4, exactcompletedsavereplayPASS.
All17defeatfixtures/menu-story-brief-startup/active replay+48/17248continuationPASS.
Presentation17PASS,257WAVsbyteexact2MiBmixerIO0/cleanunload,18newWAVsStormLibexact.
NativeARM11PASS; originalmodelstory/animatedbrief/ornateupperframe/loading/live/
pauseobjectivesPASS, scenario32normalheap31318424,linearfree25600with96KiBshot.
EmulatorSTOPPED/config/profile/progress/all saves byte-restored, nophysicalSD.
ReportCAMPAIGN_SEVENTEEN_VALIDATION.md; privateevidencelocal-data/campaign-seventeen-
validation. Stagedexe matchesout25f9. Overlaycampaign-seventeen-update-2026-10-29.zip
41756970bytes SHA256fd4ffd4ccd0df7a941c6a633a35e83e0098775e103b4a2bd2968bdace5434060,
126hashes/CRC/exactexePASS; --missions terran05 expprotoss02 includesbothnewmissions
forpriorfull15install. Preserve16fallbackZIP. Hostobj/tmp/sc3ds-seventeen-engine.o.
BWT2privatepilot /tmp/sc3ds-bwt2-pilot2.cpp captures3ships at673/961/1393 thenstalls
220000. Capturedships owner2staged, owner1uncaptured. After3originalenemy6customscore
shouldsetswitch8 tospawnnextcrew; pilotcurrentlywaitswhennoCivilian. Original4groups
3/4/5/6ships (18total); noBWT2catalogue/shippeddatauntilactualwin/replayproof.
Temporarydiagnosticengine /tmp/sc3ds-bwt2-diagnostic-engine.cpp expandsheaderlocally
withscore/switchlogging only, no committedproductchange. Continuefromevidence.

Laterdiagnostic correction: BWT2'ssectortransitionIS working, nottriggerblocked.
/tmp/sc3ds-bwt2-diagnostic.log: capture scoresenemy6=3 at1357; switch8sameframe;
nextcrew4civilians/3Ghosts createdat1388. Pilotmovesallcivilians throughfire; by2161
allciviliansandGhostsdead, sector2skippedwith0captures andnextscript waits. DoNOT
"fix" triggertimingtohidebadteststrategy. Pilot4 hold/escorttrialalso flawed: its
waypointskipleftfirstsectorwith2ships. Pilot5 removesskipandholdscrew2civilians
untilmilitaryarrives, cloakattempts forGhosts, stillstalls3ships/90000; notvictory.
Need realGhost/Medicstealth/escortstrategy, inspectoriginalunitproperties/terrain.
Privatecopiesofpilot2-5/diagnosticcpp/logs savedcampaign-seventeen-validation.
LongoriginalBWT8runtimeauditwithnew101 reachesoriginaloutcome25969/706units
withoutrejection; nofullwinclaim. Broad64*36000 auditrunning/tmp/sc3ds-seventeen-
all-long-audit.log using17hostobj, shouldcollectbeforeending. No sourceedits.

Finalpost-checkpointtests: all17paid/publiccontrolvictorydrivers freshlyrebuilt
against25f9hostengine and rerunASanPASS, privatepermissionlogs/regressions.json
campaign-seventeen-validation; nofailures. Broad64originalnormalizedrecords up to
36000ticks/originaloutcome:61withoutrejection; expzerg06opcode111at1522; unusedcut
terran07/terran10parseerrors. Privateall-long-runtime.csv/summary.json saved.
BWT2pilot6-7still3captures, noproductchange, copiedprivately. NoteCloakoption is
TOGGLE, mustnotperformeach240frames; pilot7avoidsrepeatbutstrategyremainsunsolved.
Report17updated. EmulatorstatusRunningFalse; config/profile/progress/savesrestored.
Quota87% at03:55, reset07:25:29Vilnius; scheduled07:30isappropriate. Endingwith
verified17package/commits; next07:30shouldreadthissectionandnotredo17regressions.
Nextpriority111boundedresource-area/townexpansion orsmartBWT2Ghost/Medicescort,
thenmoreoriginalmissionvictory/catalogue/assetclosure. AllcampaigngoalINCOMPLETE.

## 2026-10-29 07:30: campaign hero command checkpoint
Quota reset confirmed0%, actualnextreset12:30:28Vilnius. Noadditionalautomations;
delete07:30one-shotatend. Source38de8aa4a2d77845: heroUIaliases17, fullnamesforhero
units, InfestedKerrigan Storm/Ensnare/Consume/Cloak. ScopednativePersonnelCloaking
exceptioninbwgame/actions, NOTghostclassification; KerriganstillcannotLockdown.
Noavailability/research/energyoverride. OriginalZ5Stormdisabled, other3enabled.
Newtests campaign_hero_commands.cpp (17alias/name/native-enabled checks+cloakexact
save48frames; isolatedownedmapfixture), campaign_hero_spells.cpp (nativevisible
cloak/energy/decloak, ensnaretimer, Consumeunitloss+energy, TassadarStormdamage).
All17victorydrivers freshlyrerunASan38dePASS, local-data/campaign-hero-final-validation.
NativeARM11PASS; original-modelslot5syntheticKerriganfixtureloadedselectedcloaked,
4icons/keybadges/portrait/energy/fullnamefit. NativeScreenshot local-data/campaign-
hero-validation/native-hero.png. Normal30853416,linearfree173056. Touchattempt
normalized.75/.4didnotprovebinding; doNOTclaimtouchverified. EmulatorSTOPPED,
config/profile/progress/allsaves byte-restored toemulator-before backup. NoSD.
Overlaycampaign-heroes-update-2026-10-29.zip41757747bytes SHA256
 e92d965b8d1827ca0efef106149ebe4f94442cf0d88fa0e5967ddc7f5ddbbf0a,126hash/CRC/exactexe.
Stageexe matchesout38de; sourcehostobj/tmp/sc3ds-hero-final-engine.o. Report
CAMPAIGN_HERO_COMMANDS.md. Enabledmissionsstill17, noallcampaigncompletionclaim.

## 2026-10-29 final authorized continuation checkpoint

Final source src-63c47a7913e9b844, host /tmp/sc3ds-eighteen-engine.o.
18 enabled missions: prior17 plus SC Terran6 Norad II, scenario33.
All18 ASan victory drivers passed on final source; Norad II original winner0
frame15768, paid SCVs/refinery/factory/starport/control tower/two Dropships,
Raynor and crashed cruiser survived; completed save exact digest. Test uses
public group perform(Move), NOT move_selected (single-unit helper). Original
Bring condition observed two ships before one died during victory dialogue.
Next scenario empty until another verified Terran mission is enabled.
18 menu/startup/active saves+48 exact continuation then17248/firstoutcome PASS;
18 original defeat fixtures PASS. Added24 owned WAVs byte-exact StormLib;
133 presentation resources. ARM11 native build PASS.
Original-model emulator actual story/animated briefing/start/loading/world/pause
PASS, >1min nativeplay. Normal31366080, linearfree25600 (4MiB cache): very tight
linear headroom; full native mission completion not claimed. Screenshot/logs in
local-data/campaign-eighteen-validation. EmulatorSTOPPED; config/profile/progress
and all virtual saves byte-restored. No physicalSD writes.

AI expansion adapter op17 nearest/op111 specified site/op112 town count;
32 requests/32 towns/250 site slots zero sentinel. Site IDs preserve CHK start
encounter order then sorted connected resource clusters. Independent deterministic
adapter, NOT byte-identical retail resarea numbering. Production still ownerwide
Side, not retail per-town queues. Paid native worker/depot construction and child
launch after completion. Core creep queue remains1024, retains dead provider until
all remaining tiles queued. Tests:18 three-race expansion cases; live-town count;
owner-normalized site order/reload; saturated queue/live-provider creep; original
BWZ6 save1600+4000 exact and2000+4000 exact PASS. Broad64-record audit62 no rejection
through36000ticks/originaloutcome; other2 unused/cut T7/T10 failparse. NOT proof of
62 victories. BWZ6 not enabled/menu; original-model active-replay fixture also
PASS normal30286544/linearfree451584 using2MiB cache. Private expansion evidence
local-data/campaign-expansion-validation. Earlier source49382 tested17 victories;
final63c4 reruns all18. Remaining static unhandled opcodes90/42 not reached by broad
audit; must investigate behavior before implementing, no blind no-ops.

Private overlay campaign-eighteen-update-2026-10-29.zip60517106bytes SHA256
f163404d463bd71d9b185a4e25346d01c405c26ac65dea85946b08a74f606aa7
133assets + T5/BWP2/T6 maps/brief/audio; requires prior full15 install. CRC/hash/
exact executable PASS. Staged exe matches out. Older-source saves incompatible;
profile/progress preserved, start a fresh mission. Earlier17/hero/expansion ZIPs
preserved as fallbacks. Expansion fallback source49382 ZIP41766124bytes SHA256
b6cf9874778931f3c8fa0d766ed4e7fd9a0df7d19493b10bd81e51bc23f7afe3.
Report nintendo3ds/reports/CAMPAIGN_EIGHTEEN_VALIDATION.md.

Goal NOT complete:18 verified/enabled missions, later SC/BW need real victories.
Private failed pilots P5 Choosing Sides, BW T2 Shipyards, Z5 Amerigo remain /tmp;
no map/unit/stat/trigger cheats shipped. P5 convoy must use public perform(Move)
for groups and route turn compression; move_selected only moves first unit.
BWT2 original second-stage Ghosts have Cloak/Lockdown; civilian escort still fails
naive pilots. Z5 Storm explicitly disabled by original map, other hero spells
work. Future useful next: native memory headroom audit, P5 transport victory or
BWT2 cloak/Lockdown+Medic escort; then add only after actual original victory,
loss and exact save proofs. Do not reclassify unused cutT7/T10 as shippedmissions.

This was the last authorized scheduled continuation. Do not create another
follow-up automation. starcraft-kampanij-t-sinys-07-30 was deleted successfully.
Code checkpoint:854b852 (this documentation follow-up records automation cleanup).

## 2026-10-29 ordinary continuation: splash/carried-powerup crash fixed
Source306433d01eb4c6ca /tmp/sc3ds-splash-engine.o. Snapshot splash targets before
damage so killed worker can show/reinsert dropped campaign powerup. ActualT7
stack bullet_deal_splash_damage->kill_unit->drop_carried_items->show_unit->finder
insert exposed bug after26401; fixed driver reaches originaldefeat46235, NOTwin.
Native SCV/PsiEmitter pickup+siegesplash death/drop+96framesASan fixture PASS.
All18 victory regressions and18openings/save48/17248 PASS; ARM11PASS. Newsource
not yet emutested/packaged; prior18ZIP remains releasedfallback. Report
CAMPAIGN_SPLASH_POWERUP.md; privatelogs campaign-splash-validation. Nativebuild
finished. Emulatornotrunning/noSD. Ordinary user reauthorized allmissionwork.
CurrentprivateT7pilots /tmp/sc3ds-trump-card*.cpp/.log. Carrier UnitView field
carried_powerup217; originalStarport+ControlTower links correct, but forwardbase
losesaddon/buildingearly. Newdefendpilot gathersarmy1280,1040 toprotect it while
mining andtrainingDropship; Kerrigan16moves416,1840. Previousnaivestrategies failed,
not productbugs. Final original objective location1=3616,1568..3776,1696 requires
PsiEmitter217; carrier can transport it. Needactualwin/loss/saveproof beforeadding.

## 2026-10-29 nineteenth mission complete / native victory verified
Final source7000481789ad5afc /tmp/sc3ds-nineteen-final-engine.o. AddedSC Terran7
The Trump Card keyterran08 scenario34. Formalcampaign_terran_seventh.cpp original
winner0frame8466, Kerrigan16alive, exactcompletedsavereplay. Realpaid Starport
relocation/controltower/3Dropships; nativeMatrix beforeflight; unload assoon as
inside original3616,1568..3776,1696 location. Previouspilots died atlastapproach;
leftedge3632,1600 + immediateunload solved. No unit/fund/map/trigger overrides.
All19ASanvictories/openings-save48-17248/defeat PASS finalsource. Five-slot
independence/date/replay/backup/foreignmetadata test PASS; addedTerran7labelassert.
SaveSlots numberingnowcounts chronological race+game catalogue, notmapfilename
suffix (unusedcutT7 makespublishedT7fileterran08).16ownedWAVs byteexactStormLib,
140presentationresources. NativeARM11PASS. Actualoriginalmodel replayed FULL
8466framecompletedpubliccontroljournal toVICTORY onfinal7000source; screenshot
local-data/campaign-nineteen-final-validation/native-victory.png. Freshnativeworld
andpausedgoals/dialogue/story/briefalsoverified. Nativecompletedreplaynormal31366584,
linearfree123904 /4MiBSFX. Freshworldnormal31342072/linearfree25600 tight. Emulator
STOPPED and config/profile/progress/allsaves byte-restored fromnineteen-validation/
emulator-before; nophysicalSD. No nativebuildrunning.
ZIPcampaign-nineteen-update-2026-10-29.zip80534080bytes SHA256
2032a7d75a81faf41af6ea4aed8c276d4ac66674b4fd3c773cfadcf386ded77d
140assets+T5/BWP2/T6/T7 maps/audio. Requirespriorfull15install, source7000strictsaves.
CRC/hash/exeverified; stageexematchesout. ReportCAMPAIGN_NINETEEN_VALIDATION.md.

NEXT investigated SC Terran8 TheBigPush keyterran09. RawUNIT6ownedliftablebuildings
havevalid_flags20 andflags4 (in transit) butloader ignoresbit4 andloadsallgrounded.
Private/tmp/sc3ds-t8-probe.cpp/log confirms at120ticks all6 flying0. Original goal
eliminateConfederates/Duke29survive. Initialown3Wraiths,2Dropships,4SCVs,marines/
ghosts,2CC/Starport/Factory/Barracks/ScienceFacility allbottomright~3800,3900.
NeedcorrectpreplacedinTransit state + test movement/landing/addon/save; do not
blindlyforce lifting animation or shift original positions. Existingnative
order_BuildingLiftoff atbwgame5144 handlesflags/occupancy/finder/elevation/shadow;
order_LiftingOff ends~5205. NoT8codeedit yet. OtherP5/BWT2/Z5pilots still private.
User asked continueallmissions in ordinary turns; nofurtherautomationauthorized.

## Tested intermediate: initial InTransit loader / T8 paid pilot
Sourcef90764e80ce4eb0e, /tmp/sc3ds-transit-engine.o; nativeout=f907, stagedexe=stable
7000. NativevirtualSDexe returned7000. Latestreleased19ZIP unchanged. Added
make_building_airborne factor usedby existingplayerliftoff; loader appliesvalid4/
flags4 toinitialliftablebuildings, no42pxposition shift; airspeed/shadow/LiftOff
animinit. campaign_initial_transit.cpp originalT8sixflags/positions/flyingproduction
restriction/move/save48/land + privatevalidmasknegative PASS. All19victoryregressions
PASS sourcef907; ARM11PASS. Originalmodelactiveflightfixture journal600/selected
Starportcorrectair/shadow/Land UI; nativeheap31588568linearfree123904. GenericSC16
fixtureaudio NOToriginalT8voicebank; noT8victoryclaim. EmulatorSTOPPED, config/
profile/progress/allsaves byte-restored, SDvirtualexe stable7000. PhysicalSDuntouched.
ReportCAMPAIGN_INITIAL_TRANSIT.md. Privateevidencecampaign-transit-validation.

T8privatepilot /tmp/sc3ds-t8-transport.cpp/log (copiedinevidence) nowlandsbaseand
transports4SCVswithprovidedDropship: deliveredframe673 tohome1600,3600. Initial
supply20/20 preventsrecruiting untilworkersbuilddepot; NOTcoreproductionbug.
Firstnaivehome2595,3052clusterhasnogas anddockSCVsstranded. Betterhome1600,3600 has
mineralcluster+geyser1472,3744; limitlandingtargets py>=hy-64 soavoidnorthbunkers.
Reservationcenters180px prevent6buildingschoosingsamelandingtile. Currentpilot
has13workers/19tanks at48k butmarinesremain4 untilbarracksland/build delayed;
phase1 at~52k kills~80enemies, laterlossesandmineraldepletion, at220k winner-1 Duke1.
No unit/resource/map/trigger edits. Needimproveeconomy/expansion orairfleet/targeting
ratherthanweakeningoriginalAI. OriginalT8goal eliminateConfeds, Duke29survive.
Futureconsider supplydepotworker/economypriorities, reservegas3 butover10kgasatlate
stage suggestsmineralincome bottleneck; train moreworkers andexpandsecond mineral
base, or use initial3Wraith+NoradII29airpush. DoNOTenableT8untilrealwin/loss/saveproof.

## 2026-10-29 twentieth mission complete — The Amerigo
Final source7625881d2c2b4a5c; /tmp/sc3ds-twenty-engine.o. AddedSC Zerg5 original
zerg05 scenario35. campaign_zerg_fifth.cpp originalwinner0frame60776, Kerrigan51
alive, exact completed replay. Public controls only: original unlock2208,2384,
Teleport1(688,3648)->Teleport2(784,1584), nativecloak/combat, natural Zerg HP regen
at safe784,1584 betweenfights. OriginalStormdisabled; nofreeunits/stats/funds.
Initial direct-goal drivers failed because they missed unlock/teleport; later
solo runs needed nativecombat against detector floortraps and regen. Attack near
invinciblebeacon stalls; switchMove whenx<650y>1950 solved finalentry. Earlier
private/tmp/sc3ds-z5-clear wins106718; near-goal/formal60776. No underlying cloak
bug: private diagnosticconfirmed nativefloortraps203/209/211 are detectors.

All20ASanvictories PASS; all20openings/menu/save48/17248 PASS; all20original
lossfixtures PASS. Routes atomicprogress/Z4->Z5 andfiveindependentslots/date/
backup/foreignsource PASS. All20presentations PASS. All20audio banks everydialogue
byteexactthrough4MiBboundedmixer IO0/fakeallocation0 PASS; new147unitresponses+
20missionspeech, latter20allbyteexactvsStormLib.147presentationresources. Native
ARM11PASS. InitialTransit testexpandedtoassertall6originalcoordinates, PASS final20.

Original-model emulatorFULL60776journalreplay toVICTORY onfinal7625; screenshot
local-data/campaign-twenty-validation/native-victory.png. AlsofreshZ5story/original
briefingportraits/dialogue/Start/world verified. Replaynormal31768728linearfree
123904 /4MiBSFX; freshworldnormal31758544linearfree25600. Original36/8limitskept.
Nativechecksarejournalreplay/opening, notinteractivefullhandheldplaythrough.
EmulatorSTOPPED; config/profile/progress/allsaves/virtualexebyterestored with
/tmp/sc3ds-twenty-emulator.py restore. Backupcampaign-twenty-validation/emulator-before.
Virtualexe restoredprevious7000; staged/out exe both7625. PhysicalSDuntouched.
ZIPcampaign-twenty-update-2026-10-29.zip99640646bytes SHA256
2ae37ab13aebc217c48bab0c64c93870517dad7b53a10ced62cdc38d7ee73acb
147assets+T5/BWP2/T6/T7/Z5maps/brief/audio, CRC/hashes/exe verified, no profiles/
progress/saves. Requirespriorfull15install. Strictsourcesaves: startfresh.
ReportCAMPAIGN_TWENTY_VALIDATION.md. Earlier19ZIPpreserved.

Fresh36kaudit onf907:64entries,62usable no runtimerejection:38running24original
outcomes;2unusedcutterran07/terran10loadreject. NOTvictorycertification. Toolnow
reportswinner+running/original-outcome status, max144k. Fresh7625audit120k FINISHED:64entries/62usable no runtimerejection,26running and
36originaldefeats withoutplayerinput;2unusedcutloadrejects unchanged, ASanstderr0.
Evidence twenty-validation/runtime-120000.csv. Native/audio/20regression finished.

NextprivateBWT2pilot /tmp/sc3ds-bwt2-beacons.cpp/log usesactualremainingownBeacon195
insteadfixed18-waypoint count (oldpilotscaptured3thenlostpilotsatphaseB). Native
Ghostcloak/Lockdown/Medicheal, alloriginalmap/controls. Finishedwithoutwin at90k; privatebeacon-state diagnosis now /tmp/sc3ds-bwt2-beacon-state.log.
T8paidpilotstillnotwin; initial6airbornefixcommita92535c. P5transportpilots can
reachislandwest3232,3424 butgroundroutearoundcliffkillsTassadar; interiorgoal
3392,3360..3616,3552 needsbetterprotectedlanding. Do NOTenable fromload alone.
No moreautomationsauthorized; usercontinuesordinaryturns. No nativebuildrunning.

## Trigger-created buildings outside exploration — final20 follow-up
Finalsource2368fc9ea1bb500b; /tmp/sc3ds-trigger-vision-engine.o. FoundactualBWT2
missingmarkers: trigger_create_unit used ordinaryfog/exploration validation.
All7originalcontrolpads rejectbeforeexploration, succeedafterrevealing isolated
terrain. Addedcan_place_building check_vision defaulttrue; onlyscenario creation
passesfalse. Stillchecksactualterrain/creep/psi/geyser/bounds/occupancy/collisions;
noartificialfogreveal. Ordinaryplayer/AIbuildrulesunchanged. Nativebeaconsgiveown
normalsight. campaign_trigger_building_visibility.cpp:7originalpads, ordinary
fogreject/scriptcreate,remotefogunchanged,duplicate/voidreject, originalBWT2opening
3beacons, save+48 exact PASS. Original-model600framefixture displaysmarker/world;
usesgenericBWT1audio, NOTBWT2voicebank/victory. Fixtureheap29930056linear451584 /2MiB.

All20ASanvictories/openings/menu/save48/17248/defeat PASS source2368. ARM11PASS.
NativeFULL60776Z5completedjournal replayagainVICTORY final2368, screenshot
campaign-trigger-vision-validation/native-zerg05-victory.png. Heap31768952linear
123904 /4MiB. Fresh120kauditall64FINISHED:62usable no runtime rejection (26running,
36originaldefeats),2unusedcutloadrejections;ASanstderr0. No newvictoryclaims for62.
ReportCAMPAIGN_TRIGGER_VISIBILITY.md. EmulatorSTOPPED andconfig/profile/progress/
allsaves/virtualexe byte-restored via /tmp/sc3ds-trigger-vision-emulator.py restore.
Backupcampaign-trigger-vision-validation/emulator-before. PhysicalSDuntouched.
Finalstaged/out2368; virtualexe old7000restored. No nativebuildrunning.
LatestZIPcampaign-twenty-trigger-update-2026-10-29.zip99640775bytes SHA256
177621b723437a81741c25fe38e8cc5e28a7b981aa55b37bbeeb689cf92d56c4
147assets+T5/BWP2/T6/T7/Z5maps/audio. CRC/hashes/exe PASS, no profiles/progress/saves.
Requirespriorfull15installation. Previous20source7625ZIPpreserved.

BWT2privateordered-escort stillcaptures3thenlosespilotsatB; nomissionenabled.
Oldbeacon-basedplayer was targeting enemiesnearanyfriendlybeacon evenonother
platforms; filteronlyrealarmy fixedthatdrivererror, butstillneedsmedics/scouting/
correctroute/safe pilots. Markersfix isseparateprovenproductbug. Notallmissionsdone.
CurrentnewprivateZ6pilot /tmp/sc3ds-z6-challenge.cpp/log usesZ3paidMutaliskmacro,
enemyowner1, preserveKerrigan51home, thenmoveheralone1904,2032 onceallProtossdead
andobjectivescontainalone. Earliermacroclearsallandfields62army butdidnotknow
challengepoint (Tassadar79onlyspawnsafterheroentry, sosearchingforhimcannotwork).
OriginalZ6goal requiresclearregions4/5 thenheroentryloc1alone(loc6AnyUnit<=1),
hallucinatedTassadarexpires/killed thenoriginalwin. Newpilotexec96064, checklog.
No changesforZ6enabledyet. No moreautomationsauthorized. Continueordinaryuserturn.

## Twenty-first mission — original Zerg6 victory / native replay
Finalsourcec87b5b69ed3a4e88; /tmp/sc3ds-twenty-one-engine.o. Addedzerg06 TheDarkTemplar
scenario36. campaign_zerg_sixth.cpp originalwinner0frame42378, Kerrigan51alive,
actualcargo and nativeVentralSacslevel1, completedexactreplay. PaidZ3Mutaliskmacro
(enemyowner1) clearsProtoSS thenoriginalchallenge. FootMove stops1576,1316 edge;
Overlordtransportto1904,2032, unload andmoveotherunitsaway fromloc6 soKerriganalone.
Early200/200Ventralresearch weakenedarmy andlost82733; deferuntilobjectivescontain
alone, temporarilystopLarvamorphs untilresearchbought, thenresume. Thiswins42378.
OriginaltriggersTassadarillusion resolution/dialogue/victory, nounts/funds/stat/AI/
terrain/triggermods. Fog-offnavigationinformation only. PrivatefailedpriorZ6pilots
notproductbugs. FormalteststillderivedfromZ3macro withconstantZergbranch; no gameplay
implementationchanges beyondcatalogue/presentation. Generatedownednormalizedmap
205451bytes human3->0,172unitresponses/14missionspeech, all14byteexactStormLib.
154presentationresources. FallbackstoryforZ6 retained (ownedrezhaslocationonly).

All21ASanvictories PASS sourcec87. All21openings/menu/save48/17248 PASS; all21original
lossfixtures PASS. Atomicprogress/Z5fallbackwithoutnextfilename->Z6 andfiveslots/
Zerg6label/date/backup/foreignsource PASS. All21presentationstates PASS. All21banks
alldialogue byteexactbounded4MiBhostmixer IO0/fakeallocation0 PASS. ARM11PASS.
Original-modelFULL42378journalreplay toVICTORY finalc87; native-victory.png under
campaign-twenty-one-validation. FreshZ6story/briefinganimatedKerrigan/speechtext/
Start/loading/worldalsoverified. Replaynormal31433232linearfree123904 /4MiBSFX;
freshnormal31329728linearfree25600. 36MiB/8MiBunchanged. Nativeisjournalreplay+
opening, notfullinteractivehandheldplaythrough. EmulatorSTOPPED; config/profile/
progress/allsaves/virtualexe byte-restored via /tmp/sc3ds-twenty-one-emulator.py
restore; backupcampaign-twenty-one-validation/emulator-before. Virtualexe7000
restored; finalstaged/outc87. PhysicalSDuntouched. No nativebuildrunning.
LatestZIPcampaign-twenty-one-update-2026-10-29.zip117851058bytes SHA256
5cec873f0f5dc68c807612294df0caba88614b3c25e5cf48033b93003bd25bb0
154assets+T5/BWP2/T6/T7/Z5/Z6maps/brief/audio. CRC/hashes/exe PASS; no profiles/
progress/saves. Overlaypriorfull15install orneweroverlay. Strictsource saves:
startfresh. ReportCAMPAIGN_TWENTY_ONE_VALIDATION.md. Earlier20/20triggerZIPspreserved.

Next potentialSC Z7TheCulling rawzerg07:human0start4Drones/4Mutas/8Hydra/4Overlords,
NOinitialHatchery. Enemyowner4Zerg151units; objectiveeradicateGarmBrood, noKerrigan.
NeedpaidfirstHatcherynearstartminerals beforeZergmacrocanrun (oldmacroifnoHome
continuesforever). CurrentZ7notstarted. OtherremainingZ8owner1/2Protoss preventDT
escape+Kerrigan survive; Z9originalDronecrystalcarry/hold/return; Z10Templethencrystal.
BWT2orderedescortstillcaptures3thenlosesatB, nofullwin; beaconscreationfixedalready.
P5needinteriorlandinggoal3392,3360..3616,3552; west3232,3424unloadwalksaroundcliff
anddies. T8originalsixinitialairbornefixincluded, paidpilotnooriginalwinyet.
No moreautomationsauthorized; noexternalpush orphysicalSD. Workcontinuesordinary
userturns, allcampaigngoalNOTcomplete:21enabledandvictorytested. Allrunprocessesdone.


2026-10-29 continuation: original result reference art exported from owned data.
Six native 640x480 victory/defeat backgrounds (three races), 18 score UI pieces,
six separate 400x240 previews and REZULTATU FONAI.png are in the ignored private
`update image screens` directory. RGB pixels match owned decoded PCXs exactly;
all PNGs reopen, previews verify 400x240. Reproducible generic tools and source
asset list in nintendo3ds/tools/prepare_result_reference.py and
result_reference_assets.txt; report RESULT_REFERENCE_EXPORT.md. No artist redraws
overwritten, assets not committed or wired into runtime. Existing 21-mission c87
executable/package unchanged, no SD writes or emulator changes this continuation.

SC Z7 The Culling remains private driver work, NOT enabled or victory-certified.
Paid first Hatchery near (1984,3360) can start original no-base map. Private ASan
pilots /tmp/sc3ds-z7-*.cpp and corresponding logs use current twenty-one engine.o,
original owned zerg07.chk, public controls, real costs and original AI/triggers.
No sanitizer fault in completed pilots, but none has won. Original mass Hydras
reached 90+ army; fixing detector escort reduced enemies to99, then lost. Sending
all Overlords ahead loses supply; keep escorts behind infantry, do not chase
flying Overlords before bases. Later tech pilots built Lair/Hive/Greater Spire
normally but failed strategically. Original script is not proved faulty by this.
Latest pilot /tmp/sc3ds-z7-mineral-guards.cpp reserves gas for 2/2 upgrades and
tries to protect the northern mining line while massing. Earlier pilots leave
workers outside army acquisition range and lose economy around frame33000;
idle army can survive much longer but cannot win once income is gone. Avoid
promoting a startup test or loss to a completed mission. Stable count remains21.

2026-10-29 CHECKPOINT TWENTY-TWO COMPLETE (supersedes unfinished Z7 notes above).
SC Zerg7 The Culling enabled as scenario37 after original victory at frame68317.
Source src-a98a180455a86d49. ASan object /tmp/sc3ds-twenty-two-engine.o. New formal
campaign_zerg_seventh.cpp: paid first Hatchery/local enemy gas clearance/western
expansion, upgraded Hydras clear mainland, Ventral Sacs cargo/flight/unload and
actual island infantry, original victory, exact completed-save replay. Read-only
separate native terrain model guides host navigation only; never changes actual
campaign. No free actors/resources, map/AI/trigger/stat changes. Native originals
include unreachable island groups; earlier ground-only failures were strategy
errors. Working script /tmp/sc3ds-twenty-two-regress.py records all22 wins PASS.
All22 openings/menu/save48 +17248/original outcome, original loss fixtures PASS;
atomic routes incl Z6->Z7 fallback/five slots/source labels/backup PASS. All22
presentation states PASS;161 resources. All22 banks all speech byteexact bounded
4MiB mixer IO0/fakequeues0. Z7 has120 total responses,6 mission/briefing WAVs;
6 independently StormLib byteexact. ARM11 verification PASS.

Original-model FULL68317 journal reconstructed and native Victory shown. Replay
normal31730168 bytes, linearfree123904; fresh Z7 story/briefing portrait/speech/
Start/objectives-loading/world checked, fresh normal31486448 linearfree25600.
36MiB/8MiB preserved. Long native journal reconstruction took ~407 seconds:
performance limitation, not an interactive full playthrough. Emulator STOPPED;
config/profile/progress/all saves/executable BYTE-RESTORED with
/tmp/sc3ds-twenty-two-emulator.py restore. Backup in twenty-two-validation/
emulator-before. Physical SD untouched. Staged/out executable a98 match.
Private package campaign-twenty-two-update-2026-10-29.zip134447501 bytes SHA256
8ebf38fdcc0270845fa0c9c448ec7265e87c2b8901aafa9ccea18fb3306e4a36.
161presentation resources +T5/BWP2/T6/T7/Z5/Z6/Z7 maps/brief/audio, overlay prior
full15 install or later overlay. CRC/hashes/privacy/exe equality PASS. No profiles/
progress/saves; strict source saves require starting a fresh mission. Report
CAMPAIGN_TWENTY_TWO_VALIDATION.md, evidence local-data/campaign-twenty-two-validation.

Further private Z8 Eye for an Eye driver work NOT enabled/certified. Raw zerg08
starts2Drones,3Hatcheries,4Nydus,6Hydra,9lings,2Ultras,5Overlords, Kerrigan; zero
initial funds. Owners1/2Protoss. Must destroy bases, no DT74 escape, hero survive.
Escape pads432,544 /3568,1024 /2704,3008; DT can run around them toward map edge.
/ tmp path names below are actual /tmp (no space):
/tmp/sc3ds-z8-nydus-guards.cpp survives to13130 before northeast DT escape;
both original Ultralisks sent through Nydus3424,576, then toward2672,3312 stop
first southern DT. Earlier global context chases vacate chokepoints: only pursue
when within200 of detector and nearby guard; keep initial guards local. Native
DT hp40/shields80 and rapid movement. Gate Overlord nearest assignment, lower
southern detector to3264, NE to1120. /tmp/sc3ds-z8-three-guards.cpp attempted split
Ultras and fixed gas IDs/nearest builder but loses first DT at2993; not a native
bug. The older two-Ultra version is the stronger starting point. Source cpps/logs
and /tmp/sc3ds-z8-prepare-three-guards.py preserved. No Z8 product change.

Overall goal remains incomplete:22 enabled/victory-tested, not all62. BWT2, T8,
P5 and later mission blockers remain as above. No further automations authorized.
Usage checked once this continuation: ordinary usage allowed,15% five-hour used;
no reset/purchases/model changes. Do useful work, not artificial token spending.

IN-PROGRESS NEXT CHECKPOINT (do not confuse with completed22 above): SC Zerg8
Eye for an Eye private driver won at39028; now formal campaign_zerg_eighth.cpp
and catalogue scenario38. Source src-a4ec4ee1972865cd, engine object
/tmp/sc3ds-twenty-three-engine.o. All23 ASan wins and openings/defeat/routes/slots/
presentation checks PASS, all23 bounded dialogue banks byteexact IO0 PASS.
Z8 has199 unit/mission responses including11 independently StormLib-exact WAVs.
168 presentation resources prepared, native ARM11 build PASS. Full39028 native
journal currently LOADING in emulator; backup at campaign-twenty-three-validation/
emulator-before; MUST STOP AND RESTORE via /tmp/sc3ds-twenty-three-emulator.py
restore when finished. Physical SD untouched. Need capture Victory, fresh Z8
story/brief/Start/world, logs/memory, restore, final report/NIGHT and local commit.
Pending private ZIP campaign-twenty-three-update-2026-10-29.zip153743473 bytes,
SHA2565c598af59d6053b87f9d44647a45aec94eb3d6c188e3ae160e5e9752900b481b.
168assets +T5/BWP2/T6/T7/Z5/Z6/Z7/Z8, overlay/full15 prerequisite, strict saves.
Source freeze while native build/replay: no fingerprinted code edits underway.

Z8 strategy now works: native Nydus sends both initial Ultras to southern guard;
Kerrigan holdsNE and casts leading Ensnare (+64,-96), four paid Mutas guardNW,
four paid Mutas guardSouth; remaining air force attacks after28Mutas. Native
Overlord Speed researched; close local guards chase only detected DTs, never
vacate all three exits for a distant cloaked actor. Actual authored LOSS areas:
location17 NW(0,0)..(224,352),19 NE(3808,320)..(4096,576),18 South(2368,3904)..
(2880,4096). Killing after entering an area cannot cancel an already scheduled
original defeat. Guard BEFORE those rectangles: NW256,672/lord256,576; southern
Ultras2768,3648,Hydra/ling2688,3616,lord2720,3584. Initial gas overassignment and
low-tech unit spam starved tech; fixed gas IDs, nearby builder, max12lings/12Hydra
until Spire. Do not report those driver errors as engine defects. No original
units/resources/triggers/AI/stat changes in working Z8. Public orders only.

Next original Z9 The Invasion of Aiur rawzerg09: starts4Drones,6lings,2Hydra,
3Overlords,Hatch576,304,Evo368,224,Pool304,352,Sunken448,384,ownbeacon528,480.
Crystal beacon owner3 at1872,2912; formation type173 owner6. BringDrone there,
hold while harvesting, then carry crystal to ownbeacon. EnemyProtoss2minor,
4/7major with multiple Carriers/Reavers/cannons; no current Z9 pilot. Original
maps/AI/trigger logic not yet certified by victory. Z10 Temple+crystal afterwards.

CHECKPOINT TWENTY-THREE COMPLETE — supersedes the in-progress notice above.
Source a4ec4ee1972865cd, all23 original ASan victories + required checks PASS.
Full39028 original-model replay now confirmed native Victory, fresh Z8 story,
brief portrait/speech, Start/objectives-loading/world confirmed. Replay normal
31628104, linearfree123904; fresh normal31473408, linearfree25600. Emulator STOPPED
and settings/profile/progress/all saves/executable BYTE-RESTORED via twenty-three
emulator script. All evidence/logs/screens in campaign-twenty-three-validation.
Physical SD untouched. Report CAMPAIGN_TWENTY_THREE_VALIDATION.md. Latest verified
private ZIP153743473bytes SHA2565c598af59d6053b87f9d44647a45aec94eb3d6c188e3ae160e5e9752900b481b,
168assets+eight later maps/brief/audio. Overlay existing full15 or later overlay,
strict source saves require fresh mission, old packages preserved. Stage/out a4ec
match. Original result-art task also complete in ignored update image screens;
6native victory/defeat backgrounds,18UI parts,6previews, contact sheet. No external
push or physical SD. Overall goal NOT complete:23 enabled/victory-tested.

Z9 trigger inspection (read-only original): location2(1824,2816)..(2080,2976)
accepts Drone, removes it through original action25 and starts600-second timer.
No need to keep that removed actor alive; defend the colony during the timer.
On expiry original triggers create a Drone and powerup219 at location3 center
1792,2912, move tracking location24, and ask return to own beacon location1
(448,416)..(608,544), center528,480. Formation173 owner6 at1984,2896. Original
field gives starting funds; no original Z9 driver yet. Potential legitimate paid
Overlord delivery/return with native Ventral Sacs; build a defense before starting
harvest, because original AI activates around switches/approach areas. Need actual
original timer, native crystal pickup/cargo/return and victory, not a synthetic
trigger fixture. Do not assume burrowing is needed: the first Drone is removed
by the authored sequence. Preserve current23 source until a Z9 win proves next.


## 2026-10-30 — BW focus, checkpoint24 complete

Latest verified source src-cefee4a794895e2a, host ASan object
/tmp/sc3ds-twenty-four-engine.o. BW Zerg2 Reign of Fire now scenario39. Three shared
fixes: native group destinations for scripted Order46; neutral/rescue campaign
artefact PowerupIdle; carried-item search accepts original item owner as well as
carrier owner for Move Location. Original autonomous intro now destroys generators,
delivers SCV217, rescues2Drones/grants850/200. Native original paid24Muta strike wins
32311, exact completed replay; original armed public-control loss2575. All24 ASan
victories/openings/save48/17248/defeats/routes/slots/presentation/audio pass. Focused
group,18artefact cases, autonomous intro active replay pass; oldsource negative
controls fail as expected. All30 BWZ2 mission/brief WAVs independently StormLib
byteexact. 175assets. Full32311 original-model journal shows Victory; fresh story/
brief/Start/world checked. Replay normal32195952, linearfree123904; fresh32082856,
linearfree25600. Emulator STOPPED and byte-RESTORED. Physical SD untouched.
Report CAMPAIGN_TWENTY_FOUR_VALIDATION.md; evidence campaign-twenty-four-validation.
Private ZIP campaign-twenty-four-update-2026-10-30.zip173898907bytes SHA256
c181f7fef961e9f3fbd467c710586d260975f2251bb3565cc3b2f48234983a6b.
Overlay full15 or later, start fresh, no profiles/saves/progress. Stage/out match.
Overall goal still incomplete:24verified, BW onlyP1/P2,T1,Z1/Z2.

BWT2 escort retest on final24 still captures3 then pilot/marines die sectorB;
/tmp/sc3ds-bwt2-escort-current.log. Prototype platform-local nearest-human-beacon
driver stalls before firstcapture, not a product fix. Need sector-specific
beacon route after original teleport, properly escort/cloak/lockdown. Preserve
original triggers. BWP3 previous macro driver mishandles gas and distant builders;
Scouts70 ARE allowed; Carriers72 disabled. Need paid air/DT solution to two
Cerebrates151 at432,2240 and848,2336. Not yet proved a corebug or victory.

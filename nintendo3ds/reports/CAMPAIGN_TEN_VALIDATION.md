# Ten-mission checkpoint — 2026-10-26

Source `src-785cc29d3d8dcef5`. Catalogue appends original Terran3, Desperate
Alliance, without changing previous scenario IDs. Its private owned map, briefing,
14 mission/briefing speech WAVs and 131 native unit responses are prepared. Saves
validate catalogue membership instead of assuming the last old enum value.

Combat now credits actual damage kills (including transport cargo and Broodlings),
unit Kills UI counters and campaign Kill conditions. Allied kills count; own units,
hallucinations and trigger removals do not. Turret/fighter kills credit the parent.
ASan direct combat/condition tests pass, including repeated lethal hits and255 cap.

Original AI interpreter now supports ownership branches, construction/training
waits,16-deep call/return, multirun, unit limits, train, expansion preference,
shared vision and alliance/rescue scripts.64 bounded thread slots; immediate
one-shot execution avoids filling waiting slots with vision triggers. Malformed
stack/return and capacity fail explicitly. Synthetic VM tests include real paid
SCV construction and actual original one-shot scripts: PASS. Adapter still is not
retail AI implementation parity; unsupported opcodes continue to fail explicitly.

Fresh ASan public-controls original-map victories on this source:
- SC Terran1 frame7649; Raynor rescue/selection/order/ending audio and defeat branches.
- SC Zerg1 frame21972; SC Protoss1 frame23544.
- BW Protoss1 frame10771; BW Zerg1 frame13725.
- SC Terran2 frame17215, Raynor alive and12 mission transmissions.
- SC Zerg2 frame6460, actual Chrysalis pickup/escort/beacon plus held-item replay.
- SC Protoss2 frame24517, timer/Fenix/Cerebrate plus completed replay.
- SC Terran3 frame43442: original30-minute countdown and evacuation, paid mining,
  Marine production, bunker loading and repair,7 mission transmissions. Completed
  save/replay digest matches. The test loses its workers but survives; it is no
  claim of sophisticated tactics or full balance testing.
All ten: menu routing,1200-tick startup, active save/replay,48 identical continuation
frames and extension to17248 ticks or mission end: PASS. ARMv6K/VFPv2 check PASS.

BW Terran1 regression remains unresolved: current public-controls autoplayer
loses frame46605. Do not carry forward the older-source victory claim. No runtime
exception in that trial; it is a failed tactic, not evidence of a passed mission.

Original-model Azahar CPU100 native Terran3: menu/briefing, black Loading both
screens, live world/countdown/no top title strip. Briefing4 played/missing0/IO0.
Normal allocation31,252,032 bytes at audio-ready; linear free123,904. About105s
idle observation:59.8FPS/23.9TPS, no missing images, cache overflow, audio I/O or
music underrun errors. Emulator startup/idle is not native mission victory or
physical-console speed proof. Model/profile/both original save slots restored
byte-exact; emulator stopped; physical SD untouched.

Private package: local-data/releases/campaign-ten-update-2026-10-26.zip.
Requires previous full six-campaign assets; contains current executable and four
additional missions only, with no profile/progress/saves. Private receipt/raw
logs/screenshots/binary/ELF: local-data/campaign-ten-validation/.

Broader compatibility audit (not enabled/verified missions):40/60 inventoried map
records reach1200 ticks;15 load rejections and5 tick rejections remain. Includes
branch variants and two unused cut maps, so60 is not a canonical mission count.
Missing trigger actions/door states and later AI operations remain. Reaching1200
frames alone does not establish correct objectives, AI behaviour or completion.

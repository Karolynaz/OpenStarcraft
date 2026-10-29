# Campaign expansion and eighteenth mission checkpoint — 2026-10-29

Final source: `src-63c47a7913e9b844`. Private owned game resources are not in Git.

## Campaign AI and creep cleanup

Added bounded adapters for original AI actions 17 (nearest resource expansion),
111 (specified resource-site expansion) and 112 (live town-count branch). Workers
use native movement, placement, construction and real player resources. A child
script starts only after its resource depot completes. Reserved workers are
excluded from the ordinary economy. Captured/dead workers are not commanded.

Resource-site IDs use CHK start-location encounter order, followed by connected
resource clusters sorted by position, with a zero sentinel and 250-entry limit.
This is an independent deterministic adapter; it does **not** establish identical
retail resource-area numbering. Town production remains the existing owner-wide
Side model rather than full retail per-town queues. These limitations remain
relevant when assessing later mission AI behavior.

Fixed the native 1024-entry creep recession queue saturation: a dead provider is
retained until all remaining tiles can be queued, then recycled. Queue capacity
is unchanged and surviving providers retain their creep.

ASan checks (pre-catalogue source `src-49382ba4c697e557`):

- All three races: paid depot construction, duplicate suppression, completed-child
  launch, occupied/depleted sites, insufficient funds, captured worker and nearest
  site expansion; 18 cases passed.
- Town-count true/false branches include live area towns and discard empty towns.
- CHK site order survives human-owner normalization and repeated map loading.
- Saturated creep queue drains after native building death; live creep survives.
- Original BW Zerg 6 saves at frames 1600 and 2000 each replayed 4000 further
  frames with exact state digests through expansion and creep cleanup.
- 64 original normalized map records audited up to 36000 ticks/original outcome:
  62 without runtime rejection. The two excluded records are unused/cut Terran
  maps `terran07` and `terran10`, not the shipped seventh/tenth missions.
  An idle-map outcome is **not** a victory test. BW Zerg 6 is not enabled yet.

Original-model emulator replay of BW Zerg 6 also reached normal gameplay and
pause objectives: normal heap 30,286,544 bytes, linear free 451,584 bytes, 2MiB SFX
cache. Emulator-only fixture, no physical SD writes.

## Eighteenth enabled mission: SC Terran 6, Norad II

Added original owned map, briefing, dialogues and presentation. The map's internal
prototype title is New Year's Day; campaign title is Norad II. Scenario ID 33.

`campaign_terran_sixth.cpp` uses public player controls: rescue the colony, mine,
train SCVs, build the paid refinery/factory/starport/control tower, train two
Dropships, load Raynor, fly the convoy and unload at the original rescue location.
The air-route driver uses fog-off information for deterministic testing, and
native group Move commands. It does not change units, funds, terrain or triggers.

Original victory fired at frame **15768**. Raynor and the crashed Norad II survived.
Two Dropships reached the original Bring condition; one subsequently died during
victory dialogue. The completed save restored with an identical state digest.
This proves a host playthrough, not a whole-mission native-console playthrough.

## Final build checks

- All **18** original/public-control victory drivers passed under ASan.
- All 18 menu routes, original objectives and 1200-frame startups passed; active
  saves each replayed 48 exact frames, then ran to 17248 ticks/original outcome.
- All 18 original defeat branches passed using isolated required-force absence
  fixtures. Shipped maps remain unmodified.
- New Terran 6: **24 original WAVs** byte-identical to independent StormLib
  extraction. Total dialogue cache contains **281** original mission/briefing WAVs.
- Native ARM11 build passed: ARMv6K/VFPv2, no Thumb-2/NEON.
- Original-model native menu/story and animated original briefing verified.
  Native gameplay (over one minute) and pause objectives also verified. Normal
  heap 31,366,080 bytes; linear free only 25,600 bytes, with a 4MiB SFX cache.
  This is within the original-model limits but has very little linear headroom;
  a longer native mission playthrough is still needed. Config/profile/progress
  and all virtual save files were byte-restored after stopping the emulator.

Private detailed evidence: `local-data/campaign-expansion-validation/` and
`local-data/campaign-eighteen-validation/` (ignored). Logs report actual test
outcomes; no assertion that all SC/BW missions are finished.

## Private test overlay

`local-data/releases/campaign-eighteen-update-2026-10-29.zip`

60,517,106 bytes; SHA256
`f163404d463bd71d9b185a4e25346d01c405c26ac65dea85946b08a74f606aa7`.
133 presentation resources, executable, and new T5/BW P2/T6 maps and mission audio.
Requires the existing full fifteen-mission private install; not standalone.
ZIP CRC, resource hashes and executable equality passed. No profiles, progress or
save slots included. Older-source saves remain incompatible by design; start a
new mission with this build. Earlier private checkpoint ZIPs are preserved.

# Six campaign openings — 2026-10-26 checkpoint

Implemented all six race/version entry points using the owner's original maps,
restrictions, forces, objectives, triggers and placed units. Owned data remains
ignored/private. Campaign indices16–21 preserve existing scenario numbers.
Human slots1/3/7/2/5/3 are normalized to zero, including restrictions and trigger
references. The menu maps the displayed race cards correctly in each version.
Briefings are paged text; movies and animated/speaking briefing portraits are
not implemented. Completion is recorded independently for each opening.

Additional CHK semantics: switches, deaths, custom score, unit properties,
invincibility, ownership, relocation, HP/shield/energy/resource modifiers,
alliances and minimap pings. Correct native CHK Exactly comparison10 and resource
Subtract9. Single Entity flag no longer aborts beacons/powerups on map load.
Resource-depot rescue transfers the entire rescuable owner's colony, fixing the
Nexus-without-pylons deadlock. Individual actors still rescue individually.
Behavior checked against primary reverse-engineering reference:
https://github.com/BoomerangAide/GPTP/blob/master/GPTP/hooks/orders/rescuable_order.cpp

Bounded adapter interprets the audited original opening AIScripts Zer1, Pro1,
TB1A, PB1A/PB1B, EnBk and Suic. Requests use native paid mining, placement,
production, upgrades and orders. This is an opening-mission subset, not a full
replacement for StarCraft's complete computer-player AI. Unsupported opcodes
fail explicitly. Later missions need a fresh opcode audit.

Each mission has its own bank so local CHK speech IDs cannot collide. All
original in-mission dialogue (5/11/6/17/10/9 WAVs) and placed hero response ranges
are included. Long dialogue streams through two reused128KiB halves; SFX cache
remains4MiB and long-voice allocation256KiB. Banks24.0–28.1MB are disk files,
not resident allocations. A31-second synthetic voice and all58 real dialogue
WAVs pass byte equality through the bounded mixer with zero IO errors. Audio
pause/reload/fragmented-cache tests also pass.

## Tests at checkpoint

- ARMv6K/VFPv2 native build PASS. Source src-57f63c0be9bfa4b8.
- ASan six-menu routing, original map/objective startup, save/replay and48-frame
  deterministic continuation PASS. Extended runs reach17248frames for five
  missions. Idle BWZerg loses at2251 after Exactly was corrected.
- Wasteland paid rescue/mining/construction/training victory frame7649, hero
  speech and defeat branches PASS (prior source; same campaign path).
- Original Zerg1 paid Spawning Pool/Extractor/Hydralisk Den, mining, hydralisks
  and combat victory frame22344 PASS on current source.
- Original Protoss1 rescue, paid workers/production/tech/combat victory
  frame21393 PASS on current source.
- Native trigger contract test covered switches, custom score, resource
  subtraction, UPRP/invincibility, HP, ownership, relocation, deaths versus
  removal and alliances. Sparse-town original Zer1 test built a Barracks and trained3 Marines with600 native minerals spent, including ongoing mining: PASS.
- Original-model/100%CPU emulator reviewed Zerg1 and all three BW openings at
  src-8d48a2f1fe3868da (before rescue/Exactly refinements). All loaded terrain,
  sprites, objectives and mission audio. Largest128x96 map audio-ready heap
  31853576bytes, linear free123904bytes. BW96x128 also fits. This does not prove
  physical hardware performance. Latest binary needs final emulator pass.
- Emulator stopped and previous settings/profile restored. No physical SD write.

## Outstanding work

Brood War win paths have not been completed with normal player orders. Naive
fixed-army route tests lose; distinguish bad tactics from trigger/AI issues
before claiming full playthrough coverage. BW Terran also needs a paid combat
completion test. Remaining native final-build check, package/manifest review,
and local checkpoint/handoff must preserve these limits honestly. Continue
these before adding further missions. Evidence is private under
local-data/campaign-six-validation and /tmp/sc3ds-* logs.

## Initial-unit flags correction (03:35 continuation)

Native UNIT loading had ignored StarEdit invincibility/burrow/cloak flags. This
made protected BWZerg hives destructible and placed ambushers immediately active.
Corrected these three flags. Original-map contract confirms7 invincible hives
and88 pre-burrowed enemies. All six extended/replay tests PASS at754ddd9a; BWZerg
now remains running at17248 idle frames. Existing candidate ZIPs predate this
fix and must be regenerated before final delivery. Last emulator57f63SCProtoss
startup also passed and settings/profile were restored; no emulator is running.

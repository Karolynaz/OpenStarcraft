# Campaign AI, Zerg4 and BW Terran1 — 2026-10-26

Final source `src-5ba4136efbd5aa80` (original3DS36MiB/8MiB budgets unchanged).

## Changes

Campaign loading now preserves the original UMS default alliance between
computer slots, across Force boundaries, before substituting the bounded AI
adapter. Original explicit alliance actions still override it. Debugging the
owned Zerg4 map showed Duke's units damaging Raynor's CC from frame120 and
killing it at717; that caused a false victory at2101 before Kerrigan existed.
After the fix the CC stays intact through the Chrysalis countdown. No purchased
map/trigger/stat edits or mission-specific victory bypass were used.

Nine additional AI operations are interpreted:45 nuke_rate,62 wait_force,
89 nuke_location,92 guard_all,98 killable,102 fake_nuke,107 create_nuke,
109 nuke_pos,110 help_iftrouble.97 kill_thread now cancels marked threads rather
than stopping its unmarked caller. Pending kills wait until the thread wakes.
Guarded regions use a5000-byte owner mask; nearby idle military responds to
visible threats, workers/committed attacks are excluded. Allied active AI can
help defend those regions. This is a bounded regional adaptation, not retail
priority/force-distribution parity. The existing64-thread/16-call/128-guard and
160-unit economy bounds remain.

Nuclear scheduling uses native Ghost/silo orders, timer resets on an actual
launch, preserves active casting orders, and purchases ordinary missile
replacements through native training. The retail create_nuke opcode is an
explicit scenario grant: it arms one empty completed silo and cannot duplicate
an existing/queued missile. Forced mission nuclear targets use original native
movement/launch/explosion behavior. There are no general resource grants.

Independent implementations use format/behavior facts from the primary
[PyMS tutorial](https://github.com/poiuyqwert/PyMS/blob/master/Help/Tutorials/AI_Basics.md)
and the researcher's [AI command reference](https://pr0nogo.wikidot.com/rs-ai).
The initial UMS-alliance behavior is also documented by the researcher in this
[Blizzard forum explanation](https://us.forums.blizzard.com/en/starcraft/t/how-do-i-make-two-ais-fight-each-other-in-the-map-editor-map-i-want-to-create-specially-with-already-placed-units/357).
No reference implementation code was copied into the product.

## Actual checks

ASan fixtures pass: cross-Force computer alliance, continued hostility to the
human, explicit enemy action and identical active save continuation; wait_force
pays50+50 for two completed Marines; marked regions respond beyond local
acquisition range and allied assistance executes; nuke_rate1 + fake_nuke launches
at2228/3674, rate0 at44/95, with native explosion damage. Empty-silo automatic
production pays200/200 before a real launch. Explicit create_nuke fills one silo
without duplicates, both forced target forms cause native damage. Marked waiting
threads stop, unmarked callers continue, all64 slots can be reused. Earlier VM
stack/guard/research/paid construction/one-shot tests also pass. Flat geometry and
initial actors in these synthetic fixtures are not substituted for mission maps.

Fresh unmodified-map/public-control host victories on the final source:

| Mission | Victory tick |
|---|---:|
| SC Terran1 /2 /3 /4 |7649 /17215 /43442 /27129|
| SC Zerg1 /2 /3 /4 |21972 /6460 /21026 /78808|
| SC Protoss1 /2 /3 /4 |23544 /24517 /31703 /129293|
| BW Protoss1 /Terran1 /Zerg1 |10771 /48606 /13605|

Zerg4: countdown observed, Kerrigan hatches at14477, objectives change, paid
Mutalisks destroy Raynor's CC, Kerrigan survives,14 speech events, completed-save
replay matches. Early false victory is explicitly rejected by the regression.
The final driver uses air combat; earlier transport experiments loaded cargo but
did not prove a landing, and are not counted as transport coverage.

BW Terran1: native paid economy/recruitment/research, Duran rescue/survival,
medics, siege combat and identical completed save replay. No enemy weakening or
free player resources. The old driver fed reinforcements piecemeal and lost;
its replacement assembles forces and regroups. Correct CPU alliances also stop
Zerg factions fighting each other in P4. Its driver now prioritizes paid cannon
defense and preserves heroes after loss of the Nexus, and again wins originally.

All15 menu/startup/active saves/replays,48 identical continuation frames and17248
idle ticks pass. All222 original dialogue WAVs stream byte-exact through a2MiB
cache,IO0/clean unload.17 new Zerg4 WAVs independently byte-exact vs StormLib.
Native ARMv6K/VFPv2 build passes. Original-model emulator on final source: Zerg4
D-pad chapter selection, briefing2 played/missing0/IO0, blackLoading and live world
with countdown/no persistent top title; normal heap31,079,712,linearfree123,904.
Emulator stopped; model/profile/both saves restored. PhysicalSD untouched.

## Limits and private evidence

Host victories are not native full playthroughs or physical-console performance
proof. Not all campaigns are enabled. Of64 inventoried map records (branches and
unused cut maps included),62 reach1200 ticks. At6000 ticks58 pass; later unsupported
operations are103(BW P3),108(BW T5a),111(BW Z6),38(SC Z8). Two unused SC cut-map
records T7/T10 reject their data format. Neither audit proves complete missions.

Detailed logs/ELF/3DSX and results.json remain private under
`nintendo3ds/local-data/campaign-fifteen-validation/`.
The private15-mission update requires the earlier full six-campaign assets.
It includes no profile/progress/save files; older-build saves are incompatible.

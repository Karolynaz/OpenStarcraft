# Campaign AI guard/control-flow stage — 2026-10-26

Source src-d77a6bf557d8ccee. Bounded128 guard requests adopt existing units,
return idle guards to town/resource positions, request paid replacements and
retire after three actual deaths. Units sent to an attack are released without
counting as guard deaths. Pending native queues satisfy replacement requests,
so a waiting guard does not continually add extra training jobs. Guard state is
included in replay digests. Guard requests bypass define_max, but retain the
existing160-unit economy bound. This is a bounded adaptation, not retail AI
byte-for-byte parity or a claim to reproduce all original guard priorities.
AI83/94 resource branches require both mineral+gas thresholds; allied/inactive
players do not satisfy enemy checks. AI93 enemy ownership and105 deterministic
seed are interpreted. Invalid AI race fails before supply-array access.

ASan VM tests: call/return/multirun, actual paid SCV started/completed waits,
original vision/attitude/rescue scripts,100 one-shots, malformed stack/thread
bounds, resource predicate/seed and actual50+50 Marine guard replacements then
three-death retirement PASS. Fixture race is set explicitly; an initial fixture
left raceNone, which exposed the need for a bounds check. No such invalid race
was found in Engine-normalized campaign sides. All12 active save/replay/48
continuation/17248 ticks PASS. ARMv6K/VFPv2 native build PASS. This backend stage
has not undergone native gameplay/memory checks and has no replacement package.

Compatibility only:51/60 map records reach1200 ticks. Remaining load errors:
four disabled-door/trap maps plus two unused cut maps. Three tick rejections:
BW Terran5a opcode45; BW Terran8/Zerg6 opcode92. Longer mission coverage will reach
further operations. These numbers are not mission-completion evidence.
Latest native-tested release stays source01D campaign-twelve-update ZIP.

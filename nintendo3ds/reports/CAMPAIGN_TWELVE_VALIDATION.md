# Twelve-mission checkpoint — 2026-10-26

Source `src-01da4c40a7f6daed`. Appends original Zerg3 The New Dominion and
Protoss3 Higher Ground. Earlier scenario IDs preserved. Both private original
maps, briefings, speech and unit response banks exported from owned data.

Fresh ASan public-controls original victories on this exact source:
T1=7649, Z1=21972, P1=23544, BW P1=10771, BW Z1=13725,
T2=17215, Z2=6460, P2=24517, T3=43442, Z3=21026, P3=38399.
Z3 uses paid multi-Hatchery/Lair/Spire/Mutalisk production and protects Chrysalis.
P3 uses paid Stargates and Scouts to clear both elevated enemy colonies; an
initial ground-only test failed to finish, which motivated testing the air route.
Both new completed saves replay to identical digests. No original map, trigger,
unit stat or resource amount is changed by these playthroughs. Test navigation
uses fog off. Earlier mission-specific rescue/hero/timer/cargo tests still pass.
BW Terran1 still has only startup/replay coverage on current code; its autoplayer
lost on the preceding source. There is no fresh full-victory claim for it.

All12 menu routes/startup/active save+replay/48 matching continuation frames and
extension to17248 ticks or mission end pass. Frontend progress/resume/header
cycling tests pass, including the third Terran stage. All167 complete campaign
speech WAVs byte-exact through bounded mixer at2MiB cache, I/O0; unload releases
all test allocations. Original ARMv6K/VFPv2 architecture check passes.

ADPCM extraction fix: stop at the requested output capacity, including an odd
or zero span, and permit a final partial stereo frame without overreading.
Reject invalid channel counts and shifts. ASan+UBSan synthetic canary/boundary
regressions pass. This resolves original Z3B01uza.wav's final-sector exception.
28 mission/briefing WAVs from T3/Z3/P3 are byte-identical to independent extraction
with the upstream StormLib reference built only in /tmp. Comparison receipt is
private. Reference: https://github.com/ladislav-zezula/StormLib/blob/master/src/adpcm/adpcm.cpp
No StormLib implementation is incorporated into the product.

Original-model Azahar CPU100 (36MiB normal/8MiB linear) exact-source checks:
- P3 original menu/briefing/world, D-pad LEFT opening->third stage, black Loading,
  no persistent top title; audio-ready normal31,288,888; linear free123,904.
- Z3 original menu/briefing/world, including fixed speech; audio-ready
  normal30,971,264; linear free123,904. Briefing2 played/missing0/I/O0.
- Short idle samples near59.8FPS/24TPS, no missing images/cache overflow/audio I/O
  or music underruns. These are emulator startup/idle checks, not native full
  mission playthroughs or physical-console performance guarantees.
Emulator stopped. Original model/profile/both saves restored byte-exact. No
physical SD writes. Screenshots, raw logs, exact ELF/3DSX and receipt stay private
in local-data/campaign-twelve-validation.

During intermediate testing I accidentally duplicated D-pad input dispatch.
The native check caught double stage changes; duplication was removed before
this fingerprint. Main input is unchanged from the preceding checked code.
Earlier NIGHT note claiming D-pad dispatch was absent was mistaken; existing
handling was farther down the same loop. Final native header cycling passes.

Private incremental package: releases/campaign-twelve-update-2026-10-26.zip.
Requires earlier full six-campaign assets and includes executable plus six added
mission sets. No profile/progress/saves. Prior ten/nine packages preserved. Old
save builds are incompatible; campaign completion keys remain stable.

All original campaigns are NOT finished. Later door/trap states, trigger actions,
AI operations and branch progression still need implementation and true victory
coverage. The broader40/60-record startup audit is compatibility evidence only.

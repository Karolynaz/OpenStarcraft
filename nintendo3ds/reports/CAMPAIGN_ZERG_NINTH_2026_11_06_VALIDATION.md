# Zerg 9 campaign checkpoint — 2026-11-06

Source: `src-cbc950f1a30aab88`. The installed catalogue now has **31 missions**:
7 SC Terran, 9 SC Zerg, 4 SC Protoss; 4 BW Terran, 3 BW Zerg, 4 BW Protoss.
This is not a claim that all campaigns are complete.

## Changes

- Implement original AI opcode 42, `value_area`, with zero operands. Its location
  center marks a pathfinding region unless hostiles already occupy it. A region
  without an owned building receives one defensive wave; one with a building
  remains defended. Separate fixed 5,000-byte region state participates in the
  deterministic digest. Existing `guard_all` territory remains independent.
- Enable original SC Zerg 9, **The Invasion of Aiur**, as scenario 46. Its owned
  CHK, briefing, eight dialogue clips and story/presentation are private local
  resources, not tracked assets. The fallback story describes the crystal task.
- Add public-control victory/loss and exact replay tests, plus progression from
  Zerg 8 to Zerg 9. The original Zerg 9 map has no SetNextScenario action; the
  catalogue supplies episode order. Zerg 10 remains unavailable.

Opcode format/semantics primary reference:
[PyMS CodeCommands.py](https://github.com/poiuyqwert/PyMS/blob/6e325d2e9474716fa2be3aca6479ff7d29dbaab5/PyMS/FileFormats/AIBIN/CodeHandlers/CodeCommands.py).
The author guide at <https://pr0nogo.wikidot.com/rs-ai#toc126> describes the defense
semantics. No bytecode is silently skipped.

## Actual host / ASan results

- `test-campaign-ai-value-area`: four native-order cases PASS: one-time empty
  territory response, repeated building defense, existing-hostile rejection,
  ally presence. Negative control against the previous header aborts on
  unsupported opcode 42 as expected.
- Existing later-AI and roaming tests PASS (guard/help, paid training, nuclear
  launch/damage/timer, scoped combat reset, deterministic wandering/cleanup).
- Zerg 9 public-control victory **frame 23,365**, with paid economy, Lair and
  transport/speed upgrades, real 600-second harvest, physical crystal pickup,
  Overlord loading/unloading and delivery to the original home beacon. No map,
  funds, statistics, tech availability, AI or trigger edits. Fog-off navigation
  is the test's only visibility aid. Original victory triggers fire normally.
- Active carried-crystal save at **21,841** and completed save: exact state
  digests after command-journal reconstruction PASS.
- An undefended original colony loses to original attacks at **65,691**; exact
  defeat save/replay PASS. This is not a surrender or injected trigger.
- All **30 previous original victory drivers** freshly built against the final
  engine PASS, including their replay assertions. Together with Zerg 9: 31.
- All 31 menu routes, original map openings, active save/replay plus continued
  startup simulation PASS. Openings alone are not counted as victories.
- All 31 private presentations PASS; Zerg 8 → 9 progression and result routing
  PASS. All six race/result art pairs PASS. An obsolete BW Zerg 2 result-test
  expectation was updated to the already installed BW Zerg 3 route.
- Zerg 9's **8 original dialogue WAVs** traverse the bounded mixer byte-exact
  with a 1 MiB cache and zero I/O errors. All 31 normalized map manifests PASS.
- Native ARMv6K / VFPv2 / no Thumb-2 or NEON verification PASS.

ASan runs use `detect_leaks=0`; this is address-safety coverage, not leak checking.
Private evidence: `nintendo3ds/local-data/campaign-value-area/`, including final
30-driver JSON/logs, native logs/screenshots and source-specific saves.

## Original-model emulator

Azahar configured as original 3DS, CPU speed 100%. Final source:

- Completed Zerg 9 replay reaches the actual **Victory, 16:13** result screen.
- Active replay displays the selected Drone with **Carry** and the crystal icon.
  Live simulation continues at approximately 24 ticks/s.
- Observed maximum normal heap **31,761,392 bytes**, linear use **8,264,704**
  bytes (123,904 bytes remain in the 8 MiB arena). This is observed usage, not
  a proof of peak usage on every mission or every physical-console workload.
- Emulator stopped. The original virtual SD tree and configuration were fully
  restored. Physical SD untouched.

The first control attempts were blocked by an account-limit approval-review
failure. After the stated reset time, the same supported emulator tools worked;
no approval bypass, model/account change or extra credits were used.

## Private build

`nintendo3ds/out/StarCraft-3DS.cia` — 1,354,605,504 bytes,
SHA-256 `502bce5b585b18d6e79b37018ec73b17acdc4ec7092030d6d984eff5bf50b732`.

`nintendo3ds/out/StarCraft-3DS.3ds` — 1,354,592,256 bytes,
SHA-256 `73d58454c28c3119d702d2d4b0f9a12bf4ecc5513a571cd1d3f14252da6a2f0b`.

Package work: `nintendo3ds/local-data/ctr-release-2026-11-06-zerg9/`.
634 static private resources; user saves/profiles/logs excluded; no ZIP/upload.
CIA and CCI contain byte-identical NCCH; ExeFS/RomFS superblock hashes PASS.
The emulator check used the matching 3DSX/ELF; no separate CIA installation test
is claimed. Saves are source-specific; older source saves are not regenerated
or represented as compatible.

## Remaining work

All campaigns are not complete. Next priority: original BW Zerg 4's six authored
branches, followed by later BW Terran/Protoss chapters and SC Zerg 10. Passive
BW Zerg 4a–f diagnostic runs currently reach original defeats without interpreter
exceptions; that is not a victory/playability claim and none is enabled yet.

A primary opcode-table comparison also exposed older interpreter mappings worth
addressing with isolated tests: original opcode 5 is `expand`; opcode 17 is
`build_bunkers` with no operands; opcode 83 is `prep_down`; resources_jump is 84.
These are **not changed here**. Existing synthetic expansion tests currently use
17, so migrating it requires correcting both implementation and tests and then
rerunning campaign victories. Do not merely relabel or ignore the instructions.

BW Zerg 4 follow-up: the six passive branch runs end at original defeats
(18,199 / 21,690 / 21,380 / 21,504 / 20,202 / 16,792). A branch-a paid
Hydralisk/Guardian prototype clears much of the opposition but stalls at
120,000 frames; no victory assertion or catalogue promotion. Its experimental
selection/economy issues are not established engine defects. The private pilot
and logs are retained in `campaign-value-area/bw-zerg4-pilot/`. The prototype's
one logging-only out-of-bounds access to the three-entry UI player summary was
fixed locally before further testing; no such code was added to the project.

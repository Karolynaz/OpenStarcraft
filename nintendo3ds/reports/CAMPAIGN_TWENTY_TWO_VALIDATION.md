# Twenty-second mission checkpoint — 2026-10-29

Source `src-a98a180455a86d49`. Added SC Zerg7, **The Culling**, original owned
`zerg07`, scenario37. The campaign continues from Zerg6 even without an authored
next-map filename; five-slot saves label it `Zerg mission 7`.

## Original victory

`campaign_zerg_seventh.cpp` wins at frame **68317**. It starts without a base,
pays for a Hatchery, clears the enemy Extractor over the local gas, builds an
economy, captures a western resource site, buys Hydralisk upgrades and destroys
the mainland forces. It then researches Ventral Sacs, loads real infantry into
Overlords, flies and unloads them on the isolated regions. Assertions require
actual cargo, infantry on an island, transport research, the original victory
and exact completed-save replay.

Fog-off navigation and a separate read-only original-terrain model guide the
host driver. The model never advances or changes the actual campaign. No map,
triggers, costs, resources, units or original AI are overridden. Earlier failed
drivers sent miners toward distant gas, chased flying Overlords, or repeatedly
attacked unreachable island colonies with ground forces. Those were driver
strategy errors, not demonstrated gameplay bugs. The final test exercises the
existing native systems; runtime changes here are campaign registration only.

## Verification

- **22/22** original/public-control ASan victory drivers pass on this source.
- All22 menu/opening routes, active save restoration +48 exact frames, continued
  simulation to17248 frames/original outcome and original loss fixtures pass.
  Zerg7's required-force absence fixture loses through original triggers at frame2.
- Atomic progress/reload, Zerg6-to-Zerg7 fallback, five save slots/date labels,
  backup recovery and foreign-source descriptions pass.
- All22 presentation state tests pass;161 private presentation resources prepared.
  Zerg7 uses its owned location card and an authored short story summary because
  its original establishing-shot resource contains only location information.
- All dialogue in all22 mission banks streams byte-exact through the bounded
  4MiB host mixer, IO0/no outstanding fake queues or allocations. Zerg7 adds120
  unit/mission responses including6 mission/briefing WAVs; those6 separately match
  independent StormLib extraction byte-for-byte.
- ARM11 ELF verification passes ARMv6K/VFPv2, without Thumb-2 or NEON.

The original-model emulator replayed the **entire68317-frame journal** and
showed **Victory**. Fresh Zerg7 story, briefing portrait/speech text, Start,
objectives/loading and world were also checked. This is deterministic native
replay plus a fresh opening, not a complete interactive handheld playthrough.
Replay normal heap **31,730,168 bytes**, linear free **123,904**; fresh opening
normal heap **31,486,448**, linear free **25,600**. The36MiB/8MiB limits and console
controls remain unchanged. Long journal restoration is slow: core-start to
core-ready took approximately407 seconds in this emulator replay. The narrow
linear-memory margin also remains to monitor.

Evidence: private `local-data/campaign-twenty-two-validation/`, including logs,
checks JSON, six independently compared speech hashes and native screenshots.
Emulator stopped and config/profile/progress/all saves/executable byte-restored.
No physical SD writes. Staged/out executable bytes match.

## Private test overlay

`local-data/releases/campaign-twenty-two-update-2026-10-29.zip`,134,447,501 bytes.
SHA256 `8ebf38fdcc0270845fa0c9c448ec7265e87c2b8901aafa9ccea18fb3306e4a36`.

Includes executable,161 presentation resources and original T5/BWP2/T6/T7/Z5/Z6/Z7
maps/briefings/audio. Overlay on the existing full fifteen-mission installation
or a later overlay; not standalone. CRC, hashes, privacy exclusions and executable
equality pass. No profiles, progress or saves. Start a fresh mission: older-source
saves remain unavailable. Earlier packages are preserved.

The full campaign goal is still incomplete: **22 enabled, victory-tested missions**.
SC Zerg8, BW Terran2, SC Terran8 and Protoss5 still need successful complete
player drivers before promotion. The preceding62-map120000-frame compatibility
audit is useful runtime coverage, not proof that all those maps can be won.

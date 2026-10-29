# Twentieth mission checkpoint — 2026-10-29

Source `src-7625881d2c2b4a5c`. SC Zerg 5, **The Amerigo**, original owned `zerg05`,
is now scenario35. Campaign progress from Zerg4 selects it; saves label it
`Zerg mission 5`. Original story, location art, briefing, portraits and speech
are extracted privately from the owned game. No original map is replaced.

## Original outcome

`campaign_zerg_fifth.cpp` reaches the original victory at frame **60776** with
Kerrigan alive. Public commands use native cloak, combat, the original door
unlock and teleport, and natural Zerg regeneration between fights. Navigation
uses fog-off information. No units, money, stats, terrain, AI or triggers are
changed. Original PTEC keeps Psionic Storm unavailable; the test confirms no
Storm was cast. Exact completed-save replay is asserted.

The original-model emulator replayed the **entire 60776-frame journal** on the
final executable and displayed **Victory**. This is a deterministic native
replay, not a full interactive handheld playthrough. Also checked a fresh
campaign selection, original story, animated briefing, Start/loading and world.
The native replay used **31,768,728 bytes normal heap**, with **123,904 bytes
linear free** /4MiB SFX cache. Fresh opening used **31,758,544 bytes normal heap**,
with only **25,600 bytes linear free**. The original36MiB/8MiB limits remain;
linear headroom is tight and should continue to be monitored.

## Checks

- **20/20** public-control original victory drivers pass under ASan.
- **20/20** menu routes, original openings, active save +48 exact frames and
  further simulation to17248 frames/original outcome pass under ASan.
- **20/20** original defeat branches pass isolated required-force/hero absence
  fixtures; Zerg5 loss triggers at frame50 without Kerrigan.
- Atomic progress/reload, Zerg4-to-Zerg5 continuation and five independent save
  slots/date labels/backup recovery/foreign-source metadata pass.
- **20/20** presentation state tests pass. All original dialogue in all20 mission
  banks streams byte-exact through the bounded4MiB host mixer, with zero IO errors
  and no outstanding fake allocations/queues. New Zerg5 bank has20 mission and
  briefing WAVs plus147 unit responses; all20 original WAVs separately match
  StormLib extraction byte-for-byte.
- All six preplaced flying building coordinates in T8 now have explicit
  regression assertions, plus flight/production rejection/save48/native landing.
  This test also passes on the final20 source. T8 is still not victory-certified
  or enabled. Its initial InTransit loader fix is included in this build.
- ARM11 native build passes ARMv6K/VFPv2/no Thumb-2 or NEON verification.

A fresh **120,000-frame** unattended compatibility audit on the final source
covered64 owned entries:62 usable maps ran to the limit or an original outcome
without a runtime rejection (26 still running,36 original defeats without player
input). No ASan diagnostics. Two unused cut maps, `terran07` and `terran10`, remain
rejected. The earlier36,000-frame audit also passed the same62 usable maps.
These are **not** victory/playability certifications for the remaining missions.
The audit tool now records the actual winner and separates running from
original-outcome results instead of calling both startup.

Private evidence: `local-data/campaign-twenty-validation/`, including native
victory/story/briefing/world images, native allocation log, completed save, host
logs, regression results and independent WAV hashes. Earlier audit evidence is
under `local-data/campaign-transit-validation/`. Emulator stopped; prior config,
profile, progress, all saves and executable byte-restored. No physical SD writes.

## Private test overlay

`local-data/releases/campaign-twenty-update-2026-10-29.zip`, **99,640,646 bytes**,
SHA256 `2ae37ab13aebc217c48bab0c64c93870517dad7b53a10ced62cdc38d7ee73acb`.
Includes executable,147 private presentation resources and original maps/audio
for T5/BWP2/T6/T7/Z5. Overlay on the existing full fifteen-mission installation;
also works over the newer overlays. Not standalone. CRC/resource hashes and
executable equality pass. No profiles, progress or save slots included.
Old-source saves remain incompatible; start a fresh mission with this build.
Previous19 package is preserved. The full SC/BW campaign goal remains incomplete:
**20 missions are enabled and victory-tested**.

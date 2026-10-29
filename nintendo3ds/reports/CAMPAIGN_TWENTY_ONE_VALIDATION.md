# Twenty-first mission checkpoint — 2026-10-29

Source `src-c87b5b69ed3a4e88`. Added SC Zerg6, **The Dark Templar**, original owned
`zerg06`, scenario36. Zerg5-to-Zerg6 campaign continuation works even when the
original victory has no next-scenario filename. Saves label it `Zerg mission 6`.

## Original victory

`campaign_zerg_sixth.cpp` wins at frame **42378**, with Kerrigan alive. Public
controls mine, build and recruit for real costs, fight the original Protoss,
research Ventral Sacs after the challenge begins, transport Kerrigan with an
Overlord, unload at the original challenge and move the other units away.
Original triggers create and resolve Tassadar's illusion and finish the mission.
The test asserts native transport research, actual cargo, lone hero delivery,
original victory, surviving Kerrigan and exact completed-save replay.

Fog-off information is used for test navigation. No original units, resources,
stats, terrain, AI or triggers are overridden. Earlier drivers cleared the enemy
but failed to deliver Kerrigan; moving toward the plateau on foot stopped at its
edge. Early transport research also weakened an initial army and lost a separate
attempt. Researching after the original challenge avoids that player-strategy
failure. None of those attempts were reported as engine victory failures.

The original-model emulator replayed the **entire42378-frame completed journal**
on this source and displayed **Victory**. This validates native deterministic
replay, not a full interactive handheld playthrough. Fresh campaign selection,
story/location art, animated original briefing, speech text, Start/loading and
world were also checked. Replay audio-ready normal heap: **31,433,232 bytes**,
linear free **123,904 bytes** /4MiB SFX. Fresh opening normal heap: **31,329,728**,
linear free **25,600**. Original36MiB/8MiB limits and console controls unchanged.
The narrow linear margin remains a limitation to monitor.

## Checks and scope

- **21/21** original/public-control victory drivers pass under ASan.
- **21/21** menu routes/openings, active save +48 exact frames, further simulation
  to17248 frames/original outcome and original defeat fixtures pass. Zerg6
  required-force absence triggers an original defeat at frame2.
- Atomic progress/reload, Zerg5 continuation without an authored filename and five
  save slots/date labels/backup recovery/foreign-source descriptions pass.
- All21 presentation state tests pass;154 private presentation resources verified.
- All dialogue in all21 mission banks streams byte-exact through the bounded
  4MiB host mixer, IO0 and no outstanding fake allocations/queues. Zerg6 adds172
  unit responses and14 mission/briefing WAVs. Those14 separately match independent
  StormLib extraction byte-for-byte.
- ARM11 build passes ARMv6K/VFPv2/no Thumb-2 or NEON verification.
- Includes the preceding tested scenario-building visibility fix: scripts can
  create markers before exploration while ordinary placement and physical checks
  remain intact. See CAMPAIGN_TRIGGER_VISIBILITY.md. Its120000-frame compatibility
  audit covered62 usable maps without runtime rejection; it does not certify
  their victories. Two unused cut entries remain rejected.

Evidence is private in `local-data/campaign-twenty-one-validation/`. Emulator
stopped; config, profile, progress, all saves and executable byte-restored.
Staged/out executable matches the package. No physical SD writes.

The overall campaign goal remains incomplete: **21 missions are enabled and
victory-tested**. BW Terran2, SC Terran8 and Protoss5 have unfinished player-driver
work and are not promoted to victory-tested missions from startup alone.

## Private test overlay

`local-data/releases/campaign-twenty-one-update-2026-10-29.zip`,117,851,058 bytes,
SHA256 `5cec873f0f5dc68c807612294df0caba88614b3c25e5cf48033b93003bd25bb0`.

Includes executable,154 presentation resources and original T5/BWP2/T6/T7/Z5/Z6
maps/briefings/audio. Overlay on the existing full fifteen-mission installation,
or a later overlay; not standalone. CRC, resource hashes and executable equality
pass. No profiles, progress or saves included. Start a fresh mission: older-source
save files remain unavailable. Earlier packages are preserved.

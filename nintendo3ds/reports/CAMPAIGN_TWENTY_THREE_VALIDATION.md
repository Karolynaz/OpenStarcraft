# Twenty-third mission checkpoint — 2026-10-29

Source `src-a4ec4ee1972865cd`. Added SC Zerg8, **Eye for an Eye**, owned `zerg08`,
scenario38, campaign continuation from Zerg7 and `Zerg mission 8` save labels.

## Original victory and strategy

`campaign_zerg_eighth.cpp` wins at frame **39028**, with Kerrigan alive. Public
orders mine and purchase the army and Overlord Speed, use the original linked
Nydus, keep ground guards at three escape approaches, assign four paid Mutalisks
each to northern/southern air sentry duty, and send the remaining air force
against the Protoss. Kerrigan casts native energy-cost Ensnare ahead of moving
Dark Templar. Assertions require her survival, accepted native Ensnare, actual
southern Nydus transit, researched detector speed, both sentry groups, original
victory and exact completed-save reconstruction.

Fog-off information guides the driver. No original map, AI, triggers, research
availability, units, funds, costs or stats are overridden. Earlier drivers moved
all guards after a distant cloaked actor or guarded too late inside the authored
loss rectangles. The original triggers already schedule defeat on entry; killing
the runner afterward cannot undo it. Leading the spell and guarding before
those zones solve the player-driver failures. Runtime changes here register the
mission; the existing native systems handle these actions.

## Checks

- **23/23** original/public-control ASan victory drivers pass on this source.
- All23 campaign menu routes/openings, active save +48 exact frames, further
  simulation to17248 frames/original outcome and original defeat fixtures pass.
- Atomic progress/reload, Zerg7-to-Zerg8 authored route, five-slot save labels,
  backup recovery and foreign-source descriptions pass.
- All23 story/scroll/briefing/replay/Start/cancel presentation state tests pass;
  **168** private presentation resources prepared. Zerg8's owned location card
  contains no story; a short authored summary introduces the original objective.
- All dialogue in all23 banks streams byte-exact through the bounded4MiB host
  mixer, IO0 and no outstanding fake allocations/queues. Zerg8 adds199 unit/mission
  responses including11 mission/briefing WAVs. Those11 independently match
  StormLib extraction byte-for-byte.
- ARM11 build passes ARMv6K/VFPv2/no Thumb-2 or NEON verification.

The original-model emulator reconstructed the **entire39028-frame journal** and
showed **Victory**. Fresh Zerg8 story, briefing portrait/speech text, Start,
objectives/loading and world were also checked. This is native deterministic
replay plus a fresh opening, not a full interactive handheld playthrough.
Replay normal heap **31,628,104 bytes**, linear free **123,904**; fresh normal
heap **31,473,408**, linear free **25,600**. Existing36MiB/8MiB limits and console
controls unchanged. Long journal restoration and the narrow linear margin
remain limitations, as noted in the preceding checkpoint.

Evidence: private `local-data/campaign-twenty-three-validation/`. Emulator stopped;
config/profile/progress/all saves/executable byte-restored. Physical SD untouched.
Staged and out executables match. The overall goal remains incomplete: **23
enabled, victory-tested missions**, not all62 usable original campaign entries.

## Private overlay

`local-data/releases/campaign-twenty-three-update-2026-10-29.zip`,153,743,473 bytes,
SHA256 `5c598af59d6053b87f9d44647a45aec94eb3d6c188e3ae160e5e9752900b481b`.

Executable,168 presentation resources and T5/BWP2/T6/T7/Z5/Z6/Z7/Z8 maps/briefings/
audio. Overlay on the existing full fifteen-mission installation or a later
overlay; not standalone. CRC, hashes, privacy exclusions and executable equality
pass. No profiles, progress or saves. Start a fresh mission: older-source saves
remain unavailable. Earlier private packages are preserved.

# Sixteenth mission: Revolution — 2026-10-29

Final source `src-77ee612af31c2ed4`. Original3DS36MiB/8MiB limits unchanged.

SC Terran5 is appended as scenario31, preserving all prior IDs. The original
owned map, objectives, rescue/infiltration, neutral-to-rescuable Antigan colony,
paid combat, hero survival, victory and defeat are used. No mission-specific
victory shortcut, free player resources, changed unit stats or modified trigger
was added. The original establishing shot/story, animated briefing/casts,17
original briefing/mission WAVs and native hero responses are privately staged.

## Backend fixes

Neutral participant slots1-8 were omitted from the native trigger schedule.
In the original T5 map, neutral Antigan slot3 belongs to Force3; its script makes
the colony rescuable after Kerrigan's infiltration. Its missing schedule left
Kerrigan at the Command Center forever. Neutral participants now get direct/
Force/AllPlayers triggers. Passive rescue, inactive and closed slots remain
excluded. An initial overbroad trial included passive rescue slots and broke
T4's teleport progression; the corrected filter restores its original win27129.
Neutral participants are distinct from Player12 (resource ownership).

Newly given rescuable units now wait in native Neutral idle instead of reaching
unimplemented RescuePassive order errors. CampaignRuntime still performs native
proximity rescue. Provoked neutral participants can become hostile through a
campaign adapter callback and use validated native attack orders, rather than
throwing at upstream's unimplemented computer-AI fallback. Other upstream users
retain the existing callback fallback. Native damage, acquisition and costs stay.

AI47 clear_combatdata clears scoped own committed-party membership; wave requests
and existing orders remain. AI101 junkyard_dog tags fully contained movable own
units, inherits original location bounds through child threads and periodically
issues deterministic reachable native attack-move orders when idle. Generation
IDs/owner checks prevent captured or replaced units inheriting old behavior.
Bounds match1700 native unit slots. Location and roaming state are replay-hashed.
This is bounded adapter wandering, not a claim of byte-exact retail AI parity.

The simulation/replay state changed. Earlier-build saves are therefore marked
unavailable, with their mission metadata preserved; they are never overwritten
by installing the update. Progress/profile files remain valid. The prior UI-only
cross-build save aliases were removed instead of pretending old journals match.

## Actual validation

- ASan16 original-map/public-control host victories pass on final backend b641
  (before only the final text-inset change). Final77ee T5 wins29112 with Raynor/
  Kerrigan alive, changed objectives,16 speech events and exact completed replay.
- Final77ee BW Protoss1 wins10765 and BW Zerg1 wins13605, both with new exact
  completed-save replay checks. T4 again wins27129. All16 original required-force/
  hero-loss defeat fixtures pass (synthetic absent actors, shipped maps unchanged).
- Final16 menu/story/briefing startup, active saves,48 exact continuation frames
  and17248 idle ticks (or an original outcome) pass. These are continuation checks,
  separate from the16 paid/native-control victory drivers.
- ASan neutral direct/Force triggers, passive/inactive/closed exclusion, delayed
  whole-colony rescue, hostile neutral retaliation and exact active replay PASS.
- ASan scoped clear_combatdata and wandering with child-location inheritance,
  real movement, collision-box containment, owner-transfer cleanup, deterministic
  RNG/digest PASS. Five save-slot exact replays/backup/metadata PASS.
- Frontend and all16 presentation state/cast/page/replay/start tests PASS. Original
  story text decoding now explicitly uses Windows1252 (T5 includes smart quotes).
  Native visual QA found text touching the ornate frame's right mount; speech and
  loading objectives now share280px width at the same font size; replay is centered
  within that content region. Native final briefing text fits the frame.
- All239 original mission/briefing WAVs byte-exact through the bounded2MiB mixer,
  IO0/clean unload. T5's17 WAVs independently match StormLib extraction byte-for-byte.
- Native ARMv6K/VFPv2/no Thumb-2 or NEON build PASS.

Original-model emulator on final77ee: menu opens Revolution via real saved progress,
original story/room/animated Raynor/speech, final padded upper frame, Start/loading/
world, pause's upper objectives, native save/load Slot4 with mission/date label and
live restored map all verified. Startup audio-ready normal31,206,248 bytes,
linearfree25,600 while96KiB photo/frame is retained. Restored world reaches
core/graphics/audio ready. No native full victory or physical-console claim.
Emulator stopped; configuration, profile, progress and saves byte-restored. No
physical SD writes.

## Private overlay and remaining work

`nintendo3ds/local-data/releases/campaign-sixteen-update-2026-10-29.zip`
23,700,961 bytes, SHA256
`fa1833033b823fa9e327e158c80884df4f6158a83f0334680b9267c7e36546a9`.
ZIP CRC,119 presentation resource hashes, new T5 map/brief/audio/manifests and exact
3DSX equality verified. Overlay on the existing complete private15-mission install.
No profiles/progress/save files included. It is not an all-campaign release.
The packagers and map validator now read the enabled catalogue rather than the
historical hardcoded six openings. Private raw results/owned assets remain under
`nintendo3ds/local-data/campaign-sixteen-validation` and the staging tree.

Broader6000-tick audit:61/64 records have no runtime rejection before the limit or
an original outcome. BW Zerg6 still reaches unsupported111 allies_watch; unused
cut SC Terran7/10 still reject format. This is not61 completed missions. AI101's
later BW Terran8 compatibility still needs a longer fresh audit. Continue later
mission objectives/AI, real victory drivers and catalogue expansion. The goal of
all SC/BW missions remains unfinished.

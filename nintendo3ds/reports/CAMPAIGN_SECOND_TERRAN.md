# Terran 2 milestone — 2026-10-26

Source `src-5d5ba6a3954a4350`, original 36 MiB normal / 8 MiB linear heaps.
The first six stages retain their scenario IDs; Backwater Station appends ID22.
This is seven available missions, not all campaigns completed.

- Privately extracts the original Terran02 map, briefing and20 original mission/
  briefing WAVs. Exports86 native unit responses; the per-map combined bank is
  27,208,311 bytes (647 effects). No purchased data is tracked or uploaded.
- Campaign cards resume the first uncompleted available stage of their episode.
  Left/right arrows and D-pad in briefing switch available stages for replay.
- Countdown condition/action, timer pause/resume, Never and Set Deaths added.
  Countdown advances on simulation ticks and is part of deterministic save state.
- ASan Backwater playthrough: victory frame16967, Raynor alive,12 transmitted
  sound requests. Native paid workers/Marines, mining/rescue/combat; original map,
  stats and triggers unchanged. Completed save replay digest matches.
- Seven menu/startup/save/replay/48-frame continuation and17248-frame idle runs
  PASS. Seven original loss branches PASS: the first six empty-human fixtures;
  Backwater removes the required Raynor across owners, as he starts rescuable.
  Backwater has no generic empty-human defeat trigger, so that fixture is wrong.
- Timer fixture reaches original victory at frame126; Never remains false;
  save/replay during active countdown stays deterministic. Separate trigger tests
  cover timer arithmetic/clamping/pause and Set Deaths set/add/subtract.
- Bounded mixer byte-for-byte streamed all117 dialogue WAVs across seven maps
  (20 Backwater) with zero I/O errors. Frontend profile/resume/replay tests PASS.
- Native ARMv6K/VFPv2 check PASS. Original-model CPU100 Azahar startup passes:
  existing completed Wasteland resumes Backwater, D-pad left/right cycles stages,
  briefing voices played4/missing0/IO0, both-screen black Loading, no title strip,
  live original scenario22 with Raynor rescued. Core26,178,912 bytes; audio-ready
  normal30,971,408 bytes, linear free173,056. Observed idle59.8FPS/23.9TPS is an
  emulator measurement, not original-console performance or native full victory.
- Initial virtual-SD preparation was rejected by automatic approval review. The
  subsequent08:40 user heartbeat explicitly requested emulator testing and scoped
  the restriction to physical SD; a fresh, narrow review approved preparation.
  Emulator stopped; original model/profile/both save slots restored byte-exact.
  Physical SD untouched.

Private screenshots/logs/receipt: local-data/campaign-six-validation.
Private incremental ZIP: local-data/releases/campaign-seven-update-2026-10-26.zip.
Install over the prior six-campaign asset package. Different-build saves remain
intentionally rejected. No original-console test is claimed.

Next: subsequent SC/BW stages need more native trigger/AI support and14 inventoried
large maps exceed the current128x128 port guard. Do not bypass the original3DS
memory limits or enable every map merely because it loads.

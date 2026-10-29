# Seventeenth mission: Dunes of Shakuras — 2026-10-29

Source `src-25f92e80eb908e7b`; native ARMv6K/VFPv2 build passes, with the original
36MiB normal / 8MiB linear limits unchanged. Appends BW Protoss2 as scenario32;
previous scenario IDs and campaign progress remain unchanged. This checkpoint
adds mission data/routing; the backend is the verified sixteen-mission backend.

The original owned CHK, briefing, establishing shot/location/story, animated cast,
18 mission/briefing WAVs and hero responses are privately staged. The public
control test establishes a paid colony in the northwest, mines resources, builds
native production, rescues four Dark Templar, follows the changed objective and
wins through the original trigger at frame35681. It grants no player resources,
changes no actor stats and modifies no shipped mission triggers. Fog is disabled
only in the test for deterministic route planning. The completed save replays to
an identical simulation digest. All four rescued Dark Templar eventually die in
the successful assault; their survival is not this mission's victory condition.

Additional ASan checks: all17 original-map menu/story/brief/startup routes, active
save replay plus48 identical continuation frames, and17248 idle ticks or an
original outcome PASS. All17 required-force/hero-loss defeat fixtures PASS;
these use temporary absent actors, not shipped map changes. All17 presentation
state/cast/page/replay/start/cancel tests PASS. All257 original dialogue WAVs
pass byte-exact bounded2MiB mixer playback, IO0 and clean unload. The18 new WAVs
also independently match original BroodWar.mpq extraction through StormLib.
Final source25f9: all17 public-control/paid original victory drivers were rerun
under ASan and passed; raw per-mission logs and regressions.json are retained.

Original-model emulator: real campaign progress opens Dunes of Shakuras, original
story/animated briefing/race frame fit, Start/objectives/loading, active world,
and pause objectives PASS. Scenario32 audio-ready normal heap31,318,424 bytes,
linearfree25,600 with the retained temporary96KiB photo/frame. This is native
startup/UI validation, not a native full victory claim. The emulator was stopped;
configuration, profile, progress and every save were byte-restored. No physical
SD writes. Screenshots and raw logs are private in
`nintendo3ds/local-data/campaign-seventeen-validation`.

Private overlay: `local-data/releases/campaign-seventeen-update-2026-10-29.zip`,
41,756,970 bytes, SHA256
`fd4ffd4ccd0df7a941c6a633a35e83e0098775e103b4a2bd2968bdace5434060`.
CRC,126 presentation-resource hashes and exact3DSX verified. Includes newT5 and
BWP2 maps/brief/audio, so overlays the prior full15-mission install (or16). It
contains no player profiles, progress or saves and is not a standalone install.
Earlier source-incompatible saves remain unavailable as explained in the16 report.

All campaigns are not yet verified. BW Terran2 remains an unpublished pilot:
three captured ships so far; sector transitions still under investigation.
BW Zerg6 AI111/resource-area expansion and remaining later-mission playthroughs
are outstanding. The final64-record runtime audit runs each map up to36000 ticks or an original
outcome:61 have no runtime rejection; BW Zerg6 rejects opcode111 at1522, and two
unused/cut Terran records fail parsing. This uncontrolled audit is not a victory
audit. BWT8 reaches an original outcome at25969 without its former opcode101
rejection. Shipyards diagnostic logging confirms the second crew is created;
the pilot sends civilians into enemy fire. No product trigger workaround was
added for that test strategy failure.

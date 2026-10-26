# Fourteenth mission: The Hunt for Tassadar

Source `src-d309c707e49eb3cc`. Adds original SC Protoss4 as appended scenario29,
with owned map/briefing and21 original briefing/mission voices. Existing mission
IDs and console controls remain stable. No mission trigger/stat/resource edits.

ASan original-map/public-controls victory at48321 ticks. Initial force reaches
and rescues the colony; paid mining, construction and production supply the army.
Both Tassadar and Raynor survive, the original ambush triggers execute, and the
original beacon conditions finish the mission.21 speech events; completed save
replay has an identical state digest. Cleaned test rerun also48321. Fog disabled
only for deterministic test navigation. All14 menu routes/startups, active save/
replay,48 identical continuation ticks and17248 idle ticks pass. Idle coverage
is not a full-mission victory check.

All205 enabled-mission dialogue WAVs stream byte-exact through the bounded2MiB
mixer,IO0, clean unload.21 new P4 WAVs independently byte-exact vs StormLib.
Native ARMv6K/VFPv2 build passes. Original-model emulator: D-padLeft selects P4,
briefing3 played/missing0/IO0, blackLoading then live world with no persistent top
mission strip. Normal heap31,347,744; linearfree123,904, unchanged36MiB/8MiB budgets.
No native full-victory or physical-console performance claim. Emulator stopped;
model/profile/both save slots restored; physicalSD untouched.

Private update `campaign-fourteen-update-2026-10-26.zip` requires the earlier full
six-campaign assets. Includes executable, all14 normalized maps/briefings and the
8 later-mission audio banks; does not overwrite profile/progress/saves. Older
build saves are incompatible. Detailed evidence stays in ignored local-data.

Limits: not all SC/BW campaigns are playable yet. BW Terran1 current automatic
strategy loses. Zerg4 idle run prematurely loses Raynor's CC before Chrysalis
hatches; not enabled. A trial AI human-target preference did not fix this and was
reverted; the root cause remains unconfirmed. Guard_all/nuke_rate AI operations
and later mission-specific behavior remain incomplete.13 original host victories
exist across the staged work (11 on95F,T4 onE811,P4 onD309); do not label all14 as
fully verified on console or all campaign missions complete.

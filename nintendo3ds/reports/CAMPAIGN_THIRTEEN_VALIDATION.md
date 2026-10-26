# Thirteenth mission: Jacobs Installation

Source `src-e811e7f7f211049c`, based on the installation backend95F.
The menu now offers original StarCraft Terran4, including its owned map,
briefing, hero responses and17 original briefing/mission WAVs. Scenario28 is
appended so existing scenario IDs remain stable. Chapter navigation/progress
routing uses the existing catalogue; no special victory shortcut was added.

ASan original-map/public-control playthrough wins at27129 ticks with Raynor
alive,22 speech events, original teleport/door/trap/combat triggers. Completed
save replay has an identical digest. No free units/resources, changed unit stats
or modified triggers. Fog disabled only for repeatable driver navigation.
All13 enabled maps pass menu routing/startup, active save/replay,48 identical
continuation frames and17248 idle ticks (unless an original outcome occurs).
This is startup/continuation coverage, not13 complete-mission victories.

Original3DS model emulator: real menu D-padLeft selects Terran4; briefing4 played,
missing0,IO0; reaches native world. Normal heap31,522,480, linearfree123,904 under
unchanged36MiB/8MiB limits. Native full victory and physical-console performance
remain unverified. Emulator stopped and original model/profile/saves restored;
physicalSD untouched.17 WAVs independently byte-identical to StormLib extraction.

Previous11 original victories are preserved on95F. Current BW Terran1 automatic
strategy still loses; later missions are not all enabled. Initial Protoss4 brute
force route loses its starting army; this alone does not establish a map bug.
All purchased data and detailed logs stay in ignored private local-data.

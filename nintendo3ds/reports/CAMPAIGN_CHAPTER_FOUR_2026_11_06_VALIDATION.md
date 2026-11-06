# Original BW chapter four — 2026-11-06

Final source `src-eac58d6bbec187e3`; ARMv6K / VFPv2 verification PASS.
30 enabled campaign entries: SC 7 Terran / 8 Zerg / 4 Protoss; BW 4 Terran /
3 Zerg / 4 Protoss. This is not completion of every original campaign.

## Newly enabled missions

**The Quest for Uraj** (`expprotoss04`, scenario44): original 96x128 map,
initial scripted landing, occupied-base combat, normal paid colony/defences,
Robotics and Support Bay, Gravitic Drive and eight paid Shuttles. Cargo-confirmed
Kerrigan reaches the original Uraj location; authored victory f40327. Original
Stargate PUNI lock is preserved. Exact active cargo save and completed replay
PASS. Public movement into the original enemy base kills Kerrigan and triggers
original defeat f2972. No altered terrain, actors, resources, damage or triggers.

**Assault on Korhal** (`expterran04`, scenario45): original paid SCV economy,
Marine/Medic/Goliath army destroys the original Physics Labs, authored victory
f68896 and original `Terran05a.scx` next-scenario field. Exact active colony save
at f28801 and completed replay PASS. Original empty-human-force absence fixture
triggers defeat f2; this is a synthetic isolated defeat fixture, not a claimed
public-combat loss. An alternative silo-targeting pilot stalled at240000 with
no result; the nuclear-silo route is NOT claimed verified. Neither fifth-mission
branch is enabled yet.

Both maps re-extracted from privately owned discs and byte-identical to the
previous independently retained normalized audit maps. New original briefing,
location cards, animations, intro fallback summaries and mission audio prepared
privately. Per-mission presentation filter avoids retranscoding unrelated
speakers when adding a mission. No purchased game data is tracked or published.

## Shared regression and presentation checks

- All 30 original-map paid/public-control victory drivers recompiled against
  final ASan engine: 30/30 PASS, exact completed replays included.
- All 30 frontend routes, authored objectives/startup, active save +48 tick
  reconstruction and extended opening simulation to f17248 PASS. Loading and
  these openings are not used as victory evidence.
- Episode progression/reload, original next branch validation and failed-write
  rollback PASS. BW P3 -> P4 and T3 -> T4 routes added explicitly.
- All 30 story/scroll/briefing timing, replay and Start/Cancel presentations PASS.
- All 30 native-size story crops, loading texture lifetime/cleanup and actual
  drawn speaker-count checks PASS. A prior empty draw loop could pass without
  faces: fixed that test. BriefingArt now reads the same original 60x56 portrait
  files as the HUD; centered 64x64 display and animated versions retained.
- New 20 Protoss and 7 Terran dialogue clips traverse the bounded 4 MiB mixer
  completely and byte-exact, with zero IO errors. No larger audio cache.
- Final-source retail SCZ2 single-A Chrysalis / BWZ2 Psi Emitter route, native
  rescue resources, carried render/HUD and exact held-save replay PASS.
- Python presentation syntax, normalized-map comparison and whitespace PASS.

Private final evidence: local-data/campaign-nov06-ch4. Initial P4 exploration and
source095 native captures: local-data/campaign-nov06-p4. The production tests
use proper scenario44/45, never the exploratory placeholder scenario19/20.

## Original-model emulator

CPU100 / original 3DS budgets (36 MiB normal, 8 MiB linear). P4 source095 fresh
story, animated briefing audio, initial world and reconstructed original Victory
28:00 observed. Final sourceeac P4 briefing now visibly shows all four animated
faces after fixing the 48x48 loader regression; screenshot retained. Final P4
reconstructed Victory28:00 also observed: heap31,484,120 / linear free123,904.
Final T4
reconstructed original Victory47:50 observed, normal audio-ready heap31,939,864,
linear free123,904. Native reconstruction of the long T4 journal took several
minutes; it is not an instant snapshot. An attempted T4 active-slot selection
actually reopened slot1; only the completed T4 replay is claimed as native
evidence (the active colony replay passes ASan). Emulator stopped and original
virtual SD/config fully restored. CIA installation/physical console are
not tested. No physical SD writes.

## Standalone private build

622 static assets, including all30 maps/presentations/audio, three HUD skins and
110 native portraits. No saves/profiles/logs or ZIP. Identical CIA/CCI NCCH
payload and ExeFS/RomFS superblock hashes PASS. Manifest:
local-data/ctr-release-2026-11-06-ch4/package-manifest.json.

- CIA1324606400 bytes, SHA256
  `49c7af71d006cd33bd6bc8cb6dae43bceee14229ee976b71b299d00285d311ef`.
- 3DS1324593152 bytes, SHA256
  `7b427dc22caaf025b5cc8e61bc9cde6fee383f033c6af2fd076428ce73969d65`.

Saves remain source-specific; chapter progress is separate. Builds are local;
no GitHub push performed. Further campaigns are still unfinished. A private
unmodified SC Zerg9 pilot reached later AI opcode42 at about48000+ ticks and
stopped with the explicit unsupported-opcode error; Zerg9 is NOT enabled or
claimed playable. Keep this failure visible rather than skipping the opcode.
Pre-existing Zerg wireframe health recoloring remains imperfect.

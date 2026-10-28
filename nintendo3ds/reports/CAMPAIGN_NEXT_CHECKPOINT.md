# Later campaign backend checkpoint — 2026-10-28

Final native source `src-f1bb1c604dd5ed45`. This is progress toward all campaigns,
not a claim that additional missions have been completed or enabled.

Added original AI operations 38 (move ordinary/hero Dark Templar to location),
103 (Disruption Web), 104 (Recall), 108 (explicit scenario unit creation).
Spells choose a completed, visible, enabled own caster with native available/
researched technology and sufficient energy, preserving active spell orders.
Raszagal/Danimoth variants are eligible. Effects use original action/order logic;
no direct damage/teleport shortcuts. Creation validates type/coordinates and is
only the explicit original script grant, not an ordinary free-production path.
No additional persistent AI allocation/state was introduced.

Ghost hero abilities are now exposed for Sarah Kerrigan, Samir Duran, Infested
Duran and Alexei Stukov. Previously perform could accept their native actions,
but options omitted them. Existing native technology/energy validation remains.
The latest released UI build edc95d348573a5a9 is also save-compatible subject to
all CRC/journal/exact reconstructed-state checks; arbitrary builds stay rejected.

Opcode format/behavior references (independent adapter, no implementation copied):
[PyMS format definitions](https://github.com/poiuyqwert/PyMS/blob/master/PyMS/FileFormats/AIBIN.py),
[researcher's AI command guide](https://pr0nogo.wikidot.com/rs-ai).

## Real checks

- ASan campaign_ai_cinematics: PASS. Own hero/ordinary Dark Templar actually move;
  other owners/types remain; explicit creation is one-shot, positioned/owned,
  does not charge resources and rejects invalid type. Web and Recall create
  their real native effects and spend energy, choosing a ready caster over the
  nearer exhausted one. Fixture permits the researched spell and settles initial
  Arbiter initialization before issuing the command; mission tech data untouched.
- ASan campaign_ai_later on the intended arena.chk fixture: PASS. Paid Marines,
  regional/allied defense, actual missile launches/damage, timers, paid replacement,
  killable threads and explicit scenario missiles. An initial run against the
  wrong protoss01 region geometry failed the defense expectation; rerunning with
  its intended arena fixture passes. No production defense change was made.
- ASan save_slots on final engine: PASS, five exact state/camera replays, timestamp
  labels, own-slot backup and compatible legacy replay (including latest edc UI).
- ASan campaign_hero_options: PASS on an isolated synthetic own-Kerrigan fixture;
  enabled Cloak appears and native activation is accepted. This fixture changes
  ownership only for UI testing and is never a substituted mission map.
- Native ARMv6K/VFPv2/no Thumb-2 or NEON build: PASS. No emulator or physical-console
  run of this backend checkpoint yet. Original36MiB/8MiB budgets unchanged.

Broader audit over64 owned normalized records: at6000 ticks61 have no runtime
rejection before tick limit or the map's original end condition, formerly58.
At36000 ticks60 do. These audits used the same four-op backend before the final
UI-only hero aliases/save-compatibility addition. Neither audit is a victory test.
Remaining runtime rejections: opcode111 allies_watch/resource-area expansion in
BW Zerg6; at longer duration opcode101 junkyard_dog in BW Terran8. Two unused cut
SC Terran7/10 records still reject their format. No map is enabled just for
passing the audit. The catalogue remains the15 previously completed missions.

## Next work

Unmodified Terran5 Revolution pilot rescues Kerrigan and preserves both heroes,
but has NOT won or acquired a player-owned CC. Earlier direct army attack lost
Raynor at2164; preserving Raynor and cloaking/moving Kerrigan reaches2786,500 near
Antigan CC. At24000 ticks original objectives have not advanced to the pilot's
expected CC ownership. Inspect original switches/Force/trigger progression next,
not victory shortcuts. Kerrigan Bring location1 bounds2624,384..2816,576, center
2720,480; other original Bring checks location3 center656,1792 and25 center1584,656.
The location1 trigger sets switch4. Pilot currently waits for ownCC and may have
the wrong progression assumption. It remains private experimental code, not a test.

Private evidence and pilot: `nintendo3ds/local-data/campaign-next-validation/`.
Final host object `/tmp/sc3ds-cinematic-final-engine.o`. No source edits while
native build is running. Before catalogue expansion build meaningful paid/native
public-control victory, defeat and completed-save regression, then export mission
maps/audio/presentation/graphics closure and check original-model memory/runtime.

Private INTERNAL overlay `campaign-backend-checkpoint-2026-10-28.zip`,5385246 bytes,
SHA256 `109378e2927fc2509b710873944232b41d6f7f8753aec240f848bce65d4242c1`.
ZIP CRC/112resource hashes/exact executable verified. Not a new all-missions
release; last emulator-tested UI package remains campaign-ui-slots-animations-
update-2026-10-28.zip. No SD writes or emulator state changes this turn.

Two one-shot current-thread continuations created at user request: Oct29 02:25
and07:30 Europe/Vilnius. Real current reset Oct29 02:20:23. First continuation must
check real account resetsAt and adjust the existing second schedule if necessary,
then delete its own completed one-shot automation. Second is the final scheduled
continuation in this request; do not create further ones automatically.

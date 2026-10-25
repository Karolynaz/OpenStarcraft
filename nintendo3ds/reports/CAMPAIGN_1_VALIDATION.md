# Wasteland campaign milestone — 2026-10-25

Source: src-88f350761cef9368
3DSX SHA256: 6333672f2be8bf887855c7c148bb786bc0d46b168bbe38b4caf15008f22797df

## Scope

First original Terran mission only. Original -> Terran Episode I opens a
four-page text briefing, then the original Wasteland map. The initial colony
has five SCVs/five Marines, an existing southern Command Center/Depot and an
unrescued Raynor. Original enemy Zerglings retain guard/combat behavior.
This map contains no campaign build-AI scripts; future maps need additional
AI script support. The custom skirmish controller is disabled in campaign.

Private extractor reads the owned disc archive's nested scenario.chk, normalizes
human slot1 to slot0 (units, player tables, forces, masks and explicit trigger
references), and writes private CHK/briefing files. No dialogue/assets are
compiled into or published with source. The adapter is scoped to this audited
mission; unsupported campaign trigger conditions/actions reject loading.

Native UMS loading preserves placed units, restrictions and initial map state;
restores SIDE races after the generic loader's open-slot random-race conversion.
Passive rescue units now initialize without aborting; campaign proximity rescue
transfers Raynor to the human player. Trigger presentation supplies objectives,
subtitles, camera requests and transmission waits. Original mouse/CTRL tutorial
tip is replaced with 3DS controls. Other narrative text comes from private STR.

Core fixes exposed by original triggers:
- Correct FORC zero-based disk IDs to one-based native force IDs.
- Preserve trigger action index across Wait rather than restarting every poll.
- Elapsed Time compares seconds; add Accumulate resource condition.
- Count specific units for the requested player, including All Players.
- Move Location centers its destination on the found unit.

Save/load reconstructs the campaign too, including trigger waits and subtitle/
objective/camera state. Victory writes campaign/progress.cfg; completed label
survives app restart, and Campaign menu returns to title rather than Custom.

## Tests

ASan campaign integration PASS:
- Original 5SCV/5Marine/0rescued start, fog ON, supply10/18, resource icons.
- Initial resource trigger40 and Find Raynor objective.
- Move armed escort and SCVs along the map; rescue Raynor; objectives update.
- Save/load at the rescued stage and 120 frame-by-frame digest comparisons.
- Mine with SCVs, pay for a Barracks, wait for construction, train Marines at
  normal costs/durations. Original victory trigger fires at frame7773 with
  10 Marines and Raynor alive. Fog was disabled through the normal API for the
  automated order-selection helper; no grants or instant builds used.
- Factory construction rejected by mission restrictions.
- Separate CHK fixtures remove Raynor or human units to exercise original
  hero-death/total-loss defeat triggers. These fixtures are not shipped.
- Double player normalization restores identical bytes.
- Save winning game for cross-architecture replay verification.

Frontend ASan PASS: unavailable data error, briefing pages/back/begin, campaign
launch setup, previous custom/version/AI/menu navigation checks.
Savegame ASan PASS: paid queue/cancel, re-save/backup, corruption/build guards,
mixed AI 20k replay and720-frame continuation, stress wall-clock replay.
Custom match and all4observer tests PASS after trigger/force fixes.
Fog-memory regression PASS. Native ARMv6K/VFPv2 architecture verification PASS.

Original-model CPU100 Azahar:
- Original Terran selection -> briefing -> start, correct original road/units,
  10/18 supply, resource icons, objectives and current mission counters.
- Native Save Game and Load Game restore a campaign and resume.
- A host-played, journaled frame7773 winning save reconstructs on ARM and passes
  digest validation. Victory/progress file confirmed. Final build repeats this
  load and Campaign menu returns to the Original title.
- Same-core gameplay iteration src-96fc32d339837666 captured max heap30,730,832
  bytes, linear8,084,480; no missing sprites/music underruns/audio IO errors.
  Short functional checks, not a physical/full-mission performance claim.
- Emulator model and profile restored. No physical SD update/publication.

## Remaining

Original mission speech audio, animated briefing/portrait playback, movies and
exact cinematic Pause Units presentation are deferred. Subtitles/waits and
scripted camera actions are present; units are not globally frozen for dialogue.
Rescue proximity is an initial96px controller. Later missions, campaign AI
bytecode, additional trigger opcodes and campaign continuation remain work.
Only the first mission's completion is recorded. Purchased game assets stay
private; update/full archives exclude runtime saves and completion files.

Private evidence: local-data/campaign-2026-10-25/ (binary/ELF, logs, screenshots,
native/host saves, progress). Private release: campaign-1-{update-,}2026-10-25.zip.

# Six playable campaign openings — 2026-10-26

Final native source: **src-aece41334bc62432**. ARMv6K/VFPv2 build and architecture
verification PASS. Original 3DS limits remain 36MiB heap, 8MiB linear and 24TPS.
All purchased maps, graphics, dialogue and data remain private/ignored.

## Verified original victories

These ASan host playthroughs issue public player commands on the original maps,
unit stats and triggers. Economy missions use paid native mining, construction,
production and research. Fog is disabled for repeatable test navigation; these
are automated players, not six complete manual console playthroughs.

| Version | Race | Mission | Original victory frame |
| --- | --- | --- | ---: |
| Original | Terran | Wasteland | 7649 |
| Original | Zerg | Among the Ruins | 22344 |
| Original | Protoss | First Strike | 21393 |
| Brood War | Protoss | Escape from Aiur | 10672 |
| Brood War | Terran | First Strike | 37012 |
| Brood War | Zerg | Vile Disruption | 13725 |

BW Protoss escorts Zeratul and rescues reinforcements. BW Terran rescues Duran,
mines gas, trains tanks, researches siege and destroys the original objective.
BW Zerg rescues all seven hives, recruits the original scripted units, crosses
the preplaced Nydus network and completes the final scripted combat.

All six empty-human **test fixtures** execute original defeat branches at
frames2/2/2/50/2/2. Wasteland also tests the original hero-death loss condition.
All six menu/race/version routes, objective startup, save/replay digest equality,
48-frame deterministic continuation and idle runs through frame17248 PASS.
Fixture maps are never included in the release.

## Campaign blockers corrected

- Native Exactly comparison uses CHK value10; resource Subtract uses9.
- Resource-depot rescue transfers the rescuable colony, including the pylons
  needed to use the rescued Protoss base. Individual actors still rescue alone.
- Initial UNIT valid flags preserve invincibility, burrow and cloak. Contract
  confirms seven protected BW Zerg hives and88 pre-burrowed enemies.
- MRGN ground and air exclusion masks were swapped. Ground-only Bring conditions
  now trigger hive rescue and scripted reinforcements correctly.
- Initial addon/Nydus serial links were ignored. Contract confirms six Terran
  addon pairs, paid player tank training, and eight reciprocal Nydus entrances.
  The final BW Zerg region cannot be reached by an ordinary ground path.
- Selected opening heroes now show Jim Raynor, Fenix, Zeratul, Samir Duran,
  Hunter Killer or Dark Templar instead of the generic Unit label.

Primary format/behavior references:
[UNIT serial links](https://docs.scmjs.dev/chk/unit/),
[MRGN exclusion bits](https://docs.scmjs.dev/chk/mrgn/),
[rescuable order behavior](https://github.com/BoomerangAide/GPTP/blob/master/GPTP/hooks/orders/rescuable_order.cpp).
The fixes retain the existing OpenBW simulation; reference implementations were
not imported as replacement engines.

## Original script AI and audio

The bounded adapter interprets audited opening AIScripts Zer1, Pro1, TB1A,
PB1A/PB1B, EnBk and Suic. Requests use native paid mining, placement, production,
research and combat. A sparse-town original Zer1 test constructs a Barracks and
three Marines, spending600 minerals while mining continues. Trigger contracts
also cover switches, custom score, properties, ownership, relocation, HP,
deaths versus removal, alliances and ground/air location masks.

Each opening has a private bank, avoiding collisions between map-local speech
IDs. **All97 original dialogue WAVs** (8/15/9/28/14/23 per mission, including39
briefing lines and58 in-mission lines) are byte-exact through the bounded mixer,
with IO0. Placed/created hero selection and acknowledgement sounds are included.
Long speech streams through two reused128KiB halves; the long-voice allocation
is256KiB and SFX cache4MiB. Bank files are25.5–30.8MB on disk, not resident RAM.
Fragmented16KiB-cache fallback, cancel/unload, read-failure recovery and long
stream tests PASS.

Briefing pages use actual renderer font widths. Long paragraphs are split into
continuation pages without cutting text or restarting the same spoken line.
Next/Back changes pages; changing dialogue lines starts their original voices.
Briefing audio is destroyed before allocating the mission world and its audio.
Frontend ASan checks cover voice IDs, text preservation/pagination, old progress
compatibility, settings, map navigation and menu transitions.

## Original-model emulator evidence

Emulator set to original model and100%CPU. Native instruction/graphics/audio
startup was reviewed for all six openings during the work. Largest128x96
opening loaded with heap31853576bytes and linear free123904bytes. Final named
hero build also restores the BW Zerg checkpoint. No physical performance claim.

At src-f31e3d3cbb6b4374 (same simulation; before the final hero-name labels):

- BW Terran briefing played4 voices, missing0, IO0, then loaded its world.
  Briefing heap≈4.5MB was released before world allocation.
- Full BW Zerg save restored at frame11233. A/context sent the selected Hunter
  Killer through Nydus. Its native save replayed in ASan with digest equality:
  selected type53 at1352,3664, beyond the impassable cliff, frame15023.
  Repeated on the final src-aece41334bc62432 build: same successful native
  transfer and cross-platform replay digest, frame13374; Hunter Killer label shown.
- BW Terran and BW Zerg sampled median59.83FPS/23.93TPS; missing sprites0,
  audio IO0 and DSP drops0. Largest observed gameplay heap31821432bytes.
- Pause dims both screens and shows the centered menu on the lower screen.

Private evidence, logs, screenshots, original preference/profile/save backups
and machine-readable receipts are under local-data/campaign-six-validation.
The emulator is stopped and original preferences, profile and both save slots
are restored before delivery. Physical SD is unchanged.

## Test packages and limits

Private full package: local-data/releases/campaign-six-tested-2026-10-26.zip.
Private update: local-data/releases/campaign-six-tested-update-2026-10-26.zip.
Extract the 3ds folder to SD root and merge. The update requires the previously
installed base MPQs/graphics/race music. Packages contain no player profile,
progress or save slots. ZIP CRCs, staged file hashes and executable identity
are verified; exact receipts are in the private validation directory.

Six openings are enabled. Later missions/progression are not included yet;
new missions need their original trigger/opcode audit. This is an opening-script
adapter, not complete StarCraft computer AI. Movies and animated briefing
portraits remain unimplemented. Saves require an exact matching source build.

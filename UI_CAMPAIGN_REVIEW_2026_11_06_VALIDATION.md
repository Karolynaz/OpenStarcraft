# UI and campaign review — 2026-11-06

Final source identity: `src-d68939a2284cf074`.

## Shipped behavior

- Story screens stream the campaign race music without loading an effects bank or voice buffers.
- Briefing subtitles wait for DSP playback and audible PCM after initial silence. Whole utterances keep their existing gradual scrolling.
- Start slides out the lower panels. Native-resolution briefing artwork and static `Loading` remain visible through blocking map initialization; upper objectives remain on the decorated race panel.
- Selected unit portraits render beneath MENU. Buildings use their race advisor. Portraits ON/OFF persists in Options. Fixed the ROMFS portrait path separator. Clock size increased; unit-name heights retained as requested.
- Owned production buildings and larvae expose their catalogue immediately, with the existing nine shortcut slots, touch hitboxes and D-pad focus. A remains a world action until catalogue navigation starts. Select and L/R+D-pad groups remain available.
- Visible enemy/neutral units can be inspected. Their commands, production, movement and queue cancellation remain blocked. With an own unit selected A still gives its contextual order; B clears selection, then A inspects an enemy.
- Pause/save/load text is centered, panel padding tightened, Options uses normal-height rows, development settings moved to Dev Menu, in-game Exit application removed. Pause objectives use a matching blue framed upper panel.
- BW Zerg1's 88 authored pre-burrowed ambushers get native cloak/detection initialization before the first rendered frame. This compatibility step is deliberately scoped to the audited BW Zerg1 scenario. Broad changes to other missions' burrow/CreateUnit semantics were tested and rejected after regressions; those changes are not shipped.
- BW Zerg2 scripted Marines already on AttackMove engage the nearby generators. The camera follows attacked generators and the authored SCV delivery Dropship. Its four original owner7 Power Generators start with 150 HP at the user's request. The Psi Disrupter retains 2000 HP; original map/AI/trigger files remain unchanged.
- Six revised victory/defeat upper backgrounds are installed. Full opaque800×960 sources and400×240 crops are local under `update image screens/03 laimejus pralaimejus/generated-2026-11-06/`. Original lower artwork remains on the lower screen. Alternate wounded Protoss defeat is a preview only. Built-in image generation was used; prompts and source manifest are in that folder's README/manifest.
- Current Terran HUD PNGs exported to `update image screens/game ui/terran-ui-background.png` and `terran-ui-background-2000x1500.png`.

## Actual verification

31/31 enabled campaign public-control victory drivers passed with AddressSanitizer, including their saved-state/replay checks. Tests use the owned original maps, AI and triggers, paid production/resources, and fog-off navigation. Loading a mission alone was not counted as a victory. No mission was newly enabled in this change.

Passed mission keys:

`expzerg03`, `expzerg02`, `zerg08`, `zerg07`, `zerg06`, `zerg05`, `terran08`, `terran06`, `expprotoss02`, `terran05`, `terran01`, `terran02`, `terran03`, `terran04`, `zerg01`, `zerg02`, `zerg03`, `zerg04`, `protoss01`, `protoss02`, `protoss03`, `protoss04`, `expprotoss01`, `expterran01`, `expzerg01`, `expterran02`, `expterran03`, `expprotoss03`, `expprotoss04`, `expterran04`, `zerg09`.

Focused checks passed:

- BW Zerg2 autonomous generators → Dropship → SCV → neutral Psi Emitter → first Drones and original funds; moving cinematic focus; exact active replay, frame1829 (previous intro frame3906).
- BW Zerg2 full paid economy/air army/native Psi Disrupter victory and completed replay: frame26266.
- BW Zerg1 seven protected Hives,88 native-undetected ambushers,eight linked Nydus entrances; complete original victory route.
- Enemy soldier/building HP and armor inspection with rejected movement/orders/production/cancellation.
- Audio leading-silence/DSP onset/reset and music-only loading; existing audio bounds, EOF transitions, buffer lifetime and PCM asset checks.
- Frontend profile persistence, pause rows, HUD/layout,4+5 command grid,31 story/briefing presentations, gradual utterance timing and all six result banks.
- Native ARMv6K/VFPv2 verification passed. No Thumb-2/NEON.

Original-model emulator was configured for36MiB heap/8MiB linear and100% CPU. Native checks covered story music initialization, voiced briefing, native Loading transition, generator camera and SCV delivery, selected SCV/building portraits, saved portrait toggle, Options/Dev Menu, centered pause/load pages and upper objectives. Final source rows reported no missing sprites, missing SFX or audio IO errors. The final SCV/Psi Emitter and pause screenshots are in the private evidence folder.

Native observed maxima: heap `32248720` bytes; linear `8264704` bytes, within the configured limits.

## Limits and evidence

These are31 enabled mission drivers, not every StarCraft/Brood War mission and not every optional branch. All missions have not been manually completed on physical hardware. Exact native X-button/touch input was not fully verified through this testbed: its X RPC emitted a Y shortcut in the current mapping, and touch RPC did not reliably deliver; host routing/hitbox/ownership tests passed. Loading video captured the retained native artwork; a screenshot RPC during blocking IO can time out to a black fallback image.

Private evidence: `nintendo3ds/local-data/ui-review-nov06/`, with final regression JSON/logs, native logs/screenshots, loading MP4/contact sheet and package verification. Intermediate failed exploratory logs are kept separately; only the final `regressions/regressions.json` is the31/31 result.

Emulator stopped; original virtual SD tree and Qt configuration restored. Physical SD untouched. No new automation or GitHub push. Owned data and generated derivatives remain private/ignored.

## Test builds

- `nintendo3ds/out/StarCraft-3DS.cia`
- `nintendo3ds/out/StarCraft-3DS.3ds`

No ZIP. Both contain identical verified NCCH content. ExeFS and RomFS superblock hashes verified. NCCH SHA256: `5c2486fbc26567f73443bbb621d61f50677d766f71887a81bfe3f44801ea48df`.

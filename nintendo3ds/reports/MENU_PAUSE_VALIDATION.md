# Menu layout and paused Game Menu — 2026-10-24

Implemented the user's accumulated title/custom/pause feedback:

- Title: Game on the left; Play Custom and Load Saved stacked on the right.
  Removed Load Replay. Matching hit rectangles and initial cursor; original
  PalNl metal mounts retain a compressed lower support. Version heading is
  Select Game, hover brightens only Original/Expansion captions.
- Custom top: padded (12,12,376,216) continuous dark panel and metal frame,
  including the map. Title, Players, description, size inside its padding;
  aspect-preserving 176-pixel maximum preview. Bottom dark backing spans the
  entire panel, no Create settings caption. Footer supports retained.
- START/SELECT/HUD menu opens a paused bottom overlay, dimming both screens.
  Circle Pad controls its cursor; D-pad/A and touch share button rectangles.
  Main entries: Save Game, Load Game, End Game, Options, Return to Game (B).
  Options retains scenarios, camera, CSV, reload, fog and application exit.
  Nested B navigation keeps pause; root B resumes. End Game confirms before
  returning to custom setup. Save/Load explicitly say unsupported; no save
  serialization was added. Title Load Saved uses the same honest limitation.
- World commands, minimap dragging and automatic camera are gated during the
  overlay. Scheduler debt resets on open/close; UI stays responsive. No menu
  atlas is retained during gameplay; existing 256 KiB HUD alone is resident.

## Verification

`make -C nintendo3ds test-frontend HOST_CXX=clang++`: ASan PASS. Covers profile,
version, three title targets, transition input gating, map browser/settings,
AI slot cap, pause-page geometry and nested back behavior. Make dependency
includes the new pause header. Native ARMv6K/VFPv2 check PASS; no Thumb-2/NEON.
`git diff --check`: PASS.

Azahar original-model / CPU100 checks: title layout and supports, version
caption-only hover, padded custom preview, START pause, save/load notices,
Options/scenarios/B nesting, SELECT pause, X/Y gating, Circle Pad camera gate,
resume, End Game cancel/confirm returning to custom, return to title. Comparing
paused top screenshots before/after X/Y and Circle Pad gives identical PNG
bytes. Physical touch and physical console not tested in this stage.

Main behavior run src-de35ed27ed02ee76: 343 CSV samples, 331 fully paused;
frames held at103 and246, paused backlog0. Resumed steady23.93TPS and median
59.83FPS. Only subsequent source change was the frontend-test Make dependency.
Final build src-795c51f4f52748fa separately booted/loaded/resumed/paused/exited:
237 samples,210 fully paused, frames held360 and614; paused backlog0, missing
sprites0, audio IO errors0. Results apply to this small opening, not stress
battle performance. Existing stress dips documented in AI_MAP_AUDIO_VALIDATION
remain. Model preferences and Player1 profile restored after emulator stop.

## Private artifacts

`local-data/menu-pause-stage-2026-10-24/`: matching ELF/3DSX/source ID, screenshots,
raw CSV, summaries, config/profile backups and update-package receipt.
Final executable SHA256:
`6b68f1f70b76d120a867b1ffdcc1b99b8da841fc1b7a88e91410775318907610`.
UI menu remains4pages/2,098,076bytes; HUD/319 animation frames unchanged.
Menu bank SHA256 `2cd742bbcc305060c67550509d90f7d391925ceab9745f6ba23a0d9048789ba9`.

Private packages under `local-data/releases/`:

- menu-pause-update-2026-10-24.zip:96files/34,846,886bytes; executable, matching
  UI, all87 additional maps, shared audio bank, catalogue and README. Requires
  the user's existing purchased MPQs, graphics and race music on SD.
  SHA256 `a159b318a2130371c0f83dcfa3f774719812859c10c678ad9aacc6b8537d7ddb`.
- menu-pause-2026-10-24.zip:120files/377,949,557bytes, complete private SD tree.
  SHA256 `684e69703e836e133807d8831d57173331760a88421688d7b2ccf5cdbb3e3963`.

Both ZIPs tested for corruption and staged hashes; full packaging verifies
owned MPQs, map catalogue, audio and UI manifests. No physical SD write, no
publication and no scheduled continuation.

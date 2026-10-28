# Campaign UI, briefing animation and five save slots — 2026-10-28

Native source: `src-edc95d348573a5a9`.

## Result

Story screens no longer show chapter arrows or a scrollbar. Touch drag, Circle
Pad and D-pad up/down scroll; left/right focus Cancel/Briefing without changing
mission. A activates the focused choice; B cancels.

Original lower briefing layout and its empty frames remain. Neither speech nor
objectives are repeated below. Upper replay text is centered in both axes;
original race-specific portrait-window decorations frame the upper dialog.
The loading objectives heading/body have additional top padding.
Original idle/talking portraits and room movies are streamed as predecoded
8 fps clips; original Start hover animation uses the native movie placement.
The face is never mirrored. Actual voice activity selects talking vs idle.

During gameplay the unselected-unit panel no longer repeats objectives. Pausing
a campaign presents its current objectives on a black upper screen, with the
game menu below.

Save and Load now offer five independent slots, including title-menu Load.
Rows are left aligned and show slot, campaign/mission, time and month/day when
the measured width permits. Empty and unavailable loads cannot be activated.
Each slot retains its own .bak recovery. The checksummed 256-byte save header
stores an RTC-derived Unix timestamp in previously reserved words; older saves
use a valid filesystem date when available. Unknown dates are omitted.
Only the two preceding released presentation builds are additionally accepted
(`src-2dc0b1b024abc416`, `src-d8212ce708a062eb`); arbitrary foreign builds remain
unavailable. All existing CRC, journal and exact reconstructed-state checks stay.

## Actual verification

- ASan frontend tests: PASS, including five-row geometry, back actions and empty
  load gating.
- All 15 private presentation state tests: PASS (story/scroll/focus, speech paging,
  cast slots, replay, start/cancel and objective separation).
- ASan real-engine five-slot test: PASS. Five distinct Zerg1 states restore their
  exact state digests/cameras; labels include a fixed timestamp; own-slot backup,
  compatible legacy build replay and foreign-build metadata checks pass.
- ASan existing save regression: PASS. Queue/cancellation/selection/groups/fog/
  camera, exact continuation, resave/backup/corruption; mixed-AI teams/tech/scout/
  transport/RNG 20,000-frame fixture plus 720-frame exact continuation; wall-clock
  journal round-trip.
- Asset preparation: 128 animated sequences, 1,484 frames, every sequence has
  multiple distinct frames. 112 manifest resources hash-verified.
- Native build: PASS, ARMv6K/VFPv2, no Thumb-2 or NEON.
- Original-model emulator: title five-slot list, story focus, original Protoss1
  room with empty objective frame, ornate gold upper frame and centered replay
  prompt visually checked. Separate snapshots changed 1,742 portrait pixels and
  2,543 Start-crystal pixels. Campaign pause shows black upper objectives.
- Native save to Slot4 and in-game Load Slot4: PASS, resumed in the live world.
  Row displayed `Slot 4 • Protoss mission 1 • 22:11 Oct 28`; saved header frame886
  and RTC timestamp were inspected. Final load log reaches core/graphics/audio
  ready using `saves/restored-map.chk`.

Original-model limits remain 36 MiB heap / 8 MiB linear. During native campaign
startup audio-ready heap usage was 31,090,928 bytes, linear free 25,600 bytes.
The retained photo/frame total 96 KiB is released before terrain/audio allocation
and restored afterward. Animation reuses the existing seven double-buffered
surfaces, rather than adding a runtime video decoder. Each room uses two atlas
pages, released before gameplay.

Emulator stopped; its configuration, keyboard bindings, profile, progress and all
previous save files were restored byte-for-byte. Physical SD was not changed.
Native touch dragging was not independently exercised by the emulator tool.
Full campaign victory playthroughs were not rerun for this UI/save change;
previously recorded later campaign-AI limitations remain.

## Private test package

`nintendo3ds/local-data/releases/campaign-ui-slots-animations-update-2026-10-28.zip`

5,384,456 bytes; SHA256
`e56b6965803511ec50bd8cb2f1bf270d99f2ea38d5ac7eb9ec5a64225e8a2280`.
ZIP CRC, all112 resource hashes and exact executable equality PASS. Overlay on
an existing complete private install; not standalone. No profiles/progress/saves
included. Private load evidence and package receipt are under
`nintendo3ds/local-data/campaign-presentation/oct28-*`.

Screenshots: `oct28-story.png`, `oct28-briefing.png`, `oct28-replay.png`,
`oct28-load-slots.png`, `oct28-restored.png` in this report directory.

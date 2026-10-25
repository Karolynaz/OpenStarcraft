# Shared title panel and centered labels — 2026-10-25

Implemented the user's queued UI changes after "padaryk":

- One shared title control panel with Load left, Play Custom right and Options
  centered below. Options retains the existing game-version chooser. Updated
  hit rectangles and initial cursor. Original metal pieces supply separate
  connectors at the left, right and bottom of their respective buttons.
- Play Custom labels move10pixels inward (x35). Cancel/OK text uses measured
  width AND height to center inside the interactive button rectangles.
- All paused Game Menu/submenu labels use measured centering. Removed the
  textual > selection marker so it does not shift the centered caption;
  selection remains visible through the blue border and yellow caption.
- Shared title frame and connectors are private generated UI assets. Removed
  two unused atlas entries; menu stays4pages (2MiB), HUD/animation bank unchanged.

Validation: ASan frontend test PASS, including all three updated title targets,
profile/version/settings/navigation gates; native build ARMv6K/VFPv2 check PASS;
git diff --check PASS. Original-model Azahar CPU100: title/common panel, custom
label padding and both centered footer labels, actual selected-map game load,
centered Game Menu, B resume, repeated START pause, End Game confirmation and
return to setup/title, Options -> version dialog and Load -> unsupported notice
checked. Normal exit exercised, emulator stopped, model/profile restored.
Physical touch/console not tested. This stage adds no save/load implementation.

Source src-394b0d7f82ecef48. Executable SHA256:
f007365608e39d3d083d05a2d1f12e14c3b975cfcc50c5372a383bb8ec622e75.
Menu atlas SHA256:
154287b7887040e142a8b74a8c21cbb95b50af0b97a56529280e7dd2ffcc662b.
Private screenshots, ELF/3DSX/source ID and package receipt:
local-data/menu-centering-2026-10-25/.

Private packages: local-data/releases/menu-centering-update-2026-10-25.zip
(96files, updated binary/UI plus previous additional maps/shared audio) and
menu-centering-2026-10-25.zip (120files, complete owned-data SD tree). Both
ZIPs CRC checked; update entries compared to staged hashes; full packager
checks owned-data/UI/audio/map manifests. No physical SD writes or publication.

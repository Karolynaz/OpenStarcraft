# Original resource icons and green counters — 2026-10-23

Replaced the top-right 'Minerals/Gas/Supply' text labels with one compact row:
original mineral icon + green amount, original racial gas icon + green amount,
original racial supply icon + green used/max. Art is native 14x14 pixels, kept
at 1:1. Number width is measured before positioning; groups have 10px gaps,
3px icon/number spacing and finish at x396 on the 400px top screen. The row
has an 18px translucent dark backing. Counts retain the existing 250ms update
cadence. Gameplay, menus and data packages are otherwise unchanged.

The purchased game/icons.grp has raw, uncompressed palette-index frames,
not normal GRP RLE. Frames0..6 are minerals; Zerg/Terran/Protoss gas; and
Zerg/Terran/Protoss supply. The [PyMS format tool documentation](https://github.com/poiuyqwert/PyMS/blob/master/Docs/pygrp.txt)
corroborates its uncompressed format; the actual file bytes and independent
private palette render were inspected before implementation. Palette colors
come from the native race console PCX, not generated art or command symbols.
No new loose game asset is needed on SD. Seven small decoded frames live in
the existing command-art loader and share the bounded GPU atlas under a
separate reserved image ID65382. Optional missing art does not disable the
previous command icons in a loose-data install.

Validation:
- Native ARMv6K/VFPv2 ELF, no Thumb-2/NEON PASS.
- ASan independent Python raw-GRP/PCX reference: all4116 resource pixels for
  seven frames/three race palettes match. Explicit expected per-race kind to
  frame mapping; invalid kind/buffer bounds PASS.
- Existing original command reference retained: total2,050,908 compared pixels,
  294 catalogue mappings, 75 battle viewports and1425 UI draws. Resource draws
  also retain actual14x14 geometry, native origin and scale1, zero stale GPU
  bindings, unchanged queued world texture pixels; all allocations freed PASS.
- Frontend/command-grid regression checks PASS.
- Original-model emulator CPU100/DSP HLE: visually checked Terran opening
  50/0/4-of-10 and Terran laboratory10000/10000/8-of-76. All three groups are on
  one row, green, in mineral/gas/supply order, aligned to the top-right edge;
  icons remain distinct and counts do not overlap or clip. Normal scenario
  reload and START/Exit PASS; prior emulator preferences restored.

Source src-361f99ef011a0937. Exact executable SHA256:
4c8965b888df236e7b47fa8bd1191d33269c177d5b44545d5ddfa2aed84e55df.
Matching ELF/3DSX, raw logs/config backup and package receipts are private in
local-data/resource-hud-2026-10-23. Full 28-file private package and compact
five-file update in local-data/releases/resource-hud[-update]-2026-10-23.zip;
CRCs, packaged executable and all compact-update file readbacks verified.
No physical SD write was requested or performed.

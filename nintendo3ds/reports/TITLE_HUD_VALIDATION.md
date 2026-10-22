# Startup menus, Figma HUD and mirrored-sprite fix — 2026-10-22

The executable starts in the campaign frontend, not in an automatic stress
battle. Top: the user's supplied 400x240 title image. Bottom: native campaign
backdrop, BIN-positioned lower panels, idle/on SMK art, original disk/cursor
and version logos. Game type replaces Cancel; profile Player1 is automatic.
Default Brood War, selected version and race persist in player1.cfg. Campaigns
and unavailable save/replay actions show a modal. B closes a modal/returns;
B at the campaign screen opens Exit/Cancel. Circle Pad/A and touch hit regions
share the same model. Frontend art is private, generated offline from the
purchased MPQs and supplied images; no assets are committed.

Custom Game uses the original panel positions at half size. It browses actual
SD maps folders, selects SCM/SCX/CHK files, scrolls six visible entries and
cycles race/type/subtype/speed. Play dispatches the existing mirror-race
Player vs AI scenarios 4/13/15. Map/type/subtype/speed remain UI selections
as requested. Version currently selects the campaign presentation, without
switching the simulation ruleset. Menu text uses the platform font; SMK menu
animations use four predecoded frames rather than a full Smacker player.

## HUD design

Figma file w51tTYajO6fKl2DkXjIS0M, node 27:30 was read through design context
with its screenshot before implementation. Its exported 320x240 background
was downloaded into private staging and packed unchanged into hud.bin.
The screenshot itself is not an implementation asset.

- Map: (40,45), 115x108; the same bounds handle camera touches.
- Commands: 32x32 at x166/206/246, y49/84/119. The ninth slot cycles pages.
  Primary commands remain visible; Y/L+Y and D-pad/A/B catalogues still work.
- Selection: up to twelve 32x32 icons at x15+34*column, y166/200, with HP bars.
  Touching a selected-unit icon selects that unit. First-unit icon/HP fills
  the portrait placeholder at (253,190), 44x44.
- Match clock at (69,25); Meniu at (279,163), hit region (244,161),66x21.
- Minerals/gas/supply are right-aligned at the top screen's upper-right edge.
  Observer/lab/scenario controls are retained in START/Meniu.

All background/static-art slots were checked against the design screenshot
and their nonempty packed entries/callsites; menu/HUD textures rendered on
hardware-emulated original 3DS. HUD texture is 256 KiB linear. Six frontend
pages (3 MiB) are freed before native core/world-atlas allocation. In-match
linear use is 8,084,480 of 8,388,608 bytes, leaving 304,128 bytes. Heap remains
within the original 36 MiB target.

## Sprite selection mismatch

[Citro2D C2Di_CalcQuad/DrawImage](https://github.com/devkitPro/citro2d/blob/master/source/base.c)
uses abs(width) for geometry and flips UVs for negative scale. The adapter
incorrectly added the sprite width to its already-correct native left edge.
Removed that shift. The software composition oracle had modeled negative
scale as a negative-width rectangle; it now follows the SDK's actual geometry.
This covers flipped SCVs and other mirrored native image layers, including
frozen fog images. Native hit/selection coordinates were already correct.
Unused units.dat shield values are now exposed only when has_shield is set,
so SCVs no longer display a spurious Protoss shield bar.

## Validation

- Native build and final ELF: ARMv6K/VFPv2, no Thumb-2/NEON PASS.
- ASan frontend tests: default/version and race persistence; modal blocking;
  folders/files; selected map; settings cycling; all three race dispatches;
  return/exit behavior and bounded cursor PASS.
- Catalogue grid: nine-cell bounds, decorative gaps, incomplete rows, all
  four directions and inverse navigation PASS. Relocated minimap bounds PASS.
- Existing input/scheduler/frame-pacer/core/preflight checks PASS.
- ASan sprite cache: 8,722,438 pixels, 25 viewport composites, all allocations
  freed PASS. Corrected flip geometry is included in the composition oracle.
- ASan original command icons: 2,046,792 source/reference RGBA pixels, 294
  mapped options, 75 world viewports/1200 UI draws, queued texture preservation
  and all allocations freed PASS.
- Reproducible host frontend extractor reads purchased SMK/PCX/GRP assets,
  including uncompressed archive entries and embedded CD MPQs PASS.

Original-model emulator, CPU100, existing DSP HLE: manually checked Circle
Pad/A hover and version selection, unsupported campaign/B dismissal, Custom
Game opening, persisted Original campaign after reboot, race cycling through
Protoss to Zerg, Play launching Zerg scenario15, Drone selection/Y catalogue,
START/Exit and clean shutdown. An intermediate executable also checked Terran
scenario4, SCV alignment and its eight-command build catalogue. Touch injection
in the testbed still does not deliver useful events; touch behavior is covered
by shared host-model/bounds tests and needs physical-console verification.

Final executable SHA256:
`ed1de87bcc44ede195e4518a183be91a076bfaf1ba7e46fc5d4ebce0acc9afc8`
Source: `src-61642441ae705794`. Exact ELF/3DSX, raw appended logs, original
emulator config and profiles are in ignored local-data/frontend-validation-2026-10-22.
Final original-model Zerg opening: 116 active one-second samples, first partial
retained, mean 59.837 FPS / 23.991 TPS. Peak heap32,429,232, linear8,084,480;
zero backlog, missing sprites, cache overflows, music underruns, audio IO and
CSV errors. This is a light opening test, not a full stress/performance claim
for physical hardware. Emulator preferences were restored after normal Exit.

Packages: title-hud-2026-10-22.zip (28 files) and the five-file compact update
ZIP. CRCs, exact executable and per-file readbacks verified. Copy the update
contents to the SD root, merging 3ds/openbw. No physical SD write was requested
or performed for this build.

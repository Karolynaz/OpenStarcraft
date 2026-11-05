# Three-race lower HUD — 2026-11-06

Implemented the approved generated Terran, Protoss and Zerg backgrounds and
Figma w51tTYajO6fKl2DkXjIS0M nodes 128:869 / 128:991. No game data was
published; original portraits/GRPs remain in private local-data.

- Native 36x34 command and selected-unit icons, no 0.82 / 35/36 resampling.
- Shared 4+5 command coordinates, uniform 12px key labels including A/X/Y.
- Twelve selected units in two columns. Single selection: name/kills or
  energy/work, native 64x64 wireframe, HP/shields, up to four deduplicated
  original upgrade icons/levels. Five-slot production/cancel UI retained.
- Native first-frame portraits 60x56 at (122,152), MENU (122,130), 10px
  nominal clock below, aspect-fit 96x96 minimap at (206,130).
- The human's actual race selects the skin, including save reloads. Each race
  has one 512x256 RGBA5551 page; only one 256 KiB HUD page is resident.
  Existing sprite cache handles the larger native portrait; budgets stay
  36 MiB normal and 8 MiB linear. Touch and draw geometry share definitions.

## Actual checks

Host command-grid and controls PASS. ASan hud-layout PASS: all three packed
skins, key dimensions, single-page allocation/release, twelve selection and
five production hit rectangles, menu hit and rectangular-map touch/camera.
ASan selection-info PASS: original costs, head/middle cancellation/refunds,
research/morph/upgrade cancellation and original icon availability. ASan
hud-controls PASS: mixed Stim/Siege/Nuke, groups, catalogue, 60x56 portrait
pixels and exact active replay. ARMv6K/VFPv2 build verification PASS.

Original-model emulator (CPU100, no New 3DS budget), source
src-f52e285957f5af5a: fresh-source save fixtures loaded via real title/pause
menus. Protoss Probe, group and five-queue Nexus; Terran SCV and Zerg Drone
visually checked. Screenshots and load logs are private in local-data/hud-nov06.
Audio-ready heap 30,566,424 (Protoss single), 30,601,344 (Nexus),
30,667,544 (Zerg); linear free 304,128. No extra resident race HUD textures.
The later Makefile addition only registers the passing HUD test target and
changes the source fingerprint; no UI behavior changed after these checks.

Touch RPC did not activate the title button, so actual touchscreen interaction
is NOT claimed. Host bounds/cancel semantics pass; real hardware remains
untested. Pre-existing Zerg wireframe health recoloring is still imperfect
(healthy Drone has orange/red detail); the new layout did not change it.
Emulator stopped and original entire virtual SD/config restored. No physical
SD writes. Standalone CIA/3DS packaging follows the next mission checkpoint;
no ZIP was made for this UI change.

# Terrain and sprite renderer — 2026-10-16

The original 3DS build now displays StarCraft terrain, animated sprite frames,
player colors, basic shadows and a terrain minimap using the user's game data.
The simulation and input schemes remain unchanged: Circle Pad cursor/edge
scrolling, A own-unit selection, B Move, touch only inside the minimap.

## Offline graphics pipeline

`tools/graphics_export.cpp` runs on the host using OpenBW's own MPQ/GRP reader.
It records the map's actual megatile indices, decodes VX4/VR4 terrain, reads WPE
and team PCX palettes, and collects image types from both probe modes. Every
frame of those image types is exported, including directional/movement frames.
A fresh simulation state is used per load, matching the device adapter.
AddressSanitizer exposed stale unit-finder pointers when reusing the upstream
live state during exporter development; the exporter now reconstructs it per
scenario and passes the sanitizer run. No upstream reset patch was introduced.

`tools/prepare_graphics.py` uses only Python's standard library. It quantizes
to RGBA5551, preserves transparency/team colors, deduplicates identical frames,
packs gutters into 512x512 atlas pages and writes PICA200 Morton-ordered texels.
The GPU orientation was verified against a rendered full-atlas reference and
corrected before delivery. No conversion or frame decoding occurs on-device.

For the user's 64x64 `(2)The Small Divide` map:

| Item | Result |
| --- | ---: |
| Unique terrain tiles | 504 |
| Observed image types | 26 |
| Original animation frames | 637 |
| Team/frame lookup records | 1911 |
| Deduplicated rectangles, including minimap | 1100 |
| 512x512 RGBA5551 pages | 6 |
| GPU texture data | 3145728 bytes (3 MiB) |
| graphics.bin including metadata | 3185760 bytes |

The wire format has explicit little-endian fields, dimension/count/rectangle
bounds, sorted lookup keys and a terrain signature matched against the loaded
engine state. Wrong-map/stale files and failed texture allocations report a
clear error and fall back to diagnostic rendering. Maximum atlas allocation is
8 pages / 4 MiB. Exported assets and screenshots containing game art stay in
Git-ignored local-data; no Blizzard images/archives are embedded in the binary.

## Device renderer

`source/graphics.cpp` loads pre-swizzled pages directly into textures once.
Terrain quads are culled to the viewport and grouped by atlas page. Sprite
layers are culled and sorted by engine elevation/Y, preserve image ordering,
use the actual iscript frame index, apply horizontal flips, and draw basic
alpha shadows. The minimap background is an offline 128x128 terrain preview.
Metadata and rendering arrays are bounded; no container grows per frame.
Texture-cache state is invalidated on reload, and GPU use is synchronized before
old atlas buffers are freed.

The total requested heap reservation stays **44 MiB**. Its split changes from
40 normal + 4 linear to **36 normal + 8 linear** to fit the atlas while keeping
headroom over the approximately 23.6 MiB normal heap used by the core. The
stack remains 256 KiB. No New 3DS speed/RAM mode is requested.

## Validation

Passed:

- ARMv6K/VFPv2 final-ELF architecture check, no Thumb-2/NEON.
- Normal host core/scheduler/controller/preflight tests.
- All four private-data host scenarios, 2400 ticks each; select/move regression.
- `make -C nintendo3ds test-graphics HOST_CXX=clang++`: end-to-end exporter,
  packer and comparison of **every packed terrain/sprite/team/alpha texel**
  against OpenBW's exported source, plus lookup/signature/bounds checks.
- Host exporter under AddressSanitizer after the fresh-state correction.
- Actual .3dsx launched in Azahar original-3DS mode at the normal clock setting.
  Inspected real terrain, red/blue animation frames, buildings/resources,
  shadow transparency, camera scrolling and repeated map reloads.
- A selects the marine; Circle Pad + B moves its animated sprite to the cursor.
- Native window was raised for reliable touchscreen injection: touching the
  minimap moved the camera from (0,8) to approximately (833,798) while leaving
  cursor=(253,120); touching the bottom command-text area changed neither.
  These were emulated touchscreen events, not a game mouse backend.

`renderer-summary.json` aggregates all logged samples. `renderer-samples.csv`
keeps the last 60 samples per logged scenario; the full log is privately saved
at `local-data/renderer-captures/benchmark-full.csv`.

| Scenario | Samples | Mean FPS | Mean TPS | Mean tick us | Max tick us | Max tick debt | Missing sprite records |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Two marines, including movement and scrolling | 6431 | 59.83 | 24.00 | 14.4 | 391 | 0 | 0 |
| Idle 1v1, 34 objects including resources | 104 | 59.83 | 24.00 | 177.6 | 823 | 0 | 0 |

The two-unit capture includes an extended run while tool work was pending;
frames 361..155116 were logged, with stable heap and no observed backlog.
Normal allocated heap peaked at 24736864 bytes (23.59 MiB); used linear memory
was 5307392 bytes (5.06 MiB). These exclude code/stack/framebuffers and do not
measure loading peaks. Emulator timing is not a real ARM11 speed measurement.
The empty-map renderer was also visually checked; it has real terrain and zero
units. No physical original Nintendo 3DS was available.

Final .3dsx: 839572 bytes; SHA-256
`b5e11d33184f40ab020bedf4e3a659e4c7f0d81bb0cecb44887073f2e3f90418`.
Graphics.bin SHA-256:
`95ebc903b537fd297794107f00b582c0ea6b21e47ae7ac3027f0e732837efca7`.
The private screenshot `local-data/renderer-captures/melee-final.png` shows
the delivered build displaying the starting base with zero missing records.

## Use and remaining scope

The private SD-ready directory is `nintendo3ds/local-data/3ds/openbw/`. Copy the
whole directory to `/3ds/openbw/` on SD, including **graphics.bin and the updated
openbw-3ds.3dsx**. Launch and press **L**; the default is now Two marines, and
loading feedback appears before reading the archives. R then L loads idle 1v1.

This is still a prototype: no opponent AI, economy/building UI, combat command
UI, fog-of-war rendering, creep edges, palette lighting/cloaking/warp effects,
health bars or complete roster. The atlas profile is the current Terran probe,
not every StarCraft image. Unsupported image records increment the HUD counter.
The static terrain cache/minimap is prepared for one map; regenerate it for a
new terrain layout. No networking or simulation-timing changes were made.

SDK references used for texture integration:
[Citro2D image drawing](https://github.com/devkitPro/citro2d/blob/master/source/base.c),
[Citro3D textures](https://github.com/devkitPro/citro3d/blob/master/source/texture.c),
[tex3ds Morton layout](https://github.com/devkitPro/tex3ds/blob/master/source/swizzle.cpp).

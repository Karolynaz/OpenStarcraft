# OpenBW on the original Nintendo 3DS

Status: terrain/sprites, stick/A/B commands and a limited Terran economy/
production/combat loop implemented;
**still an experimental prototype with no opponent AI**. No original-console measurements yet.
Updated: 2026-10-16. Baseline: `heiner/openbw` commit
`469d2037e2fd03085dac4729912f4188bc2a778b`.

## Completed milestone

- Cloned the requested repository into this workspace, preserving history.
- Created local branch `3ds-port`. The connected GitHub identity was available,
  but the connector exposes no fork endpoint and `gh` is absent, so used the
  explicitly allowed local-clone fallback. No GitHub repository was modified.
- Renamed the source remote `upstream` and disabled its push URL. No push made.
- Compiled the complete upstream headless simulation probe to an ARM11 object.
- Built and linked a 3DS application containing the real OpenBW simulation,
  map loader and action system, without SDL, ASIO, BWAPI or networking.
- Produced `nintendo3ds/out/openbw-3ds.3dsx` and launched it in Emuautotest/Azahar.
  On the 3DS binary, the MPQ DCL decoder / unaligned endian / fixed-point
  self-test reports PASS. Both screens render; diagnostic rendering runs at
  approximately 60 FPS in this emulator with no game loaded.
- Added host tests for exact 24 Hz scheduling, retained tick debt, pause reset,
  stick input, minimap-only touch, and invalid-map / missing-asset recovery.
- Added three load modes: empty map, two marines, and two idle melee players.
  All three now load the user-provided MPQs in host and emulator tests. The
  desktop movement test also verifies the selected marine reaches (256,128).
- User supplied their purchased game data in the ignored `data files/` folder.
  Private test data was staged locally and into the emulator SD directory.
  Do not fetch game archives from
  third-party download sites or run the inherited `web/scdat.py fetch` command.

## Renderer milestone

The upper screen now draws the actual map terrain and simulation-driven sprite
frames, including player colors, horizontal flips and basic shadows. The bottom
screen has a real terrain minimap. The default scenario is **Two marines**:
press L to load it, A to select, Circle Pad to aim, B to move/interact/attack.
The enemy marine now starts at (512,128), on the same accessible test area. R cycles the next
load mode (idle melee, empty, two marines). A loading message is shown before
blocking SD reads. See `nintendo3ds/reports/RENDERER_VALIDATION.md`.

Graphics are prepared offline from the user's data and kept out of Git:

```sh
make -C nintendo3ds test-graphics HOST_CXX=clang++
make -C nintendo3ds DEVKITPRO=/Users/karolynaz/devkitpro -j2
cp nintendo3ds/out/openbw-3ds.3dsx nintendo3ds/local-data/3ds/openbw/
```

Run the staging command below before `test-graphics` on a fresh checkout. The
resulting private `/3ds/openbw/` folder must contain **graphics.bin as well as
the updated executable**. Copying only the .3dsx will retain the diagnostic
fallback if graphics.bin is absent. The package is tied to this map's terrain;
prepare it again after changing maps. To use different data/map locations:

```sh
make -C nintendo3ds test-graphics HOST_CXX=clang++ \
  ASSET_ROOT="/path/to/classic/data" MAP="/path/to/map.scx"
```

## Build on this Mac

The existing SDK was found at `/Users/karolynaz/devkitpro`, outside PATH.
Compiler: devkitARM GCC 16.1.0. Existing libctru, Citro2D, Citro3D and 3dsxtool
were used without installing or modifying the SDK.

From the repository root:

```sh
export DEVKITPRO=/Users/karolynaz/devkitpro
export DEVKITARM="$DEVKITPRO/devkitARM"
make -C nintendo3ds -j2
make -C nintendo3ds test HOST_CXX=clang++
```

Output: `nintendo3ds/out/openbw-3ds.3dsx`. ELF and linker map are adjacent.
`make -C nintendo3ds clean` removes only this port's generated build/output.
Python 3.11+ is used for staging/tests; the C++ target requires GNU C++14.
The Makefile uses relative project paths so this workspace's spaces work.
For a different SDK location set `DEVKITPRO`; `DEVKITARM` can also be overridden.
A missing toolchain produces an actionable error. For a new machine use the
[official devkitPro pacman installation instructions](https://devkitpro.org/wiki/devkitPro_pacman)
and install `3ds-dev` using `sudo dkp-pacman -S 3ds-dev`.
The official [3DS examples](https://github.com/devkitPro/3ds-examples) document
SDK usage; [pacman releases](https://github.com/devkitPro/pacman/releases) provide
the macOS installer. Installation was unnecessary on this machine.

## Data and SD layout

No Blizzard data is committed or embedded in the executable. Game data should
come from the user's own installation/extracted purchase. The observed
`/Applications/StarCraft` installation has CASC `Data/` storage and maps, rather
than the three classic MPQs expected by upstream. The user subsequently provided classic MPQs in `data files/`; only Patch_rt,
BrooDat and StarDat plus a 64x64 map from their installation were staged for
testing. The large StarCraft.mpq/BroodWar.mpq CD containers are not needed for
this core milestone. The requested
`/Applications/Games` directory did not contain those MPQs either.

```text
sdmc:/3ds/openbw/
  Patch_rt.mpq
  BrooDat.mpq
  StarDat.mpq
  maps/probe.scx
  graphics.bin
  openbw-3ds.3dsx
  logs/                 # optional CSV output
```

Names above are exact. Use a trusted map 32..64 tiles in each dimension, with
at least two playable start positions for melee mode. A `.scm` map may be
copied as `probe.scx`: the engine uses the archive contents, not the extension.
Large maps and duplicate/truncated DIM sections are rejected before global
asset initialization. The loader is not hardened for arbitrary hostile maps.
The internal MPQ reader requires a header at offset zero and supports a subset
of compression methods; newer archives may require offline extraction.

To stage a private SD folder from the supplied files:

```sh
python3 nintendo3ds/tools/stage_assets.py \
  --source "data files" \
  --map "/Applications/StarCraft/Maps/(2)The Small Divide.scm"
```

This locates archive names case-insensitively, checks MPQ signatures, hashes
files, and refuses to overwrite different existing files. It prepares
`nintendo3ds/local-data/3ds/openbw/`, which is Git-ignored. Copy that directory
onto SD under `/3ds/openbw/`. Map size is checked by the application when loaded.
A desktop host probe accepts a raw `.chk` map as well:

```sh
nintendo3ds/build/host-sim nintendo3ds/local-data/3ds/openbw \
  nintendo3ds/local-data/3ds/openbw/maps/probe.scx empty 240
```

Replace `empty` with `two`, `move` (assert select-and-move behavior), or `melee`.
Host timings are not 3DS benchmarks. The optional private-data suite runs all
four modes for 2400 ticks each, then asserts gathering, training/refunds,
construction, supply, Marine production and damage/death without cheats:

```sh
python3 nintendo3ds/tests/assets.py --data "data files" \
  --map "/Applications/StarCraft/Maps/(2)The Small Divide.scm"
```
An optional loose-asset loader is compiled too: when `arr/units.dat` exists,
it reads required paths directly below the supplied data root. Offline CASC
extraction is not implemented or validated; renaming CASC blobs to MPQ cannot
work. Loose filenames must match upstream's path spelling and case.

## Controls: stick and A/B, no mouse

The game has no mouse input backend. The touchscreen is used **only** inside
the bottom minimap rectangle (x=8..127, y=144..231).

| Input | Current behavior |
| --- | --- |
| Circle Pad | Move the top-screen virtual cursor; scroll camera when pushed into an edge |
| A | Select an own unit inside its collision box plus 6-pixel tolerance; cancel an active building preview |
| B | OpenBW contextual order: move, gather minerals, attack enemy, resume construction/repair, or rally; confirm building placement during preview |
| Minimap touch/drag | Center/clamp the camera; never move the virtual cursor or issue orders |
| L | Load/reload the currently chosen probe scenario from SD |
| R | Choose the scenario for the next load |
| SELECT | Pause/resume simulation |
| X / Y | Attack-move to cursor / Stop while running; CSV toggle / one simulation step while paused |
| D-pad Up | Train SCV at selected Command Center or Marine at selected Barracks |
| D-pad Right / Down | Preview Barracks (150 minerals) / Supply Depot (100); green/red footprint follows cursor, B places, A cancels |
| D-pad Left | Cancel placement, incomplete selected building, or last production queue entry |
| START | Exit to the homebrew launcher |

On load, the camera and cursor focus on the first own visible unit. Actual
terrain and supported unit/building/resource sprites replace the diagnostic
grid when graphics.bin passes validation. A/B selection/movement and minimap
camera relocation are verified in the emulator; touching outside the minimap
leaves camera/cursor unchanged. Resources, supply, selection HP and queue length
are displayed, with a health bar over the selected unit. Fog-of-war rendering
and a full command panel remain future work. Glow/cloak/warp palette-remap layers
are omitted to avoid drawing their indices as colors. The renderer reveals the map and
all sprite-list units; it does not change the simulation's vision state.

## Architecture analysis

| Component | Source | Port decision / observed result |
| --- | --- | --- |
| Simulation, movement, combat, triggers, region generation | `bwgame.h` | Header-based C++14 core; compiles for ARM11 after one MPQ fix |
| Shared types and fixed-point math | `data_types.h`, `game_types.h`, `util.h` | Keep; platform `int_fastN_t` widths differ, so desktop behavior alone cannot prove ARM correctness |
| Orders, selection and action parsing | `actions.h` | Reuse action_select/action_order for physical controls; no synthetic mouse events |
| MPQ, DAT, CHK, binary readers, PKWARE decoder | `data_loading.h` | Reuse paged stdio reader on SD; one explicit lambda return type required |
| Allocators and containers | `containers.h`, `game_types.h` | Standard allocator, vector/deque/list/map and chunked object pools; not an allocation-free engine |
| Desktop software graphics | `ui/ui.h`, `ui/native_window_drawing.h`, `ui/sdl2.cpp` | Paletted software drawing/palette expansion; replace platform/rendering layer with Citro2D |
| Desktop sound | `ui/native_sound.h`, `ui/sdl2.cpp` | SDL audio/mixer implementation excluded. ndsp init/cleanup exists; game WAV decoding/mixing/playback remains future work |
| Desktop input | `ui/native_window.h`, `ui/sdl2.cpp` | SDL events excluded. libctru HID drives a platform-independent controller |
| Browser UI/backend/bot | `web/` | Excluded; useful existing `wasm_sim_test.cpp` demonstrates headless separation |
| Networking / sync | `sync.h`, `sync_server_asio_*.h`, `deps/asio` | Excluded for initial local single-player experiments |
| Replay / BWAPI | `replay.h` and external BWAPI wrapper | Not required by this probe |

`game_player::init(loader)`, `game_load_functions::load_map_data` and
`game_player::next_frame()` are sufficient to drive simulation without the UI.
The engine uses stdio for file access, not an essential desktop OS service.
No SSE/NEON or desktop thread layer is required by the included core.
However, headless does **not** mean graphics data is unnecessary: global_init
loads image metadata, GRPs, animation/iscript and LO offsets, plus VF4/CV5 for
all eight tilesets. Animation state and collision dimensions participate in
simulation. Do not disable iscript or discard GRPs without proving equivalent
behavior.

### ARM11 compatibility fixes and build decisions

1. `data_loading.h`: the DCL decoder's get_length lambda mixed integer literals
   with expressions derived from uint_fast8_t. On devkitARM this promotes to
   unsigned int, causing inconsistent deduced return-type errors. Explicit
   `-> int` preserves decoder values and compiles on both targets. This is the
   only modification to upstream engine files.
2. Retain exceptions and RTTI. The usual SDK example's `-fno-exceptions` and
   `-fno-rtti` cannot be copied unchanged: core code uses `typeid`, and recoverable
   load errors use C++ exceptions. The UI catches load/tick failures and reports
   them. OpenBW's alternative OPENBW_NO_EXCEPTIONS abort mode is not enabled.
3. Use `-march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft`, GNU C++14,
   `-O2`, function/data sections and linker GC. No New 3DS speedup API or extra
   RAM mode. No fast-math or simulation-timing changes.
4. `-fno-strict-aliasing` accommodates upstream binary/type-punning conventions.
   Existing bounds/alignment checks remain. Unaligned little-endian access is
   part of the in-application smoke test.
5. This SDK's standard 3dsx.specs includes sync-dmb.specs, pulling an ARMv7 DMB
   helper into link attributes even though its code was discarded. The isolated
   `arm11.specs` uses the SDK's sync-cp15dmb.specs with the normal 3DSX startup
   objects/linker script. The final ELF advertises ARMv6K, Thumb-1 and VFPv2.
   `make` verifies these attributes and rejects Thumb-2/NEON. This is an ISA
   compatibility check, not a substitute for running on an original console.

## Original 3DS performance and memory plan

Treat approximately 268 MHz ARM11 and constrained application memory as the
baseline; total system RAM is not the application heap, and the second CPU core
is not assumed to double available simulation throughput. Reserve 36 MiB normal
heap, 8 MiB linear heap and 256 KiB stack explicitly. These are chosen budgets,
not the device's entire available RAM. The tested map and idle 1v1 state fit:
normal heap was about 23.6 MiB, plus about 5.06 MiB linear usage including
3 MiB of atlases in the
emulator. This does not establish load-time high-water or real-console RAM
usage. The final ELF's resident sections
are approximately 0.81 MiB before stacks/heaps/framebuffers; the debug ELF's
15 MB on-disk size includes debug information.

The first emulator build reported `sizeof(state)=38740`, `sizeof(unit_t)=716`
on the target versus 47720/976 on this ARM64 host. Actual dynamic allocations
are additional. The application displays allocated normal heap and used linear
heap separately; these are **not** total process memory or peak memory. CSV
samples once per second can miss short loading peaks.

Likely bottlenecks identified from source; initial game-data measurements
are now in `nintendo3ds/reports/ASSET_VALIDATION.md`:

- Global GRP/LO/iscript decoding and all-tileset VF4/CV5 loading may consume the
  budget even for an empty map. Asset preload and loading time must be measured
  before committing a texture atlas or changing gameplay.
- `regions_create()` allocates a fixed 1 MiB walkability scratch buffer even for
  small maps. Region storage reserves 5000 entries; small maps do not eliminate
  every upstream allocation. Measure before safely specializing these paths.
- Long/short pathfinding, collision checks, unit-finder updates and trigger work
  can spike when units move together. Measure idle, movement and combat apart.
- The unit pool supports 1700 units and grows in chunks of 17; sprite/image/order
  pools also grow in chunks. Low live-unit counts do not directly equal total
  memory use. No dangerous upstream pool-size changes have been made.
- 64-bit arithmetic, division, virtual calls, containers and scattered pointers
  warrant ARM profiling. Avoid broad algorithm changes before a baseline.
- CPU palette conversion/full-frame texture uploads would spend memory bandwidth;
  the eventual terrain/sprite renderer should use preconverted atlas pages,
  bounded caches, view culling and batching. Runtime conversion is not implemented; eight pre-swizzled atlas pages are
  now loaded once on SD initialization, with bounded metadata and no frame
  allocations in the renderer.

The scheduler adds exactly 24 tick credits per second using integer arithmetic.
Each simulation step still invokes one unchanged next_frame. At most two ticks
are processed before rendering, but remaining debt is kept and displayed.
An overloaded device therefore visibly falls behind rather than silently
changing gameplay timing. Pause and suspend/resume explicitly clear wall-time
debt. The target tick budget is 41.667 ms; rendering aims for at least 30 FPS.
Mean/max tick wall time, TPS, FPS, active-unit count and heap figures are shown;
this is elapsed tick time, not a hardware CPU-utilization counter. CSV and the
HUD have overhead and must be compared enabled/disabled during profiling.

## Test evidence and remaining work

See `nintendo3ds/reports/GAMEPLAY_VALIDATION.md` for current command results,
`RENDERER_VALIDATION.md` for the initial renderer measurements, and
`ASSET_VALIDATION.md` / `VALIDATION.md` for the earlier milestones.
Asset-backed initialization, 2400-tick host scenarios and emulator A/B movement
now pass; see `nintendo3ds/reports/ASSET_VALIDATION.md` and the raw CSV results.
The emulator was temporarily set to original-3DS mode at 100% CPU-clock setting.
The tested scenarios sustain approximately 24 TPS and 60 FPS there, with zero
observed tick debt. This does not prove original-hardware performance.
The two melee slots remain human controlled with no opponent AI; networking
and a pause menu are not implemented. The supported command profile is SCV,
Marine, Command Center, Supply Depot and Barracks. Economy, construction,
training and combat use unchanged OpenBW rules, costs and build times. See
`nintendo3ds/reports/GAMEPLAY_VALIDATION.md` for this milestone. Rendering
is bounded to 512 on-screen image layers and 128 minimap/selection unit entries;
these are renderer limits, not gameplay unit limits. The atlas profile covers
the current Terran probe scenarios, not the full unit roster or effects set.

Next milestone:

1. Run the current empty/two/movement/idle-melee tests on an **original 3DS**;
   record load-time memory high-water, sustained TPS and tick-time distributions.
2. Extend rendering with fog visibility and broaden the bounded graphics
   profile; investigate map-independent streaming before expanding the roster.
3. Profile active economy/combat on hardware, enforce a practical unit budget
   and then add a small opponent AI. Preserve stick/A/B and minimap-only touch.
4. New 3DS enhancements remain deferred until original hardware is evaluated.

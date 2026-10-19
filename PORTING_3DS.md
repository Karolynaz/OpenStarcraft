# OpenBW on the original Nintendo 3DS

Status: standard Terran, Protoss and Zerg production, construction, research/upgrades
and controller ability menus are integrated through native OpenBW actions.
The default Zerg prepared full-army 1v1 has a short score/time limit; all three
races have a manual laboratory. Original
music and effects play through NDSP. The earlier gas build was measured on the
user's original console at 58.47 FPS / 24.03 TPS; the larger Terran build still
needs its own hardware run. See `nintendo3ds/reports/ZERG_VALIDATION.md`, the Protoss and Terran reports.
Still a limited experimental profile. Updated: 2026-10-19. Baseline: `heiner/openbw` commit
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

## Current observer and full Terran milestone

The application automatically loads **All Terran battle** on a flat 64×64
arena prepared from the user's original terrain. Each side has 114 troops,
a prepared nuclear missile and native supply providers (200 supply). Combat
includes ground/air units, sieging, spell casts, transport and nuclear strikes.
The minimum 30-second loss threshold and maximum 180-second score/time limit
avoid waiting for distant buildings to be destroyed. START also offers a full
Terran manual lab and the earlier economy scenarios. These are small explicit
test controllers, not the Blizzard campaign AI interpreter.

The previous Small Divide terrain has no ground route between the start areas;
it remains a historical asset/renderer fixture. AI ground modes now reject
unconnected starts explicitly. Blood Bath has four start locations, but only
two player slots are active, giving the requested 1v1.

START opens scenario/camera/logging/exit settings. SELECT pauses. Circle Pad
moves the virtual pointer, A acts, B cancels; L/R are modifiers only. X selects
military units on screen, L+X all own workers, Y/L+Y primary/secondary menus.
R+D-pad saves four groups; a double tap on a direction recalls and centers its
group. Bottom menus support buttons and touch. See `nintendo3ds/CONTROLS.md` and
`nintendo3ds/reports/AI_VALIDATION.md` for current controls and test evidence.

Terrain, simulation-driven sprites/player colors, basic shadows, health bars,
resource/supply readouts and the terrain minimap remain in use. The renderer
shows the whole map; no fog is drawn. A loading message is presented before
blocking SD IO.

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
  maps/arena.chk
  graphics.bin
  graphics-arena.bin
  audio/effects.bin
  audio/terran1.pcm ... terran3.pcm
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
  --map "/Applications/StarCraft/Maps/(4)Blood Bath.scm"
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
  --map "/Applications/StarCraft/Maps/(4)Blood Bath.scm"
```
An optional loose-asset loader is compiled too: when `arr/units.dat` exists,
it reads required paths directly below the supplied data root. Offline CASC
extraction is not implemented or validated; renaming CASC blobs to MPQ cannot
work. Loose filenames must match upstream's path spelling and case.

## Controls and touch

The current mapping is documented in `nintendo3ds/CONTROLS.md`. A executes
contextual actions/selection or confirms a menu/placement; B cancels the current
UI action or clears selection. L and R alone never load/restart anything. START
provides system options, including player vs AI and the two-Marine probe.

The touchscreen controls camera movement inside the minimap rectangle
(x=8..127, y=144..231). Following the updated user request, touching an **open**
bottom menu also activates that row. All other touch locations are ignored.
No desktop mouse backend is included. In observer mode game orders are blocked
so incidental input does not change either bot's match.

Selections/bookmarks use generation-checked 32-bit IDs, with four group slots
and up to 192 members. Native orders are batched in groups of twelve. Double tap
within 350 ms centers the recalled group without moving the pointer. Dead
members are removed on recall; reload clears every group.

All twelve standard SCV buildings, six add-ons, fourteen production products
(including a nuclear missile), eleven research technologies and fifteen
upgrade types are exposed through Y/L+Y. Ability menus cover siege, cloaking,
spells, mines, transport and nuclear targeting. See `nintendo3ds/CONTROLS.md`.

## Architecture analysis

| Component | Source | Port decision / observed result |
| --- | --- | --- |
| Simulation, movement, combat, triggers, region generation | `bwgame.h` | Header-based C++14 core; compiles for ARM11 after one MPQ fix |
| Shared types and fixed-point math | `data_types.h`, `game_types.h`, `util.h` | Keep; platform `int_fastN_t` widths differ, so desktop behavior alone cannot prove ARM correctness |
| Orders, selection and action parsing | `actions.h` | Reuse action_select/action_order for physical controls; no synthetic mouse events |
| MPQ, DAT, CHK, binary readers, PKWARE decoder | `data_loading.h` | Reuse paged stdio reader on SD; one explicit lambda return type required |
| Allocators and containers | `containers.h`, `game_types.h` | Standard allocator, vector/deque/list/map and chunked object pools; not an allocation-free engine |
| Desktop software graphics | `ui/ui.h`, `ui/native_window_drawing.h`, `ui/sdl2.cpp` | Paletted software drawing/palette expansion; replace platform/rendering layer with Citro2D |
| Desktop sound | `ui/native_sound.h`, `ui/sdl2.cpp` | SDL excluded; native sound events feed eight NDSP effect voices and streamed Terran music; WAV conversion happens offline |
| Desktop input | `ui/native_window.h`, `ui/sdl2.cpp` | SDL events excluded. libctru HID drives a platform-independent controller |
| Browser UI/backend/bot | `web/` | Excluded; useful existing `wasm_sim_test.cpp` demonstrates headless separation |
| Networking / sync | `sync.h`, `sync_server_asio_*.h`, `deps/asio` | Excluded for initial local single-player experiments |
| Replay / BWAPI | `replay.h` and external BWAPI wrapper | Not required by this probe |

`game_player::init(loader)`, `game_load_functions::load_map_data` and
`state_functions::next_frame()` are sufficient to drive simulation without the UI.
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
- CPU palette conversion/full-frame texture uploads spend memory bandwidth.
  The full Terran renderer reuses resident GRPs and uploads only frame-cache
  misses into bounded GPU pages, with view culling and pinned frame lifetimes.
  Terrain/minimap pages remain prepared and swizzled offline.

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

See `nintendo3ds/reports/TERRAN_VALIDATION.md` for current functionality,
original-model emulator performance and hardware-test instructions. Earlier
AI/gameplay/renderer/asset/audio reports preserve their historical milestones.

Full construction, production, research/upgrades, ability and stress-battle
integration tests pass with the user's private files. ASan verifies audio,
CSV and sprite-cache lifetimes and MPQ ADPCM boundary handling. The new large
battle maintains simulation timing in the emulator but drops rendering FPS
heavily during dense combat; the original console must measure this profile.
The earlier smaller gas profile already has console performance evidence.

Still missing: original campaign AI/presentation completeness, save/load and
networking. Fog is intentionally deferred. Normal/linear heaps remain 36/8
MiB; GPU texture storage is bounded at 4 MiB. The prepared arena is a test
profile rather than a completed general StarCraft port.

Next work: use the original-console full-battle CSV to identify whether
pathfinding/collision, rendering or storage needs attention. New 3DS-specific
enhancements remain deferred until that original-hardware result.

### Audio performance build (2026-10-18)

Native sound events now drive eight DSP effect voices, with streamed original
Terran music on a ninth channel. See `nintendo3ds/reports/AUDIO_VALIDATION.md`.
The original three MPQs contain effects; this user's music comes from the
embedded MPQ in their GOG `data files/StarCraft.mpq` CD container. Host-only
`make -C nintendo3ds prepare-audio HOST_CXX=clang++` requires FFmpeg and prepares
private `local-data/3ds/openbw/audio/{effects.bin,terran1.pcm,terran2.pcm,terran3.pcm}`.
Copy the entire audio folder alongside the new executable on SD. Runtime PCM is
22050 Hz, mono effects/stereo music, with no WAV/MPQ music decoding during play.
DSP initialization requires the console's `sdmc:/3ds/dspfirm.cdc`.
The new automatic log is `logs/ai-audio-benchmark.csv`; the older no-audio log is
preserved. Check `audio_ready=1`, `sfx_played>0`, increasing `music_buffers`, and
zero `music_underruns`/`audio_io_errors` alongside FPS/TPS. `Audio OFF` is an asset
or DSP problem and cannot validate playback performance.

### Historical Refinery/gas milestone (2026-10-19, superseded below)

The default scenario is now Gas AI vs AI (scenario 5), with seven workers per
side and one assigned to the completed Refinery. The original six-worker
Marines AI vs AI baseline remains in START (scenario 3); Player vs AI remains
scenario 4 and also permits manual Refinery construction. The 40-unit budget
and original resource/build/supply rules remain in force. Gas is accumulated;
Factory/tank production is a later milestone.

SCV Y opens Depot/Barracks/Refinery. Placement snaps to a neutral geyser and is
revalidated on arrival. A selected SCV's A on an own completed Refinery issues
a gas harvest order. Native worker hiding/return trips remain active; temporarily
hidden workers retain their saved control-group identity. Eight START rows use
12-pixel drawing/touch spacing and fit the lower screen.

The first GPU present and initial CSV header flush finish before the simulation
clock starts; normal-play debt is never discarded. CSV now includes red_gas,
blue_gas and csv_flush_us (the preceding CSV flush duration, or initial header
flush for the first sample). A hardware run is needed to confirm whether this
removes the initial two-second logging stall observed in both old baselines.

Prepare graphics/audio again and update the executable, graphics.bin and
complete audio/ folder together. Existing map and three MPQs remain valid.
See nintendo3ds/reports/GAS_VALIDATION.md and HARDWARE_AUDIO_VALIDATION.md.

### Full Terran milestone (2026-10-19)

The current implementation supersedes the older roster/buffer/atlas limits
above. View buffers hold 512 units / 2048 image layers; groups hold 192 IDs.
The normal/linear reservations remain 36/8 MiB. Graphics format OB3DGFX2 stores
only terrain/minimap; native resident GRPs supply a bounded GPU frame cache,
with no SD sprite reads during play. This avoids preallocating every animation
frame. PCM8 mono effects keep the larger complete sound set in normal RAM;
stereo music stays PCM16 and streams on a worker thread. Palette glow against
the destination remains an approximation, rather than full original software
palette compositing.

A hardware gas log exposed a 2.064-second main-thread CSV flush. CSV writes now
use a bounded asynchronous queue; Exit drains it. Logs count queue drops and
write errors explicitly. The largest full battle must still be measured on
the original console; emulator timing is not hardware evidence.

Update the executable, both graphics files, maps/arena.chk and effects.bin
together. Existing MPQs, maps/probe.scx and the three complete music files are
retained. Full verification details are in TERRAN_VALIDATION.md. Campaign
presentation, original campaign AI, save/load and networking remain unfinished.

Private-data preparation order on a fresh checkout:

```sh
make -C nintendo3ds prepare-arena HOST_CXX=clang++
make -C nintendo3ds prepare-graphics prepare-arena-graphics prepare-audio HOST_CXX=clang++
make -C nintendo3ds test test-mpq-audio test-audio test-csv test-sprite-cache HOST_CXX=clang++
```

Stage owned MPQs/map first using the earlier staging instructions. Asset tools
exercise full construction, production, technology and ability scenarios
before extracting private art/audio. No Blizzard assets belong in Git.

### Protoss milestone (2026-10-19 overnight)

The current local build defaults to Protoss battle; START retains Terran and
all earlier scenarios. All sixteen Probe buildings, fourteen manufacturing
products (including fighters), both Archon merges, seven techs, nineteen
upgrade types, shields, transport and native abilities have integration tests.
The 200-supply fixture uses 78 troops per side plus fighter ammunition and the
same short score/time rules. See PROTOSS_VALIDATION.md for exact scope and
emulator limitations. Zerg remain the next integration step.

Keep separate resident sound banks: Protoss audio is under audio/protoss and
uses protoss1.pcm..protoss3.pcm; earlier Terran audio remains under audio.
Neither bank may be concatenated into the other within the original RAM
budget. Reproduce the private Proto package using:

```sh
make -C nintendo3ds prepare-protoss-audio prepare-protoss-graphics HOST_CXX=clang++
make -C nintendo3ds test-protoss-cache HOST_CXX=clang++
```

The actual SD was already removed after the earlier Terran delivery. The new
Proto executable/audio have been prepared locally; hardware testing and copying
the new package to SD are still pending.

## Zerg milestone (2026-10-19)

Native Drone construction with creep, sixteen structure recipes including
five building morphs, thirteen manufactured/morphed products, six research
technologies, sixteen upgrade types, burrow, Overlord transport, Nydus pairs
and Queen/Defiler spells are integrated. Explicit fixtures are used for manual
lab and 200-supply symmetric stress testing. Campaign AI and ordinary Zerg or
Protoss opening economy bots are not implemented. Three race-specific sound
banks keep normal RAM bounded; all nine original music tracks stream.

Creep uses original CV5/VX4/VR4 megatiles and native tileset GRP edge frames,
updated from live creep state through the existing bounded GPU sprite cache.
No larger static sprite atlas or change to the 36/8 MiB heap reservations.
Full test results and remaining visual/hardware limitations are in
`nintendo3ds/reports/ZERG_VALIDATION.md`.

```sh
make -C nintendo3ds prepare-zerg-audio prepare-zerg-graphics HOST_CXX=clang++
make -C nintendo3ds test-zerg-cache HOST_CXX=clang++
./nintendo3ds/build/host-sim "data files" nintendo3ds/local-data/3ds/openbw/maps/arena.chk zerg
```

### Glow, shadow and afterimage correction (2026-10-19)

Nexus/fire glow now uses additive RGB blending rather than an ordinary alpha
layer that painted dark discs. Shadow cache pixels contain black coverage;
trans50 effects keep source/team colors at half opacity. Native Templar echo
sprites retain iscript frames/positions with half opacity instead of solid
copies. The ordered translucent pass restores alpha blending before UI drawing.
All three ASan cache/reference tests and 75 full-viewport composition comparisons
pass, including 234 moving Templar afterimages. The original destination-indexed
palette remaps still have an RGB approximation; warp/cloak limitations remain.
See `nintendo3ds/reports/EFFECTS_VALIDATION.md` for release and validation details.

### Original cursor art (2026-10-19)

The Circle Pad pointer now uses original cursor GRPs from the owned MPQs:
animated arrow, ownership hover rings, command targets and eight edge-scroll
arrows. Native canvas offsets preserve the hotspot; no extra release assets are
needed. Two 64x64 textures add 16 KiB linear memory and reuse 8 KiB fixed scratch;
no cursor file reads occur during play. Input mappings remain unchanged.
ASan independently checks all 17 kinds / 74 frames / 303104 texture pixels and
24 cross-map/race reloads; all three existing cache suites and controls pass.
See `nintendo3ds/reports/CURSOR_VALIDATION.md` for implementation limits/tests.

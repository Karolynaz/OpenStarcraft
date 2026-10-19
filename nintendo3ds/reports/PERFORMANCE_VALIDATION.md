# Large-battle rendering performance — 2026-10-19

Baseline: local commit 0eb6f71 (scenario-loading fix), plus the same phase timers
used by the candidate. Private assets, scenario fixtures, sound and CPU/model
settings are shared. Original 3DS normal/linear heaps remain 36/8 MiB.

## Implementation

- Track both display VBlank counters. A fast frame still waits for the next
  refresh; a slow frame that has already crossed both refreshes can submit
  immediately. This removes an extra VBlank wait without uncapping the display.
- Draw binary-alpha world sprites in texture groups, using distinct depth
  values for the original painter order and alpha rejection for transparent
  holes. Draw translucent effects in their original order against opaque depth.
  Disable depth testing/writes for subsequent health bars, cursor and UI.
- Resolve team/light palette lookups during loading, then use direct RGBA5551
  lookups in the GRP RLE decoder. Retain only the sampled light-remap colors,
  rather than keeping seven complete decoded PCX tables resident.
- Add mean per-frame CPU rendering, VBlank/GPU wait, sprite collection/drawing,
  frame-end and SDK GPU timing columns to the existing CSV. The summarizer
  understands the optional fields and older CSV runs retain their old format.

The existing whole-linear-memory flush is retained. Its SDK behavior is described
in [Citro3D's render queue source](https://github.com/devkitPro/citro3d/blob/master/source/renderqueue.c).
Avoiding it safely would also require explicit flushing of Citro2D vertex/index
buffers; this change does not depend on private SDK layouts or undocumented hooks.

## Correctness and memory checks

All three race ASan cache/reference tests pass: Terran 8,722,438, Protoss
11,195,327 and Zerg 16,053,103 cached pixels checked exactly, plus independent
original GRP palette/team/alpha references and 48,576 creep-reference pixels.
A new independent software compositor compares 25 complete 400x240 viewports
per race against the original ordered stream. It checks real submitted texture
bytes, flip, tint/alpha, depth state and overlapping translucent images, and
requires UI depth testing to be disabled afterward.

Palette/RLE optimization retained these complete moving-scene pixel fingerprints:
Terran 5ed8e21b94dd3fee; Protoss d4eec65501c0a32e; Zerg 9dbf0823c96991f3.
Contiguous texture runs on those 200-frame moving-camera fixtures decreased:
Terran 4344→3279, Protoss 3870→3023, Zerg 5289→2905. These are CPU test-double
texture-run counts, not measurements from the physical console's GPU.

Pacing tests cover fast-frame limiting, slow frames, dual-screen advancement
and counter wrap. Common scheduler/input/controls/map-recovery checks, the
18-load single-engine cross-race ASan lifetime test and allocation-failure audio
test pass. ARM11 ELF validates ARMv6K / VFPv2 / no Thumb-2 or NEON.

## Emulator measurements

Matched runs both used scenario order Zerg→Terran→Protoss, completed frame 720
with identical winners, scores and end reasons, and retained startup samples.
Means use one-second intervals with 0 < frame < 720, excluding paused and final
partial intervals. Each race has 29 active intervals. All optimized runs kept
~23.98–23.99 TPS, with audio ready, zero missing sprites, cache overflows,
music underruns, audio I/O errors, DSP drops and CSV errors.

| Race | Baseline mean FPS | Optimized mean FPS | Gain | Baseline / optimized minimum FPS |
|---|---:|---:|---:|---:|
| Terran | 44.36 | 53.79 | 21.3% | 11.43 / 19.97 |
| Protoss | 51.27 | 59.72 | 16.5% | 34.74 / 58.44 |
| Zerg | 47.64 | 57.75 | 21.2% | 17.71 / 41.20 |

An independent earlier baseline Zerg run measured 47.76 FPS, consistent with
47.64 FPS in the matched repeat. The pacing-only intermediate measured 56.16
FPS and pacing+batching 56.53 FPS; final precomputed palettes reached 57.75 FPS.
These intermediate runs are private, retained with the matched full CSVs under
`local-data/performance-2026-10-19/`. Full comparison and phase/memory summaries
are in `performance-2026-10-19.json`.

Mean Zerg CPU rendering time decreased 9198→6307 us per frame; its sprite work
fell 7045→4158 us. Peak normal-heap usage decreased by about 110 KiB, with the
same 7,789,568-byte linear peak. GPU timing from HLE is not used to infer
physical PICA performance. The phase columns are available for the next SD test.

The optimized binary also loaded Protoss laboratory after the three battles;
A selected Nexus and Y queued a Probe, with correctly visible selection health
and shield bars. All four loads completed with graphics and audio ready.
Emulator settings were restored and emulation stopped after comparison.

The emulator uses original 3DS mode, CPU 100% and DSP HLE. Actual console FPS
must be measured again; earlier physical-console results were roughly 11–20 FPS
in these large battles and are not replaced by emulator results.

## Delivery

Final native/staged/tested/emulator/private-ZIP executable SHA-256:
`1660ed166b3ed5c9334269c994e3dfea1fe33a5fc9c0aed43624d7f2f2e4e34f`.
Private package: `local-data/releases/all-races-performance-2026-10-19.zip`,
25 files, 344048452 bytes, SHA-256
`faae7753e6c38f2b5ce6868c3e794cefdf9bd839c3add8322bf7b992f087da8d`.
Owned-input hashes, ZIP CRCs and executable equality passed. The previous loading
fix package remains available. The performance build was subsequently copied to the physical SD as described below.



### Subsequent SD delivery

On the user's next insertion, disk4s1 was mounted after UUID verification.
All25 package files passed read-back hashes; only executable and README changed.
All15 existing DSP/log/Luma files were backed up privately and verified unchanged.
The card was synced and disk4 successfully unmounted. The user is testing the
exact1660ed166b3ed5c9334269c994e3dfea1fe33a5fc9c0aed43624d7f2f2e4e34f build.
Receipt: `local-data/performance-2026-10-19/sd-delivery/receipt.json`.

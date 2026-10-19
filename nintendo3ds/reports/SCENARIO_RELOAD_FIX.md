# Console scenario-loading crash investigation — 2026-10-19

The user returned the SD after two crashes during scenario loading. Both new
Luma ARM11 dumps and the original executable/ELF were preserved privately in
`local-data/console-crashes-2026-10-19/`; no owned game assets were published.

## Evidence and limits

Both dumps identify the same PC 0x001b33ec (`ndspiReadChnState`, SDK libctru
2.7.0). The faulting instruction pushes 40 bytes onto the DSP thread stack;
FAR 0x08082c80 equals SP 0x08082ca8 minus 40. This is an unmapped-stack write,
not evidence of a specific corrupt wave-buffer address. The stack dump is empty.

An uncaught exception can abort the application without the normal main-loop
NDSP shutdown. SDK process cleanup then unmaps heaps while DSP is still active.
This is a plausible explanation of the dumps, not proof of the original
exception or the user's exact scenario sequence.

A deterministic host test confirmed a real uncaught `std::bad_alloc` path in
`Audio::load`: its single resident PCM vector required up to 8 MiB of contiguous
heap. Limiting individual allocations to 64 KiB made the old implementation
throw out of `load` (recorded failing assertion). Engine and graphics loading
already catch exceptions, but audio loading did not.

## Change

- Allocate resident sound effects in blocks of at most 64 KiB, within the same
  8 MiB bank limit. Copy DSP voices across block boundaries without changing PCM.
- Catch loading allocation failures, release partial resources and show an audio
  error rather than abort. The input bank file has RAII ownership.
- Precompute music paths in fixed buffers, removing C++ heap allocation from
  streaming track transitions.
- Release old audio/graphics before core initialization, then reserve graphics
  scratch before loading segmented audio. The first candidate loaded audio
  first and exposed a caught graphics allocation failure after several reloads;
  that candidate was rejected. GPU/heap capacities and sound quality are unchanged.
- Close a record in `logs/scenario-load.log` after each loading stage, including
  failure details and heap/linear availability. This is independent of CSV.

## Automated checks

AddressSanitizer audio tests cover bounded allocations, exact voice bytes across
64 KiB boundaries, injected effect-table and partial-bank allocation failures,
recovery and repeated reloads. Existing tests cover both PCM widths, missing or
empty assets, track transitions, pause, bank unload and DSP-buffer lifetime.
All three private sound banks retain exact PCM equality with their exports.
Scheduler, controls, input, core self-test, map rejection/recovery and async CSV
checks pass. Native ELF validates ARMv6K / VFPv2 with no Thumb-2 or NEON.
Normal/linear heaps remain 36/8 MiB; this does not require New 3DS mode.

## Physical-console performance before the fix

`console-2026-10-19-performance.json` separates runs and excludes final frame-720
paused samples from large-battle averages. Active Zerg runs average 12.54 and
12.66 FPS (23.68/23.79 TPS); Protoss 19.85 FPS (23.98 TPS); Terran 10.73 FPS
(19.79 TPS). Terran lab averages 59.79 FPS (23.99 TPS). The returned CSV reports
no music underruns, audio I/O errors or DSP drops in these new runs. These are
actual console results, substantially slower than prior emulator FPS numbers.
This loading fix does not claim to solve dense-battle rendering performance.

## Emulator and delivery

Final candidate `reload-safe-v2` completed boot scenario 10 and eight reloads
in one process: 7 → 8 → 9 → 6 → 11 → 8 → 10 → 6. Each completed with both
graphics-ready and audio-ready, with no failed loading stage. This covers all
three race labs/battles and returns to the previously failed Terran case.
`reload-fix-emulator.json` records the exact stage evidence. The final Terran
battle screenshot showed normal sprites, audio ON, no missing sprites and no
music/I/O errors. Emulator model/CPU were original 3DS/100%; original settings
were restored and the emulator stopped. DSP was HLE, so actual DSP/hardware
stability still requires another console test.

Built, staged, emulator-tested, packaged and SD-read-back executable SHA-256:
`2117f7153527fc98ec2a233055ed8dcc31c8c9c107674add62c11b6fd9001b81`.
Private ZIP: `local-data/releases/all-races-reload-fix-2026-10-19.zip`, 25 files,
344045935 bytes, SHA-256
`7b4b3686e313631b51015a1964846ca8937866e44e8574f608fe02bafd4c32f8`.
The previous all-races ZIP remains available unchanged.

All 25 SD files passed read-back hashes. Only the executable and README needed
replacement; their old versions were backed up privately. User DSP firmware,
both Luma dumps and existing benchmark CSV passed unchanged hash checks.
The exact new ELF is saved alongside the crash evidence for future symbolizing.
Preserve any new Luma dumps and `logs/scenario-load.log` after the next console test.

SD disk4 was synced and successfully unmounted after verification.

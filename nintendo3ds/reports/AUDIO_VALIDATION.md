# Original-3DS audio integration — 2026-10-18

## User hardware baseline, without audio

The user ran the existing build on their Nintendo 3DS, let the AI battle finish,
and exited through START → Exit. The mounted SD's original ai-benchmark.csv was
copied without modification into ignored local-data/hardware-captures. The
machine-readable summary is hardware-no-audio-summary.json.

548 active samples: mean 58.668 FPS / 24.016 TPS (partial startup/catch-up samples
included), blue wins at frame 13200, maximum tick 7132 us, peak normal heap
24,734,264 bytes, linear heap 6,355,968 bytes, zero missing images. One startup
sample at frame 25 has 47 ticks of debt; every later sample has zero debt. This
is encouraging evidence for this particular 64x64 Terran test, not a general
performance claim about all StarCraft units/maps. Hardware model was not
independently identified.

## Audio implementation

- A derived state_functions captures native iscript/economy/combat sound events
  in a fixed 64-event queue. It does not alter simulation RNG, costs or timing.
  Host scenarios produce the same frame-13200 winner and 36/33 losses.
- Offline export profiles the complete deterministic battle, delayed deaths,
  and gameplay tests, then includes all select/acknowledgment/ready variants for
  the current SCV/Marine/CC/Depot/Barracks roster: 30 effect IDs, 9054 native
  events during profiling. A selection or accepted order also queues a voice.
- Original WAVs come from the user's three classic data MPQs. Original Terran
  music is read from the embedded MPQ in their GOG StarCraft.mpq disc container;
  the host reader validates an aligned version-0 MPQ header within its first
  MiB. The 3DS does not need this large container or the WAV decoders.
- FFmpeg converts complete effects to mono signed PCM16 at 22050 Hz and three
  complete music tracks to stereo PCM16 at 22050 Hz. No shortened tracks or
  synthesized substitute sounds. Audio stays in ignored local-data.
- Effects use 1,751,362 bytes of normal heap. Eight DSP voices each reserve
  128 KiB of linear memory; the longest effect is 124,660 bytes, so none is
  truncated. Playback has camera-distance culling, stereo panning, 80 ms
  same-effect debounce, flag-0x10 duplicate suppression, priority arbitration.
- Music uses three 32 KiB linear buffers. A 16 KiB-stack worker on application
  core 0 reads raw PCM; it does not request extra system-core time or New3DS
  clocks. It cycles Terran 1 → 2 → 3 → 1 and joins before freeing buffers.
- NDSP handles mixing/resampling. Pause/victory pauses channels, restart clears
  effect voices, exit joins the reader, clears DSP queues, then frees linear
  memory before ndspExit. Missing assets/DSP report Audio OFF; failed playback
  must not be treated as a successful audio performance test.
- CSV uses a new ai-audio-benchmark.csv, preserving the original hardware log.
  It adds audio readiness, played/culled/missing effects, voice/event drops,
  streamed buffers, complete-queue music underruns, I/O failures, track number
  and the DSP dropped-frame counter. Voice drops are intentional saturation
  of the eight-voice mixer; music underruns and I/O errors are not.

## Repeatable validation

```
make -C nintendo3ds DEVKITPRO=/Users/karolynaz/devkitpro -j2
make -C nintendo3ds test HOST_CXX=clang++
make -C nintendo3ds prepare-audio HOST_CXX=clang++
make -C nintendo3ds test-audio HOST_CXX=clang++
```

The portable audio contract test runs the actual audio.cpp against a DSP API
contract double under AddressSanitizer. It checks missing DSP, malformed effect
bounds, complete streaming chunks across track EOF, positional effects,
pause/unpause, reset/reload, missing/empty music, and stopping queues before freeing memory. It is
not an emulator of the DSP or a hardware performance test. LeakSanitizer is
not enabled on this macOS host.

Asset verification independently decodes every source WAV with FFmpeg and
compares every packaged PCM byte: all 30 complete effects and all 3 complete,
non-silent music tracks pass. A separate AddressSanitizer run of the native
battle/gameplay/audio exporter passes with no queue overflow. ARM11/VFPv2 ELF
verification passes; main/audio printf formats also pass -Werror=format.

## Emulator and SD delivery

The final release completed the full battle in Azahar, original-3DS mode
(`is_new_3ds=false`, override default false), CPU clock 100%, with pause/resume
exercised. Blue wins at frame 13200 with the unchanged 36/33 losses. All three
music tracks were streamed. The final result is audio-summary.json; the raw CSV
is private local-data/audio-captures/emulator-audio-benchmark.csv. The first
short development run used the default model and is excluded by selecting the
last CSV run. A second run was made with verified original-model settings.

550 active samples average 59.828 FPS / 23.930 TPS, including partial intervals
around pause/resume and victory. Zero-TPS pause/terminal rows are excluded and
counted explicitly (9). Maximum tick is 2128 us; zero simulation debt, missing
sprites, missing sounds, event-queue drops, music underruns, audio I/O errors or
DSP dropped frames. Peak normal heap is 26,518,336 bytes; peak linear allocation
is 7,502,848 bytes (within the unchanged 36 MiB + 8 MiB heaps). The engine played
1916 effects; 3650 were culled as off-camera, and 229 were rejected by the
8-voice priority limit. These voice-limit drops are intentional, not music
buffer underruns. Combat samples (frame >= 5000, partial intervals retained)
average 59.831 FPS / 23.969 TPS. Emulation cannot establish actual-console
playback quality or audio performance.

The executable and complete audio folder were copied to the user's mounted
Nintendo SD. Each copied file's SHA-256 was read back and checked. The original
hardware CSV and previous executable were backed up privately; existing MPQs,
graphics and map were preserved. The SD already has its DSP firmware dump at
/3ds/dspfirm.cdc. Final release hashes are in audio-artifacts.json.

Real-hardware audio performance still needs the user's next SD-card test.

To summarize a future SD-card run without editing or filtering its raw log:

```
python3 nintendo3ds/tools/summarize_benchmark.py /path/to/ai-audio-benchmark.csv
```

It selects the latest header/restart run, reports every zero-TPS exclusion, and
retains all active partial/startup/catch-up samples.

# Raynor and Wasteland mission audio — 2026-10-25

Source src-29f5fa8e6b3d1c6f
3DSX SHA256 55ea5a4b256fb7b10c7d7e60f14eed07df6a7329bfb01de11b278e8fc126dad8

The previous bank omitted hero response IDs and campaign Transmission/Play WAV
actions only displayed text. Export 24 owned Raynor Marine/Vulture response
sounds and five original in-mission WAVs from the owned nested disc archive.
Mission IDs use reserved1200+map-string-index; native unit IDs remain unchanged.
The existing fixed eight-voice mixer plays them, with global (non-positional)
mission dialogue and higher priority. Trigger waits enqueue a Transmission once,
not on every waiting tick. Save reconstruction drains historical audio events.

Shared bank now606effects/23,204,191PCMbytes, below its24MiB disk bound.
Fixed4MiB cache and existing linear voice scratch remain; no full mission-audio
allocation at runtime. Rebuild tools preserve an optional private campaign bank.
Packaging rejects a shared bank lacking required Raynor/dialogue IDs.

Tests PASS:
- ASan complete mission, explicit SCV/Raynor/Advisor speech event assertions,
  Raynor select423 and move acknowledgement427; paid victory at frame7649.
- Campaign save/load continued digest equality; independent general save tests.
- Every606 bank effect arrives byte-exact at the DSP contract double;413 use
  streamed fallback with4MiB cache, zero IO errors.
- Independently decode all29 owned WAVs; prepared PCM exact, complete, non-silent.
- Audio pause/reload/error/buffer lifetime and fragmented-cache/stream cancel/
  failed-read recovery tests pass.
- ARMv6K/VFPv2 final binary architecture check passes.

Original-model CPU100 emulator with updated bank:
Load a host-recorded approach, let the original rescue happen, then B clears
workers, Circle Pad/A selects Raynor and A gives him a movement order.
Playback count advances from3 scripted sounds to5 including select/ack; no
missing sounds, voice drops, music underruns or audio IO errors. Last captured
active samples59.83FPS/23.93TPS, heap30,741,760bytes, linear8,084,480.
This validates event submission/playback counters, not a human listening test
or full-battle physical performance. Native sound data was verified separately.
Emulator model/profile restored; no physical SD write or publication.

Scope: in-mission dialogue and hero responses. Briefing narration/portraits and
movies remain pending. Active speech is not resumed midway by replay-backed
Load. Purchases/assets stay private; package runtime saves/progress excluded.
Evidence under local-data/raynor-audio-2026-10-25.

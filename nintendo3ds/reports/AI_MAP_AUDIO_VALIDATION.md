# Console AI, owned maps and bounded audio — 2026-10-24

## Scope and AI reuse decision

The user authorized Easy/Medium opponents, more selectable maps and additional
useful stability/gameplay work during this session. No scheduled continuation,
paid credits, public asset upload or physical SD installation was performed.

The owned archive audit found scripts/aiscript.bin (62050 bytes after patch
precedence) and scripts/bwscript.bin (26458 bytes). These are scripts, not a
standalone bot executable. This checkout does not implement their interpreter.
The PyMS project documents their melee/campaign role:
https://github.com/poiuyqwert/PyMS/blob/master/Help/Files/aiscript.bin.md
BWAPI describes a PC-side API with compiled bot modules and StarCraft/Windows
installation dependencies: https://github.com/bwapi/bwapi . Importing one of
those DLLs would not supply an ARM/3DS-compatible controller. This stage
extends the existing bounded native-action controller; no third-party bot code
or original AI script data was committed, and no Blizzard-AI compatibility is
claimed.

## Implemented

- Create settings adds a sixth Difficulty row. Easy is the default; Medium
  persists alongside version/race in the existing Player1 profile. Older
  profiles still load. Both active AI slots use the selected difficulty.
- Easy: target8 workers/14 army, nominal42-object purchase budget, one-second
  thinking, at most8 units sent per offensive wave, 6min offensive grace,
  60sec subsequent wave intervals. Medium:14/24, nominal56, half-second
  thinking,16-unit waves,3min grace,35sec intervals. Nearby visible threats
  may be defended before grace ends. Costs and production/research times are
  native; no resource grants, shortened construction or revealed enemies.
- Medium can purchase a second reachable base after8min when it has12 workers,
  eight army and600 minerals. Main-base anchoring remains stable after an
  expansion; a worker explores a prospective expansion before placement.
- Native refinery placement no longer inherits the extra non-Zerg structure
  spacing margin. That margin could falsely block a legal refinery beside an
  existing depot, deadlocking the larger Medium economy.
- Count incomplete workers when deciding replacements; reserve early Zerg army
  room for Mutalisks. One Observer/Overlord follows the army for detection.
- Sixteen bounded last-seen building records per AI, confirmed-empty-location
  invalidation, visited starts and a16-sector ground search replace hidden
  enemy-building-count target selection. Hidden death does not clear memory.
  Global building-elimination victory conditions remain the existing rules.
- Click Map to browse five filenames per page with Circle Pad/A/B or D-pad;
  arrows still cycle maps. Five original rows were spaced into six shared
  draw/hit-test baselines (51,74,97,120,143,166); text sizes stay readable.

## Owned map catalogue

87 additional original melee/ladder maps (27 StarCraft,60 BroodWar), copied
privately from the user's StarCraft.mpq and BroodWar.mpq, not downloaded as game
assets. Only filename metadata was downloaded from the author's CC0 dictionary:
https://github.com/jcfieldsdev/mpq-list-files . Original compressed map bytes
are retained. Shared DiscReader/extract helpers support the embedded MPQ stub.

Exporter excludes campaign/scenario/allied/web special maps, dimensions beyond
32..128, fewer than two starts and maps with disconnected ground start regions.
87 accepted/68 rejected. All offered starts are checked, including inactive
slots. There are18 two-player and69 maps that can offer two AI slots; extra
slots remain closed. Dimensions:55 128x128,13 96x96,10 128x96,6 96x128,
2 64x64 and1 64x96. These are compatibility checks, not a balance certification
of every map. Per-map hashes/provenance remain in private map-catalogue.json.

Full release packaging now includes all staged maps instead of only the two
old test files, and verifies map/UI/owned-input/combined-audio hashes and ZIP CRC.

## Audio memory and mixed-race sounds

Splitting the entire bank into16KiB chunks alone did NOT fix the reload issue:
the earlier candidate still failed loading the8.37MB Protoss bank. The final
backend retains at most4MiB of PCM in16KiB blocks, shortest sounds first. Cache
allocation can fall back to fewer blocks. Uncached sounds read on the existing
music/audio thread, at most one16KiB effect read per iteration after music
refills. Main-thread effect file IO is eliminated. Eight existing DSP voices
and their bounded linear buffers remain; pending IO reserves a voice. Atomic
publication/cancellation and a short queue/reset lock prevent queued data from
being overwritten/freed during reset/unload. IO failures drop that request and
increment the existing audio error count without discarding the music stream.

The three owned race banks merge losslessly by native sound ID into
NEW audio/effects-all.bin:577 unique effects,22000725 PCM bytes,22012285 file
bytes, SHA256 424cb888841dd14d2e7bbacaf1cb2bb401c06896730bf6013c6b3e41e17f2c73.
Duplicate IDs must match samples/priority/flags exactly. File bound24MiB,
resident bound4MiB, ID bound1200. Constant-time ID lookup avoids scanning the
larger table per event. Music still follows the player's race. This supplies
existing prepared effects for all three races in mixed games; it does not
invent missing effects beyond those prepared banks.

The whole update must be copied: the new shared bank is required by the
executable. A cold uncached long sound can be delayed by SD latency; original
hardware timing/audio listening remains necessary. PCM is not resampled or
truncated by the merge/cache; no extra core or New3DS clock mode is used.

## Host verification

- Frontend ASan: old/new profile, difficulty save/reload, six-row draw/hit
  coordinates, paginated map selection, D-pad, retained map settings, existing
  campaign/custom transitions and AI Random/Off/cap rules PASS.
- Native economy fixtures: six36000-frame runs (all races, Easy/Medium) from
  actual50 minerals, invulnerable non-fighting human. Worker limits, production,
  native upgrades, offensive grace and bounded live population asserted.
  Three further36000-frame Medium runs on original Lost Temple assert two
  resource depots and advanced race technology. Exact counts/first-wave frames
  are retained in private melee-test.log; six-minute Easy waves begin frame8640.
- Native sight fixture: never-seen building absent, seen building remembered,
  hidden death retained, newly visible empty location forgotten; Easy responds
  to a nearby invader at frame0 despite offensive grace PASS.
- A further actual three-race/two-AI Medium match on Ice Mountain finished
  natively at frame11833 (Zerg victory), peak147 units; Zerg trained35/built11,
  Protoss trained24/built14. Per-side population bounds and zero native sound
  event-queue drops asserted PASS.
- Terrain/fog repeated600-tile/496-resident-slot eviction:385560 checked pixels;
  128/sparse-start load, UMS/256 rejection and victory/defeat/surrender PASS.
- Final map catalogue: all87 actual maps mixed-race start/load/terrain generation
  and240 native ticks each under ASan;69 with both AI slots PASS.
- Audio ASan: original bounds/DSP/EOF/race music/pause/reload/buffer lifetime
  PASS. Allocation failures at16 cache chunk positions preserve playback via
  cache or IO; complete waveform equality across chunks, zero-cache fallback,
  in-flight reset/unload and mid-stream truncation recovery PASS.
- All original audio banks plus merged bank exercised effect-by-effect under
  ASan: Terran202 (66 streamed), Protoss202 (70), Zerg192 (37), merged577
  (387), every DSP waveform byte-exact and zero IO errors. These host doubles
  verify bytes/lifetime, not actual DSP sound or physical SD performance.

## Native build and emulator evidence

Final source: src-35e9cceb44d81d8f.
3DSX SHA256: be0596b848d32eac3910dd209f4e5d1e236228cb7ee9dbcd582a5af4561ceeb1.
ARMv6K/VFPv2/no Thumb2 or NEON verification PASS. Original-model emulator,
CPU100,36MiB normal heap/8MiB linear. Matching ELF, build log, screenshots,
CSV/load journal and package receipts stay in private local-data/
ai-map-stage-2026-10-24. Normal Exit completed; emulator stopped and saved model/profile preferences
restored. Final exact executable was exercised through seven successful core/
graphics/audio initializations: three-race Ice Mountain, Protoss laboratory,
Zerg all-unit battle, Ice Mountain again, Protoss all-unit battle, Ice Mountain
again and Terran all-unit battle. All seven reported4096KiB SFX cache and no
audio-off/core-failed stage. Both stress results and B return to Create settings
worked, preserving map/races/Medium. Restarting the frontend also verified
stored Medium/Terran settings.

Active CSV intervals only (TPS>0/FPS>0; result-paused rows retained in raw CSV):

| Scenario | Samples | Peak units | Median FPS | Min FPS | Median TPS | Min TPS |
|---|---:|---:|---:|---:|---:|---:|
| Mixed-race melee | 476 | 140 | 59.83 | 59.67 | 23.93 | 23.85 |
| Protoss laboratory | 129 | 43 | 59.83 | 59.69 | 23.93 | 23.88 |
| All Zerg battle | 30 | 342 | 59.745 | 39.41 | 23.925 | 19.67 |
| All Protoss battle | 30 | 350 | 59.805 | 58.39 | 23.93 | 19.97 |
| All Terran battle | 30 | 332 | 59.71 | 20.89 | 23.925 | 17.96 |

All these scenarios had zero missing sprite/sound IDs, music underruns and
audio IO errors. Eight-voice saturation intentionally drops effects in dense
battles (up to the counts recorded in the private summary); this is separate
from IO errors. Zerg's lowest-FPS interval was frame265:243 units,13.46ms mean
simulation tick and13.12ms sprite draw/upload work, with zero cache overflows.
Terran's lowest-FPS interval was frame314:274 units,32.13ms mean native
simulation tick and7.82ms sprite draw/upload work, with zero IO errors or
cache overflows. The stress dips were not eliminated; no physical-hardware FPS gain is claimed.
The mixed-match camera stayed at the idle human base, so use the three all-unit
battle samples when assessing visual combat load. Host native AI match results
and emulator timing measure different things.

## Remaining limits

Custom console difficulty tuning still needs player feedback; these names do
not imply Blizzard easy/medium script equivalence. AI remains bounded, with
one expansion, a limited roster and no transport-based island invasion/full
spell micro/campaign script interpreter. Purchase budgets count native units
approximately: paired spawns, larvae, add-ons and tank subunits can add a few
live objects beyond the nominal limit. All-race music/effects are private and
must not be redistributed publicly. Physical touch and physical SD/audio/frame
performance are not established by the emulator or host tests.


## Deliverables

- Full private package: easy-medium-maps-2026-10-24.zip,120 files,377947601 bytes;
  SHA256 ce20f1604933c31dd52e9b647d616ed388822f10c5a5e3ad7119c3a60d11cbbf.
- Private update: easy-medium-maps-update-2026-10-24.zip,96 files,21479084 bytes;
  SHA256 e384ed40e7c6fdf0ce136fe49edbae57f1c627d936bef0fecc1a48bd43ca8fb7.
- Copy the whole update over the SD root, merging3ds/openbw; keep existing
  profile/logs/dspfirm and purchased MPQs. NEW shared audio bank and all87 maps
  are included. No physical SD installation in this session.

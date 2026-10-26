# Large-map infrastructure — 2026-10-26

Source `src-774686bb25688432`, original3DS36MiB normal /8MiB linear heaps.
Dimensions accept up to256 on either axis. Fog creep memory uses actual map
stride/size instead of a fixed128 stride. Fog textures use256² for larger maps.
Large maps reserve one fewer512² sprite atlas page (seven total), freeing enough
linear memory for the larger masks. Maps above16384 cells use a2MiB short-effect
cache; full original effects/dialogues still stream without truncation. Smaller
maps retain their4MiB cache and existing graphics allocation.

ASan:192x192 and128x256 private no-trigger fixtures load, tick, save and write
streamed terrain. Far-corner terrain/sprites, all65536 Morton-ordered fog texels,
minimap texture and release pass for both. Existing nine startup/save/replay/
continuation/17248 idle tests PASS. All139 original dialogue WAVs stream byte-
exact with the2MiB budget, IO0; fragmented-allocation/fallback/unload tests PASS.
ARMv6K/VFPv2 architecture check PASS.

Original-model CPU100 Azahar geometry/memory probes:
-192x192:411 original placed units; core27,088,896 bytes, graphics27,722,552,
 audio-ready29,858,496 normal,451,584 linear free,2MiB SFX cache.
-128x256:444 placed units, later445 with native larvae; core27,158,984,
 graphics27,790,416, audio-ready29,926,360 normal,451,584 linear free.
 Three-minute sample:~59.6FPS/~24TPS, no missing images/cache overflows/audio IO
 errors/music underruns. This is emulator idle observation, not console speed.
 GPU texture bytes3,948,544 (seven atlas pages +cursor +full256 fog pair).
Emulator stopped; original model/profile/both original saves restored. Physical
SD untouched. Private screenshots/logs/binary/ELF/fixtures archived locally.

IMPORTANT: probe fixtures remove original TRIG in temporary copies so geometry
and memory can be isolated from missing script operations. They are not shipped
missions and prove neither original objectives/AI nor campaign victories. No
original source map is modified. Custom256x256 has not been hardware validated.

A fresh60-map runtime compatibility audit now has25 startup successes,20 load
rejections and15 early tick rejections. The14 size rejections are gone; later
AI/trigger/doodad issues are now reached. This is1200-frame compatibility, NOT a
victory audit. Private raw CSV: campaign-all-audit/runtime-large.csv.

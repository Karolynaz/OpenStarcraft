# Zerg and all-race test milestone — 2026-10-19

Zerg is integrated through original OpenBW actions, with private purchased
art/audio, within unchanged original 3DS 36 MiB normal / 8 MiB linear heaps.
Terran and Protoss profiles remain available. No game assets are committed.

## Native command and simulation coverage

- Sixteen structure recipes: eleven Drone buildings and five native building
  morphs (Lair, Hive, Greater Spire, Sunken Colony, Spore Colony).
- Thirteen products: Drone, Overlord, Zergling pair, Hydralisk, Mutalisk,
  Scourge pair, Queen, Defiler, Ultralisk, Guardian, Devourer, Lurker and
  Infested Terran. Larva are selected from their connected Hatchery/Lair/Hive.
- Six technologies: Burrowing, Spawn Broodlings, Ensnare, Plague, Consume and
  Lurker Aspect. Burrowing is offered at all three Hatchery tiers.
- Sixteen upgrade types, including all three levels of the five combat
  upgrades. Overlord research is also available at Hive; flyer upgrades at
  Greater Spire.
- Native burrow/unburrow, Overlord Ventral Sacs loading/unloading, Nydus exit
  construction and passenger transit, Parasite, Broodlings, Ensnare, Dark
  Swarm, Plague, Consume and Command Center infestation.

`tests/zerg_build.h` uses public Drone orders to build every structure, rejects
an off-creep Pool, verifies each Drone is consumed, cancels a Pool and checks
Drone return and native 75% refund, completes all five building morphs and
constructs a paired Nydus exit. A selected Drone then enters via contextual A
and emerges near the paired exit. A building selection stays a selection.

`tests/zerg.h` checks all products and double-unit eggs, all research with
cancel/refund/restart, all upgrade levels and max-level rejection, Egg and
Lurker Egg cancellation/refunds, native burrow flags, Overlord cargo, spell
energy use, two Broodlings replacing a target, a Dark Swarm object, Consume
sacrificing an own unit and actual Command Center type/ownership transfer.
The infestation target has native integer HP between the red damage threshold
and the 50% infestation threshold, so long research tests do not burn it down.

## Fixtures and controller menus

Default scenario 10 is All Zerg battle: each side has 144 prepared troops
across twelve worker/army types, 25 Overlords and a Hatchery, 200 supply and
explicit resources/research. The controller issues native movement, Lurker
burrow, Queen Ensnare and Defiler Plague/Dark Swarm. Larvae, Eggs, Overlords,
fighters and buildings are excluded from army victory scoring. Ordinary
production retains native supply limits; explicit stress fixtures are not
normal opening economies.

Both sides have native supply providers for the full 200 supply; the UI caps
its displayed maximum at the native 200-use limit. The host stress test completes at frame 720: peak 342 native objects, blue
wins, scores 2050/4000, losses 130/121. The 30-second 65%-loss rule, elimination
and 180 simulation/active real-second limit remain unchanged. This is a
prepared performance controller, not Blizzard campaign AI. Zerg/Protoss normal
opening economy bots and campaign missions are not implemented.

Scenario 11 is a manual Zerg lab with completed prerequisites, 20,000 resources
of each kind and initially unresearched techs. Scenarios lists six race
battle/lab entries, Player vs AI, and an Earlier scenarios submenu. No rows
are lost to the eight-row lower-screen limit. Circle Pad/A/B, X/L+X, Y/L+Y,
bookmarks and minimap/command-menu touch retain their existing mappings.

## Graphics, audio and memory checks

Live creep uses original CV5/VX4/VR4 tiles and tileset GRP edge frames through
the existing bounded GPU cache. Original-reference ASan tests validate 48,576
creep pixels, 18,426,789 Zerg body/team/shadow reference pixels and 16,053,103
moving-battle/cache pixels (2,361 misses), with zero cache overflow. The test
also moves the viewport over live creep around both bases. Terran and Protoss
reference/cache tests pass unchanged.

The private exporter covers 193 image types / 12,598 frames. A separate Zerg
bank has 192 complete PCM8 effects/voices, 6,433,618 sample bytes, maximum
130,847 bytes per effect; all three original Zerg tracks stream in full stereo
PCM16. Every packaged sample and track matches independent FFmpeg conversion.
Terran/Protoss banks remain separate and unload before core initialization on
reload, preserving the earlier original-model allocation fix.

A single-engine ASan test performs eighteen successive battle/lab loads
across all three races and verifies state, selection and GRP lifetimes.
Scheduler, input, map rejection/recovery, MPQ ADPCM bounds, audio lifetime,
async CSV and all three race simulation suites pass. Final native compilation
passes ARMv6K/VFPv2 checks, with no Thumb-2 or NEON. Existing renderer limitations
remain: warp/cloak/burrow and palette glow have visual approximations; this is
not a pixel-identical renderer or a complete campaign port.

## Emulator and delivery

Final 25-Overlord build measurement (see zerg-emulator-summary.json and CSV):
mean48.18 FPS, min17.38/max60.17; about23.98 TPS excluding the partial final
sample (23.77 with that sample retained). Peak342 native objects; sampled
heap32312768 bytes, linear7789568 bytes. The first active sample includes a
123.49 ms cold simulation tick; maximum later tick35.49 ms. No backlog,
missing sprites/effect IDs, cache overflow, sound-event queue loss, music
underrun, audio I/O/DSP error or CSV error/drop. Dense combat therefore still
has visible frame-rate dips, despite maintaining the native 24 TPS.

Eight bounded effect voices played207 effects;2984 requests could not obtain
a voice and13 were culled outside the view. Music streamed without recorded
gaps. This is explicit SFX concurrency limiting, not evidence of playing every
requested effect. The emulator was set to original-model/CPU100 and uses DSP
HLE with its existing empty DSP placeholder, not actual console firmware.
No console FPS or physical controller ergonomics are inferred from this run.

Manual native checks loaded Zerg lab from battle, selected connected Larva
and produced a Drone through A/Y (50 minerals charged). Original creep was
visible. Zerg battle→Protoss lab→Zerg lab→Protoss lab reloads also passed. One
earlier automated navigation attempt ended emulation/RPC without a diagnosed
cause; explicit menu-row verification and repetition did not reproduce it.
Emulator stopped and prior model settings restored after testing.

The physical SD is absent. Prepare a private local package with all three
MPQs, both map/terrain packages and the three separate race audio folders.
No new version has been copied onto the physical SD. Preserve the previous
Terran release and the Protoss checkpoint executable locally.

The private local ZIP is local-data/releases/all-races-2026-10-19.zip,
25 files / about344 MB, with owned data-file symlinks dereferenced. CRC checks
and built/staged/emulator/ZIP executable SHA256 equality pass. Exact hashes
are in all-races-artifacts.json. No DSP firmware is included; keep the existing
SD /3ds/dspfirm.cdc. Next performance work should target the dense rendering
path, because simulation TPS and audio streaming remain stable.

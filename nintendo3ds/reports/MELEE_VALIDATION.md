# Selected-map melee, match lifecycle and expanded AI — 2026-10-23

## Behavior

Play Custom now launches the selected SCM/SCX/CHK, including its actual terrain,
resources and native start positions. Sparse start owners are compacted into
the three UI slots; chosen races and Off settings control real player slots.
Extra slots remain inactive. Native melee initialization ignores campaign/UMS
triggers and custom starting armies. Melee and Free for all launch with hostile
active sides. Use Map Settings is explicitly rejected with an error dialog.
The existing StarCraft/Brood War selector remains a frontend campaign choice;
this change does not add a separate classic-only simulation ruleset.

The original-3DS map bound is now 32..128 tiles per dimension. Larger maps
remain previewable but cannot launch. Invalid maps, insufficient/duplicate
starts and loading/terrain-cache preparation errors return to retained Create settings.
Uppercase CHK extensions are accepted alongside lowercase ones.

Terrain is prepared directly from owned VX4/VR4/palette assets on the console,
while audio is unloaded, into a private map-terrain.bin tile bank. Two fixed
512px GPU pages hold a protected 128px minimap and 496 terrain slots. Viewport
tiles are pinned before eviction; only cache misses read tile pixels from SD.
Direct tile-to-slot lookup avoids scanning the entire cache on each draw.
Uploads synchronize preceding GPU work before overwriting textures. Existing
prepacked graphics remain available for laboratory/benchmark scenarios.
Fog textures stay 64px for small maps and become 128px for larger maps; fog
creep memory now covers the full supported dimensions. Linear/heap limits
remain 8/36 MiB and the native target remains ARMv6K/VFPv2/CPU100/24TPS.

Destroying every active enemy building gives Victory; losing the player's
last building gives Defeat. Closed slots do not block victory. Both AIs Off
is a sandbox. Surrender produces a result screen; A on Play Custom or B returns
to Create settings, retaining map/parameters within the session. A second
match can then load without restarting the executable. Result menus also
provide Exit; the original system menu/laboratories remain available.

## AI changes

Replaced the opening-only controller with a bounded melee progression for all
three races. It still uses native purchases/orders rather than Blizzard AI
scripts: no resources, research, buildings or units are granted by the controller.

- Ten-worker target, up to three gas workers, reactive supply production,
  replacement workers, bounded building-placement search and SCV resumption of
  abandoned foundations. Terran factories reserve add-on placement space.
- Terran: Barracks/Academy/Engineering Bay/Factory/Armory/Starport progression;
  infantry, Medics, Firebats, Goliaths, Siege Tanks and Wraiths. Machine Shop,
  native Siege Mode research and distance-based siege/unsiege behavior.
- Protoss: Gateway/Core/Forge/Citadel/Stargate/Robotics/Observatory progression;
  Zealots, Dragoons, Scouts and an Observer.
- Zerg: Pool/Extractor/Den/Evolution Chamber/Lair/Spire/Queen's Nest/Hive/
  Ultralisk Cavern progression; Zerglings, Hydralisks, Mutalisks and Ultralisks.
- Native weapon, armor, range and speed upgrades, one level per type. Production
  composition quotas reserve room for vehicles and aircraft. The controller
  budget is 56 units per side, with Zerg transient larva/pair accounting bounded
  by tests. This does not cap human production or change native unit pools.
- Visible nearby threats trigger defense; attack waves use enemy start positions.
  Target-unit selection does not inspect unseen enemy positions. Active-side
  elimination is determined by building presence.

Remaining AI limits: no expansion-base strategy, transport island invasions,
complete spellcasting/tactical micro or Blizzard campaign AI compatibility.
Higher tiers depend on the map economy and sufficient buildable/powered/creep
space. This is an expanded melee controller, not a complete original AI port.
Mixed matches retain the selected player's audio bank; opponent-specific sound
coverage remains the previous limitation.

## Validation

ASan host runs:

- `test-melee`: actual 128x128 map conversion/loading with sparse start owners,
  real starting bases/workers, terrain bank creation, world/minimap texel
  readback while moving through the full map, full-map fog bounds, explicit UMS
  and 256px rejection, native surrender and victory/defeat conditions: PASS.
- Synthetic 600-tile bank against 496 resident slots: repeated viewport
  eviction with protected minimap, **385560 pixel comparisons**, allocations
  released: PASS. This test supplements native-map terrain pixel checks.
- Each race ran 36000 native frames from 50 minerals, with an invulnerable
  non-fighting human fixture to allow technology progression. Native harvesting
  funded all development. Terran trained36/built17/completed10 upgrade levels,
  Protoss31/22/9, Zerg38/21/13. Peak controlled units56/50/51. Tanks, Wraiths,
  Dragoons, Scouts, Lair/Hive, Mutalisks and Ultralisks were observed: PASS.
- `test-custom-match`: real mixed two-AI production, closed first/second AI slots
  and solo sandbox: PASS.
- `test-selection-info`: selected stats, five-slot queues/progress, exact
  cancellations/refunds, construction/research/upgrade/morph paths: PASS.
- `test-fog`: scouting/revisit memories, fog texture texels, native Scanner
  Sweep and energy consumption: PASS.
- `test-frontend test-command-grid`: persisted profile, settings/random/Off/
  map caps, dialog dismissal, command navigation: PASS.

Native build and ARMv6K/VFPv2/no Thumb2/no NEON check: PASS.

Final exact executable in Azahar, original model, CPU100:

1. Use Map Settings returned an explicit error; settings were retained.
2. Selected 128x128 CHK launched with music, effects and fog.
3. Surrender -> Defeat -> Play Custom worked.
4. Selected a different 64x64 SCX and launched a second match in the same process.
5. Camera pan rendered new terrain; normal Exit completed.

Final source has144 CSV samples,137 with nonzero TPS (including transition
samples). FPS min/median/max59.67/59.83/60.65; TPS10.83/23.93/24.92. Peak sampled
heap34299512 bytes, linear8133632 bytes. Missing sprites, music underruns and
audio IO errors were zero in those running samples. These are opening/flow
checks, not a physical-device or large-battle performance guarantee.

Emulator model settings restored and temporary test map removed. Physical
3DS interaction/performance still needs user testing. No physical SD write.

## Artifacts

Final source: `src-bb5cf4a33b8d25fc`

3DSX SHA256:
`6e994bceb7ca2b53897ab671a451669940606f908be245b526cdf0702808a49f`

Private packages under `nintendo3ds/local-data/releases/`:

- `melee-2026-10-23.zip`:28 files,348652404 bytes;
  SHA256 `aed044c9e508ba924745398714d5e15b0dac727300bc59637faf8f1c75050feb`.
- `melee-update-2026-10-23.zip`:5 files,988039 bytes;
  SHA256 `25223bb4bd9fe96c28808d216277c523cd0b6ed34c9a63f7401e3e121cb0e5d3`.

Full package checks owned input/UI hashes and ZIP CRC. Update package entries
are byte-identical to staging. Matching ELF, source header, host/native logs,
CSV/load journal, emulator summary and package receipts are retained in
`nintendo3ds/local-data/melee-2026-10-23/`. No purchased assets published.

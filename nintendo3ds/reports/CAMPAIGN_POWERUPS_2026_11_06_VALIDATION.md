# Campaign artefact controls — 2026-11-06

Verified source: `src-058bfbddcb3c23ea`. HUD checkpoint: `dc5b0ad`.
Original 36 MiB heap / 8 MiB linear budgets and controller combinations retained.

## Change

Workers now target visible campaign artefacts under an overlapping friendly unit
when A/context is pressed. One explicit pickup command survives a partial path
endpoint: the worker replans toward the same reachable item instead of stopping.
Ordinary Move, collision, beacons and pickup distance are unchanged. Fog still
limits targeting. A carried artefact uses its original image above the carrier
and its icon in the selected worker panel; rendering does not modify game state.

## Actual checks

- ASan retail BW Zerg2: original autonomous SCV/Psi Emitter intro, a single A
  route to the authored rescue, two Drones and original 850/200 resources;
  attached image, Carry info and exact active save replay plus 96 ticks PASS.
- ASan retail SC Zerg2: fog enabled, own Mutalisk reveals/overlaps Chrysalis,
  one A with Drone selected physically collects it, without repeated commands.
  Selection retained, attached image and Carry info PASS.
- Original Egression victory f6460 and exact replay PASS.
- Original BW Reign of Fire paid economy/army/Psi Disrupter victory f32311,
  exact completed replay PASS.
- All 28 existing original campaign victory drivers recompiled against this
  ASan engine and passed. No new mission enabled by this change.
- Eighteen synthetic artefact ownership/type pickup, Bring, death drop,
  second-race pickup and explicit drop cases PASS on the flat arena fixture.
  Initial attempt using Egression terrain failed fixture placement before the
  assertions; the arena run is the relevant isolated-mechanics check.
- ASan selection cancellation/refunds and HUD controls/replay PASS.
- ARMv6K/VFPv2 build and whitespace checks PASS.

Original-model emulator, CPU100: loaded current-source active journals through
real title/pause Load menus. BW Zerg2 SCV visibly carries the Psi Emitter and
shows Carry; SC Zerg2 Drone visibly carries Chrysalis and shows Carry. Screenshots
and logs are private under local-data/powerup-nov06. Audio-ready heap 32,144,456
(BWZ2) / 31,240,456 (SCZ2); linear free 123,904. No physical console or CIA
installation claim. Emulator stopped and original virtual SD/config restored.

## New private builds

Standalone CIA and CCI (.3ds), 598 static private assets including all three HUD
skins and 110 native 60x56 portraits; saves/profiles/logs excluded, no ZIP.
CIA 1,260,180,416 bytes, SHA256
`efa5e625dfe3a7106a068638c764f2f7b650b0eafb5fdf2b14bfc4f7b4d211c8`.
3DS 1,260,167,168 bytes, SHA256
`0fa3f36f0711d166af1c40c9852ed494560b30515aa93bb9c93e71d797464baf`.
Identical NCCH payload and both ExeFS/RomFS superblock hashes PASS.
Private package manifest: local-data/ctr-release-2026-11-06/package-manifest.json.

## Remaining work

Still 28 enabled missions, including three per BW race; this is not a claim that
all campaigns work. Unmodified BW Protoss4 exploration builds a paid base,
defences, Robotics/Support Bay and speed-upgraded Shuttle. Its original PUNI
locks Stargate; do not unlock it to make a test win. A cargo-confirmed Kerrigan
transport still dies to authored anti-air (latest defeat f33633). No P4 victory
or production scenario entry yet. Private pilot/logs retained for continuation.
Pre-existing Zerg wireframe health recoloring remains imperfect. No physical SD
writes or GitHub upload were performed for this checkpoint.

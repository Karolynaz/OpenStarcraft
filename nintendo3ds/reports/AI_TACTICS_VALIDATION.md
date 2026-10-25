# Scout, spell, transport and observer AI stage — 2026-10-25

User requested broader Custom AI and title-accessible observer scenarios.
All production orders, build times, research, energy, loading, movement and
spell effects still use the original OpenBW/native actions. No external bot
DLL or campaign-script interpreter was imported, and normal matches receive
no extra resources, units or technology flags.

## Implemented

- One explicit scout role per side, including flying scouts for islands;
  damaged/threatened scouts retreat and retry. Builders/harvesters/attack waves
  respect tactical reservations. Searches consider possible map starts rather
  than directly selecting the unseen enemy's units.
- Sight-limited aircraft intelligence with expiry; production favors Hydralisks,
  Dragoons or Goliaths when aircraft have actually been observed. Existing
  last-seen building memory guides attacks and airlift destinations.
- One bounded transport mission per side: collect/load, depart, find ground,
  unload/reposition (at most12 retries), attack and return. Uses Dropship,
  Shuttle, or Overlord with paid Ventral Sacs. Ground casters receive cargo
  priority. Loaded/marshalling units are not stolen by ordinary attack orders.
  Slow Overlords have a longer flight timeout. Native unload/collision rules
  remain authoritative; no cargo teleport or placement grants.
- Paid caster buildings/production and selected combat research. Added tactical
  Psionic Storm, EMP, Irradiate, Defensive Matrix, Spawn Broodlings, Ensnare,
  Plague, Dark Swarm and Consume decisions, plus Stim/Wraith Cloak and existing
  Siege behavior. Ground support follows the forward army. Native target/tech/
  energy validation; area spells avoid friendly blobs and duplicate casts.
- Reserve unit headroom for technology/support so a low-tier army cannot fill
  the entire original-3DS budget before transport exists. Limits remain42Easy /
  56Medium, including native actors/subunits rather than raising memory caps.
  Hive/Greater Spire retain the applicable lower-tier upgrade menu parents.
- MatchSetup supports observer control of all3slots, per-side difficulty and
  native alliances. Team victory counts surviving teams: a dead Easy partner
  does not end the game while the other ally still has buildings.
- Title -> AI tests: Terran/Protoss/Zerg island Medium mirrors, plus Terran
  Medium vs allied Protoss Easy and Zerg Easy. Two privately extracted owned
  island maps included. Fog defaults OFF, camera visits all2/3bases plus action
  and loaded transports. End Game returns to title for another test.
- Bottom observer counters: Scout dispatches, accepted Cast commands (not
  guaranteed hits), confirmed dropped passengers. M/E denote each profile.

## Host tests (ASan)

`test-frontend`: PASS, including the four dialog targets and exact teams/races/
individual difficulties. `observer-test`: all4maps/configurations load natively,
all active slots are AI, fog OFF, every base receives a distinct camera focus.
`ai-tactics-test`: hidden aircraft ignored, visible aircraft remembered, stale
knowledge expires; mixed-team victory survives one ally's death. All3native
transports delivered8passengers to a region unreachable by ground. Direct
native casts spent energy for Storm/Irradiate/EMP/Broodlings/Ensnare/Plague.
Isolated mechanics fixtures explicitly seed research/energy/units; those seeds
are test-only and are not used by the shipped observer scenarios.

Three60,000-frame ordinary-resource island economies against a passive,
invulnerable-base opponent passed. No initial tech/army/resource grants beyond
normal50minerals: each builds transport and caster technology and lands troops.
Final observed per-side results (caster/drop/command use is target-dependent):

| Race | Workers | Live at end | Built | Trained | Dropped | Cast commands | Scouts |
|---|---:|---:|---:|---:|---:|---:|---:|
| Terran |14|52|16|28|13|21|33|
| Protoss |14|49|18|29|16|0|7|
| Zerg |14|48|20|46|22|46|1|

Protoss also delivered a High Templar; no safe/eligible storm was chosen in
that passive-opponent run. This is not claimed as spellcasting in every game.
Six36,000-frame original connected-map economy regressions (3races x Easy /
Medium), native harvesting/production/upgrades, terrain/fog and victory/defeat
PASS. Full mixed3AI observer match finished naturally at frame65736 (~45.65game
minutes), winner2 (surviving Easy team), peak189native units. Trained114/44/44,
dropped49/12/20, cast commands56/0/16. No forced victory/timer shortcut.
Logs are in the private stage directory.

## Native/emulator verification

Final source `src-65651930a6229cc5`; ARMv6K/VFPv2/noThumb2/NEON check PASS.
Original-model Azahar, CPU100: booted all4title observer scenarios sequentially;
confirmed no fog, M/M and M/E/E profiles, third-base camera, End Game return,
normal exit. Each retained its4MiB shared SFX cache; sprite misses, audio IO
errors and music underruns0 in all4opening runs. Median59.83FPS/23.93TPS;
recorded minima31.93/39.78/31.91/28.94FPS across the respective runs including
entry/early upload periods. Heap peaks30,664,904 /31,087,352 /30,672,184 /
30,753,544bytes; these are short openings, not physical/full-battle performance
proof. Existing stress dips remain. Screenshot/CSV/load logs and summary saved;
model preferences and Player1 restored after stopping. Physical SD not written.

## Limits and artifacts

Initial tactical controller, not full competitive AI. Counters mainly react to
observed air, not a general strategy classifier. Air routes are straight,
landing retries can fail, evacuation/transport-based expansion is not complete,
and not every native ability is automated (e.g. nukes, Recall, Mind Control).
The2Easy allies have native alliances but no shared strategic coordinator.
Scenarios begin with an ordinary economy; advanced abilities take game time.

Private stage: `local-data/ai-tactics-2026-10-25/`, including matching ELF/3DSX,
source ID, screenshots, all tests, full-game logs and package receipts.
Executable SHA256:
`3ae9399e998ef95624ba44115551d8179bbefeef350b420b6d08be3c1331f171`.
Private release ZIPs under `local-data/releases/`:

- ai-tactics-update-2026-10-25.zip:99files/21,581,942bytes. Includes executable,
  matching UI, previous extra maps/shared audio plus both observer maps and
  observer-maps.json. Needs existing owned MPQs/graphics/race music.
  SHA256 `0aa82cd942067ced422e6ba41bb5033c1c86bb82d5800c54676e5c90249d9c33`.
- ai-tactics-2026-10-25.zip:123files/378,075,811bytes, complete private SD tree.
  SHA256 `b2506447bf273297c0d23d6ee2bfd8cbb02c3f67d6069a8daec92227969aad43`.

ZIP CRCs and staged entry hashes checked; full packager verifies purchased
inputs, maps/audio/UI and the new observer-map manifest. No publication,
physical SD update or scheduled continuation.

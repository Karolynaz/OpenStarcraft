# Twenty-fourth mission checkpoint — 2026-10-30

Source `src-cefee4a794895e2a`. Added BW Zerg2 **Reign of Fire**, original owned
`expzerg02`, scenario39. Campaign continues from BW Zerg1; five-slot labels use
`BW Zerg mission 2`.

## Progression fixes

Three real shared-engine problems blocked the original opening:

- Scripted Order squads collapsed on one exact center, preventing a narrow Marine
  rendezvous. The campaign adapter now uses native group destinations, per owner
  in classic batches of12, preserving eligibility and burrow handling.
- The neutral-slot Psi Emitter entered Neutral instead of PowerupIdle, preventing
  pickup. The six supported campaign artefacts retain their native pickup idle
  when assigned to neutral/rescue controllers.
- Move Location searching a carried item by its original owner did not find its
  worker. Carried-item matching now accepts either carrier or artefact owner;
  ownership itself is unchanged.

The unchanged authored intro destroys the generators, delivers the SCV, picks up
Psi Emitter217 and follows it to rescue the original two Drones and grant850/200.
A public-control driver rescues nearby Hunter Killers, purchases a Hatchery and
normal economy/tech, assembles24 Mutalisks and destroys Psi Disrupter190. Native
original Victory occurs at **frame32311**; completed-save reconstruction is exact.
Fog-off information guides this test. No shipped map, AI, units, costs, funds,
research availability or original trigger sequence is replaced or cheated.

## Validation

- **24/24** original/public-control ASan victory drivers pass on this source.
- All24 openings, active save +48 exact frames, simulation through17248 frames or
  original outcome, original defeat branches, routes, progress/reload, five slots
  and story/briefing/replay/Start presentation pass.
- BWZ2 original defeat is exercised after the intro/rescue, using public orders
  into original enemy defenses; native loss at frame2575. No defeat fixture is
  substituted for this mission's authored state.
- Focused native group Move/Attack comparison, all six artefacts with ordinary,
  neutral and rescue owners (18 cases), and autonomous original BWZ2 intro plus
  exact active replay pass. Prior source fails group/neutral-item negative controls.
- All24 audio banks stream byte-exact through the bounded4MiB fake3DS mixer, IO0;
  BWZ2's30 mission/briefing WAVs independently match BroodWar.mpq StormLib bytes.
- **175** private presentation resources and24 exported maps verified.
- All **32** normalized owned BW map entries audited with this ASan engine through
  120000 frames or original outcome:16 remain running,16 original unattended
  defeats, zero load/tick/ASan errors. This compatibility audit is not victory
  proof and does not enable those unvalidated missions.
- ARM11 build passes ARMv6K/VFPv2/no Thumb-2 or NEON verification.

Original-model emulator reconstructs the **entire32311-frame journal** and shows
Victory. Fresh BWZ2 story, original briefing portrait/speech text, Start/objectives
loading and world were checked. Native deterministic replay plus fresh opening
is not a full interactive handheld playthrough. Replay normal heap32,195,952bytes,
linear free123,904; fresh normal heap32,082,856, linear free25,600. Existing36MiB/
8MiB budgets and console controls retained. Long save restoration and very narrow
linear margin remain known limitations.

Evidence: private `local-data/campaign-twenty-four-validation/`. Emulator STOPPED;
config/profile/progress/all saves/executable byte-restored. Physical SD untouched.
Overall goal remains incomplete: **24 enabled/victory-tested**, including BW P1/P2,
T1, Z1/Z2. BW Terran2 escort and Protoss3 are not yet victory-validated.

## Private overlay

`local-data/releases/campaign-twenty-four-update-2026-10-30.zip`,173,898,907bytes,
SHA256 `c181f7fef961e9f3fbd467c710586d260975f2251bb3565cc3b2f48234983a6b`.

Executable,175 presentation assets and T5/BWP2/T6/T7/Z5/Z6/Z7/Z8/BWZ2 map/brief/audio.
Overlay the existing full fifteen-mission installation or later overlay; not a
standalone install. CRC, hashes, privacy exclusions and executable equality pass.
No profiles, progress or saves included. Start fresh: older-source saves remain
unavailable. Earlier private packages preserved. No external push or physical SD.

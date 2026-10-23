# Selected-unit information and production panel — 2026-10-23

User request: the bottom selected-unit area should show an individual unit or
building's name, native information and current production; expose five queue
places and allow touching an occupied item to cancel that exact order.

## Result

Single selection now shows the original wireframe, name, health and applicable
shields, energy, armor, kills and weapon damage. Supply providers show used,
provided and total supply (maximum 200). Groups retain the previous icon grid.
Active training shows the native product icon/name, native remaining-time
progress and five queue positions. Empty positions 2–5 are numbered; occupied
positions are individual cancellation targets. Existing production commands
add units to the queue. Research, upgrades, morphs, add-ons and construction
also expose current work/progress and an active cancellation target. Enemy
selection and groups cannot issue cancellations through this panel.

Cancellations call OpenBW actions, including exact queue indices and native
refund rules. Rapid repeated head cancellation is guarded until the native
simulation clears the dying current product, preventing an unrefunded queue
removal before the next tick. While paused, that guard can require resuming.

Original wirefram.grp is loaded from the user's existing archives (306897
bytes). It uses the existing bounded UI texture atlas. Damage coloration is
an overall-health approximation, not the original per-part damage pattern.
The panel uses the existing platform font and the previously supplied layout.

## Validation

- `make -C nintendo3ds test-selection-info HOST_CXX=clang++`: PASS under ASan.
  Idle Command Center native HP/supply; five-slot production and progress;
  exact middle/head cancellation and full queue refund; heterogeneous Marine /
  Firebat / Medic queue; rapid repeated cancellation guard; research and upgrade
  cancellation; real SCV construction cancellation with native 75% refund;
  Protoss shields and Probe training; Zerg Larva-to-Egg morph cancellation;
  original wireframe pixels; group/invalid-slot rejection.
- `test-command-icons`: PASS, including command icon mapping and bounded GPU
  atlas lifetime checks (75 battle viewports / 1425 UI draws).
- `test-frontend test-command-grid`: PASS.
- Native build / ARMv6K / VFPv2 / no Thumb2 or NEON architecture checks: PASS.
- Final exact executable booted in Azahar with original-model configuration,
  CPU100. Player vs AI opened with human Terran / Terran and Zerg opponents.
  Visually checked idle Command Center supply view, active SCV production,
  advancing green progress and empty numbered 2–5 positions; normal menu Exit.
  Final-source CSV contains 172 samples, no missing sprites in first/last
  samples. This opening check is not a new large-battle benchmark.
- Touch injection in this emulator setup is unreliable. Shared hit geometry
  and native cancellation actions are tested on host; actual finger interaction
  and text legibility still require physical 3DS evaluation.
- Emulator model preference restored after test. No physical SD write.

## Deliverables

Source ID: `src-867dc906d2e0277e`

3DSX SHA256:
`820ddedd45b3eb5a5be1d655703f094bcd3399a11f94fdb4a3f297d3adffb3f5`

Private local packages under `nintendo3ds/local-data/releases/`:

- `selection-panel-2026-10-23.zip`: 28 files, 348633284 bytes;
  SHA256 `18621dc5eeacf993ca0e2e419dbbe53d53df5c28e1f6cbe2503269de1dbcd81f`.
- `selection-panel-update-2026-10-23.zip`: five files, 977362 bytes;
  SHA256 `e12e228a279ab19fdcc99c363746161fdcc470f30110257da7604389d7961dd0`.

Both archives passed CRC/readback checks. Full package verifies existing owned
MPQs and UI hashes. Update archive entries match staging byte-for-byte. Matching
ELF, build/test logs, emulator CSV/journal and receipts are retained privately in
`nintendo3ds/local-data/selection-panel-2026-10-23/`. No purchased data committed.

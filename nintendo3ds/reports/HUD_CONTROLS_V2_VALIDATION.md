# In-game HUD/control update — 2026-10-25

Implemented the user's supplied `protoss_updated_UI.png`, with geometry from
Figma file w51tTYajO6fKl2DkXjIS0M node28:30. Private assets remain ignored.
The skin is shared for now; no Terran/Zerg artwork redesign.

## Behavior

- Exact 320×240 background; 4+5 command grid and pixel-rounded key badges.
- Clock and MENU only above panels; original static 48×48 unit portrait below
  MENU; enlarged selected stats/native wireframe/production cancellation.
- Aspect-fit minimap centered in 80×100 region, including touch/fog/viewport
  projection. Recent hostile HP loss adds a ten-second blinking attack marker;
  touching the marker centers the attack. Other touches navigate normally.
- Contextual world/catalogue input, fixed nine recipe shortcuts, D-pad/A and
  touch routes share geometry. See CONTROLS.md for the complete mapping.
- Combined/deduplicated mixed-unit powers, prioritized Stim/Siege/Nuke; four
  powers plus Next page if needed, otherwise up to five. Native eligibility
  filtering prevents casting Stim on workers/tanks; original group preserved.
- Empty-selection A drag rectangle, SELECT viewport selection, R+X/Y military/
  workers, L+D-pad store, R+D-pad recall/double center. START pauses.
- Catalogue availability refreshes during mining; disabled items keep slots.
- HUD portrait ID65384 uses existing bounded atlas (no extra GPU portrait bank).
  `portrait_export.cpp` reads owned portdata/idle SMKs; prepare_hud.py converts
  only first frames. 110 portraits, each4608bytes, hashes verified on package.

Resolved conflicts: catalogue L+B chooses fourth recipe, plain B cancels;
catalogue R+X/Y choose recipes while world R+X/Y select units. L+R gives fifth
power/page. Added L+A attack move to fill fourth world movement cell. SELECT
now selects; START/MENU pauses. Unmodified world D-pad has no group action.

## Validation

- Native ARMv6K/VFPv2 architecture verification PASS.
- Input chord precedence/all nine shortcuts/group double recall PASS.
- 4+5 touch/badge bounds and uneven-row D-pad navigation PASS.
- Minimap square/wide fitting, touch margins, camera/pointer separation PASS.
- ASan hud-controls: paid Marine/Ghost/Tank production, real Stim/Siege research,
  mixed ability discovery (including Nuke), unchanged selection/digest after
  catalogue discovery, Marine-only10HP Stim cost, native tank transformation,
  patrol/stop, viewport workers, nine SCV builds, original portrait decode,
  rectangle/worker/ability journal replay and exact continuation PASS.
- ASan selection-info: five-slot queue, head/middle refunds, production/research/
  upgrade/morph cancellation, wireframe/native stats, all races PASS.
- ASan custom-match: native terrain preview, mixed two-AI growth, closed slots
  and no-AI sandbox PASS.
- ASan campaign: real paid Wasteland playthrough/victory(frame7649), hero-death/
  total-loss defeats, campaign save replay PASS. Existing savegame tests cover
  mixed AI/scouts/transports and stress wall clock replay PASS.
- Discovered prior campaign integration regression: non-campaign map UMS
  triggers reset prepared lab funds to50. Non-campaign trigger actions now
  suppressed; custom melee/labs use their adapter rules. Original campaign
  triggers remain active and paid-mission test passes.

Original-model emulator CPU100 final binary: 43-unit Protoss lab, selected
Probe/portrait and build catalogue. Visual review at native400×480, successful
X catalogue, D-pad/A placement, B cancel, SELECT group, START pause/dimming.
61 active one-second samples: FPS59.82–60.63, TPS23.85–24.93; last sample
59.85/23.94. Heap max30,505,224bytes, linear max8,084,480bytes; missing sprites/
atlas overflows0. Evidence is in local-data/hud-v2-2026-10-25.
This is an emulator observation, not a physical console performance claim.
Touch RPC did not deliver presses even to the unchanged title screen; native
GUI fallback also rejected input (noWindowsAvailable). Touch hit geometry and
camera actions are tested on host; physical touchscreen confirmation remains.
Model preferences/player profile/previous save restored after testing.

## Final build and private packages

Source `src-66443a1419587c1c`.
3DSX SHA256 `8c7fdff68230ee5dede07c422789efabfd40e19a0c2e432e258b84def57e0cc9`.
HUD SHA256 `75a3c32313b9b0db7d83544c44d4c389942dad1b64d78dd9011f628a23941dd4`.

- Full236file379937351byte `hud-controls-v2-2026-10-25.zip`:
  `261e41156cf82b47bf4a8f2980f241d754c36c4117c554f529ae4dd34cfcbc83`.
- Update212file36834680byte `hud-controls-v2-update-2026-10-25.zip`:
  `2bb5869f4420284f8dee94746a6ed310ce0128cc5a9e34f029a105fe903ac8b2`.

Packages include new executable/HUD/portraits and prior private campaign/audio
assets; no runtime saves/progress. Merge `3ds` into SD root. No physical SD was
modified. Existing saves remain exact-build-specific. No public upload.

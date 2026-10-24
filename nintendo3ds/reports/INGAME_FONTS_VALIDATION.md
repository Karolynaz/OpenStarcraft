# In-game font test build — 2026-10-24

User explicitly requested larger in-game fonts and a physical test build,
with detailed layout refinement deferred until their feedback.

## Changes

- Top resource counts .42->.56; measured right alignment kept, background21px.
- Clock .34->.48; HUD Menu .38->.52; command names .28/.32->.44/.46;
  system menu .32->.46; result heading .48->.60; action/error message .34->.46.
- Selected title .34->.48 with automatic shrinking removed. Native stats
  .31->.43; current work .29->.43 (one-line ellipsis at fixed larger size).
- HP .25->.38, shields .23->.36, active queue number .23->.36,
  empty queue places .56->.64, progress/cancel hint .26->.38.
- Minor local baselines adjusted for HP/shields and the progress bar; shielded
  wireframe slightly reduced/raised to separate its numeric rows. Portrait HP
  gets a dark strip. The HUD arrangement, icon/touch targets and game logic
  remain. This is a readability/fit test, not a complete HUD redesign.

## Verification

- Native build/ARMv6K/VFPv2/no Thumb2 or NEON: PASS.
- Exact final executable in original-model Azahar/CPU100: standard match boot,
  enlarged resource counts/clock, idle Command Center stats, active SCV queue,
  system menu, Protoss laboratory10000-resource row and Nexus HP/shields
  visually checked; normal Exit completed. Model preference restored.
- Long system menu rows reach the right edge at this size (notably Surrender /
  Play Custom). Kept for the user's requested fit review; no broad relayout.
- Standard match music/effects initialized successfully. Mid-process switch to
  Protoss lab logged audio-off: 'Not enough heap memory for audio'. This run
  does not establish whether that reload allocation issue predates the font
  change. It was recorded without expanding this font-only patch into an audio
  allocator change. Native core/graphics and font rendering remained available.
- ZIP CRC/input/UI hashes pass; six-file update entries match staging exactly.
  No game-logic tests were added or repeated for this font-only change.

Physical legibility/overflow/touch review remains for the user. No physical SD
write. Existing menu metal/animation assets are included in the update so the
user can merge the whole archive over their current SD installation.

## Deliverables

Source ID: `src-0e959a2c62a058b2`

3DSX SHA256:
`5a50edc938fa71a3d0b7161e6e0e4992faccbe74040e63709dbd5c792dae1e23`

Private local packages in local-data/releases:

- ingame-fonts-2026-10-24.zip:29 files,349822463 bytes; SHA256
  `2a005f4dcfd1d5a5b09a3b3288b5da36681169d906094f3f36a87d1ce5b1ed3c`.
- ingame-fonts-update-2026-10-24.zip:6 files,1281939 bytes; SHA256
  `f6267b43eb7de874bf037a7a03e9d7e31380148a3006aceda932b1c895f9731b`.

Matching ELF/source header, native build log, screenshots, CSV/load journal
and package receipts are in local-data/ingame-fonts-2026-10-24. Original data
remain private and ignored by Git.

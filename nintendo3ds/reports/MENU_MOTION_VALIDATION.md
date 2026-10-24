# Campaign menu readability, cursor, animation and transitions — 2026-10-24

The user's two messages were implemented together. The supplied screenshot was
used as a layout reference; original owned assets remain private.

## Changes

- Native menu cursor recolored green, retaining its silhouette/black outline.
- Four146x28 buttons in two rows: Load Saved / Load Replay and Play Custom /
  Game Type. Font scale .55 (previously .37–.40). Every button can highlight
  independently, including the unavailable actions that retain their dialog.
- Race labels .57 (previously .40), episode labels .42 (previously .36).
  Figures are65% of their previous prepared size, leaving room for labels.
- Complete native idle/hover/disk cycles sampled at8fps:13 sequences/319 frames.
  Previously only four initial frames were repeated; those initial frames were
  nearly identical. Hover graphics are overlays over the idle figure, rather
  than replacing it. Both StarCraft and Brood War sequences are included.
- New private ui/animations.bin stores the frame bank. Seven double-buffered
  128px surfaces use448KiB of linear RAM, independent of cycle length. Frames
  stream on demand; all menu surfaces/files are released before gameplay.
  Static menu art shrank from8 pages/4MiB to4 pages/2MiB.
- Play Custom moves campaign panels off opposite edges before custom panels
  enter; Cancel/B runs the reverse sequence. Each phase is380ms, with input
  gated during movement. Returning retains current custom selections.
  Cursor initially rests over the enlarged Play Custom button.

## Verification

- ASan `test-menu-animation`:13 complete cycles,319 frames,1630054 GPU texel
  comparisons, frame variation in every sequence, all14 surfaces released,
  missing-file failure handled: PASS.
- ASan `test-frontend`: enlarged button bounds/default cursor, outgoing and
  incoming phases, motion input gate, Cancel/B return, modal dismissal,
  persistence and existing custom settings: PASS.
- Prepared menu cursor:36 colored opaque pixels all green-dominant; menu/hud/
  animation hashes match the manifest: PASS.
- Native ARMv6K/VFPv2/no Thumb2 or NEON build check: PASS.
- Final executable tested in original-model Azahar/CPU100. Green cursor,
  enlarged text/buttons, idle and hover animation, Play Custom and Cancel/B
  return observed. A match then loaded with terrain, fog, music and effects;
  load journal shows menu memory released before core initialization. Normal
  Exit completed; emulator model and player profile restored.
- Removed redundant within-frame GPU waits: the frontend already begins with
  SYNCDRAW and uses alternating surfaces. Final menu RPC reported56.05 FPS in
  one sample, versus28.04 during intermediate wait-heavy implementation. These
  are spot checks, not a physical-console performance benchmark.

Native visual checks used the Brood War campaign screen. Both versions' complete
sequences and all GPU pixels were checked by the host test. Physical 3DS text
legibility, SD streaming and timing still need user evaluation. No physical SD
write was performed.

## Deliverables

Source ID: src-248f76a3f5e2d2be

3DSX SHA256:
`e1de4471761fb3df334f7aaf747b8370c9346ec85ed69c18b3b334a5a3bf6856`

UI menu SHA256:
`05a925aa5fe6399734297903b4a3d693da192e38d80e3f643542f49bb575c005`

Animation bank SHA256:
`d86bad9014459d36f28735a95768867bbc81b237617cd9aa52f30d8be09f690c`

Private packages in nintendo3ds/local-data/releases:

- menu-motion-2026-10-24.zip:29 files,349818499 bytes, SHA256
  `9ea04a7e97c1390777377879e212bff3a46313da5489432a7799f034e21016cc`.
- menu-motion-update-2026-10-24.zip:6 files,1270308 bytes, SHA256
  `cadf357e5f5cc54f5eafcfb34fd2dcc58c85699b9baf29afc2d2e8b44e9648a1`.

Both ZIPs passed CRC verification; full package validates owned data and UI
hashes, update entries match staging byte-for-byte. Copy the entire update,
including the new ui/animations.bin and matching menu/manifest, not only3DSX.
Matching ELF, source header, logs, screenshots, original emulator preferences/
profile and receipts are retained in local-data/menu-motion-2026-10-24.
No purchased images, frame banks or archives were added to Git or published.

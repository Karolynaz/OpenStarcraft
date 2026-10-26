# Campaign briefing on two screens

Build: `src-83ddecfdb581c29c`.

Objectives and spoken briefing pages now render only on the 400×240 top
screen, in a padded metal frame. Font scale is .65 (previously .46), with
348 pixels of text width and eight rows. Pagination uses those same values;
continuations retain their voice IDs, so page navigation does not restart
the same recording. No briefing asset or campaign rule changes are required.

The bottom screen retains the mission title, chapter arrows, page count,
and framed Back / Next / Begin buttons with their existing touch hit boxes.
It no longer contains an objectives or dialogue text box. This change uses
the existing private menu art; it does not add original briefing-room video
or speaker portraits that the current menu asset bank does not contain.

Validation:

- ASan frontend tests PASS: pagination/content retention/voice IDs, page
  back/next/start, chapter navigation, existing menu interactions.
  Corrected a stale test fixture that expected Terran 3 to wrap to Terran 1
  despite the catalogue now containing Terran 4; added its missing briefing.
- Native build and ARMv6K/VFPv2/no Thumb-2 or NEON verification PASS.
- Original-model emulator: Backwater Station objectives and spoken pages
  visible above, navigation below; A advances, B returns, D-pad LEFT changes
  chapter to Wasteland. Long six-line speech is legible and inside the frame.
  Screenshot: `briefing-dual-screen.png`.
- Emulator touch RPC did not advance the page; no physical touch test claimed.
  Touch rectangles and input routing are unchanged and covered by host tests.
- Emulator stopped; its temporary model configuration restored byte-for-byte.
  Player profile, saves and physical SD unchanged.

Current native executable is in `out/openbw-3ds.3dsx`, copied to the private
staging tree. The older fifteen-mission ZIP remains the previous build;
this UI-only executable can replace it without re-exporting game assets.
Campaign victory suites were not rerun for this rendering-only change.
Earlier pending in-game timer/font/empty-selection HUD requests remain pending.

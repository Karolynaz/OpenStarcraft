# Custom Game settings and three-slot opening — 2026-10-23

Implemented the user's collected layout changes, then the follow-up requesting
separate Cancel/OK panels and original-style slide-in motion. No new Figma
reference was supplied for this stage. The two supplied PNG backgrounds stay
private in local-data, with their native 400x240 / 320x240 dimensions preserved.

Top screen: full-screen native frame over custom_play_bg_top. The real selected
map's terrain preview fits a 208x208 region at (178,16), preserving map aspect
ratio, leaving at least fourteen pixels to the screen edge. The left panel has
native STR/STRx title, description, player count, size and filename. A filename
helps distinguish derived fixtures sharing a map title (arena/probe).
Bottom: custom_play_bg_bottom, one settings panel at (14,35),292x158 containing
Game Type/Map/Player/AI1/AI2, with bidirectional arrows. Cancel and OK have their
own framed 124x28 panels at (18,205)/(178,205). Panel/frame/text groups ease in
from opposite edges over 380ms; hit regions stay disabled until settled.
Circle Pad/A/B retained; D-pad Up/Down focuses rows, Left/Right changes values.

Map discovery includes bounded recursive SD maps subfolders, SCM/SCX/CHK.
Preview reads bounded CHK metadata and native CV5/VX4/VR4/WPE samples directly,
without initializing a game. It supports previews through 256x256 while actual
original-3DS simulation remains bounded to the existing 64x64 test scenario.
Invalid/truncated previews fail with a message rather than entering a match.
Map/type remain frontend-only selections, preserving the user's earlier scope;
slot capacity and race/Off selections configure the actual simulation.

AI choices are native Zerg/Terran/Protoss, Random and Off. Random resolves once
on launch. Player+two computers is the hard limit, even for an eight-slot map;
AI2 is disabled if the selected map has only two slots. Both computers can be
Off for a solo sandbox. Slots 3..11 remain inactive in the loader. Opening AI
now has three independent sides, derives its build order from each side's
native race and targets an active opponent. It preserves legacy two-side
fixtures and their victory behavior; custom victory ignores closed opponents
and does not immediately finish a no-AI sandbox. Third-side green team palette
and atlas keys are distinct from neutral textures. Creep is loaded when any
configured side needs it. The existing selected-player audio bank is retained;
not all opponent-specific sounds are guaranteed in mixed-race games yet.

## Validation

- Native ARMv6K/VFPv2 ELF verification, no Thumb-2/NEON PASS.
- ASan frontend: recursive discovery, profile/version persistence, animation
  endpoints/input gating, both directions of each settings cycle, random/Off,
  two-/three-/eight-slot cap and D-pad navigation PASS.
- ASan custom match: actual 64x64/four-start map metadata and 16,384 opaque
  preview pixels; synthetic 128x128/eight-start preview; malformed-map failure;
  native Terran player vs Zerg+Protoss, all twelve starting workers and three
  bases; 4000 ticks and independent production (10/6 trained, 7/8 workers);
  AI2 closed, AI1 closed while AI2 active, and both closed sandbox PASS.
- Existing Protoss economy 11761-frame battle and Zerg economy 12049-frame
  battle retain the prior outcomes/losses (34/10 and 23/46); native graphics,
  audio-event banks, manual orders and dead bookmarks PASS. The test's 'both
  AIs' assertion now explicitly examines its two fixture players, not the new
  inactive third snapshot entry. No production requirement was relaxed.
- Sprite cache: 8,722,438 RGBA pixels, 25 composition viewports, same pixel
  fingerprint, bounded allocation/freeing PASS. Grid/minimap bounds PASS.

Original-model emulator, CPU100/DSP HLE: verified supplied backgrounds, native
frame positions and an intermediate slide-in image; map switching changes
actual terrain preview; player race cycling; AI2 Off then Random restoration;
separate OK launches; START/Exit closes normally. Final journal identifies
human=1, ai1=2, ai2=2 (Terran vs two independently randomized Protoss sides).
121 active one-second opening samples: mean59.837FPS/23.996TPS, first partial
retained, peak heap33,763,336 bytes, linear8,084,480 of8,388,608. Zero backlog,
cache overflow, missing sprites, music underruns, audio IO/CSV errors and
missing SFX in this opening. This does not establish physical-console or
large mixed-army performance. Touch injection remains a testbed limitation;
shared touch bounds/model are tested, physical touchscreen still needs QA.
Eight menu atlas pages plus a 32KiB preview are released before core loading;
in-game memory remains within the original 36MiB heap / 8MiB linear target.
Original emulator model preferences were restored after normal shutdown.

Exact executable SHA256:
`3d92656211bdfd545c23c3e61c497e73c5c2597e43e9ccdbb0d6087e37fad6d7`
Source `src-5c4330564beb133b`. Matching ELF/3DSX, original config, raw logs,
package receipts and emulator summary: ignored local-data/custom-settings-2026-10-23.
Menu asset SHA256:
`e3010af675d6ff47189ac17d047264192d4072a95207513341cc14d02faef83a`.

Full private package custom-settings-2026-10-23.zip:28 files /348,622,234bytes,
SHA256 f8a4f23d51203d0fd16717451dcb5de9eb09226d04059ca9ec844436f9f82d79.
Compact custom-settings-update-2026-10-23.zip: five files, verified CRCs and
byte-for-byte readbacks. Merge 3ds/openbw into the SD root. No physical SD
write was requested/performed for this stage; no new automation was created.

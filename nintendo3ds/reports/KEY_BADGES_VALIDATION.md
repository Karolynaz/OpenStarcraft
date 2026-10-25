# Readable command key badges — 2026-10-25

Inspected the user's local Diablo3Ds project, without changing it:
`Source/platform/ctr/ui_geometry.hpp` defines 23×23 badge bounds, ui.cpp loads
exported RGBA, and display.cpp overlays it on exact output pixels after palette
conversion. Its default body font is GameFont12; larger font sizes are explicit.
`art/3ds/hud/button_x.png` and button_y.png are user Figma exports.

Reused the original generic X/Y exports and generated matching A/B and 35×23
combination capsules with 14px Arial labels. Checked-in PNGs avoid requiring a
font on the target/build machine. Every badge draws at integer coordinates,
scale1, nearest filtering. No small Citro2D font drawing inside badges remains.
The 13 sprites occupy unused columns324+ of the existing 512×256 HUD page;
the background320×240 is byte-equivalent after5551 conversion and graphics RAM
remains256KiB. Badge touch bounds extended to include their new23px height.

Validation:
- Native ARMv6K/VFPv2 ELF PASS.
- 4+5 command grid navigation/hit bounds including full badge height PASS.
- ASan native HUD/abilities/selection/portrait/replay regression PASS.
- key_badges.py checks13entries, dimensions, pixel-exact5551 conversion,
  dark/bright glyph contrast, original X/Y byte identity and unchanged supplied
  HUD pixels PASS.
- Original-model CPU100 emulator visual review: selected Probe movement and
  power keys, then X build catalogue. All single/combination glyphs visible and
  centered with the existing layout. Native screenshots in private
  local-data/key-badges-2026-10-25/{world,catalogue}.png.
- Emulator settings/profile/original save restored. Physical SD not modified.

Final source src-2fa7384bbd9ea1e1.
3DSX SHA256 5753cdd7f990f0de5496177078a52e5c56d53e0e313e9cd56c224b49f0d862d8.
HUD SHA256 f50cd58e0dc1170dedb70f5a346aadd1e764837850812e13ad37de68011c4d0a.
Private update212files22683568bytes, readable-key-badges-update-2026-10-25.zip:
04391dd647a0f53132a7846c443715e6762400d3d730e8ab48bee812bca2851e.
Full236files379937746bytes, readable-key-badges-2026-10-25.zip:
3973cff8f9e02c67e679fb4b9526630fff2556c495c4a424a155e67e789e5bdf.
Archives verified, no saves/progress, no public publication. Exact-build save
restriction remains. Install executable and HUD together from the update.

# Selected-unit panel — 2026-10-26

Implemented Figma node31:59 (session71d1b353-3fec-465d-9597-ce678feb1682), with
user overrides: retain existing text colors, native64px wireframe, textual
upgrade information instead of upgrade icons. Figma itself places a56px Probe
image; the explicit64px request takes precedence.

Geometry: wireframe64×64 at(17,153), HP green then shields blue side by side at
y219; name at(94,156), stats atx94 with12px line pitch. The five production
cells move with the narrower right column: head94,174,26×26; queued cells
134+26*n,205,24×24. Shared touch geometry updated, native cancellation unchanged.

Typography: actual Roboto Bold font at nominal10px, matching Figma's name font,
pre-rasterized into native glyphs, not a Citro2D scale approximation. Font10px
means a10px typographic size; visible capital glyph ink is typically7–8px.
Existing colors preserved:190/200/245 text,70/240/85 HP,45/110/230 shields.
117 glyph sprites (ASCII white; digits/slash additionally green/blue) fit in
unused existing HUD columns, y112+. Same512×256 one-page256KiB allocation.
No font TTF is bundled; generic raster glyphs/advances are checked in.

Displayed armor/shield/ground/air upgrade levels come directly from native
unit/weapon upgrade types and owner upgrade levels. Damage includes native
upgrade bonuses. Existing kills, energy, supply and production progress remain.
No new upgrade icon graphics. Wireframe palette behavior is unchanged.

Validation:
- Figma screenshot/high-fidelity context read; downloaded background pixel-
  identical to the existing user-provided HUD. Probe remains dynamic GRP data.
- ARMv6K/VFPv2 final ELF PASS.
- ASan selection-info/native stats, five-slot production/refunds, construction,
  research, upgrades, morph cancellation and all-race wireframes PASS.
- ASan HUD mixed powers/selection/portrait/rectangle journal replay PASS; added
  Nexus five-Probe production fixture for visual testing.
- Badge atlas/background tests PASS with117 additional bounded font glyphs.
- Original-model CPU100 emulator: Probe shows full native wireframe, side-by-
  side HP/shields, new font/name/stat placement. Nexus reviewed idle and during
  production: progress and all five queue cells fit, including large HP values.
  Private screenshots under local-data/panel10-2026-10-26.
- Emulator model/profile/original save restored. No physical SD write.

Source src-957604597e5b9ddf.
3DSX SHA256 be2ee5a0def4cb3465baafa82c3bab026957adec679c10707eb26e0e2e5c959a.
HUD SHA256 0825dd6d4ad49593b162d08fcc8bc7273df165d98f61d42f14f1c0e806895cad.
Private full236file379945826byte selection-panel-10px-2026-10-26.zip:
c4e1ea232ce82d75b3256157fc1106bad7507a22caca37d0ca4327ccc2d63bd1.
Update212file22690982byte selection-panel-10px-update-2026-10-26.zip:
d7f3e0f322306b7b7e4bd6e7725c98a43f4d4bf8e7d69be0b696477201dddfb1.
Archives CRC verified; no saves/progress/publication. Install executable and
HUD together. Saves remain exact-source-build-specific.

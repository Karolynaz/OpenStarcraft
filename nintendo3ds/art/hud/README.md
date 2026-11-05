# Lower HUD backgrounds

Approved generated 320x240 race skins, 2026-11-06. These are the image_gen
proposals accepted by the user, not the old four-panel arrangement.
Geometry: Figma w51tTYajO6fKl2DkXjIS0M nodes 128:869 and 128:991.

`tools/prepare_hud.py art/hud <private-original-SMK-directory> <private-stage>`
packs three separate 512x256 RGBA5551 atlas files. Only the selected human
race's 256 KiB atlas is resident. Backgrounds, fixed 10px text and uniform
12px key labels share that atlas. Original 60x56 portrait first frames are
loaded on demand in the existing sprite cache; purchased portraits are not
stored in this source-art directory.

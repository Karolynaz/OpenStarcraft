# Native key badges

X/Y references are the user's Figma-exported 23×23 HUD badges from their
Diablo3Ds project (`art/3ds/hud/button_{x,y}.png`). Its renderer overlays these
at exact output pixels, preserving their legibility. The other generic button
labels follow the same 19px face plus shadow allowance. Combinations use a
35px capsule with a 14px letter size, and are never scaled at runtime.

`tools/prepare_key_badges.py --font /path/to/Arial.ttf` regenerates the generic
PNGs. The font file is not bundled. `prepare_key_badges.py <private-SD-root>`
packs all badges into unused space in the existing HUD texture page.

# Native key badges

All thirteen labels use the same 12px-high capsule, 8px Arial text and
native-pixel rendering. Single buttons are 12x12; chords are 23x12. X/Y use
the same generation path as A/B so their visible sizes agree.

`tools/prepare_key_badges.py --font /path/to/Arial.ttf` regenerates the PNGs.
The font file is not bundled. The original Diablo X/Y reference PNGs remain
as historical source references, but no longer feed the packed HUD.
`prepare_hud.py` installs these badges into each race's single HUD page.

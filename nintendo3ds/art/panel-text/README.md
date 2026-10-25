# Selected-unit panel font

Roboto Bold, nominal size10px, matching Figma node31:93. The source font is
installed locally by the user; only generic raster glyphs and advances are
stored here, not the font file. font10.png contains ASCII32–126 in12×13 cells.
Runtime draws use exact pixels and a12px line pitch, with no font scaling.

`tools/prepare_panel_text.py --font /path/to/Roboto-Bold.ttf` regenerates these
sources. `prepare_panel_text.py <private-SD-root>` bakes white/green/blue glyphs
into unused space in the existing HUD page. Colors match the previous panel:
white190,200,245; HP70,240,85; shields45,110,230. The font is baked in existing
colors over the panel's black backing; there is no extra GPU texture bank.

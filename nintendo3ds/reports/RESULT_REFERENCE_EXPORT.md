# Original result art reference export — 2026-10-29

Added six original640×480 result backgrounds to the private `update image screens/
03 laimejus pralaimejus` directory: Terran, Protoss and Zerg victory/defeat.
The original sources are the owned `glue/pal{t,p,z}{v,d}/backgnd.pcx` resources.
Also exported18 original scorebox/pinset/untab UI fragments, six separate400×240
center-cropped previews and a labelled contact sheet `REZULTATU FONAI.png`.
Native PNG RGB pixels were compared byte-for-byte with decoded PCX pixels; all
PNG files reopen and verify. JSON asset inventory now records source names,
dimensions and original PCX SHA256 hashes. Existing artist redraws are untouched.

The export is reproducible with `result_reference_assets.txt`, the existing
owned-data presentation extractor, and `prepare_result_reference.py`. No game
assets are embedded in source control. The previous placeholder README was
replaced with actual result-art guidance. These images have not yet been wired
into the3DS Victory/Defeat dialog or a custom-game statistics screen.

# Native fog and power controls — 2026-10-20

## Scope

Normal Manual base / Player vs AI / Protoss player vs AI start with native
exploration and player-0 fog. Black means unknown; explored terrain outside
current sight is darkened. START > Fog toggles player vision/full-map observation.
The toggle changes presentation without erasing native exploration or granting
unexplored building sites.
Prepared laboratories and observer stress fixtures retain their already-explored
terrain and default fog off; toggling it there darkens areas outside current sight.

The adapter reads native tile visible/explored bits and native sprite visibility
and unit detection, including undetected hostile cloaking. It withholds unseen
units from the battlefield, minimap, hover, selection and direct unit targeting.
Native movement can still be ordered into unknown terrain. Creep is drawn only
where currently visible while fog is enabled. Own units retain their native
selection/control. Simulation sight ranges, detection rules and 24 TPS are unchanged.

A double-buffered 64x64 RGBA4 mask adds 16 KiB linear memory, shared between
battlefield and minimap, refreshed only on simulation frame changes. Each view
uses one quad rather than per-tile draw calls. Original 36/8 MiB heaps retained.
No extra data files or SD reads during fog rendering. No-fog sprite batching is
unchanged. This first version has tile boundaries and does not retain enemy
building/creep last-seen snapshots; a hidden building disappears instead of
leaving its original ghost. Full palette fog-edge art is later work.

## Powers

The existing complete race powers still use native OpenBW commands, research,
energy and target validation. Unit Y invokes the first power; L+Y invokes the
second when there are exactly two, otherwise opens the extended power menu.
Workers retain basic/advanced build menus and buildings retain production and
research menus. Zero-power units still have the extended movement/order menu.

Targeted powers show their name/energy cost while aiming. Rejected targeted commands keep
the mode active, B cancels; selection changes cancel stale targeting. Disabled
options cannot enter targeting. Power availability checks eligible selected
casters' energy rather than only the first unit. Scanner Sweep requires 50 whole
energy, consumes native energy and reveals native vision for its native lifetime.

## Verification

ASan fog test: a Probe revealed the enemy base after 400 ticks; retreat restored
explored fog after 75 ticks. Hidden unit/image/hover data stays withheld; all
4096 fog mask texels and shared minimap mask match native visibility. All three
unknown/explored/visible states exercised. No graphics allocation leak.

Native Scanner Sweep reveals the target in 1 tick; after exhausting energy,
its option is disabled. Fog returns 174 ticks after the final test cast; this
is the measured remaining lifetime, not a hard-coded duration. Y/L+Y policy
checks cover zero, one, two, three and four powers. 24 cross-map/race reloads pass.

Full Terran/Protoss/Zerg construction, production, research, upgrades and powers
suites pass; prepared stress outcomes unchanged. Common controls, pacing,
scheduler, map rejection/recovery and original cursors pass. All three ASan
cache/reference suites retain their pixel fingerprints and 75 viewport
composition comparisons. ARM11 build validates ARMv6K/VFPv2, no Thumb-2/NEON.

## Separate SD delivery

The user-requested physical SD update earlier this turn installed the verified
cursor release 39607cd... (25 read-back hashes, 15 DSP/log/dump files preserved,
safely unmounted). This new fog/power stage is local only, pending later delivery.
Use the cursor candidate ELF for any new physical-console dump from that card.
Returned prior-performance evidence was backed up, not overwritten.

## Emulator workflow and private release

Original-model / CPU100 / DSP HLE: visually checked player skirmish shroud in
both viewport and minimap, full-map toggle, and explored terrain darkening.
Selected Comsat via Circle Pad/A: Y showed Scanner Sweep name/50-energy target,
B cancelled, Y/A cast consumed native energy and returned to normal cursor.
After energy fell below 50, Y showed an unavailable option instead of targeting.
No errors among the listed image/cache/sound/music/I/O/DSP/CSV counters.

The manual workflow ran on candidate `3d23bc749528441e6accccb7c23192e6c5fb87db40eef9a3cd98028917a7df18` before a
final one-predicate tightening of generic effect visibility. Across mixed
Fog ON/OFF player-mode workflow samples, mean 59.836 FPS /
23.996 TPS (152 samples); lab workflow mean
59.834 FPS / 23.995 TPS (214 samples).
These are workflow measurements, not a matched physical-console benchmark.
Summary: `fog-abilities-emulator.json`; private raw logs and both exact ELF
candidates are retained under `local-data/fog-abilities-2026-10-20/`.

Final executable SHA-256:
`1edfb1a2c630115fb9d5af396e7bed9fdc26eaa1c702819a469ea1aa366a8eef`.
Local `local-data/releases/fog-abilities-2026-10-20.zip`: 25 files,
344061886 bytes, SHA-256 `3c378a7630fe2385440598785df56bc73f3fcae1f0ff9631661f32f39cdd302a`.
Owned MPQ hashes, ZIP CRC and packaged executable verified; previous releases
and current SD cursor-build identity are preserved.

Final executable smoke check after the stricter effect filter: complete Zerg
stress battle ends at frame 720, blue winner, scores 2050/4000, losses 130/121.
Across 29 active samples, mean 57.371 FPS / 23.989 TPS.
Reloaded the player Protoss skirmish: own Nexus/Probes remain drawn correctly;
panning into unknown terrain blacks out viewport and minimap while HUD/cursor
remain visible. No listed errors. Exited normally, stopped emulator and restored
its original preferences. A final native ordinary-economy command test and full
Protoss AI match also pass: frame 11761, losses 34/10, original outcome retained.

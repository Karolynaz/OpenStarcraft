# Shadow, glow and afterimage correction — 2026-10-19

The user's captures exposed three distinct renderer approximations: ordinary
alpha blending darkened Nexus glow pixels, shadow GRP palette indices could
retain body colors in the cache, and native Templar afterimage sprites were
drawn as fully opaque bodies.

Shadow textures now contain only black coverage and retain translucent drawing.
Fire/glow remap colors use additive blending so dark source pixels cannot paint
black discs over a building. The trans50 class uses original source/team colors
at half opacity instead of remapping against black and then blending again.
The native High Templar afterimage sprite retains its original animation and
positions but draws at half opacity. No extra units or artificial trail sprites
are created. Simulation, native shadow positions and control mappings are unchanged.

Opaque texture grouping/depth ordering remains enabled. Translucent effects keep
their native draw order, switch blend state only when necessary, and restore
ordinary alpha blending before UI/minimap/text. The large-view fallback also
restores the blend state. Normal and linear heap limits remain 36/8 MiB.

## Validation

ASan cache tests cover all three races, original GRP/team/coverage references,
75 independently composed complete viewports, and moving battle camera views.
Dedicated source checks cover every exported Nexus glow, Nexus/Probe shadow and
Templar afterimage frame; 234 moving Templar trail images pass style checks.
The tests verify additive/alpha ordering and restoration before drawing the UI.

This remains an RGB approximation of the original destination-indexed PCX
blending tables, not exact palette-index framebuffer composition. Warp-in and
cloaking approximations remain separate work. Physical-console FPS impact has
not been measured; the user's existing SD performance test build is preserved.

## Local release

ARM11 validation passes ARMv6K / VFPv2 with no Thumb-2 or NEON.
Executable SHA-256:
`fd6a1cb0c9cd1412eb9b74d395948fae0fd2916f0721ca550024505fef767b60`.
Matching candidate/ELF and private test logs are retained under
`local-data/effects-2026-10-19/`.

Local `local-data/releases/effects-fix-2026-10-19.zip` contains 25 verified files,
344,051,472 bytes, SHA-256
`466dc305ff184b9a645585a54deae14bf9cefd0cb198208dae1642efa110edec`.
Owned MPQ hashes, ZIP CRC and packaged executable are verified. No replacement
assets are needed for this correction; existing private graphics/audio are used.
The older performance and Protoss economy ZIPs are retained. No physical SD
write occurred during this rendering fix.

## Final executable emulator check

Original-model / CPU100 / DSP HLE emulation completed the Protoss stress match:
frame 720, blue winner, scores 4400/8227, losses 62/41, matching the native host
fixture. Across 29 active samples from the final boot, mean 59.684 FPS /
23.997 TPS. Maximum heap 34,066,936 bytes, linear 7,789,568 bytes.
Audio/music remained enabled; all listed missing-image/cache/sound/music/I/O/
DSP/CSV error counters were zero. Repeated CSV headers delimit earlier runs;
only the final executable boot is included in these measurements.

Reloaded the Protoss laboratory and visually checked animated Nexus glow without
black discs and black ground shadow coverage. Normal UI colors remain intact.
Exited via START > Exit, stopped emulator and restored original model preferences.
Summary: `effects-emulator.json`; private CSV/load logs are alongside the exact ELF.
No physical-console performance claim is made.

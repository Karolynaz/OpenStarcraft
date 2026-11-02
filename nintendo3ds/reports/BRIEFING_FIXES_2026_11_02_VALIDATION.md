# Briefing feedback fixes — 2026-11-02

Source `src-f676e8da74d37355`. User-authorized bundled feedback implemented:

- Story location/title on lower picture bottom-aligned to y232 with readable wrapping, eight-pixel bottom margin, no dark rectangle.
- Speech subtitles remain hidden until NDSP reports the matching voice PLAYING. Their timer starts with playback, including after replay and speaker changes. Missing/unavailable audio falls back to timed readable text rather than hanging.
- A rich briefing utterance stays in one wrapped paragraph, preserving its entire original timing and voice/cast. Overflow scrolls gradually through the same paragraph after a short reading lead-in, with an end hold. No pagination that prematurely replaces the paragraph with its last line. Short replies stay stationary. Original voice ends still gate moving to the next reply.
- Start immediately replaces the upper dialogue with objectives. Bottom Start/Cancel labels and cursor disappear; faces and panels move off their original entry edges over 0.65s. Green centered Loading cycles one/two/three dots. The upper text panel remains fixed.
- The same native 400x240 race text frame is retained in a compact 512x256 loading bank, freed before terrain/audio. The previous scaled 128x128 frame is replaced. Extra230KiB linear is used only during briefing/core setup, not in-world.
- Optional throttled main-thread asset-read progress callback refreshes the loading screen during core initialization. Long individual CPU/IO operations can still briefly delay redraw; no extra worker, heap or linear budget was added. Callback has no effect on simulation inputs/state.

## Extracted private icons

`update image screens/04 command icons/`: all390cmdicons.grp frame IDs, three race console palettes, enabled/disabled =2340transparent36x34PNGs. Original crop offsets retained on the full native canvas. Thirteen existing controller badges, three numbered contact sheets, manifest and README included. Decoded from the owned game's existing independent PCX/GRP reference; no image generation or approximation. All2340PNG dimensions checked. Assets stay ignored/private.

## Checks

- ARMv6K/VFPv2/noThumb2-or-NEON final build PASS.
- ASan all25presentation metadata/timing/replay/Start-exit routes PASS.
- ASan all25native story/briefing/panel/art lifetime checks PASS, including absent early subtitles and absent Start/Cancel during loading.
- ASan frontend/profile/maps/settings regression PASS.
- Dedicated long-utterance playback-start/end timing, preserved full duration and delayed-launch test PASS.
- ASan load-progress/default engine initialization has identical state digest at load and after96ticks;956callbacks observed. This is a loading UI change, not new campaign completion evidence. Existing25mission victory milestone unchanged.
- Original-model emulator preliminary build: bottom story text location, blank pre-speech panel then text, objectives in same native upper panel, Loading below, world launch observed; native capture files in local-data/briefing-fixes-nov02. Fresh BWZ3world heap32069064, linearfree25600. Final loading-callback build tested separately below.

Self-contained private CIA/CCI, noZIP, 559staticassets. Final packages:

CIA1155044288bytes SHA25686f25c99622c5f41068fcea44ec9777dff507427d197797e7f54a17edf09f03f.
CCI1155031040bytes SHA256842010e8cc459c68bdc99cb7aa14a24b2508fd7b8752f74a60c20d3441b6d641.

PhysicalSD untouched. CIA installation itself is not claimed; emulator executes CCI. New-source saves retain the existing source-fingerprint compatibility restriction.

Final original-model CCI src-f676e8da74d37355: story→briefing→objectives/loading→BWZ3world PASS. Audio-ready heap32069096/linearfree25600; screenshot world.png and native logs in local-data/briefing-fixes-nov02-final. Final CIA/CCI NCCH identical SHA2562eb674611b31ef726433f6b1a90dd4eef5b99d38485556871f1f9200684088a4. Emulator stopped; original virtualSD/config restored.

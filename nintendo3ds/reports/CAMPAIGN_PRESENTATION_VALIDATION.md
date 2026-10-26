# Campaign introduction, briefing and objectives

Final native source: `src-d8212ce708a062eb`.

All 15 currently offered chapters have a presentation flow:

1. Establishing shot: owned original background, proportionally resized to
   400 pixels wide and vertically center-cropped to 400×240. Mission/location
   labels are large green text. Lower screen has large green history text on
   black, automatic scrolling, D-pad/Circle Pad scrolling and touch dragging.
   A enters the briefing. Left/right choose a chapter; B cancels.
2. Briefing: upper establishing shot remains, all establishing labels disappear,
   with a 20% black overlay and a padded dialogue panel. Objectives do not appear
   during speech. Original transmission timing, voice and speaker-slot metadata
   drives automatic paragraph progression. Continuations keep the same recording.
3. Lower briefing screen: original race-specific room, original rightmost portrait
   frame mirrored to the left, unmirrored original static speaker portrait, larger
   Start button and Cancel where the old objectives box was. No dialogue box.
4. On completion the upper panel reads `Press (Y) to replay briefing`.
   Y restarts the dialogue and voice without reloading the cached audio bank.
   Start/A or touch Start launches the mission; B/touch Cancel returns to the menu.
5. While loading: lower screen is black with green Loading. Upper shot remains,
   with original objectives replacing the dialogue until loading finishes.

Original campaign prose, establishing-shot labels, backgrounds, race rooms and
speaker portraits are privately extracted from purchased archives. Original
story introductions are used where available. Nine later chapters lacking a
standalone original introduction have short authored context summaries in the
preparation tool. No campaign maps, triggers, AI, sound files or save rules changed.

## Reproduction

Build `tools/campaign_presentation_export.cpp` as a host C++14 tool with `-I.`
and `-Inintendo3ds/include`. Run it with owned-data directory,
`tools/campaign_presentation_assets.txt`, and a private output directory.
Run `tools/prepare_campaign_presentation.py RAW STAGED_ROOT` using Pillow. It
reads the existing normalized campaign CHKs and briefings, verifies their action
counts, and writes 63 private resources plus their hash manifest. Existing private
`portraits/*.bin`, audio banks and UI banks remain required.

`tools/package_campaign_presentation.py STAGED_ROOT EXECUTABLE OUTPUT.zip`
builds and verifies a small private overlay. Full-release packaging also includes
and verifies the presentation manifest. Do not distribute owned resources publicly.

## Tests and limits

- ASan `tests/campaign_presentation.cpp` passes for all 15 real private chapters:
  history/scroll bounds, original objectives separated from dialogue, pagination,
  speaker metadata, timed completion, replay, Start/cancel and launch scenario.
- Existing ASan frontend suite passes; native build and ARMv6K/VFPv2/no Thumb-2
  or NEON verification pass.
- Original-model emulator visually checked the history/scroll, dialogue and
  correct portrait orientation, natural dialogue completion/replay prompt, and
  objective/loading screens. Physical touchscreen was not tested.
- An intermediate build failed to allocate audio because retaining the shot
  fragmented linear memory. Releasing it *before terrain/atlas allocation*, then
  uploading after ndsp allocation, fixes it. Final Terran 2 load logs confirm
  graphics ready, audio ready, zero load failure: normal heap 30,989,208 bytes,
  linear free 107,520 while the temporary shot is retained. The shot is released
  after the warming frame. 36 MiB heap / 8 MiB linear limits remain unchanged.
- To keep the loading shot within original-model headroom it uses a 64 KiB
  256×128 texture mapped to the requested 400×240 display rectangle; crop and
  aspect ratio are preserved. Room texture is released before loading.
- Portraits are original static frames, with no new lip-sync/video decoder.
- This is presentation validation, not a rerun of the 15 full mission victories.
  Campaign AI and later unsupported mission operations are unchanged.

Screenshots: `campaign-story.png`, `campaign-speaker.png`, `campaign-replay.png`,
`campaign-objectives-loading.png`.

Native Y replay was verified after aligning the emulator-only keyboard Y binding
with the test driver's D key. The original configuration is restored afterwards;
no console key bindings were changed. A respects the focused Start/Cancel control.

Private update: `local-data/releases/campaign-presentation-update-2026-10-27.zip`.
1,172,459 bytes, SHA256
`0d741b8bc6bbc44fcb03c4fdbf57da7934e124b92bb8809cc2a8c9bdeb0f3d81`.
ZIP CRC, all 63 resource hashes and exact executable verified. Requires the
existing full private campaign install; no profiles, progress or saves included.

Emulator stopped; configuration (including temporary Y mapping), player profile,
campaign progress and save files restored byte-for-byte. Physical SD untouched.

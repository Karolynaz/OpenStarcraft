# Original briefing layout restored

Source: `src-2dc0b1b024abc416`.

The lower screen is now assembled from the purchased game's `glurdy*.bin`
positions, original race-specific PCX components and original SMK stills. The
complete 640×480 scene is composed first, then uniformly resized once to 320×240.
No panel is moved, flipped, removed or separately enlarged. It includes all four
portrait frames, the empty central dialogue panel, the original objectives panel
and original Start/Replay/Cancel controls. Tutorial-only controls remain hidden
in these normal missions, as specified by the original layout's visibility flags.

Only spoken dialogue text is omitted from the lower panel. Objectives are again
visible in their original lower window, as requested by the latest full-layout
instruction. Original MBRF slot assignments are retained alongside each speech
paragraph, so all visible portraits occupy their original slots; the speaking
portrait uses its original highlight. Faces are not mirrored. Replay is clickable
and also uses Y. Start and Cancel touch regions match their scaled native labels.

The upper dialogue and loading-objective panel again has the campaign screen's
metal frame decorations, restored from the original campaign art. The existing
history / upper dialogue / Y replay / objective loading sequence is preserved.
The 64 KiB shot plus 32 KiB frame are freed before terrain allocation and uploaded
after the contiguous ndsp allocation. Room/portrait textures are released before
loading; the original-model 36 MiB heap / 8 MiB linear budgets are unchanged.

Validation:

- ASan presentation tests pass for all 15 private chapters, including four-slot
  cast metadata across pagination, original Adjutant slot, replay and updated
  Start/Replay/Cancel hit boxes. Existing ASan frontend suite passes.
- All 79 presentation resource hashes checked. Each race's 320×240 room and four
  highlight sprites fit a single 512×256 texture page. Native ARMv6K / VFPv2 /
  no Thumb-2 or NEON build verification passes.
- Original-model emulator visual/startup verification recorded below.
- Original UI movie/portrait resources are currently static frames; no additional
  lip-sync or full original UI movie playback is claimed.

The classic BIN field layout was checked against the primary
[PyMS format implementation](https://github.com/poiuyqwert/PyMS/blob/master/PyMS/FileFormats/DialogBIN.py).
The preparation tool reads only the needed fields and the user's owned data;
no reference implementation was copied into the game runtime.

Private update: `local-data/releases/campaign-original-briefing-update-2026-10-27.zip`.
1,145,476 bytes, SHA256
`7b0547def339e021bcadccfe738b94e8bc5bd5115916a2b6a8b36782f3728020`.
ZIP CRC, all 79 resource hashes and exact executable verified. Overlay on the
existing complete private campaign installation; no saves/progress/profile included.

Original-model Zerg 4 native startup verified on this exact source: original Zerg
room and speaker slots, metal upper frame, objectives/loading, then live gameplay.
Audio ready; normal heap 31,081,384 bytes, linear free 25,600 while the temporary
96 KiB shot/frame are retained. They are released after the warming frame.
This is startup/UI validation, not another complete Zerg 4 mission playthrough.

Original-model Terran 2 visual check also passes: original four-slot layout,
Raynor and Adjutant in their native positions, active portrait outline, original
buttons and objectives, empty central dialogue box, upper metal frame. B cancels;
audio counters: played=5, missing=0, IO=0. Physical touch was not tested.
Screenshots: `original-briefing-terran.png`, `original-briefing-zerg.png`,
`original-briefing-loading.png`. Emulator stopped; config (including temporary
Y keyboard mapping), profile, campaign progress and saves restored byte-for-byte.
Physical SD untouched. No additional campaign victory claims are made.

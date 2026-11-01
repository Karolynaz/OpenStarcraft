# Campaign UI build — 2026-11-01

Source: src-bad96e699308d125.

Removed Cancel/Briefing labels, action bar and invisible button hitboxes from the
pre-briefing story screen. A always enters briefing, including after LEFT input.
B returns as before. Vertical stick/D-pad and full lower-screen touch scrolling
remain available. Horizontal navigation no longer selects hidden actions.

## Actual validation

- Fresh ASan engine: 24/24 existing campaign victory drivers passed. Openings,
  defeat branches, campaign routes/progress, five save slots and presentation
  checks passed. These are controlled host drivers, not full handheld playthroughs.
- Host UI tests campaign_presentation, campaign_story_art and result_screen passed;
  result_stats native completion/mining counters and exact save replay passed.
- Native ARM11 build passed ARMv6K/VFPv2 instruction checks.
- Original-model emulator: fresh BW Zerg2 story without buttons, A into briefing,
  objectives/loading and fresh world observed. Completed Terran5 journal replay
  reached Victory with top statistics and bottom original art/only Ok. Ok opened
  Terran6 story without buttons; LEFT then A entered its decorated briefing with
  new upper-room art, START launched Terran6 world. Screenshots/logs are private in
  local-data/campaign-nov01-validation. The completed journal replay is not an
  interactive campaign playthrough.
- BWZ2 audio-ready heap 32,082,680 bytes; T6 31,368,752. Both had only 25,600 bytes
  free linear memory at audio-ready. Existing 36MiB/8MiB budgets retained, margin
  remains narrow. Result-art-ready heap 26,547,040, linear free 5,415,936.
- Emulator stopped. Backed-up emulator config/profile/progress/all saves/executable
  byte-restored. Physical SD untouched.

## Private test package

local-data/releases/campaign-ui-update-2026-11-01.zip — 176,963,481 bytes.
SHA256: c4858a712dfe592d8dc42a003463b18e66722b9b955df58d4cc5bd90b5e9abb5

211 presentation resources verified by hash/size; ZIP CRC/privacy/executable checks
passed. Includes prior story, briefing and results art changes. Overlay on the
existing full fifteen-mission installation (or later updates); not standalone.
Start a fresh mission: saves from older source builds remain source-restricted.

No additional missions enabled by this UI change. Current 24 victory-tested routes
are not evidence that all StarCraft/Brood War campaigns are complete.

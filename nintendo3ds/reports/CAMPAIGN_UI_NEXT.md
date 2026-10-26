# Campaign continuation — 2026-10-26

Checkpoint source src-d3916d80e599ff23.

- Removed the permanent top-screen campaign title/progress strip. Resources and
  actual scripted transmissions remain.
- Two rapid A clicks on the same own unit (350ms, generation-aware native unit
  ID) select visible same-type units in the400x240 viewport. Larvae are included;
  buildings remain single-select. Different targets, empty ground and other
  controls reset the sequence. The first A selection/drag behavior is retained.
- Appended a new replay operation for same-type selection. ASan test selects
  three larvae and verifies save/replay plus48 deterministic continuation frames.
- Loading/reconstruction/warmup shows black screens and centered green Loading
  on the lower screen. Native GPU preparation still occurs behind the overlay.
- Original-model emulator verified the loading appearance, absent mission bar,
  and a real double A selecting three larvae. Replay of the new group operation
  is separately verified by the ASan test. A native save read raced the GUI
  save request and read the earlier state, so it is not used as group-replay
  evidence. Screenshots/logs are private in
  local-data/campaign-six-validation. All six startup/save/replay/17248 idle runs
  also PASS. ARMv6K/VFPv2 architecture check PASS.
- Emulator stopped; original preferences/profile/save slots restored. No SD write.

Private executable-only update:
local-data/releases/campaign-ui-update-2026-10-26.zip.
Requires the previous complete six-campaign data package. Receipt is private.

Two user-authorized one-time follow-ups are scheduled for2026-10-26 at08:40 and
13:45 Europe/Vilnius. No further self-scheduling is authorized beyond these.

## All-mission inventory

Owned archive audit found60 numbered/alternate map records;14 exceed128tiles on
at least one axis, up to192x192 and128x256. Includes unused/cut maps and branch
variants, so this is not a count of60 canonical campaign missions. Additional
Bonus.scx and Zerg04c–f references need path discovery. All176 referenced AI
script entries exist and their reachable instruction streams decode without
bounds/unknown-format errors. This is a static format audit, not runtime support
or victory verification. Later actions/conditions and AI opcodes remain work.

Private raw+normalized maps and machine-readable inventory/opcode audit:
local-data/campaign-all-audit. Existing six-mission runtime still rejects maps
larger than128 on either axis. Preserve36MiB heap/8MiB linear while investigating
larger-map allocations. Do not label later missions playable from static audit.

# Campaign and CIA/CCI validation — 2026-11-02

Source fingerprint: `src-cec9ff4aa9e3a9e8`. Native original-model ARM11 build passes the ARMv6K/VFPv2 instruction check. Budgets remain 36 MiB heap / 8 MiB linear; no core simulation changes were needed for this milestone.

## New completed mission

Enabled Brood War Zerg 3, **The Kel-Morian Combine** (`expzerg03`, scenario 40), with owned original map, triggers, mission AI, briefing, location, portraits and bounded audio bank. A short authored story summary precedes the original briefing. Victory driver uses normal public commands/costs, a paid colony, Drones and Sunken/Spore defenses. Fog is disabled only for automation navigation; actors, technology restrictions, funds, AI and triggers are unchanged.

Original victory: winner 0 at frame 33071, 10030 minerals banked, Fenix alive, original next map `Zerg04a.scx`. Exact completed-save state digest replay passes. Separate public Fenix movement into the original enemy base reaches original defeat, winner 1 at frame 680. Optional Command Center infestation was not separately proven.

## Actual host checks

Fresh current-engine AddressSanitizer build:

- 25/25 campaign victory drivers pass, including the new mission. These are scripted gameplay completions; they do not establish that every human strategy is correct.
- All 25 opening/active-save replay checks and extended frame 17248 simulations pass. These opening checks alone are not completion evidence.
- Original defeat fixtures for all enabled maps pass. Some pre-existing fixtures deliberately remove required human forces/heroes; the new BW Zerg 3 fixture instead uses public movement and normal enemy damage.
- Campaign route/progress persistence, five save slots and campaign presentation pass.
- 25/25 native story/briefing/art lifecycle and panel motion/geometry checks pass with current staged assets.
- Separate read-only asset root / writable profile/progress/custom-map test passes under ASan.

Two stale fixtures required correction: Start must wait for the existing briefing entrance animation, and deleting player 0 alone cannot kill BW Zerg 3 rescuable Fenix initially owned by player 2. These changes fix the test assumptions, not the original mission rules.

Private raw evidence: `local-data/campaign-nov02-work/regressions/regressions.json`, `checks/checks.json`, `bwz3-final.log`, `bwz3-defeat.log`, native screenshots/logs and package verification JSON. Native build log: `/tmp/sc3ds-nov02-native-build.log`.

## Native package test

Created self-contained `.cia` and `.3ds` using local makerom/bannertool, title ID `000400000F9B3000`. Owned static assets are bundled read-only in ROMFS; saves, cache, profile, progress and logs remain in `sdmc:/3ds/openbw/`. User saves/profiles are excluded from the bundle. Existing homebrew SD asset layout remains a fallback for `.3dsx`.

Original-model emulator booted `.3ds` with an isolated virtual SD containing no game assets. Observed title, fresh BW Zerg 3 story, animated briefing and world; map loaded from `romfs:/openbw/campaign/expzerg03.chk`. Replayed the exact completed journal through the native Load menu and observed Victory, 22:57, 83 completed units, 9 completed buildings, 28 remaining units, 6 remaining buildings, 14205 minerals gathered. This is native save replay, not a full interactive console playthrough.

Fresh BW Zerg 3 audio-ready normal heap: 32069280 bytes; completed replay: 32113288 bytes. Lowest observed fresh linear free: 25600 bytes, still a tight pre-existing margin. Result art release recovers linear free to 5415936 bytes. Physical original hardware has not been tested here.

CIA header/TMD identity, content SHA256, NCCH exheader and ExeFS/ROMFS superblock hashes pass. CIA and tested CCI contain byte-identical 1154322432-byte NCCH content. CIA installer GUI remained unavailable on the locked desktop; installation itself is **not** claimed tested. Emulator stopped, original entire virtual SD tree and configuration restored; physical SD untouched.

## Deliverables — no ZIP

- `out/StarCraft-3DS.cia`: 1154352064 bytes; SHA256 `dc58201889df2a7ca5bcd61e8d7c36cc70b1b4cda6e31f219aff68f35ad87e39`.
- `out/StarCraft-3DS.3ds`: 1154338816 bytes; SHA256 `78a920c7c191e809b72202c306c5b27e916d7896b3f1caffc76bc1cd5ae150df`.

Private content stays local; these packages must not be published. Current staged/out `.3dsx` are identical as well. Older-source save replay remains intentionally rejected; begin a fresh mission when updating from a different source fingerprint.

## Unfinished campaign work

25 enabled, victory-tested missions is **not all StarCraft/Brood War campaigns**. BW Protoss 3 and BW Terran 2 remain unproved; later missions are not automatically enabled. BW Zerg 4a public-control pilot lost after reducing the enemy force, so it is not included as a completed mission. Pilot evidence is retained privately for the next attempt. All-map compatibility simulation from older reports must not be presented as all-map victory proof.

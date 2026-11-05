# Brood War three-mission chapters — 2026-11-05

Source: `src-57d2ac684dbb34be`. There are now 28 enabled campaign entries: SC Terran 7, SC Zerg 8, SC Protoss 4; BW Terran 3, BW Zerg 3, BW Protoss 3. This milestone does not enable every retail mission.

## Added original missions

| Mission | Scenario | Actual final ASan gameplay outcome |
| --- | --- | --- |
| BW Terran 2 — The Dylarian Shipyards | 41 | Original victory, frame 15283; 10 battlecruisers commandeered, final enemy fleet destroyed; exact completed-save replay passes. |
| BW Protoss 3 — Legacy of the Xel'Naga | 42 | Original victory, frame 27971; both Cerebrates destroyed by a paid, Corsair-escorted Dark Templar Shuttle landing; exact completed-save replay passes. |
| BW Terran 3 — Ruins of Tarsonis | 43 | Original victory, frame 135392; four defending Hives destroyed and Duran survives extraction; exact completed-save replay passes. |

Drivers issue ordinary public controls, including normal resource/supply/research restrictions. Fog is disabled for repeatable test navigation. T2 uses a second read-only terrain instance for routing; it does not modify the played instance. No purchased mission map, actors, initial funds, technology restrictions, AI bytecode or triggers are substituted. All 25 prior staged map files are byte-identical to the previous release.

Owned original maps, briefings, mission/hero audio and presentation assets are staged privately for the new entries. Short introductory story summaries are authored adapter text. These three retail maps have no Set Next Scenario action: existing chapter ordering supplies T1 → T2 → T3 and P1 → P2 → P3. Route/progress persistence is tested separately.

## Corrections needed by the missions

- Bring locations fitted exactly to a solid floor beacon count a unit touching its boundary. A one-pixel contact allowance applies only to that exact marker fit. Ordinary location searches and all collision/placement rules remain intact. Removing beacon collision, or expanding every location, changed old mission behavior and was rejected during regression testing.
- Explicit rescue-owner triggers run, including Duran's authored handover. Rescue owners do not inherit Force/All Players trigger queues. Inactive/closed owners remain excluded.
- Custom score and death counters can use explicit inactive player slots. The shipyards' backup-pilot counter in an unused slot otherwise never advanced.
- Targeted spell validation reads its caster's weapon/order, rather than the last sprite-animation context. The focused fixture checks real Yamato damage/150 energy and Lockdown duration/100 energy, including unrelated/null animation context.

No heap/linear budget increases or new threads. Original-model budgets remain 36 MiB heap / 8 MiB linear.

## Final host checks

- Fresh ASan engine: 28/28 original mission gameplay victory drivers pass, including the old Escape from Aiur route unchanged. All three new completed saves reconstruct exact state digests.
- All 28 menu/startup/active-save/extended frame-17248 checks pass. Startup checks alone are not victory proof.
- Catalogue defeat fixtures pass. BW T3 uses public Duran movement into original enemy defenses (defeat frame 1977). BW T2 is checked separately using the complete public platform/fleet controller in `defeat` mode: original all-warships-lost defeat at frame 15128 after capturing ten ships. Some older absence fixtures deliberately omit required forces/heroes; these are isolated tests only.
- Episode progression, atomic progress persistence, five save slots and presentation pass. The opening fixture now waits for the already-existing panel exit animation before asserting launch.
- All four focused ASan contact/spell/rescue/counter fixtures pass. All 28 native story/briefing/portrait/loading-art lifecycles pass. Read-only bundled assets versus writable SD profile/progress paths pass.
- Native ARM11 build and ARMv6K / VFPv2 / no Thumb-2 or NEON verification pass.

Raw private evidence: `local-data/campaign-nov03/final/{regressions,checks,focused}`, `last-preflight/bwt2loss.log`, `validation-summary.json`, `story-art.log`, `asset-roots.log`, `native-build.log`. Tests establish these scripted completions, not every possible strategy or every mission in the full retail campaigns.

## Native package checks

Standalone private CIA/CCI bundle 595 static owned assets in ROMFS. Saves, profile and progress are excluded. No ZIP is created.

- CIA: 1259123648 bytes, SHA256 `93a1811d1663375e31e24384524dca511363f03435fc1ac8bb5881d623d14b26`.
- CCI/.3ds: 1259110400 bytes, SHA256 `fd23eb11ac7acea2560a81e1619c8d4acb67d9a99aa3d97801007eb95015e3cc`.
- CIA/CCI contain identical NCCH bytes. ExeFS and ROMFS superblock hashes verify. See private `package-verify.json` and `ctr-release-2026-11-05/package-manifest.json`.

Original-model emulator uses an isolated virtual SD containing only test profile/progress/completed saves, without game assets. Observed fresh P3 story, animated briefing and world from ROMFS; native P3 completed-save replay reaches Victory 19:25. Native T2 replay reaches Victory 10:36, and confirming it proceeds to the new T3 story/animated briefing/fresh world. Fresh audio-ready heaps: P3 31395464 bytes; T3 31704344. Replay audio-ready heaps: P3 31584640; T2 29961008. Lowest observed world linear free: 25600 bytes. CIA installation and physical-console gameplay are not separately tested.

Native T3 completed-save replay also reaches Victory 94:01 (234 completed units, 21 completed buildings, 50 remaining units, 15 remaining buildings, 20024 minerals and 9466 gas gathered). Its audio-ready heap is 31879760 bytes, linear free 123904 before result textures. This full long replay took about eight minutes on the original-model emulator; journal-based loading remains slow for long matches. These are native replay checks, not full interactive console playthroughs.

The emulator was stopped and the entire original virtual SD tree and configuration restored byte-for-byte. Physical SD was never written. Private screenshots and restored native logs remain under `local-data/campaign-nov03`. CIA installation/physical console are still untested. Saves remain source-build-specific; chapter completion/profile progress persists separately.

# Nineteenth mission checkpoint — 2026-10-29

Final source `src-7000481789ad5afc`. Added SC Terran 7, **The Trump Card**, scenario34,
original owned map `terran08`. Original unused/cut `terran07` remains excluded.

## Original victory and replay

`campaign_terran_seventh.cpp` wins at frame **8466**, preserving Kerrigan. Public
controls lift and relocate the existing Starport, mine real resources, build a
paid Control Tower and three Dropships, pick up the SCV carrying the Psi Emitter,
cast native Defensive Matrix with the existing Science Vessel, fly a threat-aware
convoy and unload immediately upon entering the original goal location.

The driver uses fog-off information for route planning. It does not change money,
units, AI, terrain, triggers, spell energy or original goals. A private failed
strategy also exposed the splash/powerup-carrier crash fixed in commit30ec119.
Original victory, required surviving hero and exact completed-save digest are
asserted. The original map's outcome is used rather than a synthetic win flag.

**Original-model emulator** replayed the entire completed player-control journal
through frame8466 on the final source and displayed **Victory**. This verifies a
native deterministic replay, not an interactive handheld playthrough. Also
verified original story, briefing, in-game dialogue, normal world, pause goals
and the corrected `Terran mission 7` load-slot label. The internal map filename
contains08 because of a cut mission; slot labels now follow campaign order.

Native replay audio-ready normal heap: **31,366,584 bytes**; linear free:
**123,904 bytes** with a4MiB SFX cache. Fresh mission opening had only25,600 bytes
linear free, so longer native memory/headroom monitoring remains worthwhile.
Original36MiB/8MiB limits unchanged. Emulator stopped; config/profile/progress and
all virtual save files byte-restored. No physical SD writes.

## Final checks

- **19/19** original/public-control victory drivers pass under ASan.
- **19/19** menu routes/startups, active saves +48 exact replay frames and further
  simulation to17248 ticks/original outcome pass under ASan.
- **19/19** original defeat branches pass isolated required-force absence fixtures.
- Five-slot test passes independent states, dates, exact replays, backup recovery,
  foreign-build metadata and explicit Terran7 numbering.
- **16** new original mission/briefing WAVs are byte-identical to independent
  StormLib extraction. Private per-mission banks and140 presentation resources
  are verified. No owned data committed or published.
- ARM11 native build passes ARMv6K/VFPv2 verification.

Evidence is private under `local-data/campaign-nineteen-validation/` and
`local-data/campaign-nineteen-final-validation/`. The goal of all SC/BW campaign
missions remains incomplete:19 are enabled and victory-tested.

## Private overlay

`local-data/releases/campaign-nineteen-update-2026-10-29.zip`,80,534,080 bytes,
SHA256 `2032a7d75a81faf41af6ea4aed8c276d4ac66674b4fd3c773cfadcf386ded77d`.

Includes executable,140 presentation resources and original T5/BWP2/T6/T7 mission
maps/briefings/audio. Overlay onto the existing full fifteen-mission install.
Not standalone. ZIP CRC/resource hashes/executable equality pass; no profiles,
progress or save slots included. Old-source saves remain incompatible; begin a
fresh mission with this source. Previous18/hero/expansion ZIPs are preserved.

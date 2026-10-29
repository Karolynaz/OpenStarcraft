# Scenario-created building visibility — 2026-10-29

Source `src-2368fc9ea1bb500b`, following the twentieth-mission checkpoint.

Original BW Terran2 creates control-pad beacons before the human explores them.
`trigger_create_unit` previously called ordinary placement validation, which
rejected every original control-pad location for lack of exploration. The same
locations succeed after revealing them in an isolated owned-terrain diagnostic.
Thus the original mission's three opening beacons were absent.

Scenario creation now explicitly skips the recipient's vision requirement while
checking actual terrain, creep/psi requirements, refinery resource, map bounds,
existing footprints and unit collisions. It does not temporarily reveal terrain.
Ordinary player/AI building placement retains the existing default vision rules.
Newly created units provide their own normal native sight.

`campaign_trigger_building_visibility.cpp` checks seven original control pads,
ordinary vision rejection, script creation without prior exploration, unchanged
remote fog, duplicate-footprint and void-terrain rejection. It then runs the
unchanged owned BW Terran2 opening, asserts all three beacons and verifies an
exact active-save replay plus48 continued frames. ASan PASS. The original-model
emulator loaded that actual600-frame journal and displayed the beacon/world.
This fixture uses generic BW Terran1 audio; it is **not** BW Terran2 audio or
victory validation. Native normal heap29,930,056; linearfree451,584 /2MiB SFX.

All20 enabled original/public-control victory drivers still pass under ASan,
including Zerg5 at60776 with Kerrigan alive and exact completed replay.
All20 original openings/menu routes/active save48/extended17248 and original
required-force absence defeat fixtures pass. ARM11 build passes.

Private evidence is in `local-data/campaign-trigger-vision-validation/`.
The separate twentieth-mission report records its original story/briefing/audio
and native full victory validation. This change enables proper scenario marker
creation; it does not certify the remaining campaigns as completable.

Final validation also replayed the full60776-frame Zerg5 journal to **Victory**
on this source in the original-model emulator. Normal heap31,768,952 and linear
free123,904 /4MiB SFX. The new120000-frame unattended audit again covers64 entries:
62 usable maps without runtime rejection (26 running,36 original defeats), two
unused cut maps rejected, no ASan diagnostics. This is compatibility evidence,
not victory certification for those remaining missions.

Emulator stopped; prior config/profile/progress/all saves/executable byte-restored.
No physical SD writes. Final staged/out executable is source2368.

Private overlay: `local-data/releases/campaign-twenty-trigger-update-2026-10-29.zip`,
99,640,775 bytes; SHA256
`177621b723437a81741c25fe38e8cc5e28a7b981aa55b37bbeeb689cf92d56c4`.
147 presentation resources plus T5/BWP2/T6/T7/Z5 maps/audio. Requires the existing
full15 installation; also overlays later packages. CRC, resource hashes and
executable equality pass. No profiles, progress or saves. Begin a fresh mission;
older-source saves are unavailable. The previous20 source7625 ZIP is preserved.

# Campaign branch infrastructure — 2026-10-26

Source src-b081b7ef2b56e4c4. Original Set Next Scenario action41 now retains the
map-local string ID, exposes the selected filename and includes it in replay
digests. Completion persists a separate route per episode atomically alongside
existing completion flags. A validated original basename maps to the matching
campaign key; paths, wrong race, wrong extension and malformed names are rejected.
Unknown valid future branch keys are preserved for later catalogue expansion.
No branch map is enabled merely because its name has been recorded.
Leaderboard presentation actions19/36 are accepted as presentation no-ops, like
existing leaderboard forms. They do not change objectives or victory logic.

ASan synthetic action41/countdown victory/active-save replay PASS. Filename
validation including BW Bonus and04a-f, per-episode route persistence, preferred
mission selection, reload and failed-write rollback PASS. Existing frontend tests
and all12 startup/save+replay/48 continuation/17248 idle checks PASS. Native
ARMv6K/VFPv2 build PASS. This stage has no native branch-victory claim: actual
branch missions are not enabled yet, and native runtime validation of this new
route persistence has not been performed. No updated test package is claimed.

Broader compatibility audit:43/60 map records reach1200 ticks;6 load and11 tick
rejections remain. Most new runtime failures now reach opcode61/94/105 rather
than being rejected for action41. These are not completed campaign playthroughs.
Private runtime-routes.csv and /tmp/sc3ds-campaign-routes.log contain raw results.
Latest native-tested private release stays campaign-twelve-update-2026-10-26.zip,
source01D. Do not overwrite it with the new backend-only build.

# Campaign powerup carrier splash crash — 2026-10-29

Source `src-306433d01eb4c6ca`.

A public-control playthrough attempt on original Terran 7 (`terran08`, The Trump
Card) exposed a native core exception after frame 26401: `attempt to modify unit
finder while search is active`. Private diagnostic stack tracing localized it to
`bullet_deal_splash_damage -> unit_deal_damage -> kill_unit -> drop_carried_items
-> show_unit -> unit_finder_insert`. Siege splash killed the SCV carrying the
Psi Emitter. Showing the dropped item modified the same finder being iterated.

The splash handler now snapshots its candidate pointer list before applying any
damage, preserving finder order, target eligibility, damage falloff and existing
air-splash selection. Newly dropped objects are not added to the same explosion.
No map-specific overrides, invincibility or resource changes.

Validation:

- `campaign_splash_powerup.cpp`: native SCV picks up an original Psi Emitter in an
  isolated flat-map fixture; native Siege Tank splash kills it and visibly drops
  the item without the exception; 96 further frames pass under ASan.
- The original Terran 7 public-control driver crosses the previous crash and
  reaches the original defeat at frame **46235**. This is a stability regression,
  **not** a win. That mission remains outside the enabled catalogue.
- All 18 enabled original/public-control victory drivers pass under ASan.
- All 18 menu routes/openings, active saves +48 exact replay frames and continued
  simulation to frame17248/original outcome pass under ASan.
- ARM11 native build passes ARMv6K/VFPv2 verification. No fresh native emulator
  test for this source yet; the prior 18-mission package remains the released
  checkpoint until a new private package is prepared.

Detailed private logs in `local-data/campaign-splash-validation/`. No physical SD
writes and no public upload. Campaign goal remains incomplete.

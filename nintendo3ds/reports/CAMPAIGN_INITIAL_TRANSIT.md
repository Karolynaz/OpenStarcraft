# Preplaced airborne buildings — 2026-10-29

Source `src-f90764e80ce4eb0e`. Original Terran8 The Big Push (`terran09`) has
six owned buildings with valid special-state bit4 and In Transit bit4 set. The
loader previously ignored this flag and created all six grounded.

The loader now applies that state to liftable buildings. Shared native liftoff
state setup handles occupancy, movement/collision/finder flags and elevation.
Initial buildings retain original UNIT coordinates; they do not replay the42px
vertical movement of a player-issued takeoff. Native airborne animation/shadow
and building movement speed are initialized. Player liftoff still follows its
existing movement/order path. Non-liftable units and unset valid-state bits are
not forced into flight.

`campaign_initial_transit.cpp` passes under ASan on the original map: six initial
completed airborne buildings; original Starport coordinate; production rejected
while airborne; native movement; exact active save +48 continued frames; native
landing. An isolated validity-mask fixture proves a set state bit alone does not
make a building fly. No original mission map is changed or replaced.

All19 enabled victory drivers pass under ASan after factoring native liftoff,
including the lift/land/paid-production transport missions T6/T7. ARM11 build
passes. Original-model emulator loaded the actual T8 active-flight journal and
showed the selected airborne Starport, shadow, Land command and original world.
Native audio-ready normal heap31,588,568 bytes, linearfree123,904 bytes /4MiB SFX.
This private fixture uses generic SC scenario16 audio; T8 audio/menu is **not**
yet integrated. Do not treat this as a T8 victory or full mission audio test.

Emulator stopped. Config/profile/progress and all saves byte-restored; virtualSD
executable returned to stable19 source7000. No physical SD writes.

A private paid T8 driver successfully transports four starting SCVs off the dock,
lands the base, mines, trains workers/marines/tanks, researches siege and fights
original Confederate forces. It does not win yet: last run reached220000 frames
without an outcome, Duke alive. Supply20/20 at mission start explains why a naive
driver cannot immediately recruit workers; the original transports are needed.
T8 remains outside the enabled catalogue. Original19 source7000 ZIP is the latest
released package; this source is a tested intermediate engine checkpoint.

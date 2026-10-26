# Campaign installation backend — 2026-10-26

Source `src-95f9442b1bb571c4`. Adds native installation doors, hidden weapon traps,
THG2 player normalization, doodad-state trigger action42, command-most/least
conditions, and skipping disabled original trigger conditions. Save digest now
includes order state/signals/timers, sprite elevation and queued orders.

ASan checks: all four original door animation/open/close/collision cycles; neutral
All Players + location/toggle state action; disabled trap causes no damage,
enabled original gun reveals, hits with its real weapon, then hides. Extrema
fixture covers global/location counts, ties, class counts and inactive players.
Disabled Bring with location0 no longer executes. All12 enabled missions pass
startup, active save/replay,48-frame continuation and17248 idle frames.

Fresh unmodified paid-map/public-order victories on this exact source:
T1=7649,T2=17215,T3=43442,Z1=21972,Z2=6460,Z3=21026,
P1=23544,P2=24517,P3=38399,BW P1=10771,BW Z1=13605.
BW T1 automatic strategy loses at46605; no current victory claim.

Original-model emulator: BW Zerg1 briefing12 voices, missing0,IO0; black dual
loading screens/green lower Loading, world startup and no persistent top mission
strip. Normal heap31,754,464 bytes, linear free123,904; original36MiB/8MiB budgets
unchanged. Emulator stopped and model/profile/both save slots restored afterward.
No physicalSD writes and no native complete-mission/performance claim.

Broader audit:59 of64 inventoried records load and reach1200 ticks. This includes
branch/unused records and is not a canonical mission count or completion proof.
Remaining blockers: AI45(nuke_rate),92(guard_all), two unused cut-map formats.
Jacobs Installation loads/runs with doors and traps, but its automatic route has
not reached the second teleport/victory; it is not yet enabled in the menu.
The iscript setspawnframe direction interpretation is independently implemented
from public opcode documentation; byte-exact retail behavior remains unverified.

Purchased maps/audio/logs remain private in ignored local-data; no assets here.

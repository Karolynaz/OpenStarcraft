# Physical controls and observer mode

The application boots straight into Terran AI vs AI on the private prepared
Blood Bath map. No input is required to start the match. Rendering shows the
whole map and both sides; fog is intentionally disabled. Two deterministic test
bots use normal mining, building, production, damage and build times. They are
not Blizzard campaign AI scripts. Each controller limits its own production to
40 live/pending units, six workers and up to 24 Marines. This is a controller
budget, not deletion of excess units or a change to native unit pool limits.

The camera follows bases, moving armies and combat. Circle Pad or minimap touch
suspends following for eight seconds. START opens the bottom system menu, where
following can be disabled completely. CSV logging starts automatically for AI
vs AI at SD /3ds/openbw/logs/ai-benchmark.csv. SELECT pauses/resumes. A winner is
declared when one side has no surviving buildings; the observer pauses there.
START lets you restart or exit. The standard opening takes a few minutes to
produce the first attacks; no resources or production times are accelerated.

Choose Player vs AI in START to control red against the blue test opponent.
Observer mode permits inspection/selection/bookmarks but rejects commands to
the AI armies so input cannot alter the performance fixture.

## Current input map

| Input | Behavior |
| --- | --- |
| Circle Pad | Top-screen virtual pointer and edge scrolling |
| A | Select a friendly unit/building; with a selection, issue its contextual move/gather/attack/repair/resume/rally order; confirm placement or menu choice |
| B | Close menu or cancel pending placement/targeting; otherwise clear selection (does not stop an existing order) |
| X | Select all completed own military units whose centers are on the current top screen |
| L + X | Select all completed own workers on the map |
| Y | SCV: basic building menu; Command Center/Barracks: train SCV/Marine; other units: native primary ability if available |
| L + Y | SCV: advanced building menu; other selections: bottom command/ability menu |
| D-pad in a menu | Navigate choices; A confirms, B closes |
| R + D-pad direction | Save the current selection in one of four direction slots |
| D-pad outside a menu | Recall that direction's selection |
| Same direction twice within 350 ms | Recall and center the camera on surviving group members |
| SELECT | Pause/resume |
| START | System menu: AI vs AI restart, player vs AI, two-Marine probe, auto-camera, CSV, reload current, exit |
| Touch minimap | Move the camera; never move the top pointer or issue a game order |
| Touch an open bottom menu | Activate the touched row |

L/R alone do nothing. L no longer reloads the game and R no longer cycles
scenarios. Touch outside the minimap and an open menu is ignored. No desktop
mouse input backend is used.

Basic SCV construction currently offers Supply Depot (100 minerals) and
Barracks (150). Move the footprint with Circle Pad, A places, B cancels. Green
means the current builder/cost/terrain/visibility/occupancy checks pass. The
native engine rechecks at arrival before spending minerals. The advanced menu
is reserved and visibly reports that advanced buildings are outside this
profile; it does not pretend to offer functioning factories or tanks.

The primary-action route includes native Siege/Unsiege commands with the
engine's technology checks. Tanks, Factories, gas production and their graphics
are not yet part of the playable profile, so that route is not a delivered tank
feature. L+Y provides room for more than two abilities as the roster expands.

Selections and four bookmarks contain up to 64 generation-checked 32-bit unit
IDs; recalling drops dead/invalid entries. Reloading clears them. Multi-unit
orders are issued in native batches of at most twelve, preserving OpenBW's
selection constraints. Buildings remain single-selection commands. Selection
HP/queue fields describe the first live member, with the total count alongside.

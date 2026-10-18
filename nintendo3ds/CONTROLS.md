# Physical controls and observer mode

The default is **All Terran battle**, a prepared symmetric 1v1 on a flat
64×64 arena using original terrain, simulation, weapons, spells and animations.
Each side has 114 troops across all thirteen standard trainable unit types,
plus one prepared nuclear missile: 200 supply per side. Tanks siege, Vultures
lay mines, transports load/unload infantry and Ghosts launch a nuclear strike.
This is a deterministic performance controller, not Blizzard campaign AI.

The battle ends after at least 30 simulation seconds when either side has lost
65% of its initial army resource value, immediately if its army is eliminated,
or at 180 simulation seconds / 180 active real seconds. The higher remaining
mineral+gas army value wins; equal scores draw. Bases are excluded from scoring.
The result pauses the game. SELECT can pause manually; paused time is excluded.

START → Scenarios → **Terran lab** provides full manual production, construction,
research and abilities, with completed prerequisite buildings and 10,000 of
each resource. Its technology starts unresearched. Normal costs, supply, build
and research durations still apply. **Player vs AI** retains the ordinary 50/0
opening against the small SCV/Marine test bot; that bot does not build the full
Terran tree. Gas AI and Marines AI remain available for earlier comparisons.

The camera follows combat. Circle Pad or minimap touch suspends following for
eight seconds; START can disable it. Automatic CSV logging uses
`/3ds/openbw/logs/ai-audio-benchmark.csv`. START → Exit drains the log before
closing. Observer mode allows selection/bookmarks but blocks army commands.

## Current input map

| Input | Behavior |
| --- | --- |
| Circle Pad | Top-screen virtual pointer and edge scrolling |
| A | Select a friendly unit/building; with a selection, issue its contextual move/gather/attack/repair/resume/rally order; confirm placement or menu choice |
| B | Close menu or cancel pending placement/targeting; otherwise clear selection (does not stop an existing order) |
| X | Select all completed own military units whose centers are on the current top screen |
| L + X | Select all completed own workers on the map |
| Y | SCV: eight basic buildings; producers: train/add-on/research menu; units: primary ability or ability menu |
| L + Y | SCV: Factory/Starport/Science Facility/Armory; buildings: research/upgrades/flight/cancel; units: abilities and common commands |
| D-pad in a menu | Navigate choices; A confirms, B closes |
| R + D-pad direction | Save the current selection in one of four direction slots |
| D-pad outside a menu | Recall that direction's selection |
| Same direction twice within 350 ms | Recall and center the camera on surviving group members |
| SELECT | Pause/resume |
| START | System menu: scenarios, auto-camera, CSV, reload current, exit |
| Touch minimap | Move the camera; never move the top pointer or issue a game order |
| Touch an open bottom menu | Activate the touched row |

L/R alone do nothing. L no longer reloads the game and R no longer cycles
scenarios. Touch outside the minimap and an open menu is ignored. No desktop
mouse input backend is used.

SCV Y offers Command Center, Depot, Refinery, Barracks, Academy, Engineering
Bay, Bunker and Missile Turret. L+Y offers the four advanced buildings. Move
the footprint with Circle Pad, A places, B cancels. Refinery snaps to a geyser.
Green means builder/cost/terrain/occupancy checks pass; the native engine
rechecks on arrival. A on an own refinery gathers gas; A on a damaged or
unfinished mechanical unit/building repairs or resumes construction.

Select a production building and use Y to choose any of its standard products
or legal add-ons. L+Y includes research, upgrades, lift/land, unload and work
cancellation where appropriate. Research and all three weapon/armor levels
require the original prerequisites and resources. Disabled rows show `x`.

Tank Y toggles Siege Mode; Marine/Firebat Y uses Stim. Ghost, Medic, Science
Vessel and Dropship expose their abilities as menus. A targeted ability asks
for the top-screen target: move with Circle Pad, A confirms, B cancels. Ground
abilities such as mines and nuclear strikes target terrain even under a unit.
A on an allied transport/Bunker loads the selected eligible infantry. Select
the transport/Bunker and choose Unload to release it. Cloaking/detection and
native weapon sight ranges remain active even though terrain is fully explored
and the observer renders both armies without fog.

Selections and four bookmarks contain up to 192 generation-checked 32-bit unit
IDs; recalling drops dead/invalid entries. Workers temporarily hidden inside
a refinery remain bookmarked and become selectable again when they leave.
Reloading clears all groups. Multi-unit
orders are issued in native batches of at most twelve, preserving OpenBW's
selection constraints. Buildings remain single-selection commands. Selection
HP/queue fields describe the first live member, with the total count alongside.

The audio build plays original Terran music and positional game effects.
SELECT and the end-of-match pause also pause audio; restarting clears old
unit effects. The lower screen shows Audio ON/OFF, effect count, and music
buffer/I/O errors. Automatic logs now use `logs/ai-audio-benchmark.csv`.

# Physical controls and in-game HUD

Current controls (2026-10-25) supersede the earlier Y/build, X/army and
SELECT/pause layouts. The supplied Protoss skin is currently shared by all
races; the command recipes and resource supply icon remain race-specific.

## World controls

| Input | Behavior |
| --- | --- |
| Circle Pad | Top-screen pointer; edge scrolling |
| A | Select when empty; otherwise contextual move/attack/gather/repair/rally; confirm placement or target |
| Hold A + Circle Pad, with no selection | Draw a selection rectangle; release A to select own mobile units inside it (building fallback if no mobile units) |
| B | Cancel target/placement/catalogue, otherwise clear selection |
| R+A | Choose patrol destination, then A targets |
| R+B | Stop selected units |
| L+A | Choose attack-move destination (fourth movement slot) |
| X / Y / L+X / L+Y | First four displayed powers |
| L+R | Fifth displayed power, or next ability page if more than five |
| Worker X / Y | Basic / advanced building catalogue |
| Producer X / Y | Production/actions / other actions catalogue |
| SELECT | Own mobile units on the current screen; building fallback |
| R+X / R+Y | Own military units / workers on current screen |
| L+D-pad direction | Save selection to one of four groups |
| R+D-pad direction | Recall group; repeat within 350 ms to center camera |
| START | Pause and bottom-screen Game Menu; START/B returns |
| Touch minimap | Move camera; aspect-fit blank margins ignore touches |
| Touch blinking red attack marker | Center on the recent attack (marker lasts ten simulation seconds) |
| Touch a command/power | Same action as its displayed shortcut |
| Touch a selected unit in a group | Reduce selection to that member |
| Touch active production/queued item | Native cancellation/refund, including middle queue entries |

A unit group combines and deduplicates available powers by type. Stim, Siege,
and nuclear strike take priority. Native actions filter incompatible members;
selection is preserved. More than five powers use four powers plus a page
button. No desktop mouse backend is used.

## Construction and production catalogues

The same positions always have the same shortcuts, regardless of race:

| Row | Left to right |
| --- | --- |
| Top four | X, Y, L+A, L+B |
| Bottom five | L+X, L+Y, R+X, R+Y, L+R |

These shortcuts apply only while a catalogue is open. Therefore L+B selects
the fourth catalogue item rather than cancelling; ordinary B still closes it.
R+X/Y choose items here rather than changing the army/worker selection.
D-pad navigates the 4+5 grid, A confirms, and every icon/badge accepts touch.
Lists longer than nine use eight entries plus Next page in the ninth slot.
Disabled recipes retain their position. X/Y open basic/advanced worker lists;
place with Circle Pad and A, cancel with B. Refinery placement snaps to geysers.

## Layout and assets

The lower screen uses the supplied 320×240 background and Figma node28:30:
4+5 commands with native 23px single-key badges and 35×23px combination
capsules (following the Diablo3Ds HUD), centered aspect-fit minimap, clock,
60×56 original
portrait, and the selected-unit/production panel. This panel uses native10px Roboto
Bold glyphs, a64×64 wireframe at(17,153), HP then shields side by side at
y219, and the name/stats column starting atx94. Armor/shield/ground/air upgrade
levels remain text for now; there are no new upgrade icons. One portrait is
uploaded on demand into the existing bounded sprite atlas; there is no video
decoder or extra resident portrait texture bank. Private converted assets live
under `/3ds/openbw/portraits/`. They must be installed with the new executable
and `ui/hud.bin`. The top screen shows mineral/gas/racial supply icons and green
numbers; selected-unit names stay in the lower information panel.

## Scenarios and pause menu

Startup opens the title menu. Play Custom uses selected map/races, up to two
AI opponents and Easy/Medium difficulty. START → Options → Test scenarios
contains race laboratories and all-unit observer battles. Labs have prerequisite
buildings and explicit 10,000/10,000 funds, but native costs/research durations.
Campaign map triggers run only in campaign mode, not custom melee/labs.

Observer battles follow combat without fog; pointer/minimap movement suspends
automatic following for eight seconds. Prepared battles end on elimination,
after 30 seconds with 65% army value lost, or after 180 active seconds. Scoring
excludes bases. Game Menu contains Save, Load, End, Options and Return (B).
Options contains test scenarios, auto-camera, CSV, reload and exit. CSV drains
on exit. Both screens dim while the game is paused.

## Saves and replays

START -> Save Game offers five slots under `3ds/openbw/saves/`.
START -> Load Game or title -> Load restores the selected slot. Load tries
that slot's `.bak` file if the primary file fails preflight validation.
B cancels reconstruction and returns to the title menu. Save while paused.

In the title Load list, highlight a slot and press Y to watch from the
beginning. START pauses or resumes the replay. Touch the minimap or move
the cursor to explore. B exits to the title.

The save embeds the starting map, deterministic input journal and camera.
Native simulation reconstructs resources, units, production, research, RNG,
AI plans, transport missions, fog exploration, selection and bookmarks.
Continuing and saving again is supported. Music restarts after loading;
unfinished UI targeting/placement is cancelled.

Saves require compatible simulation rules and the same original game data.
Long games take longer to load because the simulation runs from
the beginning; progress is shown during replay. Limits: 12 game hours,
1,000,000 recorded input events, and a 2 MiB starting map. Session journals
stream to SD with bounded buffers. They are not crash-autosaves. Installation
packages exclude player saves and session journals.


## First campaign mission (2026-10-25)

Title -> Options -> Original -> Terran (Episode I) starts Wasteland.
The lower screen shows the private original text briefing: A/Next advances,
B/Back returns, and Begin starts the mission. Other campaigns remain unavailable.

Find and rescue Raynor, build a Barracks, and reach 10 Marines while keeping
Raynor alive. The original map units, fog, resource triggers, unit restrictions,
victory and defeat conditions are used. Top HUD shows objective counters;
deselect units to see current objectives below. Conversations show subtitles.
Original music/effects and in-mission speech are enabled. Briefing narration, animated portraits,
movies and exact cinematic unit-freeze presentation are not implemented yet.

Save Game / Load Game also work in this mission. Winning records completion
in campaign/progress.cfg and returns to the campaign menu; later missions
are not unlocked yet. This build's saves require this exact executable.
Campaign assets are privately extracted from the purchased StarCraft.mpq:
campaign/terran01.chk and campaign/terran01.brief. Keep both with the build.


Raynor audio fix (2026-10-25): the shared bank now includes both hero Raynor
response sets and Wasteland's five scripted WAVs (SCV, Raynor, Advisor, beeps).
Install the matching executable AND audio/effects-all.bin/effects-all.json.
Existing race music and the fixed 4 MiB SFX cache/eight-voice mixer remain.
Briefing narration and movies are separate pending work. Saves require the
matching source build as before; completion progress is a separate file.


### Lower screen layout (2026-11-06)

The player race selects its own approved Terran/Protoss/Zerg background.
The left panel shows up to twelve selected units in two columns. For larger
groups it shows eleven units and a green count of the remaining selection.
With one unit selected, it displays that
unit name/kills, 64×64 wireframe, HP/shields and up to four native 36×34
upgrade icons. Busy buildings show a cancellable five-slot production grid
in that panel instead of upgrades. Command icons use native 36×34 pixels
in the upper-right four-plus-five grid, with uniform 12px key labels.
The 60×56 portrait is at (122,130), with the clock below it and the aspect-fit minimap in the 96×96 area at (206,130).
Rendering and touch routing share these coordinates. Existing key chords
and catalogue/world input precedence are unchanged.

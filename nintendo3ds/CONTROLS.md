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
| Touch MENU | Same pause menu |
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
4+5 commands, centered aspect-fit minimap, clock, MENU, 48×48 static original
portrait, and the enlarged selected-unit/production panel. One portrait is
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

## Persistent Save / Load (2026-10-25)

START -> Save Game writes slot 1 under 3ds/openbw/saves/slot1.ob3.
START -> Load Game or title -> Load restores it. Previous saves are retained
as slot1.ob3.bak; Load tries that backup if the main file fails validation.
B cancels reconstruction and returns to the title menu. Save while paused.

The save embeds the starting map, deterministic input journal and camera.
Native simulation reconstructs resources, units, production, research, RNG,
AI plans, transport missions, fog exploration, selection and bookmarks.
Continuing and saving again is supported. Music restarts after loading;
unfinished UI targeting/placement is cancelled.

This first disk format is build-specific: keep the matching executable with
important saves. Different builds are rejected. The purchased MPQs must still
be installed. Long games take longer to load because the simulation runs from
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

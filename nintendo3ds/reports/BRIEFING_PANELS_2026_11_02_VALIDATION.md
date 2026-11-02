# Native briefing panels — 2026-11-02

Implemented from user PNGs in private `update image screens/02 briefing/new-brriefing`
and Figma w51tTYajO6fKl2DkXjIS0M nodes110:789 /107:758. These were read through
Figma design context including screenshots; no design screenshot is a runtime asset.

Four portraits use Figma rectangles (x,y,w,h):25,144,73,67;39,20,73,67;
134,20,73,67;222,41,73,67. No mirroring. The active speaking portrait has a red
1px outline with5px corners. Cancel text is centered in123,194,87,14; its hitbox
is113,186,100,31. Removed Replay label and old invisible Replay hitbox; Y still
replays. Start label, hitbox, original idle/hover animation and position remain.

Entry duration0.65s, cubic ease-out: left from-320, right from+320, two upper
portrait panels and top-screen speech panel from-240. Cancel panel enters from
below(+240). Portraits travel with their own panels. Speech/text/audio begin
only once panels settle. Replay leaves settled panels in place. B can abort.

Top text area43,52,340,163 matches Figma. Font reduced0.65->0.60,9rows; both actual
font pagination and rendering share width340/rows9. Loading objectives use the
same area with a title and7rows. Bottom story location overlay removal from the
preceding request remains included.

## Assets and memory

prepare_briefing_panels.py removes edge-connected white matte from the12opaque
bottom exports and unpremultiplies pale silhouette antialias to avoid white halos.
Enclosed highlights/dark interiors remain. Three already-transparent upper-panel
images are untouched. All15assets retain original320x240/400x240 canvas geometry;
lossless silhouette cropping in GPU bank does not alter their screen coordinates.

Each race has one512x512 RGBA5551 panel atlas(524612bytes including metadata),
released before mission core allocation. Existing32KiB loading frame replaced
by small version of the supplied top panel, without increasing retained loading
texture memory. Existing background rooms/portraits/Start movies reused. Six new
panel bank/layout resources are in private presentation-manifest.json. Full asset
export tool accepts --panels SOURCE; individual prepare command documented below.

```
/usr/bin/python3 nintendo3ds/tools/prepare_briefing_panels.py \
 'update image screens/02 briefing/new-brriefing' \
 nintendo3ds/local-data/3ds/openbw \
 nintendo3ds/local-data/briefing-panels-validation/layers
```

## Actual validation

- ASan host campaign_presentation: all24story/briefing inputs, delayed speech timer,
  entry Start gating, new Cancel/removed Replay hitboxes, Y replay, shared new font
  pagination and campaign launch behavior PASS.
- ASan campaign_story_art: all24mission lifecycles, exact offscreen/settled layer
  positions, native top panel, portrait73x67 orientation/geometry, only Start+Cancel
  text below, release-to-two-small-textures and full cleanup PASS.
- All15PNG dimensions/nonempty silhouettes/transparent outside PASS. All3upper
  panels decoded pixel-identical to source. All3atlases: single512x512page/5entries,
  every-pixel RGBA5551 decode and manifest bytes/SHA256 PASS.
- Native source src-92e9137f5b68570c built ARMv6K/VFPv2 with noThumb2/NEON PASS.
- Original-model emulator: BW Protoss1, BW Terran1 and BW Zerg1 new briefing layouts,
  animated portraits and Start art observed. Zerg Y replay exercised; Start loaded
  actual BW Zerg1 world. Protoss screenshot precedes matte-edge cleanup; Terran and
  Zerg screenshots use final cleanup. No campaign victory claim from this UI check.
- Briefing audio-ready linear free648192(Protoss),656384(Terran/Zerg); BWZ1 after
  releasing panels core-start5579776; game audio-ready heap31777576 and linear free
  25600. Existing36MiB/8MiB limits retained. Final gameplay margin remains narrow.
- Emulator stopped; every modified virtual-SD file and config byte-restored.
  Physical SD untouched. Screenshots/logs in local-data/briefing-panels-validation.

Native out executable and staged executable match; no new release ZIP produced.
The November1 release ZIP still contains the prior UI/assets. New executable
requires updated private staged panel banks. This UI change adds no missions.

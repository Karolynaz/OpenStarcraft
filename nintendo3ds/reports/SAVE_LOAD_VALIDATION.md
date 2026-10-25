# Persistent Save / Load — 2026-10-25

Build: src-a3274172bb26d242
3DSX SHA256: b5ddee234af6abbae02ecf34cedcf31b3c2939a1b4d0a32a29d749fd88a36e77

## Implementation

Native OpenBW has an in-memory state copier, but no portable disk serializer.
This initial implementation embeds the original map and replays frame-indexed
player commands into the deterministic native engine. AI remains frame-driven;
stress scenario wall-clock events are journaled separately. Nested command
helpers generate one outer event. The journal streams to SD with fixed buffers.

Header/payload checksums, source identity, sizes, settings, event ordering and
exact EOF are checked before replacing the current simulation. Reconstruction
must match the saved simulation digest before it becomes playable. Save writes
and validates a temporary file before rotating the previous slot to .bak.
Load tries the backup when primary preflight fails.

One save slot; exact-build compatibility; 12-hour/1-million-event/2-MiB-map
limits. Loading cost grows with simulated duration. Music restarts and transient
targeting UI closes. B cancellation during reconstruction returns to frontend;
a preflight failure leaves the existing native state intact. Session files
are not crash recovery. This replaces previous unsupported Save/Load dialogs.

## Validation

ASan host savegame test:
- Paid SCV queues and cancellation, movement, selection, groups, fog and camera
  round-trip; 300 subsequent frames match digest frame by frame.
- New commands after restore and a second save/load; another 100 frames match.
- Previous slot backup exists; corrupt payload, truncated header and wrong-build
  files rejected; initial and in-replay cancellation handled.
- Three-race Medium-vs-two-Easy observer economy on the owned three-island map:
  save at 20,000 frames, reconstruct and compare every frame for another 720.
  Includes native RNG, tech, units, AI counters/targets and fog in state digest.
  Digest is an explicit broad state check, not an exhaustive serializer.
- Prepared Terran battle wall-clock journal round-trip and 60-frame continuation.

ASan frontend test: title Load sets the load request; existing navigation,
animation, settings, map list and AI observer setup checks pass.
Native build: ARMv6K/VFPv2, no Thumb-2/NEON check passes.

Original-model Azahar, CPU100, purchased private data:
- Play Custom -> human Terran vs AI; START Save creates a 32,597-byte slot
  at frame27 (final build; earlier iteration also verified frame521).
- Resume then START Load reconstructs successfully and gameplay continues.
- Stop and restart emulator; title Load reconstructs the same slot, continues
  to frame49, and START pause works.
- Logs show core/terrain/audio ready after both loads. Heap after restoring
  at most30,699,736bytes; linear8,084,480bytes; no sprite misses, music underruns or
  audio IO errors in captured samples. These are short functional checks,
  not physical hardware or full heavy-battle performance results.
- Model/profile preferences restored after testing. No physical SD installation,
  publication, purchased asset redistribution or automation.

Private evidence: local-data/save-load-2026-10-25/ (ELF/3DSX/source identity,
slot/session copies, logs, screenshot, test outputs). Release archives exclude
runtime saves. Physical console testing remains with the user.

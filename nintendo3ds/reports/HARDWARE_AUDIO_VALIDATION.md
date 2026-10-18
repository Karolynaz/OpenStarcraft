# User-console audio benchmark — imported 2026-10-19

The user completed the battle with original effects/music, exited through START
→ Exit, and mounted the SD for retrieval. Its ai-audio-benchmark.csv was copied
without modification to ignored local-data/hardware-captures/2026-10-18-audio.csv.
The machine-readable summary and a final 120-active-sample excerpt are tracked
as hardware-audio-summary.json and hardware-audio-samples.csv. The console model
has not been independently identified.

| Same Blood Bath Marines fixture | Without audio | With audio |
| --- | ---: | ---: |
| Active samples | 548 | 548 |
| Mean FPS | 58.668 | 58.597 |
| Mean TPS | 24.016 | 24.015 |
| Maximum simulation tick, us | 7132 | 7134 |
| Peak normal heap, bytes | 24,734,264 | 26,518,224 |
| Peak linear allocation, bytes | 6,355,968 | 7,502,848 |
| Peak objects | 94 | 94 |
| Winner/frame | Blue / 13200 | Blue / 13200 |

Means retain startup, catch-up and partial intervals. Zero-TPS pause/terminal
samples are excluded and counted explicitly by the summary tool. The observed
FPS difference is 0.071 (about 0.12%); this single run does not isolate the cause
of such a small difference. Simulation sustained its target rate.

Every recorded audio sample reports ready. All three tracks were streamed,
1951 effects played, 3653 off-camera effects were culled, 159 were rejected by
the eight-voice priority limit. There are zero missing effects, event queue
drops, complete-queue music underruns, I/O errors or DSP dropped frames. Voice
saturation is separate from music-buffer starvation. These counters are not a
subjective listening test.

Both audio/no-audio runs have a single 47-tick debt spike immediately after the
first CSV sample, followed by catch-up to zero. It occurs at frame 25 without
audio and frame 26 with audio; all later samples report zero debt. It therefore
predates audio integration. The consistent location suggests initial SD log
write/commit latency; the logs do not time the write and cannot prove that
cause. The next build explicitly flushes the header during loading, starts its
simulation clock after the first completed GPU present, and records the prior
CSV flush duration. It retains all scheduling debt during normal play.

For this fixture, audio is not a performance blocker. This does not establish
performance for a larger map, more units, advanced effects or campaigns.

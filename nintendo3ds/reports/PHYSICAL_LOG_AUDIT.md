# Physical SD evidence and source identity (2026-10-20)

The CSV returned with the later cursor delivery is byte-for-byte identical to
the CSV preserved before the performance delivery: both are 179509 bytes,
SHA256 a091c027100469b072bb5a95e4c8e5f1b8a5cb2d1dc56388baa90ced4c47ba91.
Its sessions have 34-, 37- and 46-column headers. None has the newer render
phase fields. The previously installed 1660ed... executable contains those
phase fields, so this returned file supplies no newly recorded performance-
build session. This does not establish which executable the user actually
ran, whether logging was enabled, or whether another location received logs.

Do not describe these old all-race results as measurements of the newer
performance/cursor/fog/UI builds. In particular, selecting only samples with
TPS >=20 produces a biased FPS mean and must not support a performance claim.
A fresh physical-console run is still needed to assess current FPS gains.

Native builds now generate a source-input fingerprint from core/adapter C++
headers and sources, the build recipe/specs and the fingerprint tool itself.
It excludes purchased game assets, absolute user paths, documentation and the
generated header. The marker appears in diagnostics, every CSV row and the
scenario-load journal. It identifies source inputs; exact executable SHA256
and matching ELF remain in each private release record. The CSV summary tool
reports source markers and labels older records as unidentified.

The current quota window was checked once for work planning: it resets at
2026-10-20 07:27:34 UTC / 10:27:34 Europe/Vilnius. No reset credits were used,
no model/account changes made, and no automatic continuation was created.

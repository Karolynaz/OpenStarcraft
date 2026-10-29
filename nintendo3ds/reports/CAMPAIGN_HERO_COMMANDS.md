# Campaign hero commands — 2026-10-29 07:30 continuation

Final source `src-38de8aa4a2d77845`. No campaign catalogue additions in this checkpoint.
The enabled seventeen original mission victories were rebuilt and rerun under ASan
against this final source; all seventeen pass. Raw logs are private in
`local-data/campaign-hero-final-validation`.

Hero commands now reuse the corresponding native validations: Tassadar's Storm
and Hallucination (without an Archon merge button), Matriarch, Unclean One,
Danimoth, Raszagal, Raynor/Gui Montag, Duke, Raynor Vulture, Tom Kazansky,
Magellan, the hero battlecruisers and Infested Duran's Consume. Infested Kerrigan
has Storm, Ensnare, Consume and Cloak. Map-specific technology restrictions and
energy costs remain; Amerigo still correctly disables Storm. Hero names replace
the former generic Unit label, including the newly supported casters.

Kerrigan's native cloak/decloak path was missing. The exception is scoped to the
Personnel Cloaking technology/action, rather than classifying her as a Ghost.
She does not gain Lockdown or nuclear targeting. Native hero spell checks prove
cloak visibility and energy drain, decloak, Ensnare's actual timer, Consume's
actual allied-unit loss/energy gain, and Tassadar Storm's actual damage. These
are synthetic isolated arenas using owned DAT/iscript data, never shipped map
changes. Seventeen command alias cases and a cloaked-Kerrigan active save plus48
identical continuation frames pass. No additional test-only API was added.

Native ARMv6K/VFPv2 build passes. Original-model emulator loads the isolated hero
fixture through slot5, restores the selection and cloak, and displays the actual
Infested Kerrigan name, four ability icons/key labels, energy and portrait. Normal
heap30,853,416 bytes, linearfree173,056. The fixture is explicitly not a campaign
victory or a new playable mission. A normalized emulator touch attempt did not
establish a reliable input result; native touch is not claimed verified here.
Emulator stopped and configuration/profile/progress/all saves byte-restored.
No physical SD writes. The36MiB/8MiB limits and console bindings remain unchanged.

Private overlay `local-data/releases/campaign-heroes-update-2026-10-29.zip`,
41,757,747 bytes, SHA256
`e92d965b8d1827ca0efef106149ebe4f94442cf0d88fa0e5967ddc7f5ddbbf0a`.
CRC/126 presentation hashes/exact3DSX pass. Overlay on the existing full15 or later
private install; includes T5 and BWP2 data. No profiles/progress/saves. Earlier
source-incompatible saves remain unavailable; profile/progress remain preserved.

All-campaign completion remains unfinished. Private P5 and Z5 public-control
pilots have not won and are not added to the menu. Their strategy failures are
not treated as engine defects. BW Zerg6 AI111 remains unsupported pending a
resource-area/town implementation with meaningful expansion tests.

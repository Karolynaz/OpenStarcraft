# Play Custom readability and original metal mounts — 2026-10-24

User requested larger fonts in the shown Play Custom window and original
metal supports around buttons/dialog, matching the campaign menu style.

## Changes

- Top map title .48->.60, description .37->.47, player count .41->.52,
  dimensions .37->.47 and filename .28->.37.
- Bottom heading .53->.60; field labels .38->.50; values .38->.52
  (map name .30->.52); arrows .48->.58; Cancel/OK .49->.59.
- Readable font sizes remain fixed. Title is limited to two lines, description
  to six, and long field values/filename use ellipsis. Measurements use a
  separate short-lived text buffer, so repeated fitting cannot exhaust or
  clear the rendered text buffer.
- Settings panel uses original pSlots/pListMap2/pInfo metal beams and corners,
  rather than a compressed full-screen border. Cancel/OK use the original
  asymmetric PalNl metal mounts, oriented to support the two footer buttons.
  No generated/replacement art; purchased source images remain private.
- Panel24..190, field rows60/85/110/141/167. Footer hit boxes124x28 at
  x18/178,y196; metal continues below the button field to the screen edge.
  Drawing and input coordinates were changed together; hover and side-slide
  behavior remain. Static atlas stays at four512px pages.

## Checks

- ASan frontend test: PASS, including shifted fields, Cancel/OK hit areas,
  non-clickable mount underhang, existing transitions and custom settings.
- Native ARMv6K/VFPv2/no Thumb2 or NEON build: PASS.
- Final source in original-model Azahar/CPU100: normal Blood Bath preview,
  enlarged settings/metal, arrows, Cancel/A, B return and frontend Exit checked.
- Temporary preview-only fixture with a long title and repeated description:
  two-line title, six-line description and ellipsized bottom map value stayed
  within their areas. Screenshots retained privately. Temporary map removed
  and original emulator model restored after the test.
- Full package owned data/UI hashes and CRC: PASS. Six-file update archive
  CRC/readback against staging: PASS.

No gameplay/AI changes. Physical 3DS font/touch evaluation remains for the
user; no physical SD write.

## Artifacts

Source ID: `src-d4e7d4454ac23955`

3DSX SHA256:
`da14ffe71c43177f8f65153563863f329f833f1480b94de5e7df5866c5c86192`

Menu SHA256:
`54c5b3172195c9fa5260872512ba9bc21c745a7206af313bfa97d1464eaea84b`

Private packages in local-data/releases:

- custom-metal-2026-10-24.zip:29 files,349821426 bytes; SHA256
  `d6a57fc3d415ab42131db0fa9949d41173d66681fcc255ee9dd289977839571c`.
- custom-metal-update-2026-10-24.zip:6 files,1281498 bytes; SHA256
  `e2cb74f5309ac10a02e36d8dbe9703ce49a2ab6b0c9b46bb06c29e6b2902754b`.

Matching ELF/source header, screenshots, asset/build/test logs and package
receipts are in local-data/custom-metal-2026-10-24. Copy the entire update
with matching menu/manifest/animation assets. No original data published.

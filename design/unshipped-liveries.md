# Liveries not in v1.0

Four kits are written and working but held out of the first release: the three bike
brands need more thought before they are as considered as the seven that shipped, and
Espresso reads too close to Zwift on the wrist (black field, warm accent, dark
secondary) to justify a slot of its own.

The definitions below are current and drop straight back into the `LIVERIES` table in
`src/c/main.c`. Each also needs its twin in `src/pkjs/preview.js`, an option in
`src/pkjs/config.js`, and a `SETTINGS_VERSION` bump so old indices do not silently
point at a different kit.

## What each still needs

- **Specialized** — red-on-black with a chrome-yellow last hour. The accent is off-brand;
  their identity is red, black and white only, and the fourth colour was a compromise to
  keep the flamme rouge distinct from the day track.
- **Trek** — white field, team blue, red flash. Closest to shippable. The vertical-stripe
  swatch is the least distinctive of the eleven.
- **Cervélo** — white, black, one red. Handsome and the highest-contrast face of the set,
  but the black day track inverts the day/night reading of every other livery.
- **Espresso** — cream, teal, chrome yellow, burnt orange. Lovely on its own; too close to
  Zwift in the hand.

## Definitions

The fabric data these entries reference:

```c
static const Band SWB_SPEC[]  = {{9,0xFF0000},{7,0x000000},{2,0xFFFFFF},{7,0x000000}};
static const Band SWB_TREK[]  = {{13,0x0055AA},{5,0xFF0000},{3,0xFFFFFF},{5,0x000055}};
static const Band SWB_CERV[]  = {{16,0x000000},{3,0xFF0000},{16,0x000000},{3,0xFFFFFF}};
static const Band SWB_BREW[]  = {{7,0xFFAA00},{5,0x000000},{7,0x005555},{5,0x000000},
                                 {4,0xFFFFAA},{5,0x000000}};
```

And the table entries themselves:

```c
  { "SPECIALIZED",
    0x000000,0xFFFFFF,0xAAAAAA,0xFF0000,0x555555,0xFFAA00,0xAA0000,0xFFFFFF,0x555555,0xFFFFFF,
    NULL,
    { SW_BANDS, SWB_SPEC, NBANDS(SWB_SPEC), NULL,0, 0,0, NULL,0 } },

  { "TREK",
    0xFFFFFF,0x000000,0x555555,0x0055AA,0x000055,0xFF0000,0x0055AA,0xFFFFFF,0x000055,0x000000,
    NULL,
    { SW_VSTRIPE, SWB_TREK, NBANDS(SWB_TREK), NULL,0, 0,0, NULL,0 } },

  { "CERVELO",
    0xFFFFFF,0x000000,0x555555,0x000000,0xAAAAAA,0xFF0000,0xFFFFFF,0x000000,0x000000,0x000000,
    NULL,
    { SW_BANDS, SWB_CERV, NBANDS(SWB_CERV), NULL,0, 0,0, NULL,0 } },

  { "ESPRESSO",
    0x000000,0xFFFFAA,0xAAAA55,0xFFAA00,0x005555,0xFF0000,0x005555,0xFFFFAA,0x005555,0xFFFFAA,
    NULL,
    { SW_BANDS, SWB_BREW, NBANDS(SWB_BREW), NULL,0, 0,0, NULL,0 } },

```

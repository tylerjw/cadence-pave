# Cadence Pavé

A cycling watchface for the **Pebble Time 2** (`emery`, 200×228, 64 colours).

The day is 24 discrete cells along the bottom — one per hour, lit through daylight,
with the last hour before sunset burning in the accent colour. Everything on the face
is a filled rectangle with a hard gutter between it and its neighbour, because that is
what a 202 ppi reflective screen is actually good at. The right-hand block is a swatch
of the livery's own fabric.

Available on the Pebble appstore: https://apps.repebble.com/7e4d835448be4ebdb749565a

Built from the prototype board in [`design/round-1.html`](design/round-1.html). Why it
looks like this, and what was tried and rejected, is in
[`design/DECISIONS.md`](design/DECISIONS.md). Release process and the invariants that
bite are in [`MAINTAINING.md`](MAINTAINING.md).

---

## Install

The built bundle is **`build/pebble-cyclist.pbw`**.

Any of these work:

**From this machine, over the network** — enable Dev Connect in the Pebble app
(Settings → Developer), then:

```sh
export PATH="$HOME/.local/bin:$PATH"
pebble install --phone <your-phone-ip>
```

**Through your developer account:**

```sh
pebble login
pebble install --cloudpebble
```

Settings live under the watchface in the Pebble app (the gear icon).

---

## Settings

| Setting | Notes |
|---|---|
| **Livery** | 11 kits. Each has its own colour roles *and* its own fabric weave. The picker shows a **live preview of the whole face** at actual size — the real complication designs, the real pixel label font (embedded), your real clock, date and chosen second zone. It repaints as you change any of those. |
| **Complications** | One fills the band; two split it. Pick from second time zone, moon phase, sunrise/sunset, weather, date. |
| **Units** | °F or °C. |
| **Second time zone** | Pick a real named zone (49 of them, Halifax and St John's included), not a raw offset. Daylight saving is handled by the zone itself. Leave the label blank and it uses the city name. |

12- or 24-hour follows the system setting. In 12-hour mode the AM/PM tag is laid out as
part of the time block and counted in its width, so it can never push into the fabric.

## Liveries

Seven in v1.0:

| | |
|---|---|
| **Zwift** | Black tarmac, watt-bolt orange, a Tron-blue last hour. Diagonal weave with a cyan pinstripe. |
| **Surly** | Acid green on black, burnt orange for the run home. Coarse check. |
| **EF Education** | Pink field, duck-egg daylight. Banded like the kit. |
| **Visma \| Lease a Bike** | Yellow and black, the most legible pair on e-paper. |
| **Ineos '26** | The orange-and-white drip, drawn as an **ordered Bayer dither** — the only honest way to fade on 64 colours. |
| **La Vie Claire** | Mondrian on a jersey in 1986. The swatch is a real subdivision. |
| **Mapei** | The cubes. A palette invented for a pixel grid forty years early. |

Each fabric is its own weave, not one pattern recoloured.

Four more (Specialized, Trek, Cervélo, Espresso) are written and working but held back
until they are as considered as these — see
[`design/unshipped-liveries.md`](design/unshipped-liveries.md) for the definitions and
what each still needs.

## Time zones and travel

The watch is sent a single number: minutes to add to *its own* local time. The phone
computes that from the zone's real UTC offset right now (`Intl.DateTimeFormat` with a
`timeZone`, so DST is whatever the zone actually observes today) minus the phone's
current offset, and resends it on every settings save and every weather refresh.

That means it follows you: fly from Denver to San Jose and the second zone stays
correct once the next refresh lands, without touching a setting. The only stale window
is up to 30 minutes after a zone change.

If a phone's JS runtime turns out to have no zone database, `FALLBACK` in
`src/pkjs/index.js` supplies standard offsets for every zone in the list — correct, but
without DST. The `Intl` path is the one that normally runs.

## Weather and sun times

Open-Meteo, no API key, refreshed every 30 minutes from the phone. Sunrise and sunset
come from the same call, so the day cells and the sun complication agree. Moon phase is
computed on the watch from a known new-moon epoch — no network needed.

## Design rules the code enforces

- **Nothing thinner than 2 px.** No hairlines, no chapter rings, no swept hands.
- **Every colour is palette-legal.** The Pebble palette is two bits per channel: every
  value is `00`, `55`, `AA` or `FF`. Zwift's orange lands exactly on `#FF5500`.
- **No two roles that touch on screen share a value.** Field, daylight, night and accent
  are four distinct colours in every livery.
- **Marks drawn *on* the track don't trust the palette.** Sunrise/sunset ticks and the
  swatch's cut edge resolve at draw time against the surfaces they actually cross
  (`mark_on()`), so a white tick never lands on a white surface.
- **Panels favour black-on-white.** On reflective e-paper a white ground with black
  ink is the most legible thing the screen can do; a mid-grey panel is close to the
  least. Zwift and Ineos use white complication windows for that reason.
- **Labels step down one rung at a time.** Silkscreen is a bitmap face, so labels try
  16 px, then 12, then 8. A 7-character label like `HALIFAX` misses 16 px by a hair and
  must not fall all the way to 8.
- **No battery meter.** Thirty days between charges is the point of this watch.

## Building

```sh
uv tool install "pebble-tool==5.0.39" --python 3.13
export PATH="$HOME/.local/bin:$PATH"
pebble sdk install latest
pebble build
pebble install --emulator emery      # try it in QEMU
pebble screenshot shot.png
```

Render the settings page to a file and open it in a browser without a watch or
emulator — this runs the real built bundle with a stubbed `Pebble` object and captures
exactly the HTML Clay would open on the phone:

```sh
node tools/gen_config_page.js      # -> build/settings-preview.html
```

Handy while iterating — push settings straight to the running emulator (all key/value
pairs must share **one** `--int` flag, or only the last is sent):

```sh
pebble send-app-message --emulator emery --int 10000=9 10001=3 10002=4
#                                              livery  comp1   comp2
```

## If `pebble build` dies with a Python traceback

```
FileNotFoundError: [Errno 2] No such file or directory:
  '.../Pebble SDK/SDKs/current/sdk-core/../.venv/bin/python'
```

The SDK builds inside its own virtualenv, and that venv is symlinked into a **specific
Homebrew Cellar version** of Python. When `brew upgrade` bumps the patch release it
deletes the old directory, the symlink dangles, and every build fails — nothing to do
with this project. Repoint it at Homebrew's stable `opt` path, which survives upgrades:

```sh
SDK="$HOME/Library/Application Support/Pebble SDK/SDKs/current"
STABLE="/opt/homebrew/opt/python@3.13/Frameworks/Python.framework/Versions/3.13"
ln -sf "$STABLE/bin/python3.13" "$SDK/.venv/bin/python3.13"
sed -i '' -E "s|/opt/homebrew/Cellar/python@3\.13/[^/]+/Frameworks/Python\.framework/Versions/3\.13|$STABLE|g" \
  "$SDK/.venv/pyvenv.cfg"
"$SDK/.venv/bin/python" -V     # should print a version, not an error
```

Patch upgrades keep the same ABI, so the packages already installed in the venv stay
valid. If it is a *minor* upgrade (3.13 to 3.14), reinstall the SDK instead:
`pebble sdk install latest`.

## Layout

```
 0        136 139     200
 ┌──────────┬─┬─────────┐  0
 │   TIME   │ │ FABRIC  │
 ├──────────┴─┴─────────┤  110
 │   COMPLICATIONS      │  113 – 176
 ├──────────────────────┤
 │  ▪▪▪▪ 24 HOUR CELLS  │  179 – 210
 ├──────────────────────┤
 │  ▨ LIVERY            │  210 – 228
 └──────────────────────┘
```

`src/c/main.c` is the whole face. `src/pkjs/` is the phone side: `config.js` is the
Clay settings schema, `index.js` fetches weather and computes the second-zone offset,
and `preview.js` draws the live face preview inside the settings page.

**`preview.js` duplicates `main.c` on purpose.** Clay serialises the custom function
with `tosource` and injects it into the settings webview, so it cannot reference
anything outside its own body — that includes the livery table, the swatch renderer,
the complication layouts and the Silkscreen label ladder, all of which are ports of the
C. It also embeds the Silkscreen TTF as a base64 `FontFace` so label sizes match the
watch rather than being approximated in a monospace stand-in.

If you change a livery's colours, a fabric, or any complication's layout, change it in
both places. `node tools/gen_config_page.js` renders the result so you can check.

## Releasing

```sh
pebble login                     # Firebase auth, opens a browser
pebble build
pebble publish
```

`pebble publish` creates the appstore listing on first run and prompts for name,
description, category, source URL and icons. Note it defaults to
`--gif-all-platforms`, which spins up an emulator per platform to capture rollover
GIFs; pass `--no-gif-all-platforms --screenshots emery_*.png` to supply your own
instead. Screenshot filenames must start with the platform name.

Bump `version` in `package.json` for each release and pass `--release-notes`.

Listing assets are captured from a **real watch**, not the emulator — the emulator has
no weather, so its captures show `--` placeholders:

```sh
./tools/capture-store-assets.sh            # via the cloud relay, needs `pebble login`
./tools/capture-store-assets.sh <phone-ip> # or direct, if you know the IP
```

It cycles livery and complications by app message, screenshots each, rejects any frame
where a notification landed on top of the face, restores your settings, and builds
`store/emery_liveries.gif`. Store copy lives in `store/description.md`.

## A note on the kit names

This project is not affiliated with, endorsed by, or sponsored by any of the brands or
teams the liveries are named after. No logos are reproduced: every fabric is original
geometry drawn from scratch, and the palettes are approximations snapped to the
Pebble's 64-colour grid. The names are used descriptively, to say which kit a colourway
evokes.

## Licences

Anton and Silkscreen are bundled under the SIL Open Font License; see
`resources/fonts/OFL-*.txt`. Livery colours and patterns are original geometry in each
brand's palette — no logos are reproduced.

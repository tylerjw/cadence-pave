# Maintaining Cadence Pavé

Operational notes: the store listing's identity, the release checklist, and the
handful of invariants that will bite silently if you break them.

For the design rationale behind the face itself, see
[`design/DECISIONS.md`](design/DECISIONS.md).

## The listing

| | |
|---|---|
| Store page | https://apps.repebble.com/7e4d835448be4ebdb749565a |
| App ID | `7e4d835448be4ebdb749565a` |
| Developer ID | `8df83e56cd3e1befd02a234e` |
| Dashboard | https://appstore-api.repebble.com/dashboard |
| App UUID | `73a54b3c-0669-4dae-837e-7773a32e2f10` |
| Source | https://github.com/tylerjw/cadence-pave |

The UUID is what identifies the app to the watch and the store. Never change it, or
existing installs become a different app.

## Releasing

1. Bump `version` in `package.json`. **The store rejects a version that already
   exists**, and the error arrives only after the upload attempt.
2. `pebble clean && pebble build`. The clean matters: waf does not treat
   `package.json` as a build input, so a plain rebuild happily ships a `.pbw` still
   carrying the old version. Confirm with
   `unzip -p build/*.pbw appinfo.json | grep versionLabel`.
3. Capture fresh screenshots if the face changed: `./tools/capture-store-assets.sh`.
4. Publish:

```sh
pebble publish --no-gif-all-platforms --replace-screenshots --non-interactive \
  --release-notes "what changed" \
  --screenshots store/emery_liveries.gif store/emery_1_zwift.png \
    store/emery_4_ineos.png store/emery_2_mapei.png store/emery_5_ef.png
```

`--non-interactive` is what makes it use the `--screenshots` flag instead of prompting
for a source. Without `--no-gif-all-platforms` it launches an emulator per platform to
capture rollover GIFs, which is both slow and unreliable (see below).

### What the CLI cannot do

`--description`, `--source`, `--category` and the icons apply **only when creating an
app**. The app now exists, so all of those are dashboard edits. In practice that means
the description has to be pasted by hand for every wording change.

The store **preserves line breaks** in the description. Each paragraph in
`store/description.md` is deliberately one long unwrapped line; re-wrapping it to 80
columns makes the listing render as ragged short lines on a wide screen. The five
complication lines are meant to stay one per line.

## Invariants

**Livery indices are persisted and sent over the wire.** The `LIVERY` message key is an
index into `LIVERIES[]`. If you add, remove or reorder a livery, every saved index now
names a different kit. Bump `SETTINGS_VERSION` in `src/c/main.c` so `load_state()`
resets the selection instead of silently showing the wrong one. This already happened
once between v1 and the seven-livery cut.

**Three files must agree on the livery list, in the same order:**

- `src/c/main.c` — the `LIVERIES[]` table
- `src/pkjs/config.js` — the picker, whose option values are the indices
- `src/pkjs/preview.js` — the settings-page preview

**`preview.js` duplicates `main.c` on purpose.** Clay serialises the custom function
with `tosource` and injects it into the settings webview, so it cannot reference
anything outside its own body. The livery table, swatch renderer, complication layouts
and Silkscreen label ladder are all ports. It also embeds the Silkscreen TTF as a
base64 `FontFace` so label sizes match the watch. Change a colour, fabric or
complication layout and you change it in both places.

Render the settings page to check the result without a watch or emulator:

```sh
node tools/gen_config_page.js     # -> build/settings-preview.html
```

**Text can silently wrap.** Pebble's `graphics_draw_text` word-wraps anything wider than
its box, and the overflow is then clipped by the panel. This shipped once: Anton's `%`
is wider than two digits, so `97%` wrapped and lost its sign. If you add a value to a
complication, measure it. `tools/` has no helper for this, but the TTF metrics are
readable directly with Python (`hmtx` / `cmap`), which is how that bug was found.

## Tooling that does not behave

**The emulator is unreliable.** QEMU starts but `pypkjs` frequently does not, so there
is no phone-side JS: no weather, no sun times, and captures show `--` placeholders.
It also throws macOS "Python quit unexpectedly" dialogs when cycled. Capture from a real
watch instead — `tools/capture-store-assets.sh` goes through the CloudPebble relay and
needs no IP.

**`pebble send-app-message` only honours the last `--int` flag.** All pairs must share
one flag:

```sh
pebble send-app-message --cloudpebble --int 10000=6 10001=3 10002=4   # right
pebble send-app-message --cloudpebble --int 10000=6 --int 10001=3     # only the last lands
```

Message key numbers are in `build/js/message_keys.json`.

**Homebrew upgrades break the SDK's virtualenv.** Covered in the README under
*If `pebble build` dies with a Python traceback*.

**Notifications land on top of the watchface** during a capture run. The capture script
verifies each frame with `tools/is_watchface.py` and retries rather than shipping a
screenshot of your calendar.

#!/usr/bin/env bash
#
# Capture store screenshots and a livery-cycling GIF from a REAL watch.
#
#   ./tools/capture-store-assets.sh 192.168.1.42
#
# Needs Dev Connect enabled in the Pebble app (Devices -> ... -> Dev Connect);
# that screen shows the phone's IP. Everything lands in ./store/.
#
# Uses the real device rather than the emulator on purpose: the emulator has no
# weather and its pypkjs side is unreliable, so its captures show "--" placeholders.

set -euo pipefail

IP="${1:-}"
if [ -z "$IP" ]; then
  echo "usage: $0 <phone-ip>" >&2
  exit 1
fi

PEBBLE="${PEBBLE:-$HOME/.local/bin/pebble}"
OUT="store"
mkdir -p "$OUT"

# livery : comp1 : comp2 : name
#   liveries  0 Zwift  1 Surly  2 EF  3 Visma  4 Ineos  5 La Vie Claire  6 Mapei
#   comps     0 none   1 zone   2 moon  3 sun   4 weather  5 date
# Chosen so all five complications appear across the set, wide and narrow.
FRAMES=(
  "0:3:4:1_zwift"
  "6:5:4:2_mapei"
  "5:2:5:3_lavieclaire"
  "4:1:4:4_ineos"
  "2:3:0:5_ef"
  "3:4:5:6_visma"
  "1:1:2:7_surly"
)

echo "Capturing ${#FRAMES[@]} frames from $IP"
for spec in "${FRAMES[@]}"; do
  IFS=: read -r liv c1 c2 name <<< "$spec"
  "$PEBBLE" send-app-message --phone "$IP" \
      --int 10000="$liv" 10001="$c1" 10002="$c2" >/dev/null 2>&1
  sleep 2                                    # let the watch redraw
  "$PEBBLE" screenshot --phone "$IP" --no-open "$OUT/emery_$name.png" >/dev/null 2>&1
  echo "  emery_$name.png"
done

# Put the watch back on the default livery
"$PEBBLE" send-app-message --phone "$IP" --int 10000=0 10001=3 10002=4 >/dev/null 2>&1

# Livery-cycling GIF: each kit with a different pair of complications.
if command -v ffmpeg >/dev/null; then
  ffmpeg -loglevel error -y -framerate 1/1.6 -pattern_type glob \
         -i "$OUT/emery_*.png" \
         -filter_complex "[0:v]split[a][b];[a]palettegen=max_colors=64[p];[b][p]paletteuse" \
         -loop 0 "$OUT/emery_liveries.gif"
  echo "  emery_liveries.gif"
else
  echo "ffmpeg not found - skipping the GIF" >&2
fi

echo
echo "Done. Review $OUT/ then publish with:"
echo
echo "  pebble publish --no-gif-all-platforms --replace-screenshots \\"
echo "    --screenshots $OUT/emery_liveries.gif $OUT/emery_1_zwift.png \\"
echo "      $OUT/emery_2_mapei.png $OUT/emery_3_lavieclaire.png $OUT/emery_4_ineos.png"

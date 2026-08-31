# Why the face looks like this

Written down so the rejected ideas stay rejected for the right reasons, and so the
odd-looking choices are legible later.

The interactive design board that produced it is
[`round-1.html`](round-1.html) — open it in a browser. It renders every concept to a
real 200×228 canvas with a 64-colour quantiser and an e-paper simulation, and it still
carries the four concepts and eleven liveries that were considered.

## The brief

A cycling watchface for the Pebble Time 2, from someone who likes watches as objects.
Reference points were Halcyon (its 24-hour ring showing sunrise and sunset), the Brew
Metric (a circular dial squared off in a rectangular case, with a coloured zone on the
chrono scale that turns a joke into a usable feature), and Bravur's cycling watches.

Two hard constraints shaped everything:

- **200×228 at 202 ppi.** Not enough resolution for a swept hand, a hairline chapter
  ring, or a drawing of a thing.
- **64 colours.** Two bits per channel: every value is `00`, `55`, `AA` or `FF`. There
  is no `#FC6719`. Zwift's orange happens to land exactly on `#FF5500`.

## Four concepts, one survivor

Round one put up four. Each carried the same idea — the shape of your day drawn around
the outside, where it costs nothing — and disagreed about the shape.

**Criterium** ran the 24-hour track around the whole perimeter, midnight at a
start/finish checker. It survived to the final two and remains the strongest
alternative: it is the one you could wear every day. Pavé won on being more distinctive.

**Metric** put a round day-ring inside the square screen, the Brew move. Cut early: a
ring wastes all four corners, and the exergue left below it was too shallow to hold two
complications at a readable size. The most watch-like of the four and the least useful.

**Route** drew the day as a road, later a single bike lane, running down the left edge.
Cut for resolution. A lane stencil, travel chevrons and edge lines are a lot of small
marks competing inside 52 px of width. It held together at 3× and dissolved at 1×.

**Pavé** made everything a filled rectangle with a hard gutter between it and its
neighbour, and turned the day into 24 discrete cells. It is the only one where the low
resolution is the point rather than the constraint, and the only one where the heritage
liveries stop being a palette swap and become the composition.

## Things that were tried and did not work

**A pixel bicycle, and a jersey.** Both were attempts to put a *drawing* on the face. A
bicycle is thin members and two round wheels; a jersey silhouette at 22×30 reads as a
blob. The lesson, learned twice: a pattern survives this pixel grid, a picture of a
thing mostly does not.

**A swatch of fabric replaced them.** A patch of pattern has no silhouette to get wrong.
Every pixel is cloth, and a weave is the one kind of graphic that gets *better* as
resolution drops. Each livery has its own weave rather than one pattern recoloured,
which is also how you tell them apart across a room.

**A battery meter, drawn as a bidon.** Charming, and cut on the owner's own reasoning:
thirty days between charges is the best thing about this watch, and a gauge you never
need to look at is a gauge in the way.

**An ambient DAY / NIGHT label** beside the time. It named the sun's phase, which the
day cells already draw in colour immediately below it. A label that only repeats the
graphic beside it is noise. What replaced it is a tag that stays silent until the last
hour of daylight begins, then says FLAMME ROUGE in the accent colour.

**Espresso, as a livery.** Cream, teal and burnt orange, built for the Brew reference.
Lovely alone, but it is Zwift's construction — black field, warm accent, dark secondary
— and reads too close to it on the wrist.

## Rules the code enforces

**Nothing thinner than 2 px**, no hands, no charts, no rounded corners.

**No two roles that touch on screen share a value.** Field, daylight, night and accent
are four distinct colours in every livery.

**Marks drawn on the track do not trust the palette.** A livery may legitimately want
white in two roles; what it must never do is paint a white tick on a white surface.
`mark_on()` resolves each mark at draw time against the specific surfaces it crosses.

**Panels favour black on white.** On reflective e-paper that pairing is the most legible
thing the screen can do and a mid-grey panel is close to the least. This was wrong at
first — Zwift and Ineos shipped grey panels until it was flagged from the wrist.

**Labels step down one rung at a time.** Silkscreen is a bitmap face, so labels try 16,
then 12, then 8 px. The 12 px rung exists because `HALIFAX` misses 16 by a hair and used
to fall all the way to 8.

## Still open

Four liveries are written and working but held back until they are as considered as the
seven that shipped: Specialized, Trek, Cervélo and Espresso. Definitions and what each
still needs are in [`unshipped-liveries.md`](unshipped-liveries.md).

The kit names are brand names used descriptively. No logos are reproduced and every
fabric is original geometry, but if that ever becomes a problem the alternative is
descriptive names — at the cost of the thing that makes the liveries fun.

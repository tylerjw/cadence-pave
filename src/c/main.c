/*
 * Cadence Pave - a cycling watchface for the Pebble Time 2 (emery, 200x228).
 *
 * The day is 24 discrete cells along the bottom. Everything is a filled
 * rectangle with a hard gutter between it and its neighbour, because that is
 * what this screen is good at. The right-hand block is a swatch of the
 * livery's own fabric.
 */

#include <pebble.h>
#include <ctype.h>

#define SCR_W 200
#define SCR_H 228

/* ---------------------------------------------------------------- layout */
#define A_W        136              /* time block width                     */
#define A_H        110
#define B_X        139              /* fabric block                         */
#define B_W        61
#define D_Y        113              /* complication band                    */
#define D_H        63
#define E_Y        179              /* day-cell band                        */
#define F_Y        210              /* livery band                          */
#define F_H        18

#define CELL_Y     188
#define CELL_H     16
#define CELL_W     7
#define CELL_STEP  8
#define CELL_X0    4

/* Distance from the top of a text box to the top of a capital / digit.
 * Pebble lays text out from the font ascent, so these are the trims that put
 * a glyph where the design says it goes. Tuned against the emulator. */
#define TRIM_ANTON_54  12
#define TRIM_ANTON_34   8
#define TRIM_ANTON_27   6
#define TRIM_SILK_16    3
#define TRIM_SILK_12    2
#define TRIM_SILK_8     2

#define CAP_ANTON_54   39
#define CAP_ANTON_34   25
#define CAP_ANTON_27   20
#define CAP_SILK_16    10
#define CAP_SILK_12     7
#define CAP_SILK_8      5

/* ------------------------------------------------------------- complications */
enum { COMP_NONE = 0, COMP_GMT, COMP_MOON, COMP_SUN, COMP_WX, COMP_DATE };

/* ------------------------------------------------------------------ swatch */
enum { SW_BANDS = 0, SW_VSTRIPE, SW_DIAG, SW_CHECK, SW_CUBES, SW_DITHER, SW_BLOCKS };

typedef struct { uint8_t px; uint32_t col; } Band;

typedef struct {
  uint8_t type;
  const Band *bands;   uint8_t nbands;
  const uint32_t *cols; uint8_t ncols;
  uint8_t size;
  uint32_t gut;
  const uint8_t *rects; uint8_t nrects;   /* x0,y0,x1,y1,colour  as 0..100 */
} Swatch;

typedef struct {
  const char *name;
  uint32_t bg, ink, dim, day, night, accent, panel, panel_ink, line, chk;
  const uint32_t *multi;                  /* 4 entries, or NULL */
  Swatch sw;
} Livery;

/* ------------------------------------------------------------- fabric data */
static const Band SWB_ZWIFT[] = {{8,0xFF5500},{7,0x000000},{3,0x55FFFF},{7,0x000000}};
static const Band SWB_EF[]    = {{10,0xFF55AA},{8,0xAAFFFF},{3,0x000000},{8,0xAAFFFF}};
static const Band SWB_VISMA[] = {{11,0xFFFF00},{8,0x000000},{3,0xFFFF00},{8,0x000000}};

static const uint32_t SWC_SURLY[] = {0xAAFF00,0x000000,0x005500,0x000000};
static const uint32_t SWC_INEOS[] = {0xFF5500,0xFFFFFF};
static const uint32_t SWC_LVC[]   = {0xFFFFFF,0xFF0000,0xFFFF00,0x0055AA};
static const uint32_t SWC_MAPEI[] = {0xFF0000,0x0055AA,0x00AA00,0xFFFF00,0xFFFFFF};

static const uint8_t SWR_LVC[] = {
   0,  0, 60, 30, 0,
  64,  0,100, 30, 1,
   0, 34,100, 60, 2,
   0, 64, 44,100, 3,
  48, 64,100,100, 0,
};

static const uint32_t M_LVC[]   = {0xFF0000,0xFFFF00,0x0055AA,0x000000};
static const uint32_t M_MAPEI[] = {0xFF0000,0x0055AA,0x00AA00,0xFFFF00};

#define NBANDS(a) (sizeof(a)/sizeof(Band))
#define NCOLS(a)  (sizeof(a)/sizeof(uint32_t))

/* Shipped in v1.0. Four more liveries are drafted but held back until they
 * are as considered as these; see design/unshipped-liveries.md. */
static const Livery LIVERIES[] = {
  { "ZWIFT",
    0x000000,0xFFFFFF,0xAAAAAA,0xFF5500,0x005555,0x55FFFF,0xFFFFFF,0x000000,0x555555,0xFFFFFF,
    NULL,
    { SW_DIAG, SWB_ZWIFT, NBANDS(SWB_ZWIFT), NULL,0, 0,0, NULL,0 } },

  { "SURLY",
    0x000000,0xFFFFFF,0xAAAAAA,0xAAFF00,0x005500,0xFF5500,0xAAFF00,0x000000,0x005500,0xAAFF00,
    NULL,
    { SW_CHECK, NULL,0, SWC_SURLY, NCOLS(SWC_SURLY), 6, 0, NULL,0 } },

  { "EF EDUCATION",
    0xFF55AA,0x000000,0xAA0055,0xAAFFFF,0xAA0055,0xFF0000,0x000000,0xAAFFFF,0x000000,0x000000,
    NULL,
    { SW_BANDS, SWB_EF, NBANDS(SWB_EF), NULL,0, 0,0, NULL,0 } },

  { "VISMA",
    0x000000,0xFFFFFF,0xAAAAAA,0xFFFF00,0x555500,0xFF0000,0xFFFF00,0x000000,0x555500,0xFFFF00,
    NULL,
    { SW_BANDS, SWB_VISMA, NBANDS(SWB_VISMA), NULL,0, 0,0, NULL,0 } },

  { "INEOS 26",
    0xFFFFFF,0x000000,0x555555,0xFF5500,0x555555,0x0055AA,0xFFFFFF,0x000000,0xAA5500,0x000000,
    NULL,
    { SW_DITHER, NULL,0, SWC_INEOS, NCOLS(SWC_INEOS), 0, 0, NULL,0 } },

  { "LA VIE CLAIRE",
    0xFFFFFF,0x000000,0x555555,0xFFFF00,0x0055AA,0xFF0000,0xFFFFFF,0x000000,0x000000,0x000000,
    M_LVC,
    { SW_BLOCKS, NULL,0, SWC_LVC, NCOLS(SWC_LVC), 0, 0x000000, SWR_LVC, 5 } },

  { "MAPEI",
    0xFFFFFF,0x000000,0x555555,0x00AA00,0x0055AA,0xFF0000,0xFFFFFF,0x000000,0x000000,0x000000,
    M_MAPEI,
    { SW_CUBES, NULL,0, SWC_MAPEI, NCOLS(SWC_MAPEI), 11, 0xFFFFFF, NULL,0 } },
};
#define NUM_LIVERIES (int)(sizeof(LIVERIES)/sizeof(Livery))

/* ------------------------------------------------------------------ state */
#define SETTINGS_VERSION 2

typedef struct {
  uint8_t version;
  uint8_t livery;
  uint8_t comp1;
  uint8_t comp2;
  uint8_t units;        /* 0 = F, 1 = C */
  int16_t tz_offset;    /* minutes from local time */
  char    tz_label[8];
} Settings;

typedef struct {
  int16_t temp, hi, lo;
  uint8_t cond;         /* 0 clear, 1 part, 2 cloud, 3 rain */
  int16_t sunrise, sunset;   /* minutes since local midnight */
  bool    valid;
  bool    sun_valid;
} WxData;

#define PKEY_SETTINGS 1
#define PKEY_WX       2

static Settings s_set = { .version = SETTINGS_VERSION, .livery = 0, .comp1 = COMP_SUN, .comp2 = COMP_WX,
                          .units = 0, .tz_offset = 0, .tz_label = "PARIS" };
static WxData s_wx = { .temp = 0, .hi = 0, .lo = 0, .cond = 0,
                       .sunrise = 6*60, .sunset = 20*60,
                       .valid = false, .sun_valid = false };

static Window *s_window;
static Layer  *s_layer;
static GFont   s_anton54, s_anton34, s_anton27, s_silk16, s_silk12, s_silk8;

/* ---------------------------------------------------------------- colours */
static inline GColor hexc(uint32_t h) {
  return GColorFromRGB((h >> 16) & 0xFF, (h >> 8) & 0xFF, h & 0xFF);
}
static int lum(uint32_t h) {
  int r = (h >> 16) & 0xFF, g = (h >> 8) & 0xFF, b = h & 0xFF;
  return (299 * r + 587 * g + 114 * b) / 1000;
}
static uint32_t snap_ch(int v) {
  if (v < 0) v = 0;
  if (v > 255) v = 255;
  return ((v + 42) / 85) * 85;
}
/* weighted mix of two palette colours, snapped back onto the 2-bit grid */
static uint32_t blend(uint32_t a, uint32_t b, int wa) {
  int r = (((a >> 16) & 0xFF) * wa + ((b >> 16) & 0xFF) * (100 - wa)) / 100;
  int g = (((a >>  8) & 0xFF) * wa + ((b >>  8) & 0xFF) * (100 - wa)) / 100;
  int bl = ((a & 0xFF) * wa + (b & 0xFF) * (100 - wa)) / 100;
  return (snap_ch(r) << 16) | (snap_ch(g) << 8) | snap_ch(bl);
}
/* Pick a mark colour for something drawn ON a surface. A livery may want white
 * in two roles; what it must never do is put a white tick on a white surface. */
static uint32_t mark_on(const uint32_t *surfaces, int n, const Livery *L) {
  const uint32_t cands[4] = { L->ink, L->bg, 0xFFFFFF, 0x000000 };
  uint32_t best = cands[0];
  int score = -1;
  for (int i = 0; i < 4; i++) {
    int worst = 1000;
    for (int j = 0; j < n; j++) {
      int d = lum(cands[i]) - lum(surfaces[j]);
      if (d < 0) d = -d;
      if (d < worst) worst = d;
    }
    if (worst > score) { score = worst; best = cands[i]; }
  }
  return best;
}

/* ---------------------------------------------------------------- drawing */
static GContext *g_ctx;

static void fillr(int x, int y, int w, int h, uint32_t col) {
  if (w <= 0 || h <= 0) return;
  graphics_context_set_fill_color(g_ctx, hexc(col));
  graphics_fill_rect(g_ctx, GRect(x, y, w, h), 0, GCornerNone);
}
static void framer(int x, int y, int w, int h, int t, uint32_t col) {
  fillr(x, y, w, t, col);
  fillr(x, y + h - t, w, t, col);
  fillr(x, y + t, t, h - 2 * t, col);
  fillr(x + w - t, y + t, t, h - 2 * t, col);
}
static void checker(int x, int y, int w, int h, int cols, int rows,
                    uint32_t a, uint32_t b) {
  for (int i = 0; i < cols; i++) {
    for (int j = 0; j < rows; j++) {
      int x0 = x + i * w / cols, x1 = x + (i + 1) * w / cols;
      int y0 = y + j * h / rows, y1 = y + (j + 1) * h / rows;
      fillr(x0, y0, x1 - x0, y1 - y0, ((i + j) & 1) ? b : a);
    }
  }
}
static int text_w(const char *s, GFont f) {
  return graphics_text_layout_get_content_size(
      s, f, GRect(0, 0, SCR_W, 100), GTextOverflowModeWordWrap,
      GTextAlignmentLeft).w;
}
/* draw with the CAP TOP at cap_y, so the design's numbers mean something */
static void text_at(const char *s, GFont f, int trim, int x, int cap_y, int w,
                    GTextAlignment al, uint32_t col) {
  graphics_context_set_text_color(g_ctx, hexc(col));
  graphics_draw_text(g_ctx, s, f, GRect(x, cap_y - trim, w, 100),
                     GTextOverflowModeWordWrap, al, NULL);
}
static void silk16(const char *s, int x, int cap_y, int w, GTextAlignment al, uint32_t col) {
  text_at(s, s_silk16, TRIM_SILK_16, x, cap_y, w, al, col);
}
static void silk8(const char *s, int x, int cap_y, int w, GTextAlignment al, uint32_t col) {
  text_at(s, s_silk8, TRIM_SILK_8, x, cap_y, w, al, col);
}
/* Silkscreen is a bitmap face. Step down one rung at a time rather than falling
 * straight to the smallest: 7-character labels miss the 16 px rung by a hair, and
 * used to land on 8 px when 12 px would have fitted with room to spare.
 * Cap bottoms are kept aligned so the baseline does not move between rungs. */
static void silk_fit(const char *s, int x, int cap_y, int w, GTextAlignment al, uint32_t col) {
  if (text_w(s, s_silk16) <= w) { silk16(s, x, cap_y, w, al, col); return; }
  if (text_w(s, s_silk12) <= w) {
    text_at(s, s_silk12, TRIM_SILK_12, x, cap_y + (CAP_SILK_16 - CAP_SILK_12), w, al, col);
    return;
  }
  silk8(s, x, cap_y + (CAP_SILK_16 - CAP_SILK_8), w, al, col);
}

static int isqrt_i(int n) {
  if (n <= 0) return 0;
  int x = n, y = (x + 1) / 2;
  while (y < x) { x = y; y = (x + n / x) / 2; }
  return x;
}
/* pixel disc, drawn row by row so it steps the way the hardware will */
static void disc(int cx, int cy, int r, uint32_t col) {
  for (int y = -r; y < r; y++) {
    int yy2 = (2 * y + 1) * (2 * y + 1);
    int hw = isqrt_i(4 * r * r - yy2) / 2;
    if (hw > 0) fillr(cx - hw, cy + y, hw * 2, 1, col);
  }
}

/* ------------------------------------------------------------------ fabric */
static void band_run(const Band *b, int n, int m, uint32_t *col, int *run) {
  int total = 0;
  for (int i = 0; i < n; i++) total += b[i].px;
  int off = ((m % total) + total) % total;
  for (int i = 0; i < n; i++) {
    if (off < b[i].px) { *col = b[i].col; *run = b[i].px - off; return; }
    off -= b[i].px;
  }
  *col = b[0].col; *run = 1;
}

static const uint8_t BAYER[4][4] = {
  { 0, 8, 2,10}, {12, 4,14, 6}, { 3,11, 1, 9}, {15, 7,13, 5}
};

static void draw_swatch(int x, int y, int w, int h, const Livery *L, bool framed) {
  const Swatch *sw = &L->sw;
  switch (sw->type) {
    case SW_BANDS:
      for (int j = 0; j < h; j++) {
        uint32_t c; int run;
        band_run(sw->bands, sw->nbands, j, &c, &run);
        if (run > h - j) run = h - j;
        fillr(x, y + j, w, run, c);
        j += run - 1;
      }
      break;

    case SW_VSTRIPE:
      for (int i = 0; i < w; i++) {
        uint32_t c; int run;
        band_run(sw->bands, sw->nbands, i, &c, &run);
        if (run > w - i) run = w - i;
        fillr(x + i, y, run, h, c);
        i += run - 1;
      }
      break;

    case SW_DIAG:
      for (int j = 0; j < h; j++) {
        int i = 0;
        while (i < w) {
          uint32_t c; int run;
          band_run(sw->bands, sw->nbands, i + j, &c, &run);
          if (run > w - i) run = w - i;
          fillr(x + i, y + j, run, 1, c);
          i += run;
        }
      }
      break;

    case SW_CHECK:
    case SW_CUBES: {
      /* cell edges derive from the box, so the weave never ends mid-cube */
      int n   = sw->size ? sw->size : 8;
      int gap = (sw->type == SW_CUBES) ? 2 : 0;
      int nx  = w / n; if (nx < 2) nx = 2;
      int ny  = h / n; if (ny < 2) ny = 2;
      if (gap) fillr(x, y, w, h, sw->gut);
      for (int gy = 0; gy < ny; gy++) {
        for (int gx = 0; gx < nx; gx++) {
          int x0 = gx * w / nx, x1 = (gx + 1) * w / nx;
          int y0 = gy * h / ny, y1 = (gy + 1) * h / ny;
          int idx = (sw->type == SW_CHECK)
                      ? (gx + gy) % sw->ncols
                      : (gx * 3 + gy * 7 + ((gx * gy) % 5)) % sw->ncols;
          fillr(x + x0, y + y0, x1 - x0 - gap, y1 - y0 - gap, sw->cols[idx]);
        }
      }
      break;
    }

    case SW_DITHER: {
      /* ordered dither: the only honest way to draw a fade on 64 colours */
      for (int j = 0; j < h; j++) {
        int t = (h > 1) ? (j * 256) / (h - 1) : 0;   /* 0..256 */
        int on = 0, off = 0;
        for (int p = 0; p < 4; p++) {
          int th = (BAYER[j & 3][p] * 16 + 8);       /* 0..256 */
          if (t > th) on++; else off++;
        }
        if (off == 0)      { fillr(x, y + j, w, 1, sw->cols[1]); continue; }
        if (on  == 0)      { fillr(x, y + j, w, 1, sw->cols[0]); continue; }
        for (int i = 0; i < w; i++) {
          int th = (BAYER[j & 3][i & 3] * 16 + 8);
          fillr(x + i, y + j, 1, 1, (t > th) ? sw->cols[1] : sw->cols[0]);
        }
      }
      break;
    }

    case SW_BLOCKS:
      fillr(x, y, w, h, sw->gut);
      for (int i = 0; i < sw->nrects; i++) {
        const uint8_t *r = &sw->rects[i * 5];
        int x0 = x + r[0] * w / 100, y0 = y + r[1] * h / 100;
        int x1 = x + r[2] * w / 100, y1 = y + r[3] * h / 100;
        fillr(x0, y0, x1 - x0, y1 - y0, sw->cols[r[4]]);
      }
      break;
  }

  /* a cut edge, so a fabric whose own ground matches the field still reads */
  if (framed) {
    uint32_t surf[1] = { L->bg };
    framer(x - 1, y - 1, w + 2, h + 2, 1, mark_on(surf, 1, L));
  }
}

/* ------------------------------------------------------------------- time */
static void fmt_hm(int mins, char *hh, char *mm, const char **suf) {
  mins = ((mins % 1440) + 1440) % 1440;
  int h = mins / 60, m = mins % 60;
  *suf = NULL;
  if (!clock_is_24h_style()) {
    *suf = (h < 12) ? "AM" : "PM";
    h = h % 12; if (h == 0) h = 12;
    snprintf(hh, 4, "%d", h);
  } else {
    snprintf(hh, 4, "%02d", h);
  }
  snprintf(mm, 4, "%02d", m);
}

/* HH : MM with a hand-built square colon, so the gap is ours not the font's.
 * The AM/PM tag is part of the block and counted in the width. */
static void draw_time(int cx, int cap_y, GFont f, int trim, int cap_h,
                      int mins, uint32_t col, uint32_t colon_col, uint32_t suf_col) {
  char hh[4], mm[4];
  const char *suf;
  fmt_hm(mins, hh, mm, &suf);

  int wh = text_w(hh, f), wm = text_w(mm, f);
  int dot = cap_h * 15 / 100; if (dot < 3) dot = 3;
  int gap = cap_h * 20 / 100;
  int sw_w = 0, sw_gap = 0;
  if (suf) { sw_w = text_w(suf, s_silk16); sw_gap = 5; }

  int total = wh + gap + dot + gap + wm + sw_gap + sw_w;
  int x = cx - total / 2;

  text_at(hh, f, trim, x, cap_y, wh + 4, GTextAlignmentLeft, col);
  int colx = x + wh + gap;
  fillr(colx, cap_y + cap_h * 28 / 100, dot, dot, colon_col);
  fillr(colx, cap_y + cap_h * 70 / 100, dot, dot, colon_col);
  text_at(mm, f, trim, colx + dot + gap, cap_y, wm + 4, GTextAlignmentLeft, col);
  if (suf) {
    silk16(suf, x + total - sw_w, cap_y + cap_h - CAP_SILK_16, sw_w + 4,
           GTextAlignmentLeft, suf_col);
  }
}

/* ------------------------------------------------------------------- moon */
/* Reference new moon: 2000-01-06 18:14 UTC. Synodic month 2551443 s. */
#define MOON_EPOCH 947182440UL
#define MOON_CYCLE 2551443UL

static int moon_pct(time_t now, int32_t *cos_out, bool *waxing) {
  uint32_t diff = (uint32_t)(now - MOON_EPOCH);
  uint32_t p = diff % MOON_CYCLE;
  int32_t angle = (int32_t)(((uint64_t)p * TRIG_MAX_ANGLE) / MOON_CYCLE);
  int32_t c = cos_lookup(angle);           /* -TRIG_MAX_RATIO .. TRIG_MAX_RATIO */
  if (cos_out) *cos_out = c;
  if (waxing)  *waxing = (p < MOON_CYCLE / 2);
  return (int)(((int32_t)TRIG_MAX_RATIO - c) * 100 / (2 * TRIG_MAX_RATIO));
}
static const char *moon_name(int pct, bool waxing) {
  if (pct < 3)  return "NEW";
  if (pct > 97) return "FULL";
  if (pct < 47) return waxing ? "WAX CRES" : "WAN CRES";
  if (pct < 53) return waxing ? "1ST QTR"  : "3RD QTR";
  return waxing ? "WAX GIB" : "WAN GIB";
}
static void draw_moon(int cx, int cy, int r, int32_t c, bool waxing,
                      uint32_t lit, uint32_t dark) {
  disc(cx, cy, r, dark);
  for (int y = -r; y < r; y++) {
    int yy2 = (2 * y + 1) * (2 * y + 1);
    int hw = isqrt_i(4 * r * r - yy2) / 2;
    if (hw <= 0) continue;
    int k = (int)(((int64_t)c * hw) / TRIG_MAX_RATIO);   /* terminator offset */
    int a, b;
    if (waxing) { a = k;   b = hw;  }
    else        { a = -hw; b = -k;  }
    if (b > a) fillr(cx + a, cy + y, b - a, 1, lit);
  }
}

/* ------------------------------------------------------------- sky glyph */
static void sky_icon(int x, int y, uint32_t ink, uint32_t dim, uint8_t cond) {
  if (cond == 0 || cond == 1) {
    disc(x + 9, y + 9, 7, ink);
    fillr(x + 8, y - 1, 3, 3, ink); fillr(x + 8, y + 17, 3, 3, ink);
    fillr(x - 1, y + 8, 3, 3, ink); fillr(x + 17, y + 8, 3, 3, ink);
    fillr(x + 2, y + 2, 3, 3, ink); fillr(x + 14, y + 2, 3, 3, ink);
    fillr(x + 2, y + 14, 3, 3, ink); fillr(x + 14, y + 14, 3, 3, ink);
  }
  if (cond >= 1) {
    int ox = (cond == 1) ? x + 8 : x + 1;
    int oy = (cond == 1) ? y + 8 : y + 4;
    fillr(ox + 2, oy + 6, 18, 7, dim);
    fillr(ox + 5, oy + 2, 9, 5, dim);
    fillr(ox + 12, oy + 4, 7, 4, dim);
  }
  if (cond == 3) {
    fillr(x + 5, y + 16, 3, 5, ink);
    fillr(x + 11, y + 16, 3, 5, ink);
    fillr(x + 17, y + 16, 3, 5, ink);
  }
}
static void triangle_px(int cx, int y, int w, int h, int dir, uint32_t col) {
  for (int i = 0; i < h; i++) {
    int num = (dir > 0) ? i : (h - i - 1);
    int ww = w - (w * num / h); if (ww < 1) ww = 1;
    fillr(cx - ww / 2, y + i, ww, 1, col);
  }
}

/* ------------------------------------------------------- complications */
static int now_minutes(void) {
  time_t t = time(NULL);
  struct tm *lt = localtime(&t);
  return lt->tm_hour * 60 + lt->tm_min;
}

static void draw_comp(int id, int bx, int by, int bw, int bh, const Livery *L) {
  if (id == COMP_NONE) return;

  fillr(bx, by, bw, bh, L->panel);
  if (L->panel == L->bg) framer(bx, by, bw, bh, 1, L->line);

  uint32_t ink = L->panel_ink;
  uint32_t dim = (L->panel == L->bg) ? L->dim : blend(ink, L->panel, 65);
  { int d = lum(dim) - lum(L->panel); if (d < 0) d = -d; if (d < 90) dim = ink; }

  /* the accent is chosen against the FIELD; on a panel it may vanish */
  uint32_t hot = L->accent;
  { int d = lum(hot) - lum(L->panel); if (d < 0) d = -d; if (d < 60) hot = ink; }

  bool wide = (bw >= 140);
  int p = 6;
  int lab_y = by + p;                        /* cap top of the label   */
  GFont vf   = wide ? s_anton34 : s_anton27;
  int vtrim  = wide ? TRIM_ANTON_34 : TRIM_ANTON_27;
  int vcap   = wide ? CAP_ANTON_34 : CAP_ANTON_27;
  int val_y  = by + bh - p - vcap;           /* cap top of the value   */

  char buf[24];

  switch (id) {
    case COMP_GMT: {
      int tt = now_minutes() + s_set.tz_offset;
      silk_fit(s_set.tz_label, bx + p, lab_y, bw - p * 2 - 12, GTextAlignmentLeft, dim);
      bool day_there = false;
      {
        int m = ((tt % 1440) + 1440) % 1440;
        day_there = (m >= s_wx.sunrise && m < s_wx.sunset);
      }
      if (day_there) fillr(bx + bw - p - 7, lab_y + 2, 7, 7, ink);
      else           framer(bx + bw - p - 7, lab_y + 2, 7, 7, 2, ink);
      draw_time(bx + bw / 2, val_y, vf, vtrim, vcap, tt, ink, ink, dim);
      break;
    }

    case COMP_MOON: {
      int32_t c; bool waxing;
      int pct = moon_pct(time(NULL), &c, &waxing);
      int r = wide ? 17 : 12;
      draw_moon(bx + p + r, by + bh / 2, r, c, waxing, ink, blend(ink, L->panel, 45));
      int tx = bx + p + r * 2 + 6;
      silk_fit(moon_name(pct, waxing), tx, lab_y, bw - (tx - bx) - p,
             GTextAlignmentLeft, dim);
      /* Anton's '%' is wider than two digits, so a full-size one pushes the
       * number onto a second line and gets clipped. Draw it as a small unit
       * marker on the number's baseline instead. */
      snprintf(buf, sizeof(buf), "%d", pct);
      int nw = text_w(buf, vf);
      int room = bw - (tx - bx) - p - 13;
      text_at(buf, vf, vtrim, tx, val_y, room, GTextAlignmentLeft, ink);
      if (nw > room) nw = room;
      silk16("%", tx + nw + 3, val_y + vcap - CAP_SILK_16, 16,
             GTextAlignmentLeft, dim);
      break;
    }

    case COMP_SUN: {
      int nowm = now_minutes();
      bool rise_next = (nowm < s_wx.sunrise) || (nowm >= s_wx.sunset);
      int at   = rise_next ? s_wx.sunrise : s_wx.sunset;
      int mins = at - nowm; if (mins < 0) mins += 1440;
      if (wide) {
        int half = bw / 2;
        uint32_t hotc = rise_next ? hot : dim;
        triangle_px(bx + p + 5, lab_y, 9, 8, -1, hotc);
        silk_fit("RISE", bx + p + 14, lab_y, half - p - 18, GTextAlignmentLeft, dim);
        draw_time(bx + half / 2 + p / 2, val_y, vf, vtrim, vcap,
                  s_wx.sunrise, ink, ink, dim);
        triangle_px(bx + half + p + 5, lab_y, 9, 8, 1, rise_next ? dim : hot);
        silk_fit("SET", bx + half + p + 14, lab_y, half - p - 18, GTextAlignmentLeft, dim);
        draw_time(bx + half + half / 2 - p / 2, val_y, vf, vtrim, vcap,
                  s_wx.sunset, ink, ink, dim);
        fillr(bx + half - 1, by + 5, 1, bh - 10, dim);
      } else {
        triangle_px(bx + p + 5, lab_y, 9, 8, rise_next ? -1 : 1, hot);
        silk_fit(rise_next ? "SUNRISE" : "SUNSET", bx + p + 14, lab_y,
               bw - p * 2 - 16, GTextAlignmentLeft, dim);
        draw_time(bx + bw / 2, val_y - CAP_SILK_8 - 4, vf, vtrim, vcap,
                  at, ink, ink, dim);
        if (mins >= 60) snprintf(buf, sizeof(buf), "IN %dH %02dM", mins / 60, mins % 60);
        else            snprintf(buf, sizeof(buf), "IN %dM", mins);
        silk8(buf, bx + p, by + bh - p - CAP_SILK_8, bw - p * 2,
              GTextAlignmentLeft, dim);
      }
      break;
    }

    case COMP_WX: {
      sky_icon(bx + p, by + p + 2, ink, dim, s_wx.cond);
      int tx = bx + p + (wide ? 30 : 26);
      if (s_wx.valid) snprintf(buf, sizeof(buf), "%d°", s_wx.temp);
      else            snprintf(buf, sizeof(buf), "--°");
      text_at(buf, vf, vtrim, tx, val_y, bw - (tx - bx) - p - (wide ? 46 : 0),
              GTextAlignmentLeft, ink);
      if (!s_wx.valid) break;
      if (wide) {
        snprintf(buf, sizeof(buf), "HI %d", s_wx.hi);
        silk_fit(buf, bx + bw - p - 46, by + bh / 2 - 12, 46, GTextAlignmentRight, dim);
        snprintf(buf, sizeof(buf), "LO %d", s_wx.lo);
        silk_fit(buf, bx + bw - p - 46, by + bh / 2 + 3, 46, GTextAlignmentRight, dim);
      } else {
        snprintf(buf, sizeof(buf), "%d/%d", s_wx.hi, s_wx.lo);
        silk8(buf, bx + p, lab_y + 2, bw - p * 2, GTextAlignmentRight, dim);
      }
      break;
    }

    case COMP_DATE: {
      time_t t = time(NULL);
      struct tm *lt = localtime(&t);
      char dow[8], mon[12], day[4];
      strftime(dow, sizeof(dow), "%a", lt);
      strftime(mon, sizeof(mon), wide ? "%B" : "%b", lt);
      strftime(day, sizeof(day), "%e", lt);
      for (char *q = dow; *q; q++) *q = toupper((int)*q);
      for (char *q = mon; *q; q++) *q = toupper((int)*q);
      char *d = day; while (*d == ' ') d++;

      int px = bx + p, py = by + p, pw = bw - p * 2, ph = bh - p * 2;
      framer(px, py, pw, ph, 2, ink);
      fillr(px + pw / 2 - 9, py, 7, 2, L->panel);       /* zip-tie punches */
      fillr(px + pw / 2 + 2, py, 7, 2, L->panel);

      int head_y  = py + 6;
      int plate_y = by + bh - p - 6;
      int cap = wide ? CAP_ANTON_34 : CAP_ANTON_27;
      GFont df = wide ? s_anton34 : s_anton27;
      int dtrim = wide ? TRIM_ANTON_34 : TRIM_ANTON_27;
      if (wide) {
        silk_fit(dow, px + 6, head_y, pw / 2 - 8, GTextAlignmentLeft, ink);
        silk_fit(mon, px + pw - 6 - (pw / 2 - 8), head_y, pw / 2 - 8,
               GTextAlignmentRight, ink);
      } else {
        silk_fit(dow, px + 6, head_y, pw / 2 - 6, GTextAlignmentLeft, ink);
        silk_fit(mon, px + pw - 6 - (pw / 2 - 6), head_y, pw / 2 - 6,
                 GTextAlignmentRight, ink);
      }
      text_at(d, df, dtrim, px, plate_y - cap, pw, GTextAlignmentCenter, ink);
      break;
    }
  }
}

/* --------------------------------------------------------------- the face */
static void update_proc(Layer *layer, GContext *ctx) {
  g_ctx = ctx;
  const Livery *L = &LIVERIES[s_set.livery % NUM_LIVERIES];
  const uint32_t fallback_multi[4] = { L->accent, L->day, L->night, L->panel };
  const uint32_t *M = L->multi ? L->multi : fallback_multi;

  int nowm = now_minutes();
  time_t t = time(NULL);
  struct tm *lt = localtime(&t);

  /* the mortar between the stones */
  uint32_t gut = (lum(L->bg) > 128) ? L->line : L->ink;
  fillr(0, 0, SCR_W, SCR_H, gut);

  /* A: the time */
  fillr(0, 0, A_W, A_H, L->bg);

  /* B: the kit, at the only size where it reads as fabric */
  draw_swatch(B_X, 0, B_W, A_H, L, false);

  /* D: complications */
  int c1 = s_set.comp1, c2 = s_set.comp2;
  if (c1 == COMP_NONE && c2 != COMP_NONE) { c1 = c2; c2 = COMP_NONE; }
  if (c2 != COMP_NONE) {
    draw_comp(c1, 0, D_Y, 98, D_H, L);
    draw_comp(c2, 101, D_Y, 99, D_H, L);
  } else {
    draw_comp(c1, 0, D_Y, SCR_W, D_H, L);
  }

  /* E: the day as 24 cells */
  fillr(0, E_Y, SCR_W, SCR_H - E_Y, L->bg);
  int now_h = lt->tm_hour;
  for (int i = 0; i < 24; i++) {
    int x = CELL_X0 + i * CELL_STEP;
    int mid = i * 60 + 30;
    uint32_t col = (mid >= s_wx.sunrise && mid < s_wx.sunset) ? L->day : L->night;
    if (mid >= s_wx.sunset - 60 && mid < s_wx.sunset) col = L->accent;
    fillr(x, CELL_Y, CELL_W, CELL_H, col);
    if (i == now_h) {
      fillr(x - 1, CELL_Y - 3, CELL_W + 2, CELL_H + 6, L->ink);
      fillr(x, CELL_Y, CELL_W, CELL_H, col);
      fillr(x + 2, CELL_Y + 6, 3, 4, L->ink);
    }
  }
  fillr(CELL_X0, 183, 192, 2, L->dim);
  {
    uint32_t surf[3] = { L->day, L->night, L->bg };
    uint32_t mk = mark_on(surf, 3, L);
    fillr(CELL_X0 + s_wx.sunrise * 192 / 1440, 181, 2, 6, mk);
    fillr(CELL_X0 + s_wx.sunset  * 192 / 1440, 181, 2, 6, mk);
  }

  /* F: the livery band */
  fillr(0, F_Y, SCR_W, F_H, M[2]);
  fillr(0, F_Y, SCR_W, 2, gut);
  checker(0, F_Y, 24, F_H, 3, 2, L->chk, L->bg);
  silk_fit(L->name, 30, F_Y + 5, 166, GTextAlignmentRight,
        (lum(M[2]) > 150) ? 0x000000 : 0xFFFFFF);

  /* the time, last */
  draw_time(68, 37, s_anton54, TRIM_ANTON_54, CAP_ANTON_54, nowm,
            L->ink, M[0], L->dim);

  /* the one ambient tag: silent until the last hour of light begins */
  if (nowm >= s_wx.sunset - 60 && nowm < s_wx.sunset) {
    silk8("FLAMME ROUGE", 4, 93, 128, GTextAlignmentCenter, L->accent);
  }
}

/* ------------------------------------------------------------ persistence */
static void load_state(void) {
  if (persist_exists(PKEY_SETTINGS))
    persist_read_data(PKEY_SETTINGS, &s_set, sizeof(s_set));
  if (persist_exists(PKEY_WX))
    persist_read_data(PKEY_WX, &s_wx, sizeof(s_wx));
  /* the livery list was renumbered in v2; an older index would name another kit */
  if (s_set.version != SETTINGS_VERSION) {
    s_set.version = SETTINGS_VERSION;
    s_set.livery = 0;
  }
  if (s_set.livery >= NUM_LIVERIES) s_set.livery = 0;
  if (s_set.tz_label[0] == '\0') strncpy(s_set.tz_label, "PARIS", sizeof(s_set.tz_label) - 1);
}
static void save_state(void) {
  persist_write_data(PKEY_SETTINGS, &s_set, sizeof(s_set));
  persist_write_data(PKEY_WX, &s_wx, sizeof(s_wx));
}

/* -------------------------------------------------------------- messaging */
static void inbox_received(DictionaryIterator *it, void *ctx) {
  Tuple *t;
  bool need_save = false;

  if ((t = dict_find(it, MESSAGE_KEY_LIVERY))) {
    int v = t->value->int32;
    if (v >= 0 && v < NUM_LIVERIES) s_set.livery = v;
    need_save = true;
  }
  if ((t = dict_find(it, MESSAGE_KEY_COMP1))) { s_set.comp1 = t->value->int32; need_save = true; }
  if ((t = dict_find(it, MESSAGE_KEY_COMP2))) { s_set.comp2 = t->value->int32; need_save = true; }
  if ((t = dict_find(it, MESSAGE_KEY_UNITS))) { s_set.units = t->value->int32; need_save = true; }
  if ((t = dict_find(it, MESSAGE_KEY_TZ_OFFSET))) {
    s_set.tz_offset = t->value->int32; need_save = true;
  }
  if ((t = dict_find(it, MESSAGE_KEY_TZ_LABEL))) {
    strncpy(s_set.tz_label, t->value->cstring, sizeof(s_set.tz_label) - 1);
    s_set.tz_label[sizeof(s_set.tz_label) - 1] = '\0';
    for (char *q = s_set.tz_label; *q; q++) *q = toupper((int)*q);
    need_save = true;
  }

  if ((t = dict_find(it, MESSAGE_KEY_TEMP))) { s_wx.temp = t->value->int32; s_wx.valid = true; need_save = true; }
  if ((t = dict_find(it, MESSAGE_KEY_HI)))   { s_wx.hi   = t->value->int32; }
  if ((t = dict_find(it, MESSAGE_KEY_LO)))   { s_wx.lo   = t->value->int32; }
  if ((t = dict_find(it, MESSAGE_KEY_COND))) { s_wx.cond = t->value->int32; }
  if ((t = dict_find(it, MESSAGE_KEY_SUNRISE))) {
    s_wx.sunrise = t->value->int32; s_wx.sun_valid = true; need_save = true;
  }
  if ((t = dict_find(it, MESSAGE_KEY_SUNSET))) {
    s_wx.sunset = t->value->int32; s_wx.sun_valid = true; need_save = true;
  }

  if (need_save) save_state();
  layer_mark_dirty(s_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units) {
  layer_mark_dirty(s_layer);
  /* ask the phone for fresh weather every 30 minutes */
  if (tick_time->tm_min % 30 == 0) {
    DictionaryIterator *out;
    if (app_message_outbox_begin(&out) == APP_MSG_OK) {
      dict_write_uint8(out, MESSAGE_KEY_WX_ERR, 0);
      app_message_outbox_send();
    }
  }
}

/* ------------------------------------------------------------------- init */
static void window_load(Window *w) {
  Layer *root = window_get_root_layer(w);
  GRect b = layer_get_bounds(root);
  s_layer = layer_create(b);
  layer_set_update_proc(s_layer, update_proc);
  layer_add_child(root, s_layer);
}
static void window_unload(Window *w) {
  layer_destroy(s_layer);
}

static void init(void) {
  load_state();

  s_anton54 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ANTON_54));
  s_anton34 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ANTON_34));
  s_anton27 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ANTON_27));
  s_silk16  = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SILK_16));
  s_silk12  = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SILK_12));
  s_silk8   = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SILK_8));

  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load, .unload = window_unload
  });
  window_stack_push(s_window, true);

  app_message_register_inbox_received(inbox_received);
  app_message_open(512, 64);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit(void) {
  tick_timer_service_unsubscribe();
  window_destroy(s_window);
  fonts_unload_custom_font(s_anton54);
  fonts_unload_custom_font(s_anton34);
  fonts_unload_custom_font(s_anton27);
  fonts_unload_custom_font(s_silk16);
  fonts_unload_custom_font(s_silk12);
  fonts_unload_custom_font(s_silk8);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
  return 0;
}

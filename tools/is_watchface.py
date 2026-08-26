#!/usr/bin/env python3
"""Exit 0 if the PNG looks like the watchface, 1 if it's a notification or menu.

The face always paints many distinct colours (fabric weave, day cells, panels);
a notification screen is a flat white body with one accent bar."""
import struct, sys, zlib

def read(path):
    d = open(path, 'rb').read()
    pos, w, h, idat = 8, None, None, b''
    while pos < len(d):
        ln = struct.unpack('>I', d[pos:pos+4])[0]
        typ = d[pos+4:pos+8]
        if typ == b'IHDR':
            w, h = struct.unpack('>II', d[pos+8:pos+16])
        elif typ == b'IDAT':
            idat += d[pos+8:pos+8+ln]
        pos += 12 + ln
    raw = zlib.decompress(idat)
    bpp, stride = 4, w * 4
    out, prev, i = bytearray(), bytearray(stride), 0
    for _ in range(h):
        f = raw[i]; i += 1
        line = bytearray(raw[i:i+stride]); i += stride
        for x in range(stride):
            a = line[x-bpp] if x >= bpp else 0
            b = prev[x]
            c = prev[x-bpp] if x >= bpp else 0
            if f == 1: line[x] = (line[x] + a) & 255
            elif f == 2: line[x] = (line[x] + b) & 255
            elif f == 3: line[x] = (line[x] + (a+b)//2) & 255
            elif f == 4:
                p = a + b - c
                pa, pb, pc = abs(p-a), abs(p-b), abs(p-c)
                pr = a if (pa <= pb and pa <= pc) else (b if pb <= pc else c)
                line[x] = (line[x] + pr) & 255
        out += line; prev = line
    return w, h, bytes(out)

w, h, px = read(sys.argv[1])
colours = {px[o:o+3] for o in range(0, len(px), 4)}
# the bottom band is a solid livery colour with a checker at its left edge
band = {px[((y*w)+x)*4:((y*w)+x)*4+3] for y in range(214, min(h, 226)) for x in range(40, w-10)}
ok = len(colours) >= 6 and len(band) <= 3
print(f"colours={len(colours)} band={len(band)} -> {'watchface' if ok else 'NOT watchface'}")
sys.exit(0 if ok else 1)

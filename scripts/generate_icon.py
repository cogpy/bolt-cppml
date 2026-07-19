#!/usr/bin/env python3
"""Generate the Bolt C++ ML application icon (resources/windows/bolt.ico).

Creates a multi-size .ico (16, 32, 48, 256 px) with a simple yellow
lightning-bolt glyph on a dark rounded background, using only the
Python standard library (no Pillow required).

Usage:  python scripts/generate_icon.py
"""

import struct
import zlib
from pathlib import Path

SIZES = [16, 32, 48, 256]

BG = (30, 34, 42, 255)        # dark slate background
BOLT = (255, 200, 40, 255)    # lightning yellow
TRANSPARENT = (0, 0, 0, 0)

# Lightning bolt polygon in a normalized 0..1 coordinate space.
BOLT_POLY = [
    (0.62, 0.04),
    (0.24, 0.56),
    (0.46, 0.56),
    (0.38, 0.96),
    (0.78, 0.42),
    (0.55, 0.42),
]


def point_in_poly(x, y, poly):
    """Ray-casting point-in-polygon test."""
    inside = False
    n = len(poly)
    j = n - 1
    for i in range(n):
        xi, yi = poly[i]
        xj, yj = poly[j]
        if (yi > y) != (yj > y) and x < (xj - xi) * (y - yi) / (yj - yi) + xi:
            inside = not inside
        j = i
    return inside


def render(size):
    """Render one RGBA frame with 3x3 supersampling for smooth edges."""
    ss = 3
    corner = 0.18  # rounded-corner radius as a fraction of size
    pixels = []
    for py in range(size):
        row = []
        for px in range(size):
            bolt_hits = 0
            bg_hits = 0
            for sy in range(ss):
                for sx in range(ss):
                    x = (px + (sx + 0.5) / ss) / size
                    y = (py + (sy + 0.5) / ss) / size
                    # rounded-rect background coverage
                    cx = min(x, 1 - x)
                    cy = min(y, 1 - y)
                    if cx < corner and cy < corner:
                        dx = corner - cx
                        dy = corner - cy
                        in_bg = dx * dx + dy * dy <= corner * corner
                    else:
                        in_bg = True
                    if in_bg:
                        bg_hits += 1
                        if point_in_poly(x, y, BOLT_POLY):
                            bolt_hits += 1
            total = ss * ss
            if bg_hits == 0:
                row.append(TRANSPARENT)
            else:
                t = bolt_hits / total
                a = int(255 * bg_hits / total)
                r = int(BG[0] + (BOLT[0] - BG[0]) * t)
                g = int(BG[1] + (BOLT[1] - BG[1]) * t)
                b = int(BG[2] + (BOLT[2] - BG[2]) * t)
                row.append((r, g, b, a))
        pixels.append(row)
    return pixels


def encode_png(pixels):
    """Encode RGBA pixel rows as a minimal PNG (for the 256px ICO frame)."""
    height = len(pixels)
    width = len(pixels[0])
    raw = b"".join(
        b"\x00" + b"".join(struct.pack("4B", *p) for p in row) for row in pixels
    )

    def chunk(tag, data):
        payload = tag + data
        return struct.pack(">I", len(data)) + payload + struct.pack(
            ">I", zlib.crc32(payload) & 0xFFFFFFFF
        )

    ihdr = struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0)
    return (
        b"\x89PNG\r\n\x1a\n"
        + chunk(b"IHDR", ihdr)
        + chunk(b"IDAT", zlib.compress(raw, 9))
        + chunk(b"IEND", b"")
    )


def encode_bmp_dib(pixels):
    """Encode RGBA pixels as an ICO-style BMP DIB (BGRA, bottom-up, AND mask)."""
    height = len(pixels)
    width = len(pixels[0])
    header = struct.pack(
        "<IiiHHIIiiII", 40, width, height * 2, 1, 32, 0, 0, 0, 0, 0, 0
    )
    body = b""
    for row in reversed(pixels):
        for r, g, b, a in row:
            body += struct.pack("4B", b, g, r, a)
    # AND mask (all zero; alpha channel drives transparency)
    mask_row_len = ((width + 31) // 32) * 4
    mask = b"\x00" * (mask_row_len * height)
    return header + body + mask


def main():
    out = Path(__file__).resolve().parent.parent / "resources" / "windows" / "bolt.ico"
    out.parent.mkdir(parents=True, exist_ok=True)

    images = []
    for size in SIZES:
        pixels = render(size)
        if size >= 256:
            images.append((size, encode_png(pixels)))
        else:
            images.append((size, encode_bmp_dib(pixels)))

    # ICONDIR
    ico = struct.pack("<HHH", 0, 1, len(images))
    offset = 6 + 16 * len(images)
    entries = b""
    data = b""
    for size, blob in images:
        dim = 0 if size >= 256 else size
        entries += struct.pack(
            "<BBBBHHII", dim, dim, 0, 0, 1, 32, len(blob), offset
        )
        data += blob
        offset += len(blob)
    out.write_bytes(ico + entries + data)
    print(f"Wrote {out} ({out.stat().st_size} bytes, sizes: {SIZES})")


if __name__ == "__main__":
    main()

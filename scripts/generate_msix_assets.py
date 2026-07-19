#!/usr/bin/env python3
"""Generate the MSIX visual assets (packaging/msix/Assets/*.png).

Renders the same lightning-bolt design as scripts/generate_icon.py (which
this script imports) at the MSIX scale-100 base sizes, using only the
Python standard library (no Pillow required):

    Square44x44Logo.png    44 x 44    app list / taskbar icon
    StoreLogo.png          50 x 50    Store / installer logo
    Square150x150Logo.png  150 x 150  medium Start tile
    Wide310x150Logo.png    310 x 150  wide Start tile

Usage:  python scripts/generate_msix_assets.py
"""

from pathlib import Path

from generate_icon import BG, BOLT, BOLT_POLY, TRANSPARENT, encode_png, point_in_poly

ASSETS = [
    ("Square44x44Logo.png", 44, 44),
    ("StoreLogo.png", 50, 50),
    ("Square150x150Logo.png", 150, 150),
    ("Wide310x150Logo.png", 310, 150),
]


def render_rect(width, height):
    """Render a width x height RGBA frame with 3x3 supersampling.

    The rounded-rect background fills the whole canvas (corner radius
    relative to the short side); the bolt glyph is mapped into a centered
    square of side min(width, height), matching the icon design.
    """
    ss = 3
    side = min(width, height)
    corner = 0.18 * side  # rounded-corner radius in pixels
    off_x = (width - side) / 2.0
    off_y = (height - side) / 2.0
    pixels = []
    for py in range(height):
        row = []
        for px in range(width):
            bolt_hits = 0
            bg_hits = 0
            for sy in range(ss):
                for sx in range(ss):
                    x = px + (sx + 0.5) / ss
                    y = py + (sy + 0.5) / ss
                    # rounded-rect background coverage (pixel space)
                    cx = min(x, width - x)
                    cy = min(y, height - y)
                    if cx < corner and cy < corner:
                        dx = corner - cx
                        dy = corner - cy
                        in_bg = dx * dx + dy * dy <= corner * corner
                    else:
                        in_bg = True
                    if in_bg:
                        bg_hits += 1
                        # bolt glyph in the centered square, normalized 0..1
                        bx = (x - off_x) / side
                        by = (y - off_y) / side
                        if 0.0 <= bx <= 1.0 and point_in_poly(bx, by, BOLT_POLY):
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


def main():
    out_dir = (
        Path(__file__).resolve().parent.parent / "packaging" / "msix" / "Assets"
    )
    out_dir.mkdir(parents=True, exist_ok=True)

    for name, width, height in ASSETS:
        out = out_dir / name
        out.write_bytes(encode_png(render_rect(width, height)))
        print(f"Wrote {out} ({width}x{height}, {out.stat().st_size} bytes)")


if __name__ == "__main__":
    main()

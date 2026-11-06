#!/usr/bin/env python3
"""Regenerate the Maatra Dark popup images (needs Pillow).

Fcitx5 can only round corners through a stretchable image, so the panel and the
selection pill are PNGs. Keep these constants in step with theme.conf:
  panel Margin = SHADOW + RADIUS, ShadowMargin = SHADOW,
  pill Margin = TextMargin = PILL_SLICE.
"""
from pathlib import Path

from PIL import Image, ImageChops, ImageDraw, ImageFilter

OUT = Path(__file__).parent / "maatra-dark"
SS = 8  # supersampling, for smooth anti-aliased corners

SHADOW = 12  # transparent space around the panel for the shadow
RADIUS = 12
PANEL_SIZE = 2 * (SHADOW + RADIUS) + 8
PANEL_TOP, PANEL_BOTTOM = (36, 36, 54), (27, 27, 41)
PANEL_ALPHA = 248
BORDER = (255, 255, 255, 26)

PILL_RADIUS = 6
PILL_INSET = 1  # keeps a hairline gap between the pill and neighbouring rows
PILL_SLICE = 7  # at least PILL_RADIUS + PILL_INSET
PILL_SIZE = 2 * PILL_SLICE + 8
PILL_TOP, PILL_BOTTOM = (136, 106, 255), (112, 80, 240)


def rounded_mask(size, box, radius):
    mask = Image.new("L", (size * SS, size * SS), 0)
    scaled = [v * SS for v in box]
    ImageDraw.Draw(mask).rounded_rectangle(scaled, radius * SS, fill=255)
    return mask


def gradient(size, top, bottom, first, last):
    """Vertical gradient that runs from row `first` to row `last`, then holds."""
    image = Image.new("RGB", (size, size))
    draw = ImageDraw.Draw(image)
    for y in range(size):
        t = min(max((y - first) / max(last - first, 1), 0), 1)
        draw.line([(0, y), (size, y)], fill=tuple(round(a + (b - a) * t) for a, b in zip(top, bottom)))
    return image


def shrink(image, size):
    return image.resize((size, size), Image.LANCZOS)


def panel():
    size = PANEL_SIZE
    inner = (SHADOW, SHADOW, size - SHADOW, size - SHADOW)
    body = rounded_mask(size, inner, RADIUS)

    # Soft shadow, dropped slightly below the panel.
    shadow = Image.new("L", body.size, 0)
    shadow.paste(body, (0, 2 * SS))
    shadow = shadow.filter(ImageFilter.GaussianBlur(4 * SS)).point(lambda v: int(v * 0.42))
    out = Image.new("RGBA", body.size, (0, 0, 0, 0))
    out.putalpha(shadow)

    # The gradient must end inside the slice, as the middle rows get stretched.
    fill = gradient(size * SS, PANEL_TOP, PANEL_BOTTOM, SHADOW * SS, (SHADOW + RADIUS) * SS).convert("RGBA")
    fill.putalpha(body.point(lambda v: v * PANEL_ALPHA // 255))
    out = Image.alpha_composite(out, fill)

    # 1px border: the body outline minus the body shrunk by one pixel.
    ring = ImageChops.subtract(
        body, rounded_mask(size, tuple(v + d for v, d in zip(inner, (1, 1, -1, -1))), RADIUS - 1)
    )
    border = Image.new("RGBA", body.size, BORDER[:3] + (0,))
    border.putalpha(ring.point(lambda v: v * BORDER[3] // 255))
    return shrink(Image.alpha_composite(out, border), size)


def pill():
    size = PILL_SIZE
    box = (PILL_INSET, PILL_INSET, size - PILL_INSET, size - PILL_INSET)
    fill = gradient(size * SS, PILL_TOP, PILL_BOTTOM, 0, PILL_SLICE * SS).convert("RGBA")
    fill.putalpha(rounded_mask(size, box, PILL_RADIUS))
    return shrink(fill, size)


if __name__ == "__main__":
    panel().save(OUT / "panel.png")
    pill().save(OUT / "highlight.png")

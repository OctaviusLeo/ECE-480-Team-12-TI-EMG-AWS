# png2pal4.py
# Utility script to convert a PNG image into a 4-bit paletted C asset:
# - 16-color RGB565 palette
# - Packed 4-bit index array (2 pixels per byte)
#
# Usage:
#   python png2pal4.py <image.png> <SYMBOL_NAME>
# Example:
#   python png2pal4.py GAME_SINGLE_LOGO.png GAME_SINGLE_LOGO

import sys
from pathlib import Path

from PIL import Image


def rgb_to_rgb565(r, g, b):
    """
    Convert 8-bit-per-channel RGB values into a packed 16-bit RGB565 value.

    Args:
        r (int): Red   component in range [0, 255].
        g (int): Green component in range [0, 255].
        b (int): Blue  component in range [0, 255].

    Returns:
        int: 16-bit RGB565 (rrrrrggggggbbbbb).
    """
    # 8-bit per channel → RGB565
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)


def emit_pal4(image_path: Path, symbol: str):
    """
    Convert a PNG image into a pair of C files: <symbol>.h and <symbol>.c.

    The image is:
      - Loaded as RGBA
      - Thumbnail-constrained to 30x30 while keeping aspect ratio
      - Quantized down to 16 colors (palette mode)
      - Exported as:
          * <symbol>_PAL[] : RGB565 palette (16 entries)
          * <symbol>_IDX[] : packed 4-bit indices (2 pixels per byte)

    Args:
        image_path (Path): Path to the source PNG image.
        symbol (str):      C symbol prefix (e.g., GAME_SINGLE_LOGO).
    """
    # Load image as RGBA
    img = Image.open(image_path).convert("RGBA")

    # Optionally could resize to a fixed size; here we keep aspect ratio
    # but bound the image to 30x30 pixels.
    # img = img.resize((96, 96), Image.LANCZOS)
    img.thumbnail((30, 30), Image.LANCZOS)
    w, h = img.size

    # Quantize to 16 colors using an adaptive palette
    img_q = img.convert("P", palette=Image.ADAPTIVE, colors=16)
    w_q, h_q = img_q.size
    if (w_q, h_q) != (w, h):
        # Safety check: quantization should not change image dimensions
        raise RuntimeError("Unexpected size change during quantize")

    # Get palette (RGB triplets, length 16*3)
    pal = img_q.getpalette()[:16 * 3]  # 16 colors * 3 components
    pal565 = []
    for i in range(0, len(pal), 3):
        r, g, b = pal[i], pal[i + 1], pal[i + 2]
        pal565.append(rgb_to_rgb565(r, g, b))

    # Get pixel indices (0–15) for each pixel in the quantized image
    idx = list(img_q.getdata())
    if len(idx) != w * h:
        # Safety check: pixel count must match width*height
        raise RuntimeError("Unexpected pixel count")

    # Pack 2 pixels per byte: high nibble = first pixel, low nibble = second
    packed = []
    for i in range(0, len(idx), 2):
        i0 = idx[i] & 0x0F
        if i + 1 < len(idx):
            i1 = idx[i + 1] & 0x0F
        else:
            # Pad last pixel if the total count is odd
            i1 = 0
        packed.append((i0 << 4) | i1)

    # Derive base name for C files (e.g. GAME_SINGLE_LOGO -> game_single_logo)
    base = symbol.lower()
    h_name = f"{base}.h"
    c_name = f"{base}.c"

    # Emit C header: declaration of dimensions, palette, and index array
    with open(h_name, "w", encoding="utf-8") as fh:
        fh.write(f"#ifndef {symbol}_H\n")
        fh.write(f"#define {symbol}_H\n\n")
        fh.write("#include <stdint.h>\n\n")
        fh.write(f"#define {symbol}_W {w}\n")
        fh.write(f"#define {symbol}_H {h}\n")
        fh.write(f"#define {symbol}_PAL_SIZE 16\n\n")
        fh.write(f"extern const uint16_t {symbol}_PAL[{symbol}_PAL_SIZE];\n")
        fh.write(
            f"extern const uint8_t  {symbol}_IDX[({symbol}_W * {symbol}_H) / 2];\n\n"
        )
        fh.write(f"#endif // {symbol}_H\n")

    # Emit C file: definitions of palette and packed index data
    with open(c_name, "w", encoding="utf-8") as fc:
        fc.write("#include <stdint.h>\n")
        fc.write(f'#include "{base}.h"\n\n')

        # Palette
        fc.write(f"const uint16_t {symbol}_PAL[{symbol}_PAL_SIZE] = {{\n")
        line = "    "
        for i, c in enumerate(pal565):
            line += f"0x{c:04X}, "
            if (i + 1) % 8 == 0:
                fc.write(line + "\n")
                line = "    "
        if line.strip():
            fc.write(line + "\n")
        fc.write("};\n\n")

        # Index data
        fc.write(
            f"const uint8_t {symbol}_IDX[({symbol}_W * {symbol}_H) / 2] = {{\n"
        )
        line = "    "
        for i, b in enumerate(packed):
            line += f"0x{b:02X}, "
            if (i + 1) % 16 == 0:
                fc.write(line + "\n")
                line = "    "
        if line.strip():
            fc.write(line + "\n")
        fc.write("};\n")

    print(f"Generated {h_name} and {c_name} for {image_path.name} ({w}x{h})")


def main():
    """
    CLI entry point: parse arguments, validate paths, and call emit_pal4().
    """
    if len(sys.argv) != 3:
        print("Usage: python png2pal4.py <image.png> <SYMBOL_NAME>")
        print("Example: python png2pal4.py GAME_SINGLE_LOGO.png GAME_SINGLE_LOGO")
        sys.exit(1)

    img_path = Path(sys.argv[1])
    symbol = sys.argv[2]

    if not img_path.is_file():
        print(f"Input file not found: {img_path}")
        sys.exit(1)

    emit_pal4(img_path, symbol)


if __name__ == "__main__":
    main()

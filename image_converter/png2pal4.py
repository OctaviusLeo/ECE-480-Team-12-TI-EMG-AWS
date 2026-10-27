# png2pal4.py
# Utility script to convert a PNG image into a 4-bit paletted C asset:
# - 16-color RGB565 palette
# - Packed 4-bit index array (2 pixels per byte)
#
# Usage:
#   python png2pal4.py <image.png> <SYMBOL_NAME> [--out-dir DIR] [--max-size N]
# Example:
#   python png2pal4.py GAME_SINGLE_LOGO.png GAME_SINGLE_LOGO
#
# Run "python png2pal4.py --help" for the full option list.

import argparse
import re
import sys
from pathlib import Path

from PIL import Image

# Default bounding box (pixels) the image is shrunk into, kept from the
# original hard-coded value. Larger assets such as full-screen story art
# are generated with a larger --max-size (the OLED is 128x128).
DEFAULT_MAX_SIZE = 30

# The symbol becomes part of C identifiers and header-guard names.
_C_IDENTIFIER = re.compile(r"[A-Za-z_][A-Za-z0-9_]*")


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


def emit_pal4(
    image_path: Path,
    symbol: str,
    out_dir: Path = Path("."),
    max_size: int = DEFAULT_MAX_SIZE,
):
    """
    Convert a PNG image into a pair of C files: <symbol>.h and <symbol>.c.

    The image is:
      - Loaded as RGBA
      - Thumbnail-constrained to max_size x max_size while keeping aspect ratio
      - Quantized down to 16 colors (palette mode)
      - Exported as:
          * <symbol>_PAL[] : RGB565 palette (16 entries)
          * <symbol>_IDX[] : packed 4-bit indices (2 pixels per byte)

    Args:
        image_path (Path): Path to the source PNG image.
        symbol (str):      C symbol prefix (e.g., GAME_SINGLE_LOGO).
        out_dir (Path):    Directory the .h/.c files are written to
                           (created if missing). Defaults to the current
                           working directory.
        max_size (int):    Bounding box edge length in pixels. Defaults to 30.
    """
    # Load image as RGBA
    img = Image.open(image_path).convert("RGBA")

    # Optionally could resize to a fixed size; here we keep aspect ratio
    # but bound the image to max_size x max_size pixels.
    # img = img.resize((96, 96), Image.LANCZOS)
    img.thumbnail((max_size, max_size), Image.LANCZOS)
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
    out_dir = Path(out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)
    h_name = out_dir / f"{base}.h"
    c_name = out_dir / f"{base}.c"

    # Files are always written with LF endings so the output is identical on
    # every OS (the repository stores text files with LF).

    # Two pixels per byte, rounded up so an odd pixel count still fits.
    idx_len = f"(({symbol}_W * {symbol}_H) + 1) / 2"

    # Emit C header: declaration of dimensions, palette, and index array.
    # The include guard must not be "<symbol>_H": that name is the image
    # height macro below, and defining it twice is a redefinition warning.
    guard = f"{symbol}_PAL4_H"
    with open(h_name, "w", encoding="utf-8", newline="\n") as fh:
        fh.write(f"#ifndef {guard}\n")
        fh.write(f"#define {guard}\n\n")
        fh.write("#include <stdint.h>\n\n")
        fh.write(f"#define {symbol}_W {w}\n")
        fh.write(f"#define {symbol}_H {h}\n")
        fh.write(f"#define {symbol}_PAL_SIZE 16\n\n")
        fh.write(f"extern const uint16_t {symbol}_PAL[{symbol}_PAL_SIZE];\n")
        fh.write(f"extern const uint8_t  {symbol}_IDX[{idx_len}];\n\n")
        fh.write(f"#endif // {guard}\n")

    # Emit C file: definitions of palette and packed index data
    with open(c_name, "w", encoding="utf-8", newline="\n") as fc:
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
        fc.write(f"const uint8_t {symbol}_IDX[{idx_len}] = {{\n")
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


def parse_args(argv=None):
    """
    Build the command-line parser and parse argv (defaults to sys.argv[1:]).
    """
    parser = argparse.ArgumentParser(
        prog="png2pal4.py",
        description=(
            "Convert an image into a 4-bit paletted C asset (<symbol>.h and "
            "<symbol>.c) for the SSD1351 OLED: a 16-entry RGB565 palette "
            "plus packed 4-bit pixel indices, two pixels per byte."
        ),
        epilog=(
            "example: python png2pal4.py GAME_SINGLE_LOGO.png GAME_SINGLE_LOGO"
        ),
    )
    parser.add_argument("image", type=Path, help="source image (PNG, JPG, ...)")
    parser.add_argument(
        "symbol",
        help="C symbol prefix, e.g. GAME_SINGLE_LOGO; output files are the "
        "lowercase name (game_single_logo.h / game_single_logo.c)",
    )
    parser.add_argument(
        "--out-dir",
        type=Path,
        default=Path("."),
        metavar="DIR",
        help="directory for the generated files (default: current directory)",
    )
    parser.add_argument(
        "--max-size",
        type=int,
        default=DEFAULT_MAX_SIZE,
        metavar="N",
        help="shrink the image to fit inside N x N pixels, keeping aspect "
        f"ratio (default: {DEFAULT_MAX_SIZE}; use 128 for full-screen art)",
    )
    args = parser.parse_args(argv)

    if not _C_IDENTIFIER.fullmatch(args.symbol):
        parser.error(f"SYMBOL_NAME is not a valid C identifier: {args.symbol!r}")
    if args.max_size < 1:
        parser.error("--max-size must be at least 1")
    if not args.image.is_file():
        parser.error(f"input file not found: {args.image}")
    return args


def main(argv=None):
    """
    CLI entry point: parse arguments, validate paths, and call emit_pal4().
    """
    args = parse_args(argv)
    emit_pal4(args.image, args.symbol, args.out_dir, args.max_size)


if __name__ == "__main__":
    sys.exit(main())

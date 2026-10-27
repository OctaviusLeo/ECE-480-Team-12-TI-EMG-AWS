"""Tests for png2pal4.py: the command line and the C files it generates.

Run from the image_converter folder:  pytest
"""

import re
import shutil
import subprocess
from pathlib import Path

import pytest
from PIL import Image

import png2pal4

CONVERTER_DIR = Path(__file__).resolve().parents[1]
REPO_ROOT = CONVERTER_DIR.parent
HAVE_GCC = shutil.which("gcc") is not None

# Pure colors survive RGB565 and 16-color quantization unchanged, so the
# round-trip test can compare exactly.
COLORS = [(255, 0, 0), (0, 255, 0), (0, 0, 255), (255, 255, 255)]


def make_image(path, size, pixel=None):
    """Save an RGBA test image; pixel(x, y) returns an (r, g, b) tuple."""
    w, h = size
    img = Image.new("RGBA", size)
    for y in range(h):
        for x in range(w):
            r, g, b = pixel(x, y) if pixel else (200, 100, 50)
            img.putpixel((x, y), (r, g, b, 255))
    img.save(path)
    return path


def convert(tmp_path, size, *extra, symbol="TEST_IMG", pixel=None):
    """Run the converter on a generated image; return (h_text, c_text)."""
    src = make_image(tmp_path / "in.png", size, pixel)
    out = tmp_path / "out"
    png2pal4.main([str(src), symbol, "--out-dir", str(out), *extra])
    base = symbol.lower()
    return (out / f"{base}.h").read_text(), (out / f"{base}.c").read_text()


def hex_values(c_text, array_name):
    """Return the integers inside `<array_name>[...] = { ... };`."""
    body = re.search(rf"{array_name}\[[^\]]*\]\s*=\s*\{{(.*?)\}};", c_text, re.DOTALL)
    assert body, f"{array_name} initializer not found"
    return [int(v, 16) for v in re.findall(r"0x[0-9A-Fa-f]+", body.group(1))]


def define(h_text, name):
    match = re.search(rf"#define {name} (\d+)", h_text)
    assert match, f"#define {name} <number> not found"
    return int(match.group(1))


def test_rgb565_primary_colors():
    assert png2pal4.rgb_to_rgb565(0, 0, 0) == 0x0000
    assert png2pal4.rgb_to_rgb565(255, 255, 255) == 0xFFFF
    assert png2pal4.rgb_to_rgb565(255, 0, 0) == 0xF800
    assert png2pal4.rgb_to_rgb565(0, 255, 0) == 0x07E0
    assert png2pal4.rgb_to_rgb565(0, 0, 255) == 0x001F


def test_round_trip_pixels_and_palette(tmp_path):
    """Decoding the generated arrays gives back the original colors."""
    size = (6, 4)

    def pixel(x, y):
        return COLORS[(x + y) % len(COLORS)]

    h_text, c_text = convert(tmp_path, size, pixel=pixel)
    pal = hex_values(c_text, "TEST_IMG_PAL")
    packed = hex_values(c_text, "TEST_IMG_IDX")

    assert (define(h_text, "TEST_IMG_W"), define(h_text, "TEST_IMG_H")) == size
    assert define(h_text, "TEST_IMG_PAL_SIZE") == 16
    assert len(pal) == 16
    assert len(packed) == (size[0] * size[1] + 1) // 2

    nibbles = [n for byte in packed for n in (byte >> 4, byte & 0x0F)]
    for y in range(size[1]):
        for x in range(size[0]):
            expected = png2pal4.rgb_to_rgb565(*pixel(x, y))
            assert pal[nibbles[y * size[0] + x]] == expected


def test_odd_pixel_count_packs_a_padded_last_byte(tmp_path):
    """5x3 = 15 pixels needs 8 bytes; the last low nibble is padding."""
    _, c_text = convert(tmp_path, (5, 3), pixel=lambda x, y: COLORS[x % 4])
    packed = hex_values(c_text, "TEST_IMG_IDX")
    assert len(packed) == 8
    assert packed[-1] & 0x0F == 0


@pytest.mark.skipif(not HAVE_GCC, reason="gcc not installed")
@pytest.mark.parametrize("size", [(1, 1), (5, 3), (6, 4), (30, 30)])
def test_generated_c_compiles_cleanly(tmp_path, size):
    """The .c compiles warning-free and the header sizes match the data."""
    w, h = size
    convert(tmp_path, size, pixel=lambda x, y: COLORS[(x * y) % 4])
    out = tmp_path / "out"
    check = out / "check.c"
    check.write_text(
        '#include "test_img.h"\n'
        f'_Static_assert(TEST_IMG_W == {w}, "width");\n'
        f'_Static_assert(TEST_IMG_H == {h}, "height");\n'
        f'_Static_assert(sizeof(TEST_IMG_IDX) == {(w * h + 1) // 2}, "bytes");\n'
    )
    for source in ("test_img.c", "check.c"):
        result = subprocess.run(
            ["gcc", "-std=c11", "-Wall", "-Wextra", "-Werror", "-c",
             source, "-o", source + ".o"],
            check=False,
            cwd=out,
            capture_output=True,
            text=True,
        )
        assert result.returncode == 0, result.stderr


def test_default_max_size_is_30(tmp_path):
    h_text, _ = convert(tmp_path, (100, 50))
    assert (define(h_text, "TEST_IMG_W"), define(h_text, "TEST_IMG_H")) == (30, 15)


def test_max_size_option(tmp_path):
    h_text, _ = convert(tmp_path, (100, 50), "--max-size", "40")
    assert (define(h_text, "TEST_IMG_W"), define(h_text, "TEST_IMG_H")) == (40, 20)


def test_out_dir_is_created(tmp_path):
    src = make_image(tmp_path / "in.png", (4, 4))
    out = tmp_path / "a" / "b"
    png2pal4.main([str(src), "ICON", "--out-dir", str(out)])
    assert (out / "icon.h").is_file()
    assert (out / "icon.c").is_file()


def test_output_uses_lf_line_endings(tmp_path):
    convert(tmp_path, (4, 4))
    out = tmp_path / "out"
    assert b"\r" not in (out / "test_img.c").read_bytes()
    assert b"\r" not in (out / "test_img.h").read_bytes()


@pytest.mark.parametrize("symbol", ["9lives", "has-dash", "has space", ""])
def test_invalid_symbol_is_rejected(tmp_path, symbol):
    src = make_image(tmp_path / "in.png", (4, 4))
    with pytest.raises(SystemExit) as exc:
        png2pal4.main([str(src), symbol])
    assert exc.value.code == 2


def test_missing_input_is_rejected(tmp_path):
    with pytest.raises(SystemExit) as exc:
        png2pal4.main([str(tmp_path / "missing.png"), "X"])
    assert exc.value.code == 2


def test_help_exits_zero(capsys):
    with pytest.raises(SystemExit) as exc:
        png2pal4.main(["--help"])
    assert exc.value.code == 0
    assert "--out-dir" in capsys.readouterr().out


def test_regenerates_committed_story_ch1(tmp_path):
    """story_ch1.png -> src/story_ch1.c reproduces the committed data.

    A golden-file check on the quantizer and packer: the palette and pixel
    bytes must match the firmware asset exactly. It assumes the Pillow version
    pinned in requirements.txt. (The array-size expression is not compared:
    the generator now rounds it up for odd pixel counts.)
    """
    png2pal4.main(
        [
            str(CONVERTER_DIR / "story_ch1.png"),
            "STORY_CH1",
            "--out-dir",
            str(tmp_path),
            "--max-size",
            "128",
        ]
    )
    generated = (tmp_path / "story_ch1.c").read_text()
    committed = (REPO_ROOT / "src" / "story_ch1.c").read_text()
    for array in ("STORY_CH1_PAL", "STORY_CH1_IDX"):
        assert hex_values(generated, array) == hex_values(committed, array)

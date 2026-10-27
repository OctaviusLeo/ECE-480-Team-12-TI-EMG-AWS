# image_converter

`png2pal4.py` converts an image into a 4-bit paletted C asset for the SSD1351 OLED (128x128, RGB565) used by the firmware in `src/` and `include/`. For an image it writes:

- `<symbol>.h`: the width/height macros and `extern` declarations.
- `<symbol>.c`: a 16-entry RGB565 palette (`<SYMBOL>_PAL`) and the pixel data (`<SYMBOL>_IDX`) as 4-bit palette indices, two pixels per byte (first pixel in the high nibble).

The image is shrunk to fit inside a square bounding box (aspect ratio kept) and quantized to 16 colors before export.

## Setup

1. Run PowerShell as Administrator, then execute: `Set-ExecutionPolicy RemoteSigned`
2. In the IDE's PowerShell terminal, go to the image_converter folder: `cd image_converter`
3. Create the venv once (it is not committed): `python -m venv .venv`
4. Activate it with `.venv\Scripts\Activate.ps1` (Linux/macOS: `source .venv/bin/activate`), then install the dependency: `pip install -r requirements.txt`

The only runtime dependency is Pillow (pinned in `requirements.txt`).

## Usage

```
python png2pal4.py <image> <SYMBOL_NAME> [--out-dir DIR] [--max-size N]
```

Example, writing `pvp_tie_pic.h` and `pvp_tie_pic.c` into the current folder:

```
python png2pal4.py PVP_TIE_PIC.png PVP_TIE_PIC
```

Copy the generated `.c` file into `src/` and the `.h` file into `include/` (or into the CCS project folder) to use them on the MCU's OLED.

| Argument | Meaning |
|---|---|
| `image` | Source image. PNG is what the repository uses; any format Pillow can open works. |
| `SYMBOL_NAME` | C symbol prefix, for example `PVP_TIE_PIC`. It must be a valid C identifier. Output files use the lowercase name. |
| `--out-dir DIR` | Where to write the `.h` and `.c` files. Created if missing. Default: current folder. |
| `--max-size N` | Fit the image inside N x N pixels. Default: 30 (icon size). Use 128 for full-screen art. |

`python png2pal4.py --help` prints the same list.

## Reproducing the committed assets

Larger assets such as `story_ch1` and `you_died` need a bigger bounding box than the default. With the pinned Pillow version, for example:

```
python png2pal4.py story_ch1.png STORY_CH1 --max-size 128
```

writes a `story_ch1.c` whose palette and pixel data are identical to `src/story_ch1.c` (the tests check this for `story_ch1`), and `you_died.png` does the same for `src/you_died.c`. Other assets may differ if they were generated with another size or another Pillow version.

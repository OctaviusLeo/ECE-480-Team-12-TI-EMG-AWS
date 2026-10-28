# ECE-480-Team-12-TI-EMG-AWS
Arm Wrestling Evaluation System using EMG (Team 12 MSU ECE 480 Fall 2025)

<img width="500" height="394" alt="image" src="https://github.com/user-attachments/assets/74e90ff7-2ca9-409e-b385-2ceaa0cf5645" /><img width="500" height="394" alt="image" src="https://github.com/user-attachments/assets/5d0b1536-6639-4c18-8b31-0596d9e4c375" />

A no-contact arm-wrestling challenge powered by surface EMG. A dry electrode armband measures muscle activation, our analog front end conditions the signal and streams it to a microcontroller for real time strength visualization and two player head to head comparison.

Demo: https://youtu.be/2_F4ziM3Mrs

**Not a medical device. Research/education prototype only.**

**Contents**

- [My role](#my-role)
- [Why this exists](#why-this-exists)
- [Key behaviors](#key-behaviors)
- [Features](#features)
- [Project status](#project-status-fall-2025-msu-ece-480)
- [Hardware](#hardware-prototype)
- [Building and flashing](#building-and-flashing)
- [Repository layout](#repository-layout)
- [Validation & Verification](#validation--verification-vv-summary)
- [Safety, ethics, and use](#safety-ethics-and-use)
- [Milestones](#milestones-public-timeline-fall-2025)
- [Team 12](#team-12)
- [License](#license)
- [Acknowledgments](#acknowledgments)

**My role**
---
Symaedchit Leo: sole software member of a four-person Texas Instruments-sponsored senior design team (Fall 2025). Wrote the TM4C123 C firmware in `src/` and `include/`: the ADS131M02 SPI ADC driver, SSD1351 OLED graphics, the menus and four game modes (Playground, PVP, Tower, Story), and the EMG measure (rate of rising zero-crossings over 100 ms windows, corrected by a 3 s rest baseline). Also wrote the `image_converter` tool.

**Why this exists**
---

Make bio-signal sensing tangible and fun.

Practice end-to-end ECE: sensor -> analog chain -> ADC/MCU -> UI -> V&V.

Deliver a repeatable, safe, and explainable user experience for Design Day.

                                        
**Key behaviors**
---
- Playground Mode: Clear EMG detected indication (single-user).
- PVP Mode: simultaneous readout + winner decision logic.
- Tower Mode: Challenge every difficulty tier by flexing.
- Story Mode: An actual story with lore, decisions and battles determined by flexing.
- END + TROPHY: Shows credit to all members, university, sponsor, and achievements of user.


**Features**
---
Dry-electrode armband (non-adhesive) for quick don/doff and multi user demo.

High CMRR front end designed to tolerate motion and 50/60 Hz mains.

Configurable gain partition (INA vs. post-filter) to prevent saturation.

EMG measure: rate of rising zero-crossings over 100 ms windows, corrected by a 3 s rest baseline.

Safety & UX: input protection, strain relief, sanitize between users flow.

---
Project Status Fall 2025 (MSU ECE 480)
---

Requirements & architecture

Prototype analog chain & firmware

UI polish & stretch PC dashboard

See [Milestones](#milestones-public-timeline-fall-2025) for the public timeline.

---
**Quickstart**
---
**Hardware (prototype)**
---
Dry-electrode EMG armband (non-adhesive).

Instrumentation amplifier + op-amp filters (HP/BP/LP + notch).

ADC (resolution/SPS chosen for EMG bandwidth & comparator latency).

MCU (developed in Code Composer Studio).

Indicators: LCD and/or LED bar, optional HDMI/USB to PC dashboard.

BOM: not included in this repository. The analog front end is described by the SPICE netlist in [`AFE`](AFE) and the schematic in [`AFE_Schematic.png`](AFE_Schematic.png).

Parts the firmware drives, as wired in `src/` and `include/`:

| Part | What the firmware uses it for | Connection |
|---|---|---|
| TI Tiva C TM4C123 microcontroller | Runs everything: sampling, EMG measure, menus and games | 80 MHz system clock from the PLL with a 16 MHz crystal (`src/main.c`) |
| TI ADS131M02 ADC | Delivers the conditioned EMG signal; the firmware reads channel 1 | SSI2: PB4 clock, PB6 MISO, PB7 MOSI, PB5 chip select (GPIO), PE3 DRDY. SPI mode 1 at 1 MHz (`src/ads131m02.c`) |
| SSD1351 OLED | 128x128 RGB565 display for the menus and games | SSI0: PA2 clock, PA5 MOSI, PA3 chip select (GPIO), PB2 data/command, PB3 reset. SPI mode 3, 4 MHz during init then 8 MHz (`src/ssd1351.c`, `include/board.h`) |
| Dry-electrode EMG armband | Signal source | Conditioned by the analog front end in [`AFE`](AFE) and [`AFE_Schematic.png`](AFE_Schematic.png) |


**Building and flashing**
---
Toolchain: TI Code Composer Studio (CCS) with TivaWare for C Series. The steps below use TivaWare_C_Series-2.2.0.295 and its `ek-tm4c123gxl` `project0` example. No CCS project file or Makefile is committed here: you create the project from TI's example and link these sources into it, as described below.

This will only have the include (.h) & source (.c) files. 
Any errors, you can exclude that file (and fix later) or fix that specific error in any steps.

Make this change if needed: 
In startup_ccs.c, find:

```c
#pragma DATA_SECTION(g_pfnVectors, ".intvecs")
void (* const g_pfnVectors[])(void) =
{
    (void (*)(void))((uint32_t)&__STACK_TOP),
    ResetISR,
    NmiSR,
    FaultISR,
    ...
};
```

Change it to:

```c
__attribute__((section(".intvecs"), used))
void (* const g_pfnVectors[])(void) =
{
    (void (*)(void))((uint32_t)&__STACK_TOP),
    ResetISR,      // The reset handler
    NmiSR,         // The NMI handler
    FaultISR,      // The hard fault handler
    ...
};
```

and delete the #pragma DATA_SECTION line.

1. Download the Software Development Kit: https://www.ti.com/tool/SW-TM4C
2. Download CCS (Code Composer Studio).
3. CCS -> "File" -> "Import Projects" -> ...\driverlib\ccs\Debug\driverlib.lib
4. "Project" -> "Build All"
5. "File" -> "Import Projects" -> ...\ti\TivaWare_C_Series-2.2.0.295\examples\boards\ek-tm4c123gxl\project0
6. "Project" -> "Build All"
7. Download include and source files (here) to same folder containing "project0".
8. Right click "Project0" in Explorer (on the left) -> "Add files/folders" -> "Select files to link" -> link ALL of source and include files except `src/blinky.c` (a separate signal-chain test program with its own `main()`).
9. Connect your pc to MCU using the given cable.
10. "Project" -> "Build All" -> "Run" -> "Debug Project"
11. in DEBUG window -> "Resume" -> Should work

Errors: After "Project" -> "Build All", check "Project" -> "Properties" -> "Arm Linker" look over all options
Also check: "Run" -> "Debug Properties" -> lookover all 4 windows.
For this MCU, if a file has too much code in it, the debugging will NOT work. You must create more files and reference those (#include) to that main file in order for the MCU to process it. 

---
Repository layout
---
| Path | Contents |
|---|---|
| `src/` | C firmware for the TM4C123: `main.c` (entry point, 3 s rest baseline, main loop), `ads131m02.c` (ADC driver), `ssd1351.c` and `gfx.c` (OLED driver and graphics), `menu.c` and `game*.c` (the menus and the four game modes: Playground, PVP, Tower, Story), `save.c` (flash save with CRC32), `cheevos.c` (achievements), and the image assets (palette plus 4-bit index arrays, the format `image_converter` writes). `blinky.c` is a separate signal-chain test program with its own `main()` |
| `include/` | Headers for `src/`, including `project.h` (sample rate, round length, color definitions) and `board.h` (OLED pin mapping) |
| `image_converter/` | `png2pal4.py`, which turns a PNG into the C image assets, with its README, the source images and `tests/` |
| `AFE` | SPICE netlist of the analog front end: an INA333 instrumentation amplifier, a 20-450 Hz band-pass stage and a right-leg-drive circuit built from OPA333 op-amps on a 3.3 V supply. The two `.INC` lines point at TI's INA333 and OPAx333 model files on the netlist author's computer; edit them to your local paths |
| `AFE_Schematic.png` | Schematic of the analog front end |
| `.github/workflows/` | CI for `image_converter/`: `ruff` lint and the `pytest` suite |
| `README.md` | This file |

---
Validation & Verification (V&V) Summary
---
Signal detection: Medium: The frequencies (Hz) from sEMG -> ADC -> MCU

Latency: Medium: The EMG measure is recomputed every 100 ms (one 100 ms sample window per loop); the UART debug line prints every 500 ms.

Repeatability: Medium: Every part can be bought.

Safety: High: No danger.


---
Safety, Ethics, and Use
---
Education prototype only. Not intended for diagnosis, therapy, or life support.

Use only on healthy, uninjured skin. Do not use with implanted medical devices.

Follow sanitize between users SOP. Stop immediately on discomfort.

---
Milestones (Public Timeline Fall 2025)
---
Pre-proposal: Sept 21, 2025

Proposal presentations: late Sept -> early Oct

Final proposal: Oct 19, 2025

Demo-1 (mid-semester): late Oct

Standards presentation: Nov 7, 2025

Demo-2 (prototype): Nov 17, 2025

Poster due: Dec 2, 2025

Final report: Dec 3, 2025

Design Day: Dec 5, 2025

Code & equipment return: Dec 10, 2025

---
Contributing (course context)
---
**Team 12**
-
- Ervin Alaij - Hardware: Doc Handler, Budget Manager
- Symaedchit Leo - Software: Online Coordinator, Meeting/Recorder and Scribe, DOM
- Andrew Perez - Hardware: Lab Coordinator, Project Demo Manager
- Pratijit Podder - Hardware: Project Manager, Presentation Handler, Communications Handler

**Facilitator**
-
Prof. Joydeep Mitra (MSU)

**Sponsor**
-
Texas Instruments POC: Gerasimos “Jerry” Madalvanos

**For external visitors: please use GitHub Issues for questions.**

---
License
---
TI & MSU

---
Acknowledgments
---
Texas Instruments (sponsorship & technical guidance)

MSU ECE 480 faculty & staff

Classmates and reviewers who provided feedback during demos

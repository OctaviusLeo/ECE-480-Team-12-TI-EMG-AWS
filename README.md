<img width="500" height="394" alt="image" src="https://github.com/user-attachments/assets/74e90ff7-2ca9-409e-b385-2ceaa0cf5645" /><img width="500" height="394" alt="image" src="https://github.com/user-attachments/assets/5d0b1536-6639-4c18-8b31-0596d9e4c375" />




# ECE-480-Team-12-TI-EMG-AWS
Arm Wrestling Evaluation System using EMG (Team 12 MSU ECE 480 Fall 2025)

A no-contact arm-wrestling challenge powered by surface EMG. A dry electrode armband measures muscle activation, our analog front end conditions the signal and streams it to a microcontroller for real time strength visualization and two player head to head comparison.

**Not a medical device. Research/education prototype only.**

**Why this exists**
---

Make bio-signal sensing tangible and fun.

Practice end-to-end ECE: sensor -> analog chain -> ADC/MCU -> UI -> V&V.

Deliver a repeatable, safe, and explainable user experience for Design Day.

                                        
**Key behaviors**
---
Playground Mode: Clear EMG detected indication (single-user).
PVP Mode: simultaneous readout + winner decision logic.
Tower Mode: Challenge every difficulty tear by flexing.
Story Mode: Actual store with lore, decisions and battles determined by flexing.


**Features**
---
Dry-electrode armband (non-adhesive) for quick don/doff and multi user demo.

High CMRR front end designed to tolerate motion and 50/60 Hz mains.

Configurable gain partition (INA vs. post-filter) to prevent saturation.

Latency aware comparator (windowed RMS/peak envelope with tunable window).

Safety & UX: input protection, strain relief, sanitize between users flow.

---
Project Status Fall 2025 (MSU ECE 480)
---

Requirements & architecture

Prototype analog chain & firmware

UI polish & stretch PC dashboard

See Milestones for the public timeline.

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

BOM: See /docs/bom.md (placeholders provided, fill with final part numbers).


**Software**
---
This will only have the include (.h) & source (.c) files. 
Any errors, you can exclude that file (and fix later) or fix that specific error in any steps.

Make this change if needed: 
In startup_ccs.c, find:

#pragma DATA_SECTION(g_pfnVectors, ".intvecs")
void (* const g_pfnVectors[])(void) =
{
    (void (*)(void))((uint32_t)&__STACK_TOP),
    ResetISR,
    NmiSR,
    FaultISR,
    ...
};


Change it to:

__attribute__((section(".intvecs"), used))
void (* const g_pfnVectors[])(void) =
{
    (void (*)(void))((uint32_t)&__STACK_TOP),
    ResetISR,      // The reset handler
    NmiSR,         // The NMI handler
    FaultISR,      // The hard fault handler
    ...
};


and delete the #pragma DATA_SECTION line.

1. Download the Software Developement Kit: https://www.ti.com/tool/SW-TM4C?utm_source=google&utm_medium=cpc&utm_campaign=epd-null-null-44700045336317887_prodfolderdynamic-cpc-pf-google-ww_en_int&utm_content=prodfolddynamic&ds_k=DYNAMIC+SEARCH+ADS&DCM=yes&gclsrc=aw.ds&gad_source=1&gad_campaignid=12236057696&gbraid=0AAAAAC068F23ah0bGYvpOZEGrYN7NGfCW&gclid=CjwKCAiAt8bIBhBpEiwAzH1w6TN2LpLLjL4gkm1rOxRtDTfot4sC7BeKlQ9Beu3hVgtyNW1VzUM2lhoCFucQAvD_BwE#downloads
2. Download CCS (Code Composer Studio).
4. CCS -> "File" -> "Import Projects" -> ...\driverlib\ccs\Debug\driverlib.lib
5. "Project" -> "Build All"
6. "File" -> "Import Projects" -> ...\ti\TivaWare_C_Series-2.2.0.295\examples\boards\ek-tm4c123gxl\project0
7. "Project" -> "Build All"
8. Download include and source files (here) to same folder containing "project0".
9. Right click "Project0" in Explorer (on the left) -> "Add files/folders" -> "Select files to link" -> link ALL of source and include files.
10. Connect your pc to MCU using the given cable.
11. "Project" -> "Build All" -> "Run" -> "Debug Project"
12. in DEBUG window -> "Resume" -> Should work

Errors: After "Project" -> "Build All", check "Project" -> "Properties" -> "Arm Linker" look over all options
Also check: "Run" -> "Debug Properties" -> lookover all 4 windows.
For this MCU, if a file has too much code in it, the debugging will NOT work. You must create more files and reference those (#include) to that main file in order for the MCU to process it. 

---
Repository Structure
---
.

|- include

|- src

|_ README.md

---
Validation & Verification (V&V) Summary
---
Signal detection: Medium: The frequencies (Hz) from sEMG -> ADC -> MCU

Latency: Medium: Detects every 0.5 seconds.

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
Ervin Alaij - Hardware
Doc Handler
Budget Manager

Symaedchit Leo - Software
Online Coordinator
Meeting/Recorder and Scribe
DOM

Andrew Perez - Hardware
Lab Coordinator
Project Demo Manager

Pratijit Podder - Hardware
Project Manager
Presentation Handler
Communications Handler

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

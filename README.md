<img width="500" height="394" alt="image" src="https://github.com/user-attachments/assets/74e90ff7-2ca9-409e-b385-2ceaa0cf5645" /><img width="500" height="394" alt="image" src="https://github.com/user-attachments/assets/5d0b1536-6639-4c18-8b31-0596d9e4c375" />




# ECE-480-Team-12-TI-EMG-AWS
Arm Wrestling Evaluation System using EMG (Team 12 MSU ECE 480 Fall 2025)

A no-contact arm-wrestling challenge powered by surface EMG. A dry electrode armband measures muscle activation, our analog front end conditions the signal and streams it to a microcontroller for real time strength visualization and two player head to head comparison.

**Not a medical device. Research/education prototype only.**

---
Demo (preview)
---
<!-- Replace with actual media once available -->

Real-time EMG strength bar (single-user)

Two player “Winner” indicator

Optional PC dashboard (stretch)

Why this exists

Make bio-signal sensing tangible and fun.

Practice end-to-end ECE: sensor -> analog chain -> ADC/MCU -> UI -> V&V.

Deliver a repeatable, safe, and explainable user experience for Design Day.

---
**System Overview (Not sure if this is correct)**
---

DRY EMG BAND(differential) -> Instrumentation Amplfier -> HP/BP/Notce?LP Filters -> Anti-alias

                                        |
                                        
                                        > ADC -> Whatever IDE -> LCD/LEDs + PC UI 
                                        
                                        |
                                        
                                        > two user comparator (RMS/peak evelope)
                                        

**Key behaviors**
---
Clear EMG detected indication (single-user).

Two-player mode: simultaneous readout + winner decision logic.

Optional stored averages/difficulty tiers for solo play.


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
Firmware: /firmware/ (CCS project)

PC dashboard (optional): /tools/dashboard/ (Python)

py -m venv .venv && .venv\Scripts\activate (Win) or source .venv/bin/activate (Unix)

pip install -r requirements.txt

python app.py

---
Repository Structure (ideally but probably not)
---
.

|- /docs

|  |- design/        # block diagrams, schematics PDFs, gain/bias calcs(Google Drive)

|  |- test/          # V&V plan, test cases, acceptance criteria

|  |- bom.md         # bill of materials

|  |- usage.md       # operator procedure & sanitation SOP

|- /hardware

|  |- pcb/           # KiCad/Altium sources, fab outputs

|  |_ prototypes/    # breadboard/EVM notes, wiring, photos

|- /firmware         # CCS project, drivers, comparator logic, UI

|- /tools

|  |_ dashboard/     # optional PC real-time visualization

|- /assets           # images, logos, demo GIFs

|_ README.md

---
Build & Run (Firmware)
---
Symaedchit Leo's job. 

---
Validation & Verification (V&V) Summary
---
Signal detection: ???

Comparator correctness: ???

Latency: ???

Repeatability: ???

Safety: ???

Full details: ???

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

Demo-2 (prototype): Nov 10–21, 2025

Poster due: Dec 2, 2025

Final report: Dec 3, 2025

Design Day: Dec 5, 2025

Code & equipment return: Dec 10, 2025

---
Contributing (course context)
---
**Team 12**
-
Ervin Alaij - Sensor & Analog Lead,
			Dry-electrode band handling, input protecting, instrumentation amp, filters, signals.
		Doc Handler + treasurer

Symaedchit Leo - Firmware & Data Lead,
			MCU code that reads ADC, then cleans, then into an integer, then shows using bars or lights or UI, and writes a winner. 
				Online Coordinator

Andrew Perez - Lab coordinator + Product Demo Manager,
PCB & Hardware,
Designs PCB, power, connectors, test points, strain relief.


Pratijit Podder - Freelance agent - start w. Assistance in hardware and then can consider the specifications or goals that are unmet - will flesh it out soon
Presentation Handler, Manager, Meeting Recorder/Scribe



**Facilitator**
-
Prof. Joydeep Mitra (MSU)


**Sponsor**
-
Texas Instruments POC: Gerasimos “Jerry” Madalvanos

**For external visitors: please use GitHub Issues for questions.**

---
Roadmap (For now)
---
Lock part selections & EVM prototype path

Bring-up analog chain on bench (gain/CMRR/noise verified)

Firmware comparator + UI MVP

Two-player integration & acceptance tests

Enclosure & demo polish; PC dashboard (stretch)

---
License
---
???

---
Acknowledgments
---
Texas Instruments (sponsorship & technical guidance)

MSU ECE 480 faculty & staff

Classmates and reviewers who provided feedback during demos

# Robot Arm Project

Six-axis robot arm control project using FPGA + ARM SoC platforms.

## Goals

Build a robust multi-axis control system for a robot arm using FPGA-based low-level motor/encoder control and higher-level planning on embedded processors.

## Hardware

- Amatrol Pegasus II robot arm
- Zybo Z7-20
- DE10-Nano
- External motor drivers
- Incremental encoders

## Repository layout

- `docs/` architecture notes, register maps, bring-up docs, roadmap
- `hardware/` board-specific notes, schematics, wiring
- `fpga/` RTL, simulation, constraints, Vivado/Quartus projects and scripts
- `software/` bare-metal, Linux, PetaLinux, userspace utilities
- `control/` kinematics, trajectory generation, calibration, experiments
- `scripts/` helper scripts for build, flash, setup, packaging
- `tools/` small utilities
- `test/` testing assets
- `notes/` lab notebook and debugging notes

## Build philosophy

Generated tool output is not tracked in Git.  
The repository should contain the source and scripts needed to recreate builds.

## Current status

- [ ] Repository initialized
- [ ] FPGA bring-up structure in place
- [ ] Software structure in place
- [ ] Register map documented
- [ ] First working hardware milestone tagged

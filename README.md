# Robot Arm Control System

Custom FPGA + ARM-based control system for a 6-axis robot arm.
Implements real-time motor control, encoder feedback, and a foundation for future trajectory planning.

---

## Quick Start

```
git clone <repo>
cd robot-arm
./scripts/run_vivado.sh
```

---

## Current Repository Structure

```
.
├── docs/
├── fpga/
│   ├── bd/
│   ├── constraints/
│   ├── ip/
│   └── rtl/
├── notes/
├── scripts/
├── software/
│   └── shared/include/
└── README.md
```

---

## FPGA Architecture

The system is built around a Zynq-7000 SoC:

* ARM Processing System (PS) communicates over AXI
* Custom FPGA logic (PL) handles real-time control
* One control channel per robot axis

Core components:

* multi_axis_motion_control (AXI slave + top-level logic)
* axis_control_channel (per-axis control)
* motor_controller_core (PWM generation and control)
* quadrature_decoder_core (encoder feedback)
* channel_signal_dispatcher (signal routing)

---

## Build System

The project uses a fully scripted Vivado flow.

### Build command

```
./scripts/run_vivado.sh
```

### What the script does

* Creates a clean Vivado project
* Loads local IP repository
* Recreates the block design
* Generates the HDL wrapper
* Runs synthesis and implementation
* Generates bitstream
* Exports hardware platform (XSA)

---

## Build Outputs

```
build/
├── vivado/
│   └── robot_controller.xpr
├── robot_controller.xsa
```

Bitstream and implementation artifacts:

```
build/vivado/robot_controller.runs/impl_1/
```

---

## Development Workflow

### FPGA Changes

1. Modify RTL or IP
2. Repackage IP if needed (Vivado IP Packager)
3. Run build script

### Block Design Changes

1. Modify BD in Vivado

2. Export updated BD Tcl:

   ```
   write_bd_tcl -force ./fpga/bd/robot_controller_bd.tcl
   ```

3. Commit changes

---

## Notes

* The `build/` directory is generated and should not be committed
* Vivado project files are reproducible from source
* Packaged IP must remain self-contained (no external path dependencies)

---

## Planned Expansion

```
.
├── control/        # kinematics, trajectory planning
├── hardware/       # wiring, schematics
├── test/           # validation and testbenches
├── tools/          # utilities and scripts
```

These will be added as the system evolves.

---

## Documentation

See:

* docs/architecture.md
* docs/register_map.md
* docs/bringup_checklist.md
* docs/roadmap.md

---

## Status

* Scripted build flow working
* Bitstream and XSA generation successful
* Block design under version control
* Ongoing cleanup of RTL and IP structure

---

# Architecture Overview

## Design Hierarchy
```
Project
└── robot_controller.xpr
    └── IP
        └── multi_axis_motion_control.sv
            └── Instance
                └── multi_axis_motion_control_slave_S00_AXI.sv
                    └── Dispatcher
                        └── channel_signal_dispatcher.sv
                            └── Channel
                                └── axis_control_channel.sv
                                    ├── motor_controller_core.v
                                    └── quadrature_decoder_core.v
```
## Module Breakdown

### Project
- **robot_controller.xpr**
  - Top-level Vivado project

### IP
- **multi_axis_motion_control.sv**
  - Top-level RTL wrapper for multi-axis system
  - Instantiates AXI slave interface and channel dispatcher

### AXI Instance
- **multi_axis_motion_control_slave_S00_AXI.sv**
  - AXI4-Lite slave interface
  - Handles register reads/writes
  - Bridges PS ↔ PL

### Dispatcher
- **channel_signal_dispatcher.sv**
  - Routes control and data signals to individual axis channels
  - Handles multi-axis scaling

### Channel
- **axis_control_channel.sv**
  - Per-axis control logic
  - Implements control loop and register interface
  - Connects motor + encoder cores

### Core Modules

#### motor_controller_core.v
- Generates PWM / motor drive signals
- Handles direction and drive control

#### quadrature_decoder_core.v
- Decodes encoder A/B signals
- Tracks position and motion direction

## Notes
- One axis_control_channel is instantiated per axis
- Design is scalable via dispatcher + channel abstraction
- AXI interface provides control from ARM (PS)
- All timing-critical control loops run in PL

## Data Flow

### High-Level Flow

```
ARM (PS)
   │
   ▼
AXI4-Lite Interface
(multi_axis_motion_control_slave_S00_AXI.sv)
   │
   ▼
Channel Dispatcher
(channel_signal_dispatcher.sv)
   │
   ▼
Per-Axis Channel
(axis_control_channel.sv)
   │
   ├──► Motor Control
   │       (motor_controller_core.v)
   │       └── PWM / Direction Outputs
   │
   └──► Encoder Feedback
           (quadrature_decoder_core.v)
           └── Position / Direction
   │
   ▼
Status + Position Registers
   │
   ▼
AXI Readback → ARM (PS)
```

---

### Per-Axis Control Loop

```
        Target Position (AXI write)
                  │
                  ▼
        axis_control_channel
                  │
                  ▼
        Control Logic (P / future PID)
                  │
                  ▼
        motor_controller_core
                  │
                  ▼
              Motor
                  │
                  ▼
             Encoder
                  │
                  ▼
        quadrature_decoder_core
                  │
                  ▼
        Position Feedback
                  │
                  └──────────────┐
                                 │
                                 ▼
                        Error Calculation
```

---

### Timing Model

- Control loop runs in **PL (FPGA fabric)**
- Loop timing driven by **servo tick**
  - Current: ~2 kHz (`Servo_Tick_Ctr`)
- AXI access is **asynchronous** to control loop
  - PS writes:
    - `Target`
    - `Control`
  - PS reads:
    - `Position`
    - `Status`
    - `Window_Count`

---

### Data Ownership

| Data                     | Source                        | Consumer                  |
|--------------------------|-------------------------------|---------------------------|
| Target                   | PS (ARM)                      | Axis control loop         |
| Control                  | PS                            | Axis control logic        |
| Position                 | Encoder (PL)                  | PS + control loop         |
| Motor Command            | Control loop                  | Motor driver              |
| Window_Count / Velocity  | Encoder core                  | PS                        |
| Status                   | Axis + system logic           | PS                        |

---

### Key Design Principles

- **Hard real-time in PL**
  - Control loop, encoder decode, PWM
- **Soft real-time in PS**
  - Planning, coordination, UI, safety logic
- **Register interface = contract**
  - AXI registers decouple PS and PL timing
- **Per-axis isolation**
  - Each channel runs independently
  - Scales cleanly to N axes

---

### Debug Strategy (important later)

- Verify **data path direction**:
  1. Write `Target` from PS
  2. Observe motor output (PWM)
  3. Verify encoder counts change
  4. Confirm `Position` updates
- Use:
  - `Debug0`, `Debug1`
  - `Window_Count`
  - `Servo_Tick_Ctr`

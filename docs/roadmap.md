# Robot Controller Roadmap

## Project Goal

Build a scalable multi-axis FPGA/SoC-based robot controller for the Amatrol Pegasus II robot arm, featuring:

- Closed-loop motor control per axis
- Encoder-based position feedback
- Homing and limit handling
- Watchdog and safety supervision
- AXI-based PS ↔ PL control interface
- PS-side trajectory planning and coordination
- Clean, maintainable, and extensible architecture

---

## Current Status

### Implemented

- AXI4-Lite register interface
- Multi-axis dispatcher architecture
- Per-axis control channel (`axis_control_channel`)
- Motor controller core
- Quadrature decoder core
- Global control and watchdog registers
- Initial register map documentation
- Architecture documentation

### In Progress

- Register map validation vs RTL
- Single-axis hardware bring-up
- Software-side register access utilities
- Documentation refinement

---

## Development Phases

### Phase 1 - Basic PL Bring-Up

- Confirm bitstream loads reliably
- Verify AXI register read/write
- Validate global registers
- Verify servo tick operation
- Validate one axis encoder feedback

---

### Phase 2 - Single-Axis Closed Loop

- Write `Target` from PS
- Verify `Position` feedback updates
- Validate `Window_Count` behavior
- Tune proportional control (`KP`)
- Confirm stable stop behavior
- Validate stall detection

---

### Phase 3 - Multi-Axis Expansion

- Enable all six axes
- Verify dispatcher routing
- Confirm register isolation per axis
- Validate simultaneous multi-axis operation

---

### Phase 4 - Safety and Homing

- Implement homing behavior
- Validate homing thresholds (`H_Enter`, `H_Exit`, `H_Center`)
- Define safe-state behavior
- Validate watchdog operation
- Define fault handling and recovery

---

### Phase 5 - PS-Side Motion Layer

- Build register access abstraction layer
- Implement coordinated motion commands
- Add trajectory generation (position/velocity profiles)
- Add state-machine-based motion sequencing

---

### Phase 6 - System Integration

- Integrate with full robot hardware
- Validate repeatable motion
- Verify limits and safety interlocks
- Develop calibration procedure
- Document full system bring-up

---

## Near-Term Next Steps

- Verify register definitions against RTL
- Resolve or rename reserved/debug registers
- Complete `register_map.md`
- Complete `architecture.md`
- Bring up one axis (encoder + motor)
- Validate closed-loop response on one axis

---

## Future Enhancements

- Velocity register and feedback
- Full PID control (add `KI`, `KD`)
- Trajectory planner (jerk-limited profiles)
- Homing state machine documentation
- Fault logging / debug capture registers
- Linux user-space control tools
- Calibration workflow
- Vision-assisted feedback integration
- Collision avoidance / dynamic limits

---

## Notes

- Keep register map, C headers, and RTL aligned
- Consider auto-generation of register definitions
- Maintain per-axis isolation for scalability

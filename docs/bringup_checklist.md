# Bring-Up Checklist

## Pre-Flight

- [ ] Vivado project builds without errors
- [ ] Bitstream generated successfully
- [ ] Hardware design (XSA) matches current RTL
- [ ] Software register definitions match RTL
- [ ] Board power supply verified
- [ ] Encoder wiring checked
- [ ] Motor power disconnected (initial tests)
- [ ] Emergency stop path available and understood

---

## FPGA / Platform Bring-Up

- [ ] Program FPGA bitstream
- [ ] Confirm design loads without errors
- [ ] Verify AXI peripheral base address
- [ ] Read `Hw_ID_Register`
- [ ] Read `Servo_Tick_Ctr` twice and confirm it increments
- [ ] Read `Sys_Status`

---

## Global Register Checks

- [ ] Read `Sys_Control`
- [ ] Write `Sys_Control[0] = 1`
- [ ] Confirm `SYS_STATUS_ENABLED` in `Sys_Status`
- [ ] Read `Watchdog_Status`
- [ ] Enable watchdog
- [ ] Kick watchdog
- [ ] Clear watchdog
- [ ] Confirm expected watchdog state transitions

---

## Single-Axis Validation (Encoder Only)

- [ ] Read axis `Position`
- [ ] Confirm stable value when stationary
- [ ] Move encoder by hand
- [ ] Confirm `Position` updates
- [ ] Confirm `Window_Count` updates
- [ ] Verify direction bit behavior
- [ ] Verify no unexpected jumps or noise

---

## Single-Axis Validation (Motor)

⚠️ Ensure safe conditions before enabling motor

- [ ] Mechanism unloaded or safe to move
- [ ] Apply low motor command
- [ ] Confirm PWM output present
- [ ] Confirm direction output correct
- [ ] Verify motor responds correctly
- [ ] Confirm encoder matches motion direction

---

## Closed-Loop Test (Single Axis)

- [ ] Write small `Target`
- [ ] Confirm axis begins motion
- [ ] Verify `Position` approaches `Target`
- [ ] Confirm stable stop near target
- [ ] Verify no oscillation or runaway
- [ ] Disable system and confirm motion stops

---

## Multi-Axis Validation

- [ ] Repeat single-axis tests for each axis
- [ ] Confirm no register overlap
- [ ] Confirm commands affect only intended axis
- [ ] Verify simultaneous multi-axis operation

---

## Bring-Up Notes

- Record base address used
- Record git commit / bitstream version
- Record any issues encountered
- Record temporary workarounds
- Record known limitations

---

## Exit Criteria

Bring-up is considered successful when:

- [ ] Global registers behave as expected
- [ ] Encoder feedback is stable and correct
- [ ] Motor control works in both directions
- [ ] Closed-loop position control is stable (single axis)
- [ ] All axes operate independently without interference

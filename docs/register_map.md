# Multi-Axis Motion Control Register Map

## Base Address

```
ROBOT_BASEADDR = XPAR_MULTI_AXIS_MOTION_CO_0_BASEADDR
```

---

## Axis Offsets

| Axis        | Offset |
|-------------|--------|
| Base        | 0x000  |
| Shoulder    | 0x040  |
| Elbow       | 0x080  |
| Right Wrist | 0x0C0  |
| Left Wrist  | 0x100  |
| Grabber     | 0x140  |

Each axis occupies **0x40 bytes**.

---

## Axis Registers (`axis_regs_t`)

| Offset | Name         | Access | Description |
|--------|--------------|--------|-------------|
| 0x00   | Control      | RW     | Control register |
| 0x04   | Motor_Cmd    | RW     | Motor command |
| 0x08   | Position     | RO     | Current position |
| 0x0C   | Target       | RW     | Target position |
| 0x10   | H_Enter      | RW     | Homing enter threshold |
| 0x14   | H_Exit       | RW     | Homing exit threshold |
| 0x18   | H_Center     | RW     | Homing center |
| 0x1C   | Status       | RO     | Status flags |
| 0x20   | KP           | RW     | Proportional gain |
| 0x24   | Limits       | RW     | Motion limits |
| 0x28   | Window_Count | RO     | [31]=Direction, [30]=Stopped, [23:0]=Encoder counts in current window |
| 0x2C   | Stall        | RO     | Stall indicator |
| 0x30   | Window_Size  | RW     | Encoder window size parameter |
| 0x34   | Reserved     | —      | Reserved |
| 0x38   | Debug0       | RW     | Debug register 0 |
| 0x3C   | Debug1       | RW     | Debug register 1 |

---

## Global Registers

Offset: `0x180`

| Offset | Name               | Access | Description |
|--------|--------------------|--------|-------------|
| 0x00   | Sys_Control        | RW     | [0]=SYS_ENABLE_REQ |
| 0x04   | Reserved0          | RW     | Reserved |
| 0x08   | Sys_Status         | RO     | [1]=SYS_STATUS_SAFE, [0]=SYS_STATUS_ENABLED |
| 0x0C   | Hw_ID_Register     | RO     | Hardware / Verilog version |
| 0x10   | Reserved1          | RW     | Reserved; verify intended use in RTL/software |
| 0x14   | Watchdog_Ctl_Set   | WO     | [2]=WD_KICK_CMD, [1]=WD_CLEAR_CMD, [0]=WD_ENABLE_CMD |
| 0x18   | Watchdog_Ctl_Clr   | WO     | Clear [1]=WD_CLEAR_CMD, [0]=WD_ENABLE_CMD |
| 0x1C   | Watchdog_Status    | RO     | [3]=WD_STATUS_ACTIVE, [2]=WD_STATUS_OK, [1]=WD_STATUS_EXPIRED, [0]=WD_STATUS_ENABLED |
| 0x20   | Servo_Tick_Ctr     | RO     | Servo tick counter (~2 kHz) |

---

## Address Calculation

### Axis Register

```
ADDR = ROBOT_BASEADDR + AXIS_OFFSET + REGISTER_OFFSET
```

### Global Register

```
ADDR = ROBOT_BASEADDR + 0x180 + REGISTER_OFFSET
```

---

## Bitfield Breakdown

### Axis Registers

#### `Window_Count` (`0x28`, RO)

| Bit(s) | Name      | Description |
|--------|-----------|-------------|
| 31     | Direction | Encoder direction for current window |
| 30     | Stopped   | Axis stopped indicator |
| 29:24  | Reserved  | Reserved |
| 23:0   | Count     | Encoder counts accumulated during window |

---

### Global Registers

#### `Sys_Control` (`0x00`, RW)

| Bit(s) | Name           | Description |
|--------|----------------|-------------|
| 0      | SYS_ENABLE_REQ | System enable request |
| 31:1   | Reserved       | Reserved |

#### `Sys_Status` (`0x08`, RO)

| Bit(s) | Name               | Description |
|--------|--------------------|-------------|
| 1      | SYS_STATUS_SAFE    | System is in safe state |
| 0      | SYS_STATUS_ENABLED | System is enabled |
| 31:2   | Reserved           | Reserved |

#### `Reserved1` (`0x10`, RW)

| Bit(s) | Name     | Description |
|--------|----------|-------------|
| 31:0   | Reserved | Reserved; verify actual use in RTL/software |

#### `Watchdog_Ctl_Set` (`0x14`, WO)

| Bit(s) | Name          | Description |
|--------|---------------|-------------|
| 2      | WD_KICK_CMD   | Kick watchdog |
| 1      | WD_CLEAR_CMD  | Clear watchdog state |
| 0      | WD_ENABLE_CMD | Enable watchdog |
| 31:3   | Reserved      | Reserved |

#### `Watchdog_Ctl_Clr` (`0x18`, WO)

| Bit(s) | Name          | Description |
|--------|---------------|-------------|
| 1      | WD_CLEAR_CMD  | Clear `WD_CLEAR_CMD` control bit |
| 0      | WD_ENABLE_CMD | Clear `WD_ENABLE_CMD` control bit |
| 31:2   | Reserved      | Reserved |

#### `Watchdog_Status` (`0x1C`, RO)

| Bit(s) | Name              | Description |
|--------|-------------------|-------------|
| 3      | WD_STATUS_ACTIVE  | Watchdog currently active |
| 2      | WD_STATUS_OK      | Watchdog healthy / not expired |
| 1      | WD_STATUS_EXPIRED | Watchdog expired |
| 0      | WD_STATUS_ENABLED | Watchdog enabled |
| 31:4   | Reserved          | Reserved |

---

## Notes

- All registers are 32-bit aligned.
- Axis stride = `0x40` bytes.
- Global block starts at `0x180`.
- Undefined bits should be written as zero unless documented otherwise.
- Reserved bits should be ignored on read unless needed for debug.
- Update this document before changing software headers or register definitions.

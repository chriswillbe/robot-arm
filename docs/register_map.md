🧠 Multi-Axis Motion Control Register Map
🧭 Base Address
ROBOT_BASEADDR = XPAR_MULTI_AXIS_MOTION_CO_0_BASEADDR
🔩 Axis Register Blocks (per axis)

Each axis occupies 0x40 bytes.

Axis	Offset
Base	0x000
Shoulder	0x040
Elbow	0x080
Right Wrist	0x0C0
Left Wrist	0x100
Grabber	0x140
📦 axis_regs_t Layout (per axis)
Offset	Register	Access	Description
0x00	Control	RW	Control register
0x04	Motor_Cmd	RW	Motor command (PWM / direction / etc.)
0x08	Position	RO	Current encoder position
0x0C	Target	RW	Target position
0x10	H_Enter	RW	Homing enter threshold
0x14	H_Exit	RW	Homing exit threshold
0x18	H_Center	RW	Homing center position
0x1C	Status	RO	Axis status flags
0x20	KP	RW	Proportional gain
0x24	Limits	RW	Motion limits
0x28	Window_Count	RO	31=Dir, 30=Stopped, 23:0=Encoder counts
0x2C	Stall	RO	Stall detection
0x30	Window_Size	RW	Encoder window size
0x34	Reserved	—	Reserved
0x38	Debug0	RW	Debug register 0
0x3C	Debug1	RW	Debug register 1
🌐 Global Register Block
Block Name	Offset
Globals	0x180
📦 global_regs_t Layout
Offset	Register	Access	Description
0x00	Sys_Control	RW	Bit 0 → SYS_ENABLE_REQ
0x04	Reserved0	RW	Reserved
0x08	Sys_Status	RO	Bit 1 → SAFE, Bit 0 → ENABLED
0x0C	Hw_ID_Register	RO	Hardware / Verilog version
0x10	Reserved1	RW	Includes watchdog kick (bit 0)
0x14	Watchdog_Ctl_Set	WO	Bit 2 → KICK, 1 → CLEAR, 0 → ENABLE
0x18	Watchdog_Ctl_Clr	WO	Bit 1 → CLEAR, 0 → ENABLE
0x1C	Watchdog_Status	RO	Bit 3 → ACTIVE, 2 → OK, 1 → EXPIRED, 0 → ENABLED
0x20	Servo_Tick_Ctr	RO	Servo tick counter (~2 kHz)
🧮 Absolute Address Formula (important for your ARM code)

For any axis register:

addr = ROBOT_BASEADDR + AXIS_OFFSET + REG_OFFSET;

Example (Elbow Position):

addr = BASE + 0x080 + 0x08;

Global register:

addr = BASE + 0x180 + REG_OFFSET;

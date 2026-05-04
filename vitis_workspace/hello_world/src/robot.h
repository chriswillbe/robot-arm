 
// robot_types.h
#pragma once

#include <stdint.h>
#include <stdbool.h>

//------------------------------------------------------------------------------
// Core intent
//------------------------------------------------------------------------------
// - Use fixed-width types for anything that crosses a boundary:
//   * HW registers (AXI / PS peripheral regs)
//   * values read/written to PL
//   * math that must be reproducible
// - Use 'int' only for small local bookkeeping (loop indices, tiny counters),
//   where width does not matter.
//------------------------------------------------------------------------------

   
//------------------------------------------------------------------------------
// Canonical fixed-width aliases (readability)
//------------------------------------------------------------------------------
typedef int8_t    s8;
typedef uint8_t   u8;
typedef int16_t   s16;
typedef uint16_t  u16;
typedef int32_t   s32;
typedef uint32_t  u32;
typedef int64_t   s64;
typedef uint64_t  u64;

//------------------------------------------------------------------------------
// Hardware register conventions
//------------------------------------------------------------------------------
// Zynq AXI4-Lite registers are typically 32-bit wide.
// Use these whenever you are reading/writing memory-mapped registers.
typedef u32 reg32_t;
typedef s32 sreg32_t;

// Helpful macros for register offsets
#ifndef REG32_OFFSET
#define REG32_OFFSET(n) ((u32)((n) * 4U))   // n-th 32-bit register offset
#endif

//------------------------------------------------------------------------------
// Robot control domain types
//------------------------------------------------------------------------------
// Encoder counts are signed: forward/backward motion should be representable.
// (If your PL reports unsigned counts, convert at the boundary.)
typedef s32 enc_count_t;

// Delta counts over a control period (signed)
typedef s32 enc_delta_t;

// Encoder velocity in counts/sec (signed). Keep 32-bit for now; promote to 64-bit
// if you multiply large deltas by high rates.
typedef s32 enc_cps_t;

// Control loop tick counter (monotonic). Unsigned makes overflow well-defined.
typedef u32 tick_t;

//------------------------------------------------------------------------------
// Timing types
//------------------------------------------------------------------------------
// Timer ticks or nanoseconds can overflow 32-bit quickly.
// - Use u32 for short-period timer ticks (e.g., 1ms ISR load values).
// - Use u64 for accumulated time or absolute timestamps.
typedef u32 time_ticks32_t;
typedef u64 time_ticks64_t;

//------------------------------------------------------------------------------
// PWM / motor command types
//------------------------------------------------------------------------------
// Signed command at the control layer: sign = direction, magnitude = duty request.
typedef s32 motor_cmd_t;

// PWM duty as an unsigned magnitude (implementation-defined scaling)
typedef u16 pwm_duty_t;

// Direction bit / polarity
typedef bool motor_dir_t;

//------------------------------------------------------------------------------
// Saturation / clamp helpers (avoid accidental unsigned promotions)
//------------------------------------------------------------------------------
static inline s32 clamp_s32(s32 x, s32 lo, s32 hi)
{
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

static inline u32 clamp_u32(u32 x, u32 lo, u32 hi)
{
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

// Absolute value for s32 without dragging in stdlib
static inline s32 abs_s32(s32 x)
{
    return (x < 0) ? -x : x;
}

//------------------------------------------------------------------------------
// Safe math patterns (optional but handy)
//------------------------------------------------------------------------------
// Multiply two s32 values into s64 to avoid overflow, then clamp if needed.
static inline s64 mul_s32_to_s64(s32 a, s32 b)
{
    return (s64)a * (s64)b;
}

// Example: counts/sec = dcount * CONTROL_HZ
// Use s64 intermediate if CONTROL_HZ might be large or dcount bursts.
static inline enc_cps_t cps_from_delta(enc_delta_t dcount, u32 control_hz)
{
    s64 cps64 = (s64)dcount * (s64)control_hz;

    // Clamp to s32 range to avoid wrap if something goes nuts
    if (cps64 > (s64)INT32_MAX) return (enc_cps_t)INT32_MAX;
    if (cps64 < (s64)INT32_MIN) return (enc_cps_t)INT32_MIN;
    return (enc_cps_t)cps64;
}

#ifndef CPU_CLK_HZ
#define CPU_CLK_HZ XPAR_CPU_CORE_CLOCK_FREQ_HZ
#endif

#define ROBOT_BASEADDR  ((uintptr_t)XPAR_MULTI_AXIS_MOTION_CO_0_BASEADDR)
/* Axis offsets     */
#define BASE_AXIS_OFFSET            0x000u
#define SHOULDER_AXIS_OFFSET        0x040u
#define ELBOW_AXIS_OFFSET           0x080u
#define RIGHT_WRIST_AXIS_OFFSET     0x0c0u
#define LEFT_WRIST_AXIS_OFFSET      0x100u
#define GRABBER_AXIS_OFFSET         0x140u
#define GLOBALS_OFFSET              0x180u

typedef struct __attribute__((aligned(4))) {
    u32 Control;           // 0x00     
    u32 Motor_Cmd;         // 0x04
    u32 Position;          // 0x08
    u32 Target;            // 0x0c
    u32 H_Enter;           // 0x10
    u32 H_Exit;            // 0x14
    u32 H_Center;          // 0x18
    u32 Status;            // 0x1c
    u32 KP;                // 0x20
    u32 Limits;            // 0x24
    u32 Window_Count;      // 0x28     31 - Direction, 30 - Stopped, 23:0 - Encoder count this window
    u32 Stall;             // 0x2c
    u32 Window_Size;       // 0x30     Window size parameter in encoder core
    u32 Reserved;          // 0x34
    u32 Debug0;            // 0x38
    u32 Debug1;            // 0x3c
} axis_regs_t;

typedef struct __attribute__((aligned(4))) {
                           // Byte    Reg   Offset   Access    Key
    u32 Sys_Control;       // 0x00     96      0       rw      Bit:  0 - SYS_ENABLE_REQ  
    u32 Reserved0;         // 0x04     97      1       rw      
    u32 Sys_Status;        // 0x08     98      2       ro      Bits: 1 - SYS_STATUS_SAFE, 0 - SYS_STATUS_ENABLED
    u32 Hw_ID_Register;    // 0x0c     99      3       ro      Verilog version
    u32 Reserved1;         // 0x10    100      4       rw      Holds full count, bit 0 -> WD_KICK
    u32 Watchdog_Ctl_Set;  // 0x14    101      5       wo      Bits: 2 - WD_KICK_CMD, 1 - WD_CLEAR_CMD, 0 - WD_ENABLE_CMD
    u32 Watchdog_Ctl_Clr;  // 0x18    102      6       wo      Clear Bits: 1 - WD_CLEAR_CMD 0 - WD_ENABLE_CMD
    u32 Watchdog_Status;   // 0x1c    103      7       ro      Bits: 3 - WD_STATUS_ACTIVE, 2- WD_STATUS_OK, 1 - WD_STATUS_EXPIRED, 0 - WD_STATUS_ENABLED
    u32 Servo_Tick_Ctr;    // 0x20    104      8       ro      confirm clocking, this is currently 2 kHz
} global_regs_t;

#define WD_STATUS_ACTIVE   (1u << 3)
#define WD_STATUS_OK       (1u << 2)
#define WD_STATUS_EXPIRED  (1u << 1)
#define WD_STATUS_ENABLED  (1u << 0)

#define WD_KICK_CMD        (1u << 2)
#define WD_CLEAR_CMD       (1u << 1)
#define WD_ENABLE_CMD      (1u << 0)

#define SYS_ENABLE_REQ     (1u << 0)

#define SYS_STATUS_SAFE    (1u << 1)
#define SYS_STATUS_ENABLED (1u << 0)

#define BASE_AXIS ((volatile axis_regs_t*)((uintptr_t)ROBOT_BASEADDR + (uintptr_t)BASE_AXIS_OFFSET))
#define SHOULDER_AXIS ((volatile axis_regs_t*)((uintptr_t)ROBOT_BASEADDR + (uintptr_t)SHOULDER_AXIS_OFFSET))
#define ELBOW_AXIS ((volatile axis_regs_t*)((uintptr_t)ROBOT_BASEADDR + (uintptr_t)ELBOW_AXIS_OFFSET))
#define RIGHT_WRIST_AXIS ((volatile axis_regs_t*)((uintptr_t)ROBOT_BASEADDR + (uintptr_t)RIGHT_WRIST_AXIS_OFFSET))
#define LEFT_WRIST_AXIS ((volatile axis_regs_t*)((uintptr_t)ROBOT_BASEADDR + (uintptr_t)LEFT_WRIST_AXIS_OFFSET))
#define GRABBER_AXIS ((volatile axis_regs_t*)((uintptr_t)ROBOT_BASEADDR + (uintptr_t)GRABBER_AXIS_OFFSET))
#define GLOBALS ((volatile global_regs_t*)((uintptr_t)ROBOT_BASEADDR + (uintptr_t)GLOBALS_OFFSET))

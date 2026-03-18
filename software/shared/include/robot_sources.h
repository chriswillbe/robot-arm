 
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

/* Axis offsets     */
#define BASE_AXIS            0x000
#define SHOULDER_AXIS        0x040
#define ELBOW_AXIS           0x080
#define RIGHT_WRIST_AXIS     0x0c0
#define LEFT_WRIST_AXIS      0x100
#define GRABBER_AXIS         0x140
#define GLOBALS              0x180

/* Register offsets */
#define CONTROL_REGISTER     0x00
#define M_CMD_REGISTER       0x04
#define HW_ID_REGISTER       0X08
#define POSITION_REGISTER    0x08       // 31 - Direction, 30 - Stopped, 23:0 - Speed
#define TARGET_REGISTER      0x0C

#define HOME_ENTER_REGISTER  0x10
#define HOME_EXIT_REGISTER   0x14
#define HOME_CENTER_REGISTER 0x18
#define STATUS_REGISTER      0x1C

#define KP_REGISTER          0x20
#define LIMITS_REGISTER      0x24
#define VELOCITY_REGISTER    0x28
#define STALL_REGISTER       0x2C

#define DEBUG0_REGISTER      0x30
#define DEBUG1_REGISTER      0x34
#define RESERVED0_REGISTER   0x38
#define RESERVED1_REGISTER   0x3C

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

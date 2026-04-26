/******************************************************************************
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/* Test link
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdint.h>
#include <stdio.h>
//#include <xil_io.h>
#include "platform.h"
#include "robot.h"
#include "xil_printf.h"

#include "xparameters.h"
#include "xparameters_ps.h"
#include "xil_exception.h"

#include "xgpiops.h"

#include "xscugic.h"
#include "xscutimer.h"

static XScuGic   Gic;
static XScuTimer ScuTimer;

static volatile int control_tick = 0;

// ---- Pick your desired loop rate here ----
#define CONTROL_HZ 1000

// ---- IDs from xparameters.h ----
// Most Zynq BSPs use these exact macros:
#define GIC_DEVICE_ID      XPAR_SCUGIC_SINGLE_DEVICE_ID
#define SCUTIMER_DEVICE_ID XPAR_SCUTIMER_DEVICE_ID
#define SCUTIMER_INTR_ID   XPAR_SCUTIMER_INTR

// Global GPIO instance
//static XGpioPs Gpio;

// Change this to your board's LED pin
// (Zybo Z7 example often uses MIO 7 or similar)
#define LED_PIN 7

/*static int InitPsGpio(void)
{
    XGpioPs_Config *Cfg;
    int Status;

    Cfg = XGpioPs_LookupConfig(XPS_GPIO_BASEADDR); // works when 0
    if (!Cfg) return XST_FAILURE;

    Status = XGpioPs_CfgInitialize(&Gpio, Cfg, Cfg->BaseAddr);
    if (Status != XST_SUCCESS) return Status;

    // Set LED pin as output
    XGpioPs_SetDirectionPin(&Gpio, LED_PIN, 1);
    XGpioPs_SetOutputEnablePin(&Gpio, LED_PIN, 1);

    // Start with LED off
    XGpioPs_WritePin(&Gpio, LED_PIN, 0);

    return XST_SUCCESS;
}*/


// Minimal ISR: acknowledge timer + set a flag
static void ScuTimerIsr(void *CallBackRef)
{
    XScuTimer *Tmr = (XScuTimer *)CallBackRef;

    // Clear the interrupt in the timer
    XScuTimer_ClearInterruptStatus(Tmr);

    // Signal main loop
    control_tick = 1;
}

static int SetupGicAndScuTimer(u32 control_hz)
{
    int Status;
    //xil_printf("SCU timer - GIC init start\r\n");
    // ---------------- GIC init ----------------
    XScuGic_Config *GicCfg = XScuGic_LookupConfig(XPAR_SCUGIC_DIST_BASEADDR); // or GIC_DEVICE_ID
    if (!GicCfg) return XST_FAILURE;

    Status = XScuGic_CfgInitialize(&Gic, GicCfg, GicCfg->CpuBaseAddress);
    if (Status != XST_SUCCESS) return Status;
    //xil_printf("SCU timer - GIC init complete, Hook GIC handler start\r\n");
    // Hook GIC handler into the exception table
    Xil_ExceptionInit();
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
                                 (Xil_ExceptionHandler)XScuGic_InterruptHandler,
                                 &Gic);
    Xil_ExceptionEnable();
    //xil_printf("SCU timer - Hook GIC handler complete, SCU timer init start\r\n");
    // ---------------- SCU timer init ----------------
    XScuTimer_Config *TmrCfg = XScuTimer_LookupConfig(XPS_SCU_PERIPH_BASE + 0x00000600U);
    //xil_printf("TmrCfg ptr = 0x%08lx\r\n", (unsigned long)(uintptr_t)TmrCfg);
    if (!TmrCfg) return XST_FAILURE;

    Status = XScuTimer_CfgInitialize(&ScuTimer, TmrCfg, TmrCfg->BaseAddr);
    if (Status != XST_SUCCESS) return Status;
    //xil_printf("SCU timer - SCU timer init start complete, Stopping timer\r\n");
    // Make sure timer is stopped and in a known state
    XScuTimer_Stop(&ScuTimer);
    XScuTimer_DisableInterrupt(&ScuTimer);
    //xil_printf("Timer stopped, Programming timer interval\r\n");
    // ---------------- Program timer interval ----------------
    u32 timer_clk_hz = CPU_CLK_HZ / 2U;          // Zynq-7000 private timer runs at CPU/2
    u32 load         = timer_clk_hz / control_hz;
    xil_printf("timer_clk_hz = %d, load = %d\r\n", timer_clk_hz, load);
    if (load == 0U) load = 1U;
    
    XScuTimer_LoadTimer(&ScuTimer, load);
    XScuTimer_EnableAutoReload(&ScuTimer);
    //xil_printf("Timer interval programmed, Connecting interrupt\r\n");
    // ---------------- Connect interrupt ----------------
    Status = XScuGic_Connect(&Gic,
                            SCUTIMER_INTR_ID,
                            (Xil_InterruptHandler)ScuTimerIsr,
                            &ScuTimer);
    if (Status != XST_SUCCESS) return Status;

    // Optional: set priority/trigger (often not necessary)
    // XScuGic_SetPriorityTriggerType(&Gic, SCUTIMER_INTR_ID, 0xA0, 0x3);

    XScuGic_Enable(&Gic, SCUTIMER_INTR_ID);

    // Enable timer interrupt and start
    XScuTimer_ClearInterruptStatus(&ScuTimer);
    XScuTimer_EnableInterrupt(&ScuTimer);
    XScuTimer_Start(&ScuTimer);
    //xil_printf("Returning 0x%0x\r\n");
    return XST_SUCCESS;
}

int main(void)
{
    int32_t hw_id = GLOBALS->Hw_ID_Register;
    int32_t window_count = 0;
    int32_t window_size = BASE_AXIS->Window_Size;
    int32_t scaler = 50000000 / window_size; 
    uint32_t cps = 0;
    int32_t position = 0;
    int32_t last_position = 0;
    int32_t delta_position = 0;
    uint32_t rpm = 0;
    uint32_t servo_ticks = 0;
    uint32_t watchdog_status = 0;
    uint32_t safe_ok = 0;
    uint32_t watchdog_count = 0;

    //InitPsGpio();
    xil_printf("\n--- AXI Slave register Velocity Test ---\n\r");
    xil_printf("\n--- Hardware Version %08X ---\n\r", hw_id);
    int Status = SetupGicAndScuTimer(CONTROL_HZ);
    //int led = 0;
    if (Status != XST_SUCCESS) {
        xil_printf("SCU timer/GIC setup failed: %d\r\n", Status);
        while (1) {}
    }
    xil_printf("SCU control tick running at %d Hz\r\n", CONTROL_HZ);
    //xil_printf("cnt=%u win=%u", window_count, window_size);
    GLOBALS->Sys_Control = SYS_ENABLE_REQ;
    xil_printf("Watchdog_Status = %d\r\n", GLOBALS->Watchdog_Status);
    while (1) {
        if (control_tick) {
            control_tick = 0;
            // --- Read encoder count from your AXI register map ---
            position = BASE_AXIS->Position;
            delta_position = (last_position - position) * CONTROL_HZ;
            last_position = position;
            window_count = BASE_AXIS->Window_Count & 0x0fffffff;
            servo_ticks = GLOBALS->Servo_Tick_Ctr;
            //watchdog timer service
            watchdog_status = GLOBALS->Watchdog_Status;
            watchdog_count++;
            if(watchdog_count < 4000)
            {
                GLOBALS->Watchdog_Ctl_Set = WD_KICK_CMD;
            }
            if(watchdog_count > 8000)
            {
                GLOBALS->Watchdog_Ctl_Set = WD_KICK_CMD;
            }
            // counts per second (CPS) calculation
            cps = window_count * scaler;
            rpm = (cps * 60) / 2048;
            // --- Compute/write motor command ---
            BASE_AXIS->Motor_Cmd = 4095;
            // Debug print occasionally (NOT every tick)
            static int n = 0;
            if (++n >= CONTROL_HZ) {
                n = 0;
            if(!(GLOBALS->Watchdog_Status & WD_STATUS_ENABLED))
            {
                xil_printf("Enabling Watchdog\r\n");
                GLOBALS->Watchdog_Ctl_Set = WD_ENABLE_CMD;  // watchdog_enable_cmd
            }
            if(!!(GLOBALS->Watchdog_Status & WD_STATUS_EXPIRED))
            {
                xil_printf("Watchdog expired, resetting...\r\n");
                GLOBALS->Watchdog_Ctl_Set = WD_CLEAR_CMD;
            }
                xil_printf("pos=%d, rpm=%d, sTicks=%d, wd_cnt=%d, wd_cmd=0x%x, wd_stat=0x%x, sys_cmd=0x%x, sys_stat=0x%x\r\n",
                    position, rpm, servo_ticks, watchdog_count, GLOBALS->Watchdog_Ctl_Set, watchdog_status, GLOBALS->Sys_Control, GLOBALS->Sys_Status);
            }
            //static int div = 0;
            //if (++div >= 500) 
            //{   // .5 Hz blink if CONTROL_HZ = 1000
            //    div = 0;
            //    led ^= 1;
            //    XGpioPs_WritePin(&Gpio, LED_PIN, led);
            //}
        }
    }
}

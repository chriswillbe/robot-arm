`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company:
// Engineer:
//
// Create Date: 12/13/2025 07:57:29 PM
// Design Name:
// Module Name: motor_control_channel
// Project Name:
// Target Devices:
// Tool Versions:
// Description:
//
// Dependencies:
//
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
//
//////////////////////////////////////////////////////////////////////////////////


module axis_control_channel(
    input  wire        clk,
    input  wire        rst_n,

    input  wire [31:0] target_pos_reg,
    input  wire        home_sig_reg,
    
    input  wire        encoder_a,
    input  wire        encoder_b,
    output wire [31:0] encoder_period_reg,
    output wire        encoder_dir,
    output wire        encoder_stopped,
    output wire [31:0] encoder_pos_reg,
    
    input  wire [31:0] motor_command_reg,
    output wire        motor_pwm_a,
    output wire        motor_pwm_b

);

motor_controller_core mcc(
    .clk(clk),
    .rst_n(rst_n),
    .command_reg(motor_command_reg),// AXI input
    .pwm_a(motor_pwm_a),            // external output, change to pwm
    .pwm_b(motor_pwm_b)             // external output, change to dir
);

quad_decoder_core qdc(
    .clk(clk),
    .rst_n(rst_n),
    .a(encoder_a),                  // external input
    .b(encoder_b),                  // external input
    .position(encoder_pos_reg),     // AXI output 32 bit for position
    .period_reg(encoder_period_reg),    // AXI output 32 bit for velocity calculation
    .stopped(encoder_stopped),      // AXI output
    .dir(encoder_dir)               // AXI output
);

endmodule

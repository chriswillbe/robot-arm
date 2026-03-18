`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company:
// Engineer:
//
// Create Date: 12/06/2025 01:10:26 PM
// Design Name:
// Module Name: motor_controller_core
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


module motor_controller_core #
(
    parameter PWM_BITS = 12
)
(
    input  wire             clk,
    input  wire             rst_n,
    input  wire [31:0]      command_reg,   // 32-bit register from bus
    output reg              pwm_a,
    output reg              pwm_b
);

    // Direction is just the sign bit
    wire sign = command_reg[31];
    // 12-bit magnitude (already fits the PWM resolution)
    wire [PWM_BITS-1:0] mag = command_reg[PWM_BITS-1:0];
    // PWM counter
    reg [PWM_BITS-1:0] counter;

    always @(posedge clk or negedge rst_n)
    begin
        if (!rst_n)
        begin
            counter <= {PWM_BITS{1'b0}};
            pwm_a <= 1'b0;
            pwm_b <= 1'b0;
        end else
        begin
            counter <= counter + 1'b1;
            pwm_a <= 1'b0;
            pwm_b <= 1'b0;
            if (mag != 0)
            begin
                if(sign)
                begin
                    pwm_a <= (counter < mag);
                    pwm_b <= 1'b0;
                end else
                begin
                    pwm_b <= (counter < mag);
                    pwm_a <= 1'b0;
                end
            end
        end
    end
endmodule



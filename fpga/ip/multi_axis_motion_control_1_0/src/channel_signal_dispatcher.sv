
module channel_signal_dispatcher (
        input  logic            clk,
        input  logic            rst_n,

        input  logic [191:0]    target_pos_bus,
        input  logic [5:0]      home_signal_bus,

        input  logic [5:0]      encoder_a_bus,
        input  logic [5:0]      encoder_b_bus,
        output logic [5:0]      encoder_dir_bus,
        output logic [5:0]      encoder_stopped_bus,
        output logic [191:0]    encoder_period_bus,
        output logic [191:0]    encoder_position_bus,

        input  logic [191:0]    motor_command_bus,        
        output logic [5:0]      pwm_a_bus,
        output logic [5:0]      pwm_b_bus
    );

    genvar i;
    generate
        for (i = 0; i < 6; i++) begin : CHANNEL
        logic [31:0] motor_cmd_in_i;
        logic [31:0] target_pos_in_i;
        logic [31:0] pos_out_i;
        logic [31:0] period_out_i;
        assign motor_cmd_in_i = motor_command_bus[32*i +: 32];
        assign target_pos_in_i = target_pos_bus[32*i +: 32];
        assign encoder_position_bus[32*i +: 32] = pos_out_i;
        assign encoder_period_bus[32*i +: 32] = period_out_i;
            
        axis_control_channel channel (
            .clk   (clk),
            .rst_n (rst_n),

            .target_pos_reg (target_pos_in_i),
            .home_sig_reg   (home_signal_bus[i]),

            .encoder_a (encoder_a_bus[i]),
            .encoder_b (encoder_b_bus[i]),
            .encoder_dir (encoder_dir_bus[i]),
            .encoder_period_reg(period_out_i),
            .encoder_stopped(encoder_stopped_bus[i]),
            .encoder_pos_reg (pos_out_i),

            .motor_command_reg (motor_cmd_in_i),
            .motor_pwm_a (pwm_a_bus[i]),
            .motor_pwm_b (pwm_b_bus[i])
        );
        end
    endgenerate
endmodule

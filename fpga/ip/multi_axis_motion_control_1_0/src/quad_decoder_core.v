`timescale 1ns / 1ps

module quad_decoder_core #(
    parameter [31:0] DT_STOP_TH = 32'd12_000_000,  // 240 ms @ 50 MHz
    parameter [31:0] DT_MAX     = 32'd12_000_000   // saturate (often = STOP_TH)
)(
    input  wire         clk,
    input  wire         rst_n,
    input  wire         a,
    input  wire         b,
    output reg  [31:0]  position,
    output reg  [31:0]  period_reg,
    output reg          stopped,
    output reg          dir
);

    // 2-flop synchronizers
    reg A_meta, A_sync;
    reg B_meta, B_sync;

    // Quadrature state history
    reg [1:0] prev_state;
    wire [1:0] curr_state = {A_sync, B_sync};
    //reg [1:0] curr_state;

    // dt counter
    reg [31:0] dt_counter;

    // delta and step_event for THIS cycle
    reg signed [31:0] delta;   // must be 32 bits to subtract from count
    //reg step_event;

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            A_meta <= 1'b0; A_sync <= 1'b0;
            B_meta <= 1'b0; B_sync <= 1'b0;

            prev_state <= 2'b00;
            //curr_state <= 2'b00;

            position      <= 32'd0;
            dir        <= 1'b0;

            dt_counter <= 32'd0;
            period_reg <= 32'd0;
            stopped    <= 1'b1;
        end else begin
            // sync inputs
            A_meta <= a;  A_sync <= A_meta;
            B_meta <= b;  B_sync <= B_meta;

            // current state (sampled)
            //curr_state <= {A_sync, B_sync};

            // default outputs for this cycle
            delta      = 0;
            //step_event = 1'b0;

            // Transition table: {prev_state, curr_state} -> delta
            // Valid Gray-code transitions only.
            // Convention (can be flipped): +1 for 00->01->11->10->00
            case ({prev_state, curr_state})
                4'b0001: delta = +1; // 00 -> 01
                4'b0111: delta = +1; // 01 -> 11
                4'b1110: delta = +1; // 11 -> 10
                4'b1000: delta = +1; // 10 -> 00

                4'b0010: delta = -1; // 00 -> 10
                4'b1011: delta = -1; // 10 -> 11
                4'b1101: delta = -1; // 11 -> 01
                4'b0100: delta = -1; // 01 -> 00

                default: delta = 0;  // no move or illegal transition
            endcase

            if (delta != 0) begin
                //step_event <= 1'b1;           // (note: <= here is ok; we don't use it below)
                position      <= position + delta;

                if (delta > 0) dir <= 1'b1;
                else           dir <= 1'b0;

                // latch dt and reset
                period_reg <= dt_counter;
                dt_counter <= 32'd0;
                stopped    <= 1'b0;
            end else begin
                // no step: keep counting time (saturating)
                if (dt_counter < DT_MAX) dt_counter <= dt_counter + 1;
                if (dt_counter >= DT_STOP_TH) stopped <= 1'b1;
            end

            // update history AFTER using prev_state in this cycle
            prev_state <= curr_state;
        end
    end

endmodule


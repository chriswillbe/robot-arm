`timescale 1 ns / 1 ps

module motor_controller_6ch_AXI_slave_lite_v1_0_S00_AXI #
(
    // Users to add parameters here

    // User parameters ends
    // Do not modify the parameters beyond this line

    // Width of S_AXI data bus
    parameter int unsigned C_S_AXI_DATA_WIDTH = 32,
    // Width of S_AXI address bus
    parameter int unsigned C_S_AXI_ADDR_WIDTH = 9
)
(
    // Users to add ports here
    input  wire [5:0] hold_mode,

    input  wire [5:0] encoder_a,
    input  wire [5:0] encoder_b,
    input  wire [5:0] home_raw,

    output wire [5:0] dir_out,
    output wire [5:0] pwm_a,
    output wire [5:0] pwm_b,

    // User ports ends
    // Do not modify the ports beyond this line

    // Global Clock Signal
    input  wire  S_AXI_ACLK,
    // Global Reset Signal. This Signal is Active LOW
    input  wire  S_AXI_ARESETN,

    // Write address (issued by master, accepted by Slave)
    input  wire [C_S_AXI_ADDR_WIDTH-1 : 0] S_AXI_AWADDR,
    input  wire [2 : 0] S_AXI_AWPROT,
    input  wire  S_AXI_AWVALID,
    output wire  S_AXI_AWREADY,

    // Write data (issued by master, accepted by Slave)
    input  wire [C_S_AXI_DATA_WIDTH-1 : 0] S_AXI_WDATA,
    input  wire [(C_S_AXI_DATA_WIDTH/8)-1 : 0] S_AXI_WSTRB,
    input  wire  S_AXI_WVALID,
    output wire  S_AXI_WREADY,

    // Write response
    output wire [1 : 0] S_AXI_BRESP,
    output wire  S_AXI_BVALID,
    input  wire  S_AXI_BREADY,

    // Read address (issued by master, accepted by Slave)
    input  wire [C_S_AXI_ADDR_WIDTH-1 : 0] S_AXI_ARADDR,
    input  wire [2 : 0] S_AXI_ARPROT,
    input  wire  S_AXI_ARVALID,
    output wire  S_AXI_ARREADY,

    // Read data (issued by slave)
    output wire [C_S_AXI_DATA_WIDTH-1 : 0] S_AXI_RDATA,
    output wire [1 : 0] S_AXI_RRESP,
    output wire  S_AXI_RVALID,
    input  wire  S_AXI_RREADY
);

    // ----------------------------
    // Localparams / constants
    // ----------------------------
    localparam int unsigned ADDR_LSB           = (C_S_AXI_DATA_WIDTH/32) + 1; // 2 for 32-bit
    localparam int unsigned OPT_MEM_ADDR_BITS  = 6;                           // 128 regs => 7 bits (0..127)
    localparam int unsigned REG_COUNT          = 128;
    localparam int unsigned STRB_W             = C_S_AXI_DATA_WIDTH/8;

    // ----------------------------
    // Register file
    // ----------------------------
    logic [31:0] slv_reg [0:REG_COUNT-1];

    // ----------------------------
    // User buses to/from motor core
    // ----------------------------
    logic [6*32-1:0] speed_bus;
    logic [6*32-1:0] target_pos_bus;
    logic [6*32-1:0] pos_out_bus;

    // ----------------------------
    // AXI write channel (1 outstanding)
    // Capture AW and W independently, then commit once both are present.
    // ----------------------------
    logic                     aw_stored, w_stored;
    logic [C_S_AXI_ADDR_WIDTH-1:0] awaddr_lat;
    logic [31:0]              wdata_lat;
    logic [STRB_W-1:0]        wstrb_lat;

    logic                     bvalid;

    wire aw_hs = S_AXI_AWVALID && S_AXI_AWREADY;
    wire w_hs  = S_AXI_WVALID  && S_AXI_WREADY;
    wire do_write = aw_stored && w_stored;

    // Ready when we are able to accept that channel.
    // We only allow one outstanding write, so stop accepting once we've latched it.
    assign S_AXI_AWREADY = S_AXI_ARESETN && !aw_stored && !bvalid;
    assign S_AXI_WREADY  = S_AXI_ARESETN && !w_stored  && !bvalid;

    assign S_AXI_BVALID  = bvalid;
    assign S_AXI_BRESP   = 2'b00; // OKAY

    // Decode write register index from latched AW address
    wire [6:0] w_reg_index = awaddr_lat[ADDR_LSB + OPT_MEM_ADDR_BITS : ADDR_LSB];

    // ----------------------------
    // AXI read channel (no outstanding read allowed beyond 1 beat)
    // ----------------------------
    logic [C_S_AXI_ADDR_WIDTH-1:0] araddr_lat;
    logic                          rvalid;
    logic [31:0]                   rdata;

    assign S_AXI_ARREADY = S_AXI_ARESETN && !rvalid; // accept new AR only when not holding a pending R
    assign S_AXI_RVALID  = rvalid;
    assign S_AXI_RDATA   = rdata;
    assign S_AXI_RRESP   = 2'b00; // OKAY

    // Decode read index from latched AR address
    wire [6:0] rd_index  = araddr_lat[ADDR_LSB + OPT_MEM_ADDR_BITS : ADDR_LSB];
    wire [2:0] rd_ch     = rd_index[6:4];  // channel number (0..7)
    wire [3:0] rd_offset = rd_index[3:0];  // offset within 16-reg block (0..15)

    // ----------------------------
    // Write whitelist helper
    // ----------------------------
    function automatic bit is_writable(input logic [6:0] idx);
        unique case (idx)
            7'd0,   7'd1,   7'd3,
            7'd16,  7'd17,  7'd19,
            7'd32,  7'd33,  7'd35,
            7'd48,  7'd49,  7'd51,
            7'd64,  7'd65,  7'd67,
            7'd80,  7'd81,  7'd83,
            7'd96: is_writable = 1'b1; // example: GLOBAL_CONTROL writable (optional)
            default: is_writable = 1'b0;
        endcase
    endfunction

    // ----------------------------
    // Sequential: AXI + register writes
    // ----------------------------
    integer i;
    always_ff @(posedge S_AXI_ACLK) begin
        if (!S_AXI_ARESETN) begin
            aw_stored  <= 1'b0;
            w_stored   <= 1'b0;
            awaddr_lat <= '0;
            wdata_lat  <= 32'd0;
            wstrb_lat  <= '0;
            bvalid     <= 1'b0;

            araddr_lat <= '0;
            rvalid     <= 1'b0;

            // Clear regs (safe default). You can narrow this later if you want.
            for (i = 0; i < REG_COUNT; i++) begin
                slv_reg[i] <= 32'd0;
            end
        end else begin
            // ----------------
            // Latch AW
            // ----------------
            if (aw_hs) begin
                awaddr_lat <= S_AXI_AWADDR;
                aw_stored  <= 1'b1;
            end

            // ----------------
            // Latch W
            // ----------------
            if (w_hs) begin
                wdata_lat <= S_AXI_WDATA;
                wstrb_lat <= S_AXI_WSTRB;
                w_stored  <= 1'b1;
            end

            // ----------------
            // Commit write once both present
            // ----------------
            if (do_write) begin
                // Enforce full-word writes only (optional strictness)
                if (&wstrb_lat) begin
                    if (is_writable(w_reg_index)) begin
                        slv_reg[w_reg_index] <= wdata_lat;
                    end
                end

                // Issue write response
                bvalid    <= 1'b1;

                // Clear captured AW/W for next transaction
                aw_stored <= 1'b0;
                w_stored  <= 1'b0;
            end

            // ----------------
            // Complete write response when master accepts it
            // ----------------
            if (bvalid && S_AXI_BREADY) begin
                bvalid <= 1'b0;
            end

            // ----------------
            // Latch AR -> produce RVALID
            // ----------------
            if (S_AXI_ARVALID && S_AXI_ARREADY) begin
                araddr_lat <= S_AXI_ARADDR;
                rvalid     <= 1'b1;
            end

            // ----------------
            // Complete read when master accepts it
            // ----------------
            if (rvalid && S_AXI_RREADY) begin
                rvalid <= 1'b0;
            end
        end
    end

    // ----------------------------
    // Combinational: read mux + live override
    // ----------------------------
    always_comb begin
        // Default to register file
        rdata = slv_reg[rd_index];

        // Live position override for offset 2 of each channel block, channels 0..5
        // (regs 2,18,34,50,66,82) return pos_out_bus
        if ((rd_offset == 4'h2) && (rd_ch < 3'd6)) begin
            rdata = pos_out_bus[32*rd_ch +: 32];
        end
    end

    // ----------------------------
    // Hook up motor control core
    // ----------------------------
    six_channel_motor_control scmc (
        .clk        (S_AXI_ACLK),
        .rst_n      (S_AXI_ARESETN),

        .speed_bus      (speed_bus),
        .target_pos_bus (target_pos_bus),
        .hold_mode      (hold_mode),

        .encoder_a  (encoder_a),
        .encoder_b  (encoder_b),
        .home_raw   (home_raw),

        .dir_out    (dir_out),
        .pwm_a      (pwm_a),
        .pwm_b      (pwm_b),
        .pos_out_bus(pos_out_bus)
    );

    // Map regs -> buses (speed + target)
    // channel stride is 16 regs:
    // ch0: speed=1 target=3
    // ch1: speed=17 target=19, etc.
    always_comb begin
        speed_bus      = '0;
        target_pos_bus = '0;

        speed_bus[32*0 +: 32] = slv_reg[7'd1];
        speed_bus[32*1 +: 32] = slv_reg[7'd17];
        speed_bus[32*2 +: 32] = slv_reg[7'd33];
        speed_bus[32*3 +: 32] = slv_reg[7'd49];
        speed_bus[32*4 +: 32] = slv_reg[7'd65];
        speed_bus[32*5 +: 32] = slv_reg[7'd81];

        target_pos_bus[32*0 +: 32] = slv_reg[7'd3];
        target_pos_bus[32*1 +: 32] = slv_reg[7'd19];
        target_pos_bus[32*2 +: 32] = slv_reg[7'd35];
        target_pos_bus[32*3 +: 32] = slv_reg[7'd51];
        target_pos_bus[32*4 +: 32] = slv_reg[7'd67];
        target_pos_bus[32*5 +: 32] = slv_reg[7'd83];
    end

endmodule

`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 05/07/2026 01:45:28 PM
// Design Name: 
// Module Name: CST466_HW2
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


module CST466_HW2(
    input  wire        clk,
    input  wire [7:0]  sw_n,    // active-low physical switches
    input  wire [3:0]  btn_n,   // active-low physical buttons
    output wire [7:0]  led
);

    // Convert active-low switches to normal logic:
    // physical switch ON/active = 0 becomes logical 1.
    wire [7:0] sw;
    assign sw = ~sw_n;

    wire [3:0] btn_pulse;
    wire [3:0] btn_level;

    // Buttons are active-low, so ACTIVE_LOW stays 1.
    button_one_pulse #(.ACTIVE_LOW(1)) b0 (
        .clk(clk),
        .button_in(btn_n[0]),
        .pressed_level(btn_level[0]),
        .pressed_pulse(btn_pulse[0])
    );

    button_one_pulse #(.ACTIVE_LOW(1)) b1 (
        .clk(clk),
        .button_in(btn_n[1]),
        .pressed_level(btn_level[1]),
        .pressed_pulse(btn_pulse[1])
    );

    button_one_pulse #(.ACTIVE_LOW(1)) b2 (
        .clk(clk),
        .button_in(btn_n[2]),
        .pressed_level(btn_level[2]),
        .pressed_pulse(btn_pulse[2])
    );

    button_one_pulse #(.ACTIVE_LOW(1)) b3 (
        .clk(clk),
        .button_in(btn_n[3]),
        .pressed_level(btn_level[3]),
        .pressed_pulse(btn_pulse[3])
    );

    reg [9:0] key_reg    = 10'b0;
    reg [7:0] result_reg = 8'b0;

    wire [7:0] encrypted_out;
    wire [7:0] decrypted_out;

    sdes_core enc_core (
        .data_in(sw),
        .key(key_reg),
        .decrypt(1'b0),
        .data_out(encrypted_out)
    );

    sdes_core dec_core (
        .data_in(sw),
        .key(key_reg),
        .decrypt(1'b1),
        .data_out(decrypted_out)
    );

    always @(posedge clk) begin
        // Latch lower 8 bits of the 10-bit key.
        if (btn_pulse[0]) begin
            key_reg[7:0] <= sw;
        end

        // Latch upper 2 bits of the 10-bit key.
        if (btn_pulse[1]) begin
            key_reg[9:8] <= sw[1:0];
        end

        // Encrypt current switch value.
        if (btn_pulse[2]) begin
            result_reg <= encrypted_out;
        end

        // Decrypt current switch value.
        if (btn_pulse[3]) begin
            result_reg <= decrypted_out;
        end
    end

    // Normal display: result.
    // Hold BTN0 to view key[7:0].
    // Hold BTN1 to view key[9:8] in LED[1:0].
    assign led = btn_level[0] ? key_reg[7:0] :
                 btn_level[1] ? {6'b000000, key_reg[9:8]} :
                 result_reg;
endmodule

module button_one_pulse #(
    parameter ACTIVE_LOW = 1
)(
    input  wire clk,
    input  wire button_in,
    output wire pressed_level,
    output wire pressed_pulse
);

    wire pressed_async;
    assign pressed_async = ACTIVE_LOW ? ~button_in : button_in;

    reg sync_0 = 1'b0;
    reg sync_1 = 1'b0;
    reg prev   = 1'b0;

    always @(posedge clk) begin
        sync_0 <= pressed_async;
        sync_1 <= sync_0;
        prev   <= sync_1;
    end

    assign pressed_level = sync_1;
    assign pressed_pulse = sync_1 & ~prev;

endmodule


// ============================================================
// S-DES Core
// ============================================================

module sdes_core (
    input  wire [7:0] data_in,
    input  wire [9:0] key,
    input  wire       decrypt,
    output wire [7:0] data_out
);

    // ----------------------------
    // Permutation functions
    // Tables use the standard S-DES ordering, where bit position 1
    // refers to the MSB of the input.
    // ----------------------------

    function [9:0] p10;
        input [9:0] x;
        begin
            // P10 = 3 5 2 7 4 10 1 9 8 6
            p10 = {x[7], x[5], x[8], x[3], x[6],
                   x[0], x[9], x[1], x[2], x[4]};
        end
    endfunction

    function [7:0] p8;
        input [9:0] x;
        begin
            // P8 = 6 3 7 4 8 5 10 9
            p8 = {x[4], x[7], x[3], x[6],
                  x[2], x[5], x[0], x[1]};
        end
    endfunction

    function [7:0] ip;
        input [7:0] x;
        begin
            // IP = 2 6 3 1 4 8 5 7
            ip = {x[6], x[2], x[5], x[7],
                  x[4], x[0], x[3], x[1]};
        end
    endfunction

    function [7:0] ip_inv;
        input [7:0] x;
        begin
            // IP^-1 = 4 1 3 5 7 2 8 6
            ip_inv = {x[4], x[7], x[5], x[3],
                      x[1], x[6], x[0], x[2]};
        end
    endfunction

    function [7:0] ep;
        input [3:0] x;
        begin
            // E/P = 4 1 2 3 2 3 4 1
            ep = {x[0], x[3], x[2], x[1],
                  x[2], x[1], x[0], x[3]};
        end
    endfunction

    function [3:0] p4;
        input [3:0] x;
        begin
            // P4 = 2 4 3 1
            p4 = {x[2], x[0], x[1], x[3]};
        end
    endfunction

    function [4:0] ls1;
        input [4:0] x;
        begin
            ls1 = {x[3:0], x[4]};
        end
    endfunction

    function [4:0] ls2;
        input [4:0] x;
        begin
            ls2 = {x[2:0], x[4:3]};
        end
    endfunction

    // ----------------------------
    // S-boxes
    // Input bits are b3 b2 b1 b0.
    // Row = {b3, b0}
    // Col = {b2, b1}
    // ----------------------------

    function [1:0] sbox0;
        input [3:0] x;
        reg [1:0] row;
        reg [1:0] col;
        begin
            row = {x[3], x[0]};
            col = {x[2], x[1]};

            case ({row, col})
                4'b0000: sbox0 = 2'd1;
                4'b0001: sbox0 = 2'd0;
                4'b0010: sbox0 = 2'd3;
                4'b0011: sbox0 = 2'd2;

                4'b0100: sbox0 = 2'd3;
                4'b0101: sbox0 = 2'd2;
                4'b0110: sbox0 = 2'd1;
                4'b0111: sbox0 = 2'd0;

                4'b1000: sbox0 = 2'd0;
                4'b1001: sbox0 = 2'd2;
                4'b1010: sbox0 = 2'd1;
                4'b1011: sbox0 = 2'd3;

                4'b1100: sbox0 = 2'd3;
                4'b1101: sbox0 = 2'd1;
                4'b1110: sbox0 = 2'd3;
                4'b1111: sbox0 = 2'd2;

                default: sbox0 = 2'd0;
            endcase
        end
    endfunction

    function [1:0] sbox1;
        input [3:0] x;
        reg [1:0] row;
        reg [1:0] col;
        begin
            row = {x[3], x[0]};
            col = {x[2], x[1]};

            case ({row, col})
                4'b0000: sbox1 = 2'd0;
                4'b0001: sbox1 = 2'd1;
                4'b0010: sbox1 = 2'd2;
                4'b0011: sbox1 = 2'd3;

                4'b0100: sbox1 = 2'd2;
                4'b0101: sbox1 = 2'd0;
                4'b0110: sbox1 = 2'd1;
                4'b0111: sbox1 = 2'd3;

                4'b1000: sbox1 = 2'd3;
                4'b1001: sbox1 = 2'd0;
                4'b1010: sbox1 = 2'd1;
                4'b1011: sbox1 = 2'd0;

                4'b1100: sbox1 = 2'd2;
                4'b1101: sbox1 = 2'd1;
                4'b1110: sbox1 = 2'd0;
                4'b1111: sbox1 = 2'd3;

                default: sbox1 = 2'd0;
            endcase
        end
    endfunction

    // ----------------------------
    // fk function
    // fk(L, R) = (L XOR F(R, subkey), R)
    // ----------------------------

    function [7:0] fk;
        input [7:0] block;
        input [7:0] subkey;

        reg [3:0] left;
        reg [3:0] right;
        reg [7:0] ep_xor;
        reg [1:0] s0_out;
        reg [1:0] s1_out;
        reg [3:0] f_out;

        begin
            left  = block[7:4];
            right = block[3:0];

            ep_xor = ep(right) ^ subkey;

            s0_out = sbox0(ep_xor[7:4]);
            s1_out = sbox1(ep_xor[3:0]);

            f_out = p4({s0_out, s1_out});

            fk = {left ^ f_out, right};
        end
    endfunction

    function [7:0] switch_halves;
        input [7:0] x;
        begin
            switch_halves = {x[3:0], x[7:4]};
        end
    endfunction

    // ----------------------------
    // Key generation
    // ----------------------------

    wire [9:0] key_p10;
    wire [4:0] key_l0;
    wire [4:0] key_r0;
    wire [4:0] key_l1;
    wire [4:0] key_r1;
    wire [4:0] key_l2;
    wire [4:0] key_r2;

    wire [7:0] k1;
    wire [7:0] k2;

    assign key_p10 = p10(key);

    assign key_l0 = key_p10[9:5];
    assign key_r0 = key_p10[4:0];

    assign key_l1 = ls1(key_l0);
    assign key_r1 = ls1(key_r0);

    assign k1 = p8({key_l1, key_r1});

    assign key_l2 = ls2(key_l1);
    assign key_r2 = ls2(key_r1);

    assign k2 = p8({key_l2, key_r2});

    // ----------------------------
    // Main S-DES datapath
    // Encryption:
    //   IP -> fk(K1) -> SW -> fk(K2) -> IP^-1
    //
    // Decryption:
    //   IP -> fk(K2) -> SW -> fk(K1) -> IP^-1
    // ----------------------------

    wire [7:0] round_key_1;
    wire [7:0] round_key_2;

    wire [7:0] after_ip;
    wire [7:0] after_fk1;
    wire [7:0] after_sw;
    wire [7:0] after_fk2;

    assign round_key_1 = decrypt ? k2 : k1;
    assign round_key_2 = decrypt ? k1 : k2;

    assign after_ip  = ip(data_in);
    assign after_fk1 = fk(after_ip, round_key_1);
    assign after_sw  = switch_halves(after_fk1);
    assign after_fk2 = fk(after_sw, round_key_2);

    assign data_out = ip_inv(after_fk2);

endmodule
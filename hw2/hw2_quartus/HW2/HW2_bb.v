
module HW2 (
	clk_clk,
	keys_export,
	leds_export,
	switches_export);	

	input		clk_clk;
	input	[3:0]	keys_export;
	output	[9:0]	leds_export;
	input	[9:0]	switches_export;
endmodule

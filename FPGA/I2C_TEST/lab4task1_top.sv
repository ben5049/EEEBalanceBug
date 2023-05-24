module lab4task1_top(
	input		logic	[0:1]	KEY,
	output 	logic	[6:0] HEX0,
	input		logic [9:0]	SW,
	inout		SDA,
	input		SCL,
	input	logic MAX10_CLK1_50
);
	
i2c i2c_slave(
	.clk(MAX10_CLK1_50),
	.SCL(SCL),
	.SDA(SDA),
	.RST(~KEY[0]),
	.LEDG(HEX0[0]),
	.LEDR(HEX0[1]),
	.SW_1(SW[0]));

endmodule
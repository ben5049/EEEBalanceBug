`timescale 1ns / 100ps						// time unit = 1ns, resolution = 100ps
module counter #(
	parameter WIDTH = 4
)(													// interface signals
	input logic						clk,		// clock
	input logic						rst,		// reset
	input logic						en,		// counter enable
	output logic [WIDTH-1:0]	count		// count output
);
	always_ff @ (posedge clk, posedge rst)
			if (rst) count <= {WIDTH{1'b0}};
			else count <= count + {{WIDTH-1{1'b0}}, en};
endmodule
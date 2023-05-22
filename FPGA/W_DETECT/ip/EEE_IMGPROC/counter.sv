module counter(
	input logic[15:0] current_value,
	input logic white, clk, rst,
	output logic [15:0] new_value);
	
	always_ff @ (posedge clk, posedge rst)
		if (rst == 1'b1) new_value <= 16'b0;
		else new_value <= current_value + {15'b0, white};
	
	
endmodule
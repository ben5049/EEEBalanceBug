module addr_logic #(
	parameter h = 4,
	parameter v = 3)(
	input logic [10:0] x, y,
	output logic [8:0] addr);
	
	
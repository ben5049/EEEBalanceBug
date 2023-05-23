module px_classify(
	input logic [7:0] red, green, blue,
	output logic white);
	
	parameter white_threshold= 8'd230;
	always_comb
		if (red >= white_threshold & green >= white_threshold & blue >= white_threshold) white = 1;
		else white = 0;
		
endmodule

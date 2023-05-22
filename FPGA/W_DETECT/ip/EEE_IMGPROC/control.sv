module control(
	input logic clk, rst,
	input logic [7:0] red, green, blue,
	input logic [10:0] x, y,
	
	);
	
	
	// Set number of vertical and horizonal blocks
	parameter [v = 4;
	parameter h = 4;
	
	parameter threshold = 0; // Threshold for number of pixels to count a block as white
	
	logic white; // Whether the current pixel is white
	
	logic [15:0] current_value, new_value; // Counts the number of white pixels in row of block
	
	px_classify(
		.red(red),
		.blue(blue),
		.green(green),
		.white(white));
	
	counter(
		.white(white),
		.clk(clk),
		.rst(1'b0),
		.current_value(current_value),
		.new_value(new_value));
			
	addr_logic(h,v)(
		.x(x),
		.y(y),
		.addr(addr));
			
	ram_px_counter	ram_px_counter_inst (
		.address_a ( address_a_sig ),
		.address_b ( address_b_sig ),
		.clock ( clock_sig ),
		.data_a ( data_a_sig ),
		.data_b ( data_b_sig ),
		.wren_a ( wren_a_sig ),
		.wren_b ( wren_b_sig ),
		.q_a ( q_a_sig ),
		.q_b ( q_b_sig )
		);

	// Trigger
	logic end_block_h_flag;
	always_comb
		if (
		
		
	
	
	// Define our states
	typedef enum {IDLE, START_BLOCK_H, END_BLOCK_H} block_state;
	block_state current_block_state, next_block_state;
	
	// State registers
	always_ff @ (posedge clk, posedge rst)
		if (rst)		current_block_state <= START_BLOCK_H;
		else			current_block_state <= next_block_state;
		
	// Next state logic
	always_comb
		case(current_block_state)
			START_BLOCK_H: next_block_state = IDLE;
			IDLE:				if (end_block_h_flag) next_block_state = END_BLOCK_H
	
	
	
	
	
	
	
	
	
	
	
endmodule
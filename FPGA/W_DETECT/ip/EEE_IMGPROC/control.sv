module control(
	input logic clk, rst,
	input logic [7:0] red, green, blue,
	input logic [10:0] x, y,
	
	);
	
	// TODO: Reset RAM to 0
	
	// Set number of vertical and horizonal blocks
	parameter v = 4;
	parameter h = 4;
	
	parameter threshold = 0; // Threshold for number of pixels to count a block as white
	
	logic white; // Whether the current pixel is white
	
	logic [15:0] current_value, new_value; // Counts the number of white pixels in row of block
	
	px_classify(
		.red(red),
		.blue(blue),
		.green(green),
		.white(white));
	
			
	logic [8:0] address_b_sig;
	logic [15:0] data_a_sig, data_b_sig, q_a_sig, q_b_sig;
	logic wren_a_sig, wren_b_sig;
	
	ram_px_counter	ram_px_counter_inst (
		.address_a ( addr ),
		.address_b ( address_b_sig ),
		.clock ( clk ),
		.data_a ( data_a_sig + counter ),
		.data_b ( data_b_sig ),
		.wren_a ( wren_a_sig ),
		.wren_b ( wren_b_sig ),
		.q_a ( data_a_sig ),
		.q_b ( q_b_sig ));

	// Flags for end of blocks
	logic end_block_h_flag, end_block_v_flag, end_block_flag, end_col_flag;
	always_comb
		begin
			if ((x == 11'd159) | (x == 11'd319) | (x == 11'd479) | (x == 11'd639)) end_block_h_flag = 1'b1;
			else end_block_h_flag = 1'b0;
			
			if ((y == 11'd159) | (y == 11'd319) | (y == 11'd479))						  end_block_v_flag = 1'b1;
			else end_block_v_flag = 1'b0;
			
			end_block_flag = end_block_h_flag & end_block_v_flag;
			
			if (x == 11'd639) end_col_flag = 1'b1;
			else end_col_flag = 1'b0;
		end
		
	// Add one cycle of delay to the flag so we can use it to reset the counter
	logic end_block_h_flag_delayed; 
	always_ff @ (posedge clk)
		end_block_h_flag_delayed <= end_block_h_flag;
	
	// Count white pixels in row of block
	always_ff @ (posedge clk, posedge rst)
		if (rst == 1'b1) counter <= 16'b0;
		else if (end_block_h_flag_delayed == 1'b1) counter <= {15'b0, white};
		else counter <= counter + {15'b0, white};
			
	// Address logic
	logic [8:0] addr_a;
	initial addr = 9'b0;
	
	always_ff @ (posedge clk, posedge rst)
		if (rst) addr <= 9'b0;
		else if (end_col_flag & ~end_block_flag) addr <= addr - 9'd3;
		else if (end_block_h_flag) addr <= addr + 9'b1;
	
	
	// Define our states
	typedef enum {IDLE, SAVE} block_state;
	block_state current_block_state, next_block_state;
	
	// State registers
	always_ff @ (posedge clk, posedge rst)
		if (rst)		current_block_state <= START_BLOCK_H;
		else			current_block_state <= next_block_state;
	
	// Next state logic
	always_comb
		case(current_block_state)
			IDLE:		if (end_block_h_flag) 	next_block_state = SAVE;
			SAVE:										next_block_state = IDLE;
			default: 								next_block_state = IDLE;
		endcase
	
	// Output logic
	always_comb
		case(current_block_state)
			IDLE:	begin
						wren_a_sig = 1'b0;
					end
						
			SAVE:	begin
						wren_a_sig = 1'b1;
					end
						
			default: wren_a_sig = 1'b0;
						
		endcase
	
	// Classify block
	always_ff @ (posedge clk)
		if (
	

	
endmodule
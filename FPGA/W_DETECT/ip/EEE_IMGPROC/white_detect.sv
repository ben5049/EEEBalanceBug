module white_detect (
	input logic clk, rst,
	input logic [7:0] red, green, blue,
	input logic [10:0] x, y,
	output logic out, out_valid
	);
	
	// TODO: Reset RAM to 0
	

	// Set number of vertical and horizonal blocks
	parameter h = 4;
	parameter v = 3;
	
	// Threshold for number of pixels to count a block as white
	parameter threshold = 10; 
	
	// Stores whether the current pixel is white
	logic white;
	
	// Classify the current pixel
	px_classify px_classify_inst(
		.red			(red),
		.blue			(blue),
		.green		(green),
		.white		(white));
			
	// Initialise the RAM for storing number of white pixels per block
	logic [15:0] stored_data, data_b_sig, q_a_sig, q_b_sig;
	logic wren_a_sig, wren_b_sig;

	ram_px_counter ram_px_counter_inst(
		.address_a	(port_1_addr),
		.address_b	(port_2_addr),
		.clock		(clk),
		.data_a		(stored_data + value),
		.data_b		(data_b_sig ),
		.wren_a		(wren_a_sig ),
		.wren_b		(wren_b_sig ),
		.q_a			(stored_data),
		.q_b			(q_b_sig));

	// Flags for end of blocks/row
	logic end_block_h_flag, end_block_v_flag, end_block_flag, end_row_flag;
	always_comb
		begin
			if ((x == 11'd159) | (x == 11'd319) | (x == 11'd479) | (x == 11'd639)) end_block_h_flag = 1'b1;
			else end_block_h_flag = 1'b0;
			
			if ((y == 11'd159) | (y == 11'd319) | (y == 11'd479))						  end_block_v_flag = 1'b1;
			else end_block_v_flag = 1'b0;
			
			end_block_flag = end_block_h_flag & end_block_v_flag;
			
			if (x == 11'd639) end_row_flag = 1'b1;
			else end_row_flag = 1'b0;
		end

	// Add one cycle of delay to the end_block_h flag so we can use it to reset the counter
	logic end_block_h_flag_delayed; 
	always_ff @ (posedge clk)
		end_block_h_flag_delayed <= end_block_h_flag;
	
	// Count white pixels in row of block
	logic [15:0] value; // Counts the number of white pixels in row of block
	initial value = 16'b0;
	always_ff @ (posedge clk, posedge rst)
		if			(rst == 1'b1) 								value <= 16'b0;
		else if	(end_block_h_flag_delayed == 1'b1) 	value <= {15'b0, white};
		else 														value <= value + {15'b0, white};
			
	// Address logic
	logic [8:0] port_1_addr;
	initial port_1_addr = 9'b0;
	
	always_ff @ (posedge clk, posedge rst)
		if 		(rst) 									port_1_addr <= 9'b0;
		else if 	(end_row_flag & ~end_block_flag) port_1_addr <= port_1_addr - (h - 1);
		else if 	(end_block_h_flag) 					port_1_addr <= port_1_addr + 9'b1;
	
	/////////////////////////////////////////////////////////////////////////////////////
	///////////////////////// State machine to control the RAM //////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////
	
	// Define states
	typedef enum {IDLE, SAVE} block_state;
	block_state current_block_state, next_block_state;
	
	// State registers
	always_ff @ (posedge clk, posedge rst)
		if (rst)		current_block_state <= IDLE;
		else			current_block_state <= next_block_state;
	
	// Next state logic
	always_comb
		case(current_block_state)
			IDLE:		if (end_block_h_flag) 	next_block_state = SAVE;
						else							next_block_state = IDLE;
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
		
	/////////////////////////////////////////////////////////////////////////////////////
	////////////////////////// State machine to reset the RAM ///////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////
	
	logic [8:0] port_2_addr;
	
	// Define states
	typedef enum {IDLE2, CLEAR} port_2_state;
	port_2_state current_port_2_state, next_port_2_state;
	
	// State registers
	always_ff @ (posedge clk, posedge rst)
		if (rst)		current_port_2_state <= CLEAR;
		else			current_port_2_state <= next_port_2_state;
	
	// Next state logic
	always_comb
		case(current_port_2_state)
			IDLE2:	if (rst)										next_port_2_state = CLEAR;
						else											next_port_2_state = IDLE2;
			CLEAR:	if (port_2_addr == ((h * v) - 1))	next_port_2_state = IDLE2;
						else											next_port_2_state = CLEAR;
			default: 												next_port_2_state = IDLE2;
		endcase
	
	// Combinational output logic
	always_comb
		case(current_port_2_state)
			IDLE2:	begin
							wren_b_sig = 1'b0;
							data_b_sig = 16'b0;
						end
						
			CLEAR:	begin
							wren_b_sig = 1'b1;
							data_b_sig = 16'b0;
						end
						
			default: begin
							wren_b_sig = 1'b0;
							data_b_sig = 16'b0;
						end
		endcase
		
	// Increment the port_2_addr every clock cycle if in CLEAR state
	always_ff @ (posedge clk, posedge rst)
		if (rst) 											port_2_addr <= 9'b0;
		else if (current_port_2_state == CLEAR) 	port_2_addr <= port_2_addr + 9'b1;
		else													port_2_addr <= 9'b0;

	
	/////////////////////////////////////////////////////////////////////////////////////
	
	// Classify block
	always_ff @ (posedge clk)
		if (end_block_flag) begin
			out_valid <= 1'b1;
			if ((stored_data + value) > threshold) out <= 1'b1;
			else out <= 1'b0;
			end
		else out_valid <= 1'b0;
	
endmodule
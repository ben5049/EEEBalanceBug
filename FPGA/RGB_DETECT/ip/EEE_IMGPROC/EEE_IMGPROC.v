module EEE_IMGPROC(
	// global clock & reset
	clk,
	reset_n,
	
	// mm slave
	s_chipselect,
	s_read,
	s_write,
	s_readdata,
	s_writedata,
	s_address,

	// stream sink
	sink_data,
	sink_valid,
	sink_ready,
	sink_sop,
	sink_eop,
	
	// streaming source
	source_data,
	source_valid,
	source_ready,
	source_sop,
	source_eop,
	
	// conduit
	mode
	
);


// global clock & reset
input	clk;
input	reset_n;

// mm slave
input							s_chipselect;
input							s_read;
input							s_write;
output	reg	[31:0]	s_readdata;
input	[31:0]				s_writedata;
input	[2:0]					s_address;


// streaming sink
input	[23:0]            	sink_data;
input								sink_valid;
output							sink_ready;
input								sink_sop;
input								sink_eop;

// streaming source
output	[23:0]			  	   source_data;
output								source_valid;
input									source_ready;
output								source_sop;
output								source_eop;

// conduit export
input                mode;

////////////////////////////////////////////////////////////////////////
//
parameter IMAGE_W = 11'd640;
parameter IMAGE_H = 11'd480;
parameter MESSAGE_BUF_MAX = 256;
parameter MSG_INTERVAL = 6;
parameter BB_COL_DEFAULT = 24'h00ff00;


wire [7:0]   red, green, blue, grey;
wire [7:0]   red_out, green_out, blue_out;

wire         sop, eop, in_valid, out_ready;
////////////////////////////////////////////////////////////////////////
// Modify this section to change what is counted as a red/green/blue pixel

// Detect red areas
wire red_detect;
assign red_detect = red[7] & ~green[7] & ~blue[7];
//assign red_detect =  mode & ({1'b0, red[7:0]} > ({1'b0, green[7:0]} + 9'd30)) & ({1'b0, red[7:0]} > ({1'b0, blue[7:0]} + 9'd30));

// Detect green areas
wire green_detect;
assign green_detect = ~red[7] & green[7] & ~blue[7];
//assign green_detect =  mode & ({1'b0, green[7:0]} > ({1'b0, red[7:0]} + 9'd48)) & ({1'b0, green[7:0]} > ({1'b0, blue[7:0]} + 9'd40));

// Detect blue areas
wire blue_detect;
assign blue_detect = ~red[7] & ~green[7] & blue[7];
//assign blue_detect =  mode & ({1'b0, blue[7:0]} > ({1'b0, green[7:0]} + 9'd20)) & ({1'b0, blue[7:0]} > ({1'b0, red[7:0]} + 9'd20));

////////////////////////////////////////////////////////////////////////

// Find boundary of cursor box

// Highlight detected areas
reg [23:0] rgb_high;
assign grey = green[7:1] + red[7:2] + blue[7:2]; //Grey = green/2 + red/4 + blue/4

always @ (*)
	begin
		if 		(red_detect 	== 1'b1)	rgb_high = {8'hff, 8'h0, 8'h0};
		else if 	(green_detect	== 1'b1) rgb_high = {8'h0, 8'hff, 8'h0};
		else if 	(blue_detect 	== 1'b1)	rgb_high = {8'h0, 8'h0, 8'hff};
		else 										rgb_high = {grey, grey, grey};
	end

// Show bounding box
reg [23:0] new_image;
wire rbb_active, gbb_active, bbb_active;
assign rbb_active = (x == r_left) | (x == r_right) | (y == r_top) | (y == r_bottom);
assign gbb_active = (x == g_left) | (x == g_right) | (y == g_top) | (y == g_bottom);
assign bbb_active = (x == b_left) | (x == b_right) | (y == b_top) | (y == b_bottom);

always @ (*)
	begin
		if			(rbb_active == 1'b1)	new_image = rbb_col;
		else if	(gbb_active == 1'b1)	new_image = gbb_col;
		else if	(bbb_active == 1'b1)	new_image = bbb_col;
		else									new_image = rgb_high;
	end
	
//assign new_image = bb_active ? bb_col : rgb_high;

// Switch output pixels depending on mode switch
// Don't modify the start-of-packet word - it's a packet discriptor
// Don't modify data in non-video packets
assign {red_out, green_out, blue_out} = (mode & ~sop & packet_video) ? new_image : {red,green,blue};

//Count valid pixels to get the image coordinates. Reset and detect packet type on Start of Packet.
reg [10:0] x, y;
reg packet_video;
always@(posedge clk) begin
	if (sop) begin
		x <= 11'h0;
		y <= 11'h0;
		packet_video <= (blue[3:0] == 3'h0);
	end
	else if (in_valid) begin
		if (x == IMAGE_W-1) begin
			x <= 11'h0;
			y <= y + 11'h1;
		end
		else begin
			x <= x + 11'h1;
		end
	end
end

//Find first and last red pixels
reg [10:0] r_x_min, r_y_min, r_x_max, r_y_max;
always@(posedge clk) begin
	if (red_detect & in_valid) begin	//Update bounds when the pixel is red
		if (x < r_x_min) r_x_min <= x;
		if (x > r_x_max) r_x_max <= x;
		if (y < r_y_min) r_y_min <= y;
		r_y_max <= y;
	end
	if (sop & in_valid) begin	//Reset bounds on start of packet
		r_x_min <= IMAGE_W-11'h1;
		r_x_max <= 0;
		r_y_min <= IMAGE_H-11'h1;
		r_y_max <= 0;
	end
end

//Find first and last green pixels
reg [10:0] g_x_min, g_y_min, g_x_max, g_y_max;
always@(posedge clk) begin
	if (green_detect & in_valid) begin	//Update bounds when the pixel is green
		if (x < g_x_min) g_x_min <= x;
		if (x > g_x_max) g_x_max <= x;
		if (y < g_y_min) g_y_min <= y;
		g_y_max <= y;
	end
	if (sop & in_valid) begin	//Reset bounds on start of packet
		g_x_min <= IMAGE_W-11'h1;
		g_x_max <= 0;
		g_y_min <= IMAGE_H-11'h1;
		g_y_max <= 0;
	end
end

//Find first and last blue pixels
reg [10:0] b_x_min, b_y_min, b_x_max, b_y_max;
always@(posedge clk) begin
	if (blue_detect & in_valid) begin	//Update bounds when the pixel is blue
		if (x < b_x_min) b_x_min <= x;
		if (x > b_x_max) b_x_max <= x;
		if (y < b_y_min) b_y_min <= y;
		b_y_max <= y;
	end
	if (sop & in_valid) begin	//Reset bounds on start of packet
		b_x_min <= IMAGE_W-11'h1;
		b_x_max <= 0;
		b_y_min <= IMAGE_H-11'h1;
		b_y_max <= 0;
	end
end

//Process bounding box at the end of the frame.
reg [1:0] msg_state;
reg [10:0] r_left, r_right, r_top, r_bottom;
reg [10:0] g_left, g_right, g_top, g_bottom;
reg [10:0] b_left, b_right, b_top, b_bottom;
reg [7:0] frame_count;
always@(posedge clk) begin
	if (eop & in_valid & packet_video) begin  //Ignore non-video packets
		
		//Latch edges for display overlay on next frame
		r_left 	<= r_x_min;
		r_right 	<= r_x_max;
		r_top 	<= r_y_min;
		r_bottom <= r_y_max;
		
		g_left 	<= g_x_min;
		g_right 	<= g_x_max;
		g_top 	<= g_y_min;
		g_bottom <= g_y_max;
		
		b_left 	<= b_x_min;
		b_right 	<= b_x_max;
		b_top 	<= b_y_min;
		b_bottom <= b_y_max;
		
		
		//Start message writer FSM once every MSG_INTERVAL frames, if there is room in the FIFO
		frame_count <= frame_count - 1;
		
		if (frame_count == 0 && msg_buf_size < MESSAGE_BUF_MAX - 3) begin
			msg_state <= 2'b01;
			frame_count <= MSG_INTERVAL-1;
		end
	end
	
	//Cycle through message writer states once started
	if (msg_state != 2'b00) msg_state <= msg_state + 2'b01;

end
	
//Generate output messages for CPU
reg [31:0] msg_buf_in; 
wire [31:0] msg_buf_out;
reg msg_buf_wr;
wire msg_buf_rd, msg_buf_flush;
wire [7:0] msg_buf_size;
wire msg_buf_empty;

`define RED_BOX_MSG_ID "RBB"

always@(*) begin	//Write words to FIFO as state machine advances
	case(msg_state)
		2'b00: begin
			msg_buf_in = 32'b0;
			msg_buf_wr = 1'b0;
		end
		2'b01: begin
			msg_buf_in = `RED_BOX_MSG_ID;	//Message ID
			msg_buf_wr = 1'b1;
		end
		2'b10: begin
			msg_buf_in = {5'b0, r_x_min, 5'b0, r_y_min};	//Top left coordinate
			msg_buf_wr = 1'b1;
		end
		2'b11: begin
			msg_buf_in = {5'b0, r_x_max, 5'b0, r_y_max}; //Bottom right coordinate
			msg_buf_wr = 1'b1;
		end
	endcase
end


//Output message FIFO
MSG_FIFO	MSG_FIFO_inst (
	.clock (clk),
	.data (msg_buf_in),
	.rdreq (msg_buf_rd),
	.sclr (~reset_n | msg_buf_flush),
	.wrreq (msg_buf_wr),
	.q (msg_buf_out),
	.usedw (msg_buf_size),
	.empty (msg_buf_empty)
	);


//Streaming registers to buffer video signal
STREAM_REG #(.DATA_WIDTH(26)) in_reg (
	.clk(clk),
	.rst_n(reset_n),
	.ready_out(sink_ready),
	.valid_out(in_valid),
	.data_out({red,green,blue,sop,eop}),
	.ready_in(out_ready),
	.valid_in(sink_valid),
	.data_in({sink_data,sink_sop,sink_eop})
);

STREAM_REG #(.DATA_WIDTH(26)) out_reg (
	.clk(clk),
	.rst_n(reset_n),
	.ready_out(out_ready),
	.valid_out(source_valid),
	.data_out({source_data,source_sop,source_eop}),
	.ready_in(source_ready),
	.valid_in(in_valid),
	.data_in({red_out, green_out, blue_out, sop, eop})
);


/////////////////////////////////
/// Memory-mapped port		 /////
/////////////////////////////////

// Addresses
`define REG_STATUS    			0
`define READ_MSG    				1
`define READ_ID    				2
`define REG_BBCOL					3

//Status register bits
// 31:16 - unimplemented
// 15:8 - number of words in message buffer (read only)
// 7:5 - unused
// 4 - flush message buffer (write only - read as 0)
// 3:0 - unused


// Process write

reg  [7:0]   reg_status;
reg	[23:0]	bb_col, rbb_col, gbb_col, bbb_col;

initial rbb_col = 24'hff0000;
initial gbb_col = 24'h00ff00;
initial bbb_col = 24'h0000ff;

//assign rbb_col = bb_col;
//assign gbb_col = bb_col;
//assign bbb_col = bb_col;

always @ (posedge clk)
begin
	if (~reset_n)
	begin
		reg_status <= 8'b0;
		bb_col <= BB_COL_DEFAULT;
	end
	else begin
		if(s_chipselect & s_write) begin
		   if      (s_address == `REG_STATUS)	reg_status <= s_writedata[7:0];
		   if      (s_address == `REG_BBCOL)	bb_col <= s_writedata[23:0];
		end
	end
end


//Flush the message buffer if 1 is written to status register bit 4
assign msg_buf_flush = (s_chipselect & s_write & (s_address == `REG_STATUS) & s_writedata[4]);


// Process reads
reg read_d; //Store the read signal for correct updating of the message buffer

// Copy the requested word to the output port when there is a read.
always @ (posedge clk)
begin
   if (~reset_n) begin
	   s_readdata <= {32'b0};
		read_d <= 1'b0;
	end
	
	else if (s_chipselect & s_read) begin
		if   (s_address == `REG_STATUS) s_readdata <= {16'b0,msg_buf_size,reg_status};
		if   (s_address == `READ_MSG) s_readdata <= {msg_buf_out};
		if   (s_address == `READ_ID) s_readdata <= 32'h1234EEE2;
		if   (s_address == `REG_BBCOL) s_readdata <= {8'h0, bb_col};
	end
	
	read_d <= s_read;
end

//Fetch next word from message buffer after read from READ_MSG
assign msg_buf_rd = s_chipselect & s_read & ~read_d & ~msg_buf_empty & (s_address == `READ_MSG);
						


endmodule


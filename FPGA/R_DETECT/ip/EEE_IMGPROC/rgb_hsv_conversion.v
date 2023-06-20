// https://www.programmersought.com/article/31054814413/
module  rgb_to_hsv(
clk,
rst,
rgb_r,
rgb_g,
rgb_b,
hsv_h,//  0 - 360
hsv_s,// 0- 255
hsv_v // 0- 255
);

input                         clk;
input                       rst;
input     [7:0]            rgb_r;
input     [7:0]            rgb_g;
input     [7:0]            rgb_b;
output reg [8:0]  hsv_h; //  0 - 360
output reg [7:0]  hsv_s; // 0- 255
output reg [7:0]  hsv_v; // 0- 255

reg [7:0] top;//molecular
reg [13:0] top_60;// *60
reg [2:0] rgb_se;
reg [2:0] rgb_se_n;//direction
reg [7:0] max;//Maximum weight
reg [7:0] min;//Minimum component
reg [7:0] max_min;// delta (max - min)
reg [7:0] hsv_s_m;
reg [7:0] max_n;
reg [7:0] division;//division

wire r_g, r_b, g_b;

//find max min 1----
assign r_g = (rgb_r > rgb_g) ? 1'b1 : 1'b0;
assign r_b = (rgb_r > rgb_b) ? 1'b1 : 1'b0;
assign g_b = (rgb_g > rgb_b) ? 1'b1 : 1'b0;

always @ (posedge clk or negedge rst) begin
	if (!rst) begin
		max <= 8'd0;
		min <= 8'd0;
		top <= 8'd0;
		rgb_se <= 3'b010;
	end
	else begin
	case ({r_g, r_b, g_b})
	3'b000:
			begin//b g r
			max <= rgb_b;
			min <= rgb_r;
			top <= rgb_g - rgb_r;//-
			rgb_se <= 3'b000;
			end
	3'b001:
			begin//g b r
			max <= rgb_g;
			min <= rgb_r;
			top <= rgb_b - rgb_r;//+
			rgb_se <= 3'b001;
			end
	3'b011:
			begin//g r b
			max <= rgb_g;
			min <= rgb_b;
			top <= rgb_r - rgb_b;//-
			rgb_se <= 3'b011;
			end
	3'b100:
			begin//b r g
			max <= rgb_b;
			min <= rgb_g;
			top <= rgb_r - rgb_g;//+
			rgb_se <= 3'b100;
			end
	3'b110:
			begin//r b g
			max <= rgb_r;
			min <= rgb_g;
			top <= rgb_b - rgb_g;//+
			rgb_se <= 3'b110;
			end
	3'b111:
			begin//r g b
			max <= rgb_r;
			min <= rgb_b;
			top <= rgb_g - rgb_b;//-
			rgb_se <= 3'b111;
			end
	default
			begin
			max <= 8'd0;
			min <= 8'd0;
			top <= 8'd0;
			rgb_se <= 3'b010;
			end
	endcase
end
end

// *60   max - min          2-----------------
always @ (posedge clk or negedge rst)
begin
	if (!rst)
		begin
		top_60 <= 14'd0;
		rgb_se_n <= 3'b010;
		max_min <= 8'd0;
		max_n <= 8'd0;
		end
	else
		begin
		top_60 <= {top,6'b000000} - {top,2'b00};//60 = 2^6 - 2^2
		rgb_se_n <= rgb_se;
		max_min <= max - min;
		max_n <= max;
		end
end
//   /(max - min)    3----------------------
always @ (*)
begin
	division = (max_min > 8'd0) ? top_60 / max_min : 8'd240;//Note that max = min  
end
// + - 120 240 360
always @ (posedge clk or negedge rst)
begin
	if (!rst)

		hsv_h <= 9'd0;

	else
	begin
	case (rgb_se_n)

	3'b000:
			//b g r
			hsv_h <= 9'd240 - division;//-

	3'b001:
			//g b r
			hsv_h <= 9'd120 + division;//+

	3'b011:
			//g r b
			hsv_h <= 9'd120 - division;//-

	3'b100:
			//b r g
			hsv_h <= 9'd240 + division;//+

	3'b110:
			//r b g
			hsv_h <= 9'd360 - division;//-


	3'b111:
			//r g b
			hsv_h <= division;//+

	default
			hsv_h <= 9'd0;
	endcase
end
end

//  s=(max - min)/max * 256
always @ (*)
begin
	hsv_s_m = (max_n > 8'd0)? {max_min[7:0],8'b00000000} / max_n : 8'd0;
end
always@(posedge clk or negedge rst)
begin
	if (!rst)
      hsv_s <= 8'd0;
	else
	hsv_s <= hsv_s_m;
end
//  hsv_v = max
always@(posedge clk or negedge rst)
begin
  if (!rst)
  hsv_v <= 8'd0;
  else
  hsv_v <= max_n;
 end
 // 3-------------------

endmodule
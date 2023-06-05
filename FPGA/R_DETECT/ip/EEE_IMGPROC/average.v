module Averaging(
  input wire clk,
  input wire reset,
  input wire [10:0] x,
  output wire [10:0] x_avg
);
  parameter WINDOW_SIZE = 50;  
  parameter THRESHOLD = 10;    

  reg [10:0] shift_register [WINDOW_SIZE-1:0];
  reg [10:0] sum;
  reg [10:0] average;
  reg [WINDOW_SIZE-1:0] anomaly_counter;
  reg [WINDOW_SIZE-1:0] valid_data_count;

  always @(posedge clk or posedge reset) begin
    if (reset) begin
      shift_register <= '{11{0}};
      sum <= '0;
      average <= '0;
      anomaly_counter <= '0;
      valid_data_count <= '0;
    end else begin
    for (int i = WINDOW_SIZE-1; i > 0; i = i-1)
      shift_register[i] <= shift_register[i-1];
      shift_register[0] <= x;

      if (x < THRESHOLD || x > THRESHOLD)
        anomaly_counter <= anomaly_counter + 1;
      else
        anomaly_counter <= '0;

      // Update valid data count and sum
      if (anomaly_counter >= WINDOW_SIZE) begin
        valid_data_count <= WINDOW_SIZE;
        sum <= sum + x;
      end else if (anomaly_counter > 0) begin
        valid_data_count <= anomaly_counter - 1;
        sum <= sum + x;
      end else begin
        valid_data_count <= anomaly_counter;
        sum <= sum;
      end
    end
  end

  assign x_avg = sum / valid_data_count;

endmodule

// Calculate the average x-coordinate
reg [10:0] sum;
reg counter;
reg [10:0] r_x_avg, y_x_avg, b_x_avg;

always@(posedge clk) begin
	if (red_detect & in_valid) begin
		sum <= sum + x;
	end
	
	else if (yellow_detect & in_valid) begin //Update bounds when the pixel is yellow
		sum <= sum + x;
	end
	
	else if (blue_detect & in_valid) begin //Update bounds when the pixel is blue
		sum <= sum + x;
	end
	
// find the average value for x-coordinate
reg [10:0] sum_red;
reg [10:0] sum_yellow;
reg [10:0] sum_blue;
reg [10:0] count_red;
reg [10:0] count_yellow;
reg [10:0] count_blue;
reg [10:0] r_x_avg;
reg [10:0] y_x_avg;
reg [10:0] b_x_avg;

always @(posedge clk) begin
    if (in_valid) begin
        if (red_detect) begin
            sum_red <= sum_red + x;
            count_red <= count_red + 1;
        end
        if (yellow_detect) begin
            sum_yellow <= sum_yellow + x;
            count_yellow <= count_yellow + 1;
        end
        if (blue_detect) begin
            sum_blue <= sum_blue + x;
            count_blue <= count_blue + 1;
        end
    end
end

always @(posedge clk) begin
    if (in_valid) begin
        if (count_red != 0)
            r_x_avg <= sum_red / count_red;
        if (count_yellow != 0)
            y_x_avg <= sum_yellow / count_yellow;
        if (count_blue != 0)
            b_x_avg <= sum_blue / count_blue;
    end
end

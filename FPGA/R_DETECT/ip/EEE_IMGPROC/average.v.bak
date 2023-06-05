module Averaging(
  input wire clk,
  input wire reset,
  input wire [10:0] x,
  output wire [10:0] x_avg,
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

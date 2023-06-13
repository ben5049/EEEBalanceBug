# Imports
import numpy as np
import matplotlib.pyplot as plt

# FIR Filter
def fir_filter(coefficients, input_array):
    output = np.zeros(len(input_array))
    for i in range(len(input_array)):
        for j in range(len(coefficients)):
            if i - j >= 0:
                output[i] += coefficients[j] * input_array[i - j]
    return output

# Plots mad+dmp pitch on one graph and mad+dmp yaw on another
def overlay_data(data1, data2, label1, data3, data4, label2):
    x = range(len(data1))  # x-axis values (assumed to be the indices of the array)

    plt.subplot(2,1,1)
    plt.plot(x, data1, label='DMP')
    plt.plot(x, data2, label='Madgwick')
    plt.xlabel('Index')
    plt.ylabel('Value')
    plt.title(label1)
    plt.legend()

    plt.subplot(2,1,2)
    plt.plot(x, data3, label='DMP')
    plt.plot(x, data4, label='Madgwick')
    plt.xlabel('Index')
    plt.ylabel('Value')
    plt.title(label2)
    plt.legend()

    plt.show()

# Inputs
coefficients = [0.02509274331174643,0.12664894963152423,0.26587759991367516,0.33589861675220817,0.26587759991367516,0.12664894963152423,0.02509274331174643,]
filename = "sensor_fusion_test.txt"  # Replace with the actual file name



# Format data
dmp_yaw = []
mad_yaw = []
dmp_pitch = []
mad_pitch = []

try:
    with open(filename, "r") as file:
        lines = file.readlines()
        for line in lines:
            line = line.strip() # Use .strip() to remove leading/trailing whitespace
            line = line.split(',')
            dmp_yaw.append(float(line[0][9:]))
            dmp_pitch.append(float(line[1][12:]))
            mad_yaw.append(float(line[2][15:]))
            mad_pitch.append(float(line[3][17:]))
            print(line[0][9:], line[1][12:], line[2][15:], line[3][17:])
        
except FileNotFoundError:
    print(f"File '{filename}' not found.")
except IOError:
    print(f"Error reading file '{filename}'.")


# Apply FIR filter
# Plot

overlay_data(dmp_yaw, mad_yaw, "Yaw", dmp_pitch, mad_pitch,"Pitch")




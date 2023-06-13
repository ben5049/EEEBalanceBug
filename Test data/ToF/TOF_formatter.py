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

# Plots Left and Right TOF sensor data + differential + differential^2
def overlay_data(data1, data2, label1, data3, data4, label2):
    x = range(len(data1))  # x-axis values (assumed to be the indices of the array)

    plt.subplot(6, 1, 1)
    plt.plot(x, data1, label='Unfiltered')
    plt.plot(x, data2, label='Filtered')
    plt.xlabel('Index')
    plt.ylabel('Value')
    plt.title(label1)
    plt.legend()

    plt.subplot(6, 1, 2)
    differential1 = np.diff(data1)
    differential2 = np.diff(data2)
    x_diff = range(len(differential1))
    plt.plot(x_diff, differential1, label='(d/dt) unfiltered')
    plt.plot(x_diff, differential2, label='(d/dt) filtered')
    plt.xlabel('Index')
    plt.ylabel('Differential')
    plt.title(label1 + ' Differential')
    plt.legend()

    plt.subplot(6, 1, 3)
    differential1 = np.diff(np.diff(data1))
    differential2 = np.diff(np.diff(data2))
    x_diff = range(len(differential1))
    plt.plot(x_diff, differential1, label='(d/dt)^2 unfiltered')
    plt.plot(x_diff, differential2, label='(d/dt)^2 filtered')
    plt.xlabel('Index')
    plt.ylabel('Differential')
    plt.title(label1 + ' Differentials')
    plt.legend()

    plt.subplot(6, 1, 4)
    plt.plot(x, data3, label='Unfiltered')
    plt.plot(x, data4, label='Filtered')
    plt.xlabel('Index')
    plt.ylabel('Value')
    plt.title(label2)
    plt.legend()

    plt.subplot(6, 1, 5)
    differential1 = np.diff(data3)
    differential2 = np.diff(data4)
    x_diff = range(len(differential1))
    plt.plot(x_diff, differential1, label='(d/dt) unfiltered')
    plt.plot(x_diff, differential2, label='(d/dt) filtered')
    plt.xlabel('Index')
    plt.ylabel('Differential')
    plt.title(label2 + ' Differential')
    plt.legend()

    plt.subplot(6, 1, 6)
    differential1 = np.diff(np.diff(data3))
    differential2 = np.diff(np.diff(data4))
    x_diff = range(len(differential1))
    plt.plot(x_diff, differential1, label='(d/dt)^2 unfiltered')
    plt.plot(x_diff, differential2, label='(d/dt)^2 filtered')
    plt.xlabel('Index')
    plt.ylabel('Differential')
    plt.title(label2 + ' Differentials')
    plt.legend()

    plt.tight_layout()
    plt.show()

# Inputs
coefficients = [0.02509274331174643,0.12664894963152423,0.26587759991367516,0.33589861675220817,0.26587759991367516,0.12664894963152423,0.02509274331174643,]
filename = "putNameHere"  # Replace with the actual file name



# Format data
R = []
L = []
try:
    with open(filename, "r") as file:
        lines = file.readlines()
        for line in lines:
            line = line.strip() # Use .strip() to remove leading/trailing whitespace
            line = line.split(',')
            R.append(int(line[0][7:]))
            L.append(int(line[1][6:]))
            print(line)
        #print(R)
        #print(L)
        
except FileNotFoundError:
    print(f"File '{filename}' not found.")
except IOError:
    print(f"Error reading file '{filename}'.")


# Apply FIR filter
L_Filtered = fir_filter(coefficients, L)
R_Filtered = fir_filter(coefficients, R)
# Plot
overlay_data(L, L_Filtered, "Left", R, R_Filtered,"Right")




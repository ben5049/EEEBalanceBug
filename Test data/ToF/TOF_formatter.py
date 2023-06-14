# Imports
import numpy as np
import matplotlib.pyplot as plt


# Inputs
coefficients = [0.02509274331174643,0.12664894963152423,0.26587759991367516,0.33589861675220817,0.26587759991367516,0.12664894963152423,0.02509274331174643,]
filename = "data.txt"  # Replace with the actual file name







# FIR Filter
def fir_filter(coefficients, input_array):
    output = np.zeros(len(input_array))
    for i in range(len(input_array)):
        for j in range(len(coefficients)):
            if i - j >= 0:
                output[i] += coefficients[j] * input_array[i - j]
    return output

# Plots Left and Right TOF sensor data + differential + differential^2
def Plot_TOF(data1, data2, label1, data3, data4, label2):
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

    plt.show()

# Plots Yaw sensor data + differential + differential^2
def Plot_Yaw(data1, data2):
    x = range(len(data1))  # x-axis values (assumed to be the indices of the array)

    plt.subplot(3, 1, 1)
    plt.plot(x, data1, label='Unfiltered')
    plt.plot(x, data2, label='Filtered')
    plt.xlabel('Index')
    plt.ylabel('Value')
    plt.title("YAW")
    plt.legend()

    plt.subplot(3, 1, 2)
    differential1 = np.diff(data1)
    differential2 = np.diff(data2)
    x_diff = range(len(differential1))
    plt.plot(x_diff, differential1, label='(d/dt) unfiltered')
    plt.plot(x_diff, differential2, label='(d/dt) filtered')
    plt.xlabel('Index')
    plt.ylabel('Differential')
    plt.title("YAW" + ' Differential')
    plt.legend()

    plt.subplot(3, 1, 3)
    differential1 = np.diff(np.diff(data1))
    differential2 = np.diff(np.diff(data2))
    x_diff = range(len(differential1))
    plt.plot(x_diff, differential1, label='(d/dt)^2 unfiltered')
    plt.plot(x_diff, differential2, label='(d/dt)^2 filtered')
    plt.xlabel('Index')
    plt.ylabel('Differential')
    plt.title("YAW" + ' Differentials')
    plt.legend()

    plt.tight_layout()
    plt.show()


# Plot all data overlayed together
def Plot_All(data1, data2, label1, data3, data4, label2, data5, label3):
    x = range(len(data1))  # x-axis values (assumed to be the indices of the array)

    plt.subplot(5, 1, 1)
    #plt.plot(x, data1, label='Unfiltered')
    plt.plot(x, data2, label='Filtered')
    plt.xlabel('Index')
    plt.ylabel('Value')
    plt.title(label1)
    plt.axvline(x = 0, color = 'b')
    plt.axvline(x = 55, color = 'b')
    plt.axvline(x = 89, color = 'b')
    plt.axvline(x = 128, color = 'b')
    plt.axvline(x = 166, color = 'b')
    plt.legend()

    plt.subplot(5, 1, 2)
    differential1 = np.diff(data1)
    differential2 = np.diff(data2)
    x_diff = range(len(differential1))
    #plt.plot(x_diff, differential1, label='(d/dt) unfiltered')
    plt.plot(x_diff, differential2, label='(d/dt) filtered')
    plt.xlabel('Index')
    plt.ylabel('Differential')
    plt.title(label1 + ' Differential')
    plt.axvline(x = 0, color = 'b')
    plt.axvline(x = 55, color = 'b')
    plt.axvline(x = 89, color = 'b')
    plt.axvline(x = 128, color = 'b')
    plt.axvline(x = 166, color = 'b')
    plt.legend()

    plt.subplot(5, 1, 3)
    #plt.plot(x, data3, label='Unfiltered')
    plt.plot(x, data4, label='Filtered')
    plt.xlabel('Index')
    plt.ylabel('Value')
    plt.title(label2)
    plt.axvline(x = 0, color = 'b')
    plt.axvline(x = 55, color = 'b')
    plt.axvline(x = 89, color = 'b')
    plt.axvline(x = 128, color = 'b')
    plt.axvline(x = 166, color = 'b')
    plt.legend()

    plt.subplot(5, 1, 4)
    differential1 = np.diff(data3)
    differential2 = np.diff(data4)
    #plt.plot(x_diff, differential1, label='(d/dt) unfiltered')
    plt.plot(x_diff, differential2, label='(d/dt) filtered')
    plt.xlabel('Index')
    plt.ylabel('Differential')
    plt.title(label2 + ' Differential')
    plt.axvline(x = 0, color = 'b')
    plt.axvline(x = 55, color = 'b')
    plt.axvline(x = 89, color = 'b')
    plt.axvline(x = 128, color = 'b')
    plt.axvline(x = 166, color = 'b')
    plt.legend()

    plt.subplot(5, 1, 5)
    plt.plot(x, data5, label='Unfiltered')
    #plt.plot(x, data6, label='Filtered')
    plt.xlabel('Index')
    plt.ylabel('Value')
    plt.title(label3)
    plt.axvline(x = 0, color = 'b')
    plt.axvline(x = 55, color = 'b')
    plt.axvline(x = 89, color = 'b')
    plt.axvline(x = 128, color = 'b')
    plt.axvline(x = 166, color = 'b')
    plt.legend()

    plt.show()

# Test offset TOF
def Plot_Offset(Filtered_Left, Filtered_Right, Yaw):
    x = range(len(Filtered_Left))  # x-axis values (assumed to be the indices of the array)
    Filtered_Left = [i+90 for i in Filtered_Left]
    Filtered_Right = [i-90 for i in Filtered_Right]
    plt.subplot(1, 1, 1)
    plt.plot(Yaw, Filtered_Left,'o', label='Left')
    plt.plot(Yaw, Filtered_Right,'o', label='Right')
    plt.style.use('seaborn-whitegrid')
    plt.xlabel('Yaw')
    plt.ylabel('Distance')
    plt.title("Left (Filtered)")
    plt.legend()

    plt.show()



# Format data
R = []
L = []
Yaw = []
try:
    with open(filename, "r") as file:
        lines = file.readlines()
        for line in lines:
            line = line.strip() # Use .strip() to remove leading/trailing whitespace
            if ("*" in line) or ("Termite log" in line) or (len(line) == 0):
                continue
            line = line.replace(" ", "")
            line = line.split(',')
            R_val = float(line[0][6:])
            if (R_val == 65535):
                R.append(-1)
            else:
                R.append(R_val)
            L_val = float(line[1][5:])
            if (L_val == 65535):
                L.append(-1)
            else:
                L.append(L_val)
            Yaw.append(float(line[2][4:]))
            #print(line)
        #print(R)
        #print(L)
        
except FileNotFoundError:
    print(f"File '{filename}' not found.")
except IOError:
    print(f"Error reading file '{filename}'.")

# Debug
#for i in range(len(R)):
    #print('R: ' + str(R[i]) + ' L:' + str(L[i]) + ' Yaw:' + str(Yaw[i]))

# Apply FIR filter
L_Filtered = fir_filter(coefficients, L)
R_Filtered = fir_filter(coefficients, R)
Yaw_Filtered = fir_filter(coefficients, Yaw)


# Plot
#Plot_TOF(L, L_Filtered, "Left", R, R_Filtered,"Right")
#Plot_Yaw(Yaw, Yaw_Filtered)
Plot_All(L[150:340], L_Filtered[150:340], "Left", R[150:340], R_Filtered[150:340],"Right", Yaw[150:340], "YAW")
Plot_Offset(L_Filtered[150:319], R_Filtered[150:319], Yaw[150:319])

# Imports
import numpy as np
import matplotlib.pyplot as plt


# Plots
def overlay_data2(data1, data2, label1):
    x = range(len(data1))  # x-axis values (assumed to be the indices of the array)

#    plt.subplot(2,1,1)
    plt.subplot(3, 2, (1, 2))
    plt.margins(x=0, y=0.3)
    plt.plot(x, data1, label='No WiFi')
    plt.plot(x, data2, label='With WiFi')
    plt.xlabel('Sample Index')
    plt.ylabel('Magnitude')
    plt.title(label1)
    plt.legend()


    plt.subplots_adjust(hspace = 0.34)
    plt.show()

def overlay_data3(data1, data2, data3, label1, location):
    x = range(len(data1))  # x-axis values (assumed to be the indices of the array)

    plt.subplot(3, 2, location)
    plt.margins(x=0, y=0.3)
    plt.plot(x, data1, label='P Term')
    plt.plot(x, data2, label='I Term')
    plt.plot(x, data3, label='D Term')
    plt.xlabel('Sample Index')
    plt.ylabel('Magnitude')
    plt.title(label1)
    plt.legend()


    plt.subplots_adjust(hspace = 0.34)
    plt.show()

def overlay_data_all(data1, data2, data3, data4, data5, data6, data7, data8, label1, label2, label3):
    x = range(len(data1))  # x-axis values (assumed to be the indices of the array)

    plt.subplot(2, 2, (1, 2))
    plt.margins(x=0, y=0.3)
    plt.plot(x, data1, label='No WiFi')
    plt.plot(x, data2, label='With WiFi')
    plt.xlabel('Sample Index')
    plt.ylabel('Magnitude')
    plt.title(label1)
    plt.legend()
    
    plt.subplot(2, 2, 3)
    plt.margins(x=0, y=0.3)
    plt.plot(x, data3, label='P Term')
    plt.plot(x, data4, label='I Term')
    plt.plot(x, data5, label='D Term')
    plt.xlabel('Sample Index')
    plt.ylabel('Magnitude')
    plt.title(label2)
    plt.legend()

    plt.subplot(2, 2, 4)
    plt.margins(x=0, y=0.3)
    plt.plot(x, data6, label='P Term')
    plt.plot(x, data7, label='I Term')
    plt.plot(x, data8, label='D Term')
    plt.xlabel('Sample Index')
    plt.ylabel('Magnitude')
    plt.title(label3)
    plt.legend()


    plt.subplots_adjust(hspace = 0.5)
    plt.show()

# Inputs

filename1 = "PID_no_wifi.txt"  # Replace with the actual file name
filename2 = "PID_wifi1.txt"  # Replace with the actual file name


# Format data
total_wifi = []
pterm_wifi = []
iterm_wifi = []
dterm_wifi = []

total_no_wifi = []
pterm_no_wifi = []
iterm_no_wifi = []
dterm_no_wifi = []

try:
    with open(filename1, "r") as file:
        lines = file.readlines()
        for line in lines:
            line = line.strip() # Use .strip() to remove leading/trailing whitespace
            line = line.split(',')
            new_val = float(str(((line[3].split(':'))[1]).strip()))
            pterm_no_wifi.append(float(str(((line[0].split(':'))[1]).strip())))
            iterm_no_wifi.append(float(str(((line[1].split(':'))[1]).strip())))
            dterm_no_wifi.append(float(str(((line[2].split(':'))[1]).strip())))
            total_no_wifi.append(new_val)
            print(new_val)
        
except FileNotFoundError:
    print(f"File '{filename}' not found.")
except IOError:
    print(f"Error reading file '{filename}'.")

try:
    with open(filename2, "r") as file:
        lines = file.readlines()
        for line in lines:
            line = line.strip() # Use .strip() to remove leading/trailing whitespace
            line = line.split(',')
            new_val = float(str(((line[3].split(':'))[1]).strip()))
            pterm_wifi.append(float(str(((line[0].split(':'))[1]).strip())))
            iterm_wifi.append(float(str(((line[1].split(':'))[1]).strip())))
            dterm_wifi.append(float(str(((line[2].split(':'))[1]).strip())))
            total_wifi.append(new_val)
            print(new_val)
        
except FileNotFoundError:
    print(f"File '{filename}' not found.")
except IOError:
    print(f"Error reading file '{filename}'.")

while (len(total_no_wifi) > len(total_wifi)):
    total_wifi.append(0)

while (len(total_wifi) > len(total_no_wifi)):
    total_no_wifi.append(0)

total_no_wifi = total_no_wifi[:120]
pterm_no_wifi = pterm_no_wifi[:120]
iterm_no_wifi = iterm_no_wifi[:120]
dterm_no_wifi = dterm_no_wifi[:120]

total_wifi = total_wifi[:120]
pterm_wifi = pterm_wifi[:120]
iterm_wifi = iterm_wifi[:120]
dterm_wifi = dterm_wifi[:120]


##overlay_data2(total_no_wifi, total_wifi, "PID Output")
##overlay_data3(pterm_no_wifi, iterm_no_wifi, dterm_no_wifi, "Contributions (No Wifi)", 3)
##overlay_data3(pterm_wifi, iterm_wifi,dterm_wifi, "Contributions (With Wifi)", 4)

overlay_data_all(total_no_wifi, total_wifi, pterm_no_wifi, iterm_no_wifi, dterm_no_wifi, pterm_wifi, iterm_wifi,dterm_wifi, "PID Output", "Contributions (No Wifi)", "Contributions (With Wifi)")

import serial
import tkinter as tk
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import matplotlib.pyplot as plt
import threading

# Setup serial connection
ser = serial.Serial('COM3', 9600)  
ser.flushInput()

voltages = []

def read_serial():
    while True:
        if ser.in_waiting > 0:
            try:
                line = ser.readline().decode('utf-8').strip()
                if line:
                    voltage = float(line)
                    voltages.append(voltage)

                    if len(voltages) > 100:  
                        voltages.pop(0)

                    update_plot()
            except Exception as e:
                print(f"Error: {e}")

def update_plot():
    ax.clear()
    ax.plot(voltages)
    ax.set_ylim(0, 10)  # ADC range 0-5V
    ax.set_ylabel('Voltage (V)')
    ax.set_xlabel('Sample')
    ax.set_title('Live ADC Voltage')
    
    canvas.draw()

# Build tkinter window
root = tk.Tk()
root.title("ADC Voltage Waveform")

fig, ax = plt.subplots(figsize=(5, 3))
canvas = FigureCanvasTkAgg(fig, master=root)
canvas.get_tk_widget().pack()

# Start background thread
thread = threading.Thread(target=read_serial)
thread.daemon = True
thread.start()

root.mainloop()

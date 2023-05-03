# Import Library

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import socket
import keyboard
import threading
import math

# Define the IP address and port number
HOST = '192.168.1.1'
PORT = 288

# Create a socket object
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Connect to the device
sock.connect((HOST, PORT))

def on_key_press(event):
    if event.name == 'esc':
        # If Escape key was pressed, close the socket and exit the program
        sock.close()
        print('Program ended.')
        exit()
    elif event.name == 'm':
        # If 'm' key was pressed, clear the degree and rarr arrays and the plot
        #print('Graph cleared.')
        degree.clear()
        rarr.clear()
        ax.clear()
        last_key = event.name.encode()  # convert the key name to a byte string
        sock.send(last_key)  # send the last key pressed to the device 
    else:
        # Otherwise, send the last key pressed to the device
        last_key = event.name.encode()  # convert the key name to a byte string
        sock.send(last_key)  # send the last key pressed to the device
        #print(f'Sent key {last_key.decode()} to device')  # print the key that was sent

# Register the callback function for keypress events
keyboard.on_press(on_key_press)

# Define arrays to store theta and r values
degree = []
rarr = []
sizearr = []

# Define a function to receive data from the device and parse it
def receive_data():
    while True:
        data = sock.recv(2048)  # receive up to 1024 bytes of data from the device
        if not data:
            break  # if no data is received, break out of the loop
        lines = data.decode().strip().split('\n')  # split the received data into lines
        for line in lines:
            if line.startswith('!'):
                # Split the line and store the values in the corresponding arrays
                theta, r, size = line.strip('!').split('!')
                degree.append(math.radians(int(theta)))
                rarr.append(int(float(r)))
                sizearr.append(size)
            else:
                # If the line does not start with '!', print it to the terminal
                print(line)

# Start a separate thread to receive data from the device
receive_thread = threading.Thread(target=receive_data)
receive_thread.start()

# Initial graph setup 
fig = plt.figure()
ax = fig.add_subplot(projection='polar')
ax.set_thetamin(0)
ax.set_thetamax(180)

# Animation
def animate(i):

    # Loop for animation
    ax.clear()
    ax.set_rlabel_position(-22.5)
    ax.set_title("R2D2 Radar System", va='bottom')
    ax.set_thetamin(0)
    ax.set_thetamax(180)
    ax.scatter(degree, rarr)

ani = animation.FuncAnimation(fig, animate, interval=1000)

plt.show()

# Wait for keypress events
keyboard.wait()

# Wait for the receive thread to finish
receive_thread.join()
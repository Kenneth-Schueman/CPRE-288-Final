# Import Library

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import socket
import keyboard
import threading

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
    else:
        # Otherwise, send the last key pressed to the device
        last_key = event.name.encode()  # convert the key name to a byte string
        sock.send(last_key)  # send the last key pressed to the device
        #print(f'Sent key {last_key.decode()} to device')  # print the key that was sent

# Register the callback function for keypress events
keyboard.on_press(on_key_press)

# Define arrays to store theta and r values
thetaarr = []
rarr = []

# Define a function to receive data from the device and parse it
def receive_data():
    while True:
        data = sock.recv(1024)  # receive up to 1024 bytes of data from the device
        if not data:
            break  # if no data is received, break out of the loop
        lines = data.decode().strip().split('\n')  # split the received data into lines
        for line in lines:
            if line.startswith('a'):
                # If the line starts with 'a', extract the integer after it and add it to the theta array
                theta_value = int(line[1:])
                #theta_value = 2 * np.pi * theta_value
                thetaarr.append(theta_value)
                print(f'Theta: {theta_value} to array')
            elif line.startswith('d'):
                # If the line starts with 'd', extract the float after it, convert it to an integer, and add it to the r array
                r_value = int(float(line[1:]))
                rarr.append(r_value)
                print(f'r: {r_value} to array')
            else:
                # If the line does not start with 'a' or 'd', print it to the terminal
                print(f'{line}')

# Start a separate thread to receive data from the device
receive_thread = threading.Thread(target=receive_data)
receive_thread.start()



# Initial graph setup 
fig = plt.figure()
ax = fig.add_subplot(projection='polar')
# ax.set_thetamin(0)
# ax.set_thetamax(180)

# Animation
def animate(i):

    # Loop for animation
    ax.clear()
    ax.set_thetamin(0)
    ax.set_thetamax(180)
    ax.scatter(thetaarr, rarr)

ani = animation.FuncAnimation(fig, animate, interval=1000)

plt.show()

# Wait for keypress events
keyboard.wait()

# Clear arrays
thetaarr.clear()
rarr.clear()

# Wait for the receive thread to finish
receive_thread.join()
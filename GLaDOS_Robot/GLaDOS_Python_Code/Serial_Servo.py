import serial
import time

#Example : servo1:090


def servo_control():   
    while(True):
        data = input("message: ")
        arduino.write(data.encode('utf-8'))
        time.sleep(1)
        read_serial()

def read_serial_alltime():
    while True:
        if arduino.in_waiting > 0:
            line = arduino.readline().decode('utf-8').rstrip()
            print(line)

def read_serial():
    if arduino.in_waiting > 0:
        line = arduino.readline().decode('utf-8').rstrip()
        print(line)

def up():
    arduino.write(b'servo0:000\n')
    read_serial()
    time.sleep(1)

arduino = serial.Serial('COM5', 115200, timeout=1)
arduino.reset_input_buffer()
time.sleep(5)
servo_control()


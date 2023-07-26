import serial

#Example : servo1:170

while(True):
    ser = serial.Serial('/dev/ttyACM0', 115200, timeout=1)
    ser.reset_input_buffer()
    data = input()
    ser.write(data.encode('utf-8'))
    time.sleep(1)
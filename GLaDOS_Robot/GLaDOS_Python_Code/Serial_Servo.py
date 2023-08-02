import serial
import time
import cv2 

#Example : servo1:090


def servo_control():   
    while(True):
        data = input("message: ")
        arduino.write(data.encode('utf-8'))
        time.sleep(1)
        """read_serial()"""

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
    """read_serial()"""
    time.sleep(1)

def move_left()
    arduino.write(b'servo4:666\n')

def move_right()
    arduino.write(b'servo4:999\n')


#Calcul middle point 

face_cascade = cv2.CascadeClassifier('haarcascade_frontalface_default.xml')
eye_cascade = cv2.CascadeClassifier('haarcascade_eye.xml') 
  
# capture frames from a camera
cap = cv2.VideoCapture(0)
  
# loop runs if capturing has been initialized.
while 1: 
  
    # reads frames from a camera
    ret, img = cap.read() 
  
    # convert to gray scale of each frames
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
  
    # Detects faces of different sizes in the input image
    faces = face_cascade.detectMultiScale(gray, 1.3, 5)
  
    for (x,y,w,h) in faces:
        # To draw a rectangle in a face 
        cv2.rectangle(img,(x,y),(x+w,y+h),(255,255,0),2) 
        roi_gray = gray[y:y+h, x:x+w]
        roi_color = img[y:y+h, x:x+w]
  
        # Detects eyes of different sizes in the input image
        eyes = eye_cascade.detectMultiScale(roi_gray) 
  
        #To draw a rectangle in eyes
        for (ex,ey,ew,eh) in eyes:
            cv2.rectangle(roi_color,(ex,ey),(ex+ew,ey+eh),(0,127,255),2)
  
    # Display an image in a window
    cv2.imshow('img',img)
  
    # Wait for Esc key to stop
    k = cv2.waitKey(30) & 0xff
    if k == 27:
        break
  
# Close the window
cap.release()
  
# De-allocate any associated memory usage
cv2.destroyAllWindows() 


xmid=x+w/2

if xmid<  :
    move_left()
if xmid>  :
    move_right()


#Calcul servo move

"""
arduino = serial.Serial('COM5', 115200, timeout=1)
arduino.reset_input_buffer()
time.sleep(5)
servo_control()"""


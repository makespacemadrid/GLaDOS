import serial
import time
import cv2 
import face_recognition
import os
import pickle

#Example : servo1:090


def servo_control(data):
    #data = input("message: ")
    arduino.write(data.encode('utf-8'))
    time.sleep(1)
    """read_serial()"""

def read_serial():
    if arduino.in_waiting > 0:
        line = arduino.readline().decode('utf-8').rstrip()
        print(line)

def up():
    servo_control('servo0:000\n')

def move_left():
    servo_control('servo4:666\n')

def move_right():
    servo_control('servo4:999\n')


def create_trainpkl():
    Encodings=[]
    Names=[]
    image_dir="./people_pictures"
    for root,dirs,files in os.walk(image_dir):
        for file in files:
            path=os.path.join(root,file)
            name=os.path.splitext(file)[0]
            person=face_recognition.load_image_file(path)
            encoding=face_recognition.face_encodings(person)[0]
            Encodings.append(encoding)
            Names.append(name)
    with open('train.pkl','wb')as f:
        pickle.dump(Names,f)
        pickle.dump(Encodings,f)

def prepare():
    Encodings=[]
    Names=[]
    with open('train.pkl','rb') as f:
        Names =pickle.load(f)
        Encodings=pickle.load(f)
    font=cv2.FONT_HERSHEY_SIMPLEX
    cam=cv2.VideoCapture(0)
    yield Names
    yield Encodings
    yield cam
    yield font

def face_id(Names,Encodings,cam,font):
    new_people=0
    while True:
        _,frame=cam.read()
        #frameSmall=cv2.resize(frame,(0,0),fx=.175,fy=.175)
        frameSmall=cv2.resize(frame,(0,0),fx=.33,fy=.33)
        frameRGB=cv2.cvtColor(frameSmall,cv2.COLOR_BGR2RGB)
        facePositions=face_recognition.face_locations(frameRGB,model='cnn')
        allEncodings=face_recognition.face_encodings(frameRGB,facePositions)
        for (top,right,bottom,left),face_encoding in zip(facePositions, allEncodings):
            name='Unknown Person'
            matches = face_recognition.compare_faces(Encodings, face_encoding)
            print(matches)
            if True in matches:
                first_match_index=matches.index(True)
                name=Names[first_match_index]
                new_people=0
            elif facePositions :
                new_people=new_people +1
                print(new_people)
                if new_people == 20:
                    add_face_picture(cam)
                    cam.release()
                    prep=prepare()
                    Names=next(prep)
                    Encodings=next(prep)
                    cam=next(prep)

                    font=next(prep)
                    break

            else:
                 new_people=0
            print(Names)
            '''
            top=top*6
            right=right*6
            bottom=bottom*6
            left=left*6
            '''
            top=top*3
            right=right*3
            bottom=bottom*3
            left=left*3          
            print("1")
            cv2.rectangle(frame,(left,top),(right,bottom),(0,0,255),2)
            cv2.putText(frame,name,(left,top-6),font,.75,(0,0,255),2)
            print("2")
            middle_face_x=(left+right)/2
            middle_face_y=(top+bottom)/2
            print("3")
            servo_control('servox:'+str(middle_face_x)+'\n')
            servo_control('servoy:'+str(middle_face_y)+'\n')
            print("4")
            print(middle_face_x)
            print(middle_face_y)
            print("5")
        cv2.imshow('Picture',frame)
        cv2.moveWindow('Picture',0,0)
        if cv2.waitKey(1)==ord('q'):
            break
    cam.release()
    cv2.destroyAllWindows()



def add_face_picture(cam):
    result,picture=cam.read()
    if result:
        name=input("I don't know you, What is your name\n")
        directory='./people_pictures'
        os.chdir(directory)
        cv2.imwrite(name+".jpg",picture)
        directory='/glados'
        os.chdir(directory)
        create_trainpkl()
'''
def arduino_start():
    arduino = serial.Serial('dev/ttyACM0', 115200, timeout=1)
    arduino.reset_input_buffer()
    time.sleep(5)
'''
arduino = serial.Serial('/dev/ttyACM0', 115200, timeout=1)
arduino.reset_input_buffer()
time.sleep(5)
prep=prepare()
face_id(next(prep),next(prep),next(prep),next(prep))

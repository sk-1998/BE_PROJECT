
import socket 
import threading 
import pandas as pd 
import csv 
import os
import numpy as np
from matplotlib import pyplot as plt 

PORT = 8282 
SERVER_IP = socket.gethostbyname(socket.gethostname())
ACK = "OK"
BUFF_FLAG = False


print(f"HOST NAME : {socket.gethostname()}")
print(f"HOST IP : {SERVER_IP}")
print(f"PORT NO. : {PORT}")

SERVER = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
SERVER.bind((SERVER_IP,PORT))

def START_SERVER():
    global BUFF_FLAG
    SERVER.listen()
    print("SERVER STARTED LISTENING... ! ")
    while True:
        CLIENT , ADDR = SERVER.accept()
        BUFF = open('C:\\Users\\SERVER\\SERVER_SCRIPT\\BUFFER\\buffer.txt', 'w+')
        while 1:
            msg = None 
            msg = CLIENT.recv(1024)
            print(f"MSG FROM {ADDR}")
            print(msg)
            print("PACK_RECV")
            CLIENT.send(ACK.encode("UTF-8"))
            if (msg.decode("UTF-8").upper() == "CLOSE" ):
                CLIENT.close()
                BUFF.close()
                BUFF_FLAG = True 
                break 
            BUFF.write(msg.decode("UTF-8"))
        
        break


def TXTtoCSV ():
    file1 = open('C:\\Users\\SERVER\\SERVER_SCRIPT\\BUFFER\\buffer.txt',"r+")

    data =  file1.readlines()
    with open('C:\\Users\\SERVER\\SERVER_SCRIPT\\CSV\\SIG_DATA.csv', 'w', newline='') as file:
        writer = csv.writer(file)
        for i in data:
            print(i.split(":"))
            writer.writerow(i.split(":"))
        
def representation() :
    
    df = pd.read_csv('C:\\Users\\SERVER\\SERVER_SCRIPT\\CSV\\SIG_DATA.csv' , header=None , names = ['SR','ACC_X','ACC_Y','ACC_Z','GYRO_X','GYRO_Y','GYRO_Z','DIST']) 
    df.dropna(inplace = True)

    fig , ax = plt.subplots(2)
    print (fig)
    ax[0].set_title("ACCELERATION SIGNAL")
    ax[0].plot(df.index,df.ACC_X)
    ax[0].plot(df.index,df.ACC_Y)
    ax[0].plot(df.index,df.ACC_Z)
    ax[0].set_xticks( np.arange(0,int(df.index.max()),10))
    ax[0].legend(['ACC_X','ACC_Y','ACC_Z'])
    ax[0].set_ylabel("ACCELERATION")
    ax[0].set_xlabel("SAMPELS")

    ax[1].set_title("GYROSIGNAL SIGNAL")
    ax[1].plot(df.index,df.GYRO_X)
    ax[1].plot(df.index,df.GYRO_Y)
    ax[1].plot(df.index,df.GYRO_Z)
    ax[1].set_xticks( np.arange(0,int(df.index.max()),10))
    ax[1].legend(['GYRO_X','GYRO_Y','GYRO_Z'])
    ax[1].set_ylabel("ROTATIONAL RATE")
    ax[1].set_xlabel("SAMPELS")
    plt.show()

while True:

    if BUFF_FLAG == False :
        START_SERVER()
        
        

    else :
        BUFF_FLAG = False
        TXTtoCSV ()
        representation()
        break

    print("DONE")
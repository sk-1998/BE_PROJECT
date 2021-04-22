import serial
import psutil
import time
import socket
import threading


HOST_NAME = socket.gethostname()
IPv4 = socket.gethostbyname(HOST_NAME)
PORT = 3090
CLT = 0
CONSOL = "HOLA"

def load_para():
   
    HN = HOST_NAME.split('-')[1]
    CPU = int(psutil.cpu_percent())
    RAM = int(psutil.virtual_memory().percent)
    HDD = int(psutil.disk_usage('C:\\').percent)

    MSG = "PARA:DT-" + str(HN) + ':' + IPv4 + ':' + str(PORT) + ':' + str(CPU) + ':' \
            + str(RAM) + ':' + str(HDD) + ':' + str(CLT) + ':'
        
    return MSG



def DisplayLoop():
    time.sleep(1)
    while(True):
        time.sleep(1.1)
        Serial_COM1.write(load_para().encode('ascii'))
        time.sleep(1.1)
        val = "CONSOL:" + CONSOL
        Serial_COM1.write(val.encode('ascii'))
        
        
        

try:
    Serial_COM1 = serial.Serial(port = 'COM7',baudrate = 250000, timeout = 1 )

except serial.SerialException:
    print("COM PORT NOT FOUND !")
    exit()
         
     
Thread_01 = threading.Thread(target = DisplayLoop)

Serial_COM1.write("OK".encode('ascii'))

time.sleep(2)

print(HOST_NAME)
print(IPv4)
print(PORT)

Thread_01.start()

i = 0 
x = 0 
while True:
    x += 1 
    CONSOL = str (x)  
    time.sleep(2)



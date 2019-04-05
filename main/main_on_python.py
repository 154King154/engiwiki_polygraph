import serial
import json

ser = serial.Serial(
    port='/dev/ttyACM1',
    baudrate=57600,
    parity=serial.PARITY_ODD,
    stopbits=serial.STOPBITS_TWO,
    bytesize=serial.SEVENBITS
)
alpha = 0
beta = 0
emgi = 0
gsr = 0
temp = 0
rr = 0

def convertData(byte_str):
    global alpha, beta, emgi, gsr, temp, rr
    decode_str = byte_str.decode('utf-8')
    data = json.loads(decode_str[:-1])

    alpha = data['alpha']
    beta = data['beta']
    emgi = data['emgi']
    gsr = data['gsr']
    temp = data['temp']
    rr = data['rr']



#ser.open()
i=0
while (i<15):
    byte_str = ser.readline()
    if (byte_str[0]==123) and (byte_str[1]==34):
        convertData(byte_str)
        print(gsr)
        i+=1

ser.close()             # close port

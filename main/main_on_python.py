import serial
import json
import time
import numpy as np
import matplotlib.pyplot as plt

ser = serial.Serial(
    port='/dev/ttyACM0',
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

    alpha = float(data['alpha'])
    beta = float(data['beta'])
    emgi = float(data['emgi'])
    gsr = float(data['gsr'])
    temp = float(data['temp'])
    rr = float(data['rr'])



#ser.open()
time_i=time.time()
time_j=time.time()
test_gsr = []
test_emgi = []
while (abs(time_i-time_j)<12):
    byte_str = ser.readline()
    if (byte_str[0]==123) and (byte_str[1]==34):
        convertData(byte_str)
        print(abs(time_i-time_j))
        test_gsr.append(gsr)
        test_emgi.append(emgi)
    time_j = time.time()
#print(np.std(test_gsr), np.mean(test_gsr))


x = list(range(len(test_gsr)))


plt.figure(1)

# gsr
plt.subplot(221)
plt.plot(x, test_gsr)
#plt.yscale('gsr')
plt.title('gsr')
plt.grid(True)


# emgi
plt.subplot(222)
plt.plot(x, test_emgi)
#plt.yscale('emgi')
plt.title('emgi')
plt.grid(True)

plt.show()

test_gsr = np.array(test_gsr)
test_emgi = np.array(test_emgi)
print(np.std(test_gsr), np.std(test_emgi))

ser.close()

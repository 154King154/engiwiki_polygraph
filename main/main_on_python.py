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

TEST_EPOCH = 2
TIME_TEST = 12
MAX_GAMMA_GSR = 0.95
MAX_GAMMA_EMGI = 0.85

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

def calibration(stdGsr, stdEmgi):
    global normalStd_gsr, normalStd_emgi, maxStd_gsr, maxStd_emgi
    if (stdGsr > maxStd_gsr):
        maxStd_gsr = stdGsr
    elif (stdGsr < normalStd_gsr):
        normalStd_gsr = stdGsr
    if (stdEmgi > maxStd_emgi):
        maxStd_emgi = stdEmgi
    elif (stdEmgi < normalStd_gsr):
        normalStd_emgi = stdEmgi

def prediction(stdGsr, stdEmgi):
    global normalStd_gsr, normalStd_emgi, maxStd_gsr, maxStd_emgi
    global MAX_GAMMA_GSR, MAX_GAMMA_EMGI

    scoreGsr = stdGsr * 100 / maxStd_gsr
    scoreEmgi = stdEmgi * 100 / maxStd_emgi
    gammaGsr = maxStd_gsr / normalStd_gsr
    gammaEmgi = maxStd_emgi / normalStd_emgi

    meanScore_gsr = (100 - normalStd_gsr * 100 / maxStd_gsr) / 2
    meanScore_emgi = (100 - normalStd_emgi * 100 / maxStd_emgi) / 2
    lightScore_gsr = (meanScore_gsr - normalStd_gsr * 100 / maxStd_gsr) / 2
    lightScore_emgi = (meanScore_emgi - normalStd_emgi * 100 / maxStd_emgi) / 2

    if (gammaGsr > MAX_GAMMA_GSR) and (gammaEmgi < MAX_GAMMA_EMGI): #смотрим насколько сильно испытуемый реагирует по тем или иным показателям
        if (scoreEmgi > meanScore_emgi):
            return "false"
        elif (scoreEmgi > lightScore_emgi):
            return "maybe false"
        else:
            return "true"
    elif (gammaGsr < MAX_GAMMA_GSR) and (gammaEmgi > MAX_GAMMA_EMGI):
        if (scoreGsr > meanScore_gsr):
            return "false"
        elif (scoreGsr > lightScore_gsr):
            return "maybe false"
        else:
            return "true"
    else:
        if (scoreGsr > meanScore_gsr) and (scoreEmgi > meanScore_emgi):
            return "false"
        elif (scoreGsr > lightScore_gsr) and (scoreEmgi > meanScore_emgi):
            return "false"
        elif (scoreGsr > meanScore_gsr) and (scoreEmgi > lightScore_emgi):
            return "false"
        elif (scoreGsr > lightScore_gsr) and (scoreEmgi > lightScore_emgi):
            return "maybe false"
        else:
            return "true"

#ser.open()
time_i=time.time()
time_j=time.time()
test_gsr = []
test_emgi = []
flag = False
while (not flag):
    try:
        print("Test---------")
        while (abs(time_i-time_j)<TIME_TEST):
            byte_str = ser.readline()
            if (byte_str[0]==123) and (byte_str[1]==34):
                convertData(byte_str)
                print(abs(time_i-time_j))
                test_gsr.append(gsr)
                test_emgi.append(emgi)
            time_j = time.time()
        flag = True
    except:
        print("Reconecting----------")
test_gsr = np.array(test_gsr)
test_emgi = np.array(test_emgi)

maxStd_gsr = normalStd_gsr = np.std(test_gsr)
maxStd_emgi = normalStd_emgi = np.std(test_emgi)


print("Start calibration---------------")
for _ in range(TEST_EPOCH):
    flag = False
    flag_console = 0
    print("Write 'continue' to start-------")
    while (flag_console != 'continue'):
        flag_console = input()
    que_gsr = []
    que_emgi = []
    time_i = time.time()
    time_j = time.time()
    while (not flag):
        try:
            while (abs(time_i - time_j) < TIME_TEST):
                byte_str = ser.readline()
                if (byte_str[0] == 123) and (byte_str[1] == 34):
                    convertData(byte_str)
                    print(abs(time_i - time_j))
                    que_gsr.append(gsr)
                    que_emgi.append(emgi)
                time_j = time.time()
            flag = True
        except:
            print("Reconecting----------")
            flag = False
    que_gsr = np.array(que_gsr)
    que_emgi = np.array(que_emgi)
    stdGsr = np.std(que_gsr)
    stdEmgi = np.std(que_emgi)
    calibration(stdGsr, stdEmgi)

print("Start trial------------")
while (True):
    flag = False
    flag_console = 0
    print("Write 'continue' to start or 'exit' to finish-------")
    while (flag_console != 1) and (flag_console != 2):
        flag_console = int(input())
    if (flag_console == 1):
        time_i = time.time()
        time_j = time.time()
        que_gsr = []
        que_emgi = []
        while (not flag):
            try:
                while (abs(time_i - time_j) < TIME_TEST):
                    byte_str = ser.readline()
                    if (byte_str[0] == 123) and (byte_str[1] == 34):
                        convertData(byte_str)
                        print(abs(time_i - time_j))
                        que_gsr.append(gsr)
                        que_emgi.append(emgi)
                    time_j = time.time()
                flag = True
            except:
                print("Reconecting----------")
        que_gsr = np.array(que_gsr)
        que_emgi = np.array(que_emgi)
        stdGsr = np.std(np.array(que_gsr))
        stdEmgi = np.std(np.array(que_emgi))
        print("Answer polygraph:", prediction(stdGsr, stdEmgi))
        calibration(stdGsr, stdEmgi)

        x = list(range(len(test_gsr)))
        x_q = list(range(len(que_gsr)))

        plt.figure(1)

        # gsr normal
        plt.subplot(221)
        plt.plot(x, test_gsr)
        # plt.yscale('gsr')
        plt.title('gsr normal')
        plt.grid(True)

        # emgi normal
        plt.subplot(222)
        plt.plot(x, test_emgi)
        # plt.yscale('emgi')
        plt.title('emgi normal')
        plt.grid(True)

        # gsr test
        plt.subplot(223)
        plt.plot(x_q, que_gsr)
        # plt.yscale('gsr')
        plt.title('gsr current')
        plt.grid(True)

        # emgi test
        plt.subplot(224)
        plt.plot(x_q, que_emgi)
        # plt.yscale('emgi')
        plt.title('emgi current')
        plt.grid(True)

        plt.show()
    else:
        print("Finish---------")
        break

ser.close()

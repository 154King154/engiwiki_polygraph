// Взаимодействие с Raspberry - чтение данных и "парсинг"
// Arduino pin D10 соединить с TxD на USBtoUART переходнике (подключен к Raspberry Pi) 
// Arduino pin Gnd соединить с Gnd на USBtoUART переходнике (подключен к Raspberry Pi) 
// 
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

SoftwareSerial piSerial(10, 11); // RX, TX
String receivedData = "";
float alpha, beta, emgint, gsr, temp, rr; 

// Данные от Raspberry приходят в виде строки в формате JSON 
// В одной строке содержатся данные по всем каналам (пакет данных).
// Частота поступления данных: 10 пакетов в секунду
// Ниже показан пример строки (пакета данных):  
// {"alpha":"12.15","beta":"5.10","emgi":"55.59","gsr":"6.037","temp":"30.239","rr":"874.8"}
// Функция readData принимает эту строку и  "парсит" - разбирает её и присвает соответствущие
// значения переменным alpha, beta, emgint, gsr, temp, rr


const int TIME = 3600;


float dispEmgint_normal = 0;
float dispGsr_normal = 0;

float dispEmgint_hard = 0;
float dispGsr_hard = 0;


float dispEmgint_trial = 0;
float dispGsr_trial = 0;

void readData() {
  //bool res = false;
  if (piSerial.available()) {
    char ch = char(piSerial.read());
    while (ch != '\n') {
      receivedData += ch;
      ch = char(piSerial.read());
    }
    if (ch == '\n') {
      DynamicJsonDocument doc;
      DeserializationError error = deserializeJson(doc, receivedData);
      if (!error) {
        JsonObject data = doc.as<JsonObject>();
        alpha  = data["alpha"];
        beta   = data["beta"];
        emgint = data["emgi"];
        gsr    = data["gsr"];
        temp   = data["temp"];
        rr     = data["rr"];
        //res = true;
      }
      receivedData = "";
    }   
  }
  
  //return res;
}




float calculateDisp(float base[TIME], float mean) {
    float disp = 0;
    for (int i = 0; i<TIME; i++) {
        disp += (base[i] - mean) * (base[i] - mean);
      }
    return sqrt(disp / TIME);
  }

  



void setup() {
  Serial.begin(57600);
  piSerial.begin(57600);
}


void loop() {
  float testEmgint[TIME];
  float testGsr[TIME];
  float meanEmgint_normal = 0;
  float meanGsr_normal = 0;
  float meanEmgint_hard = 0;
  float meanGsr_hard = 0;
  float meanEmgint_trial = 0;
  float meanGsr_trial = 0;


//тестовый сбор в нормальном состоянии
  Serial.print("Test"); Serial.print("\n");
  for (int i = 10; i>0; i--) {
    Serial.print(i); Serial.print("\n");
    delay(50);
    }
  int i = 0;
  while(i<TIME) {
            Serial.print("Ya_uzhe_zaebalsa"); Serial.print("\n");
            readData();
            testEmgint[i] = emgint;
            testGsr[i] = gsr;
            meanEmgint_normal += emgint;
            meanGsr_normal += gsr;
          
            Serial.print(gsr); Serial.print("\n");
            i++;
            //delay(100);
          }
    
  meanEmgint_normal = meanEmgint_normal / TIME;
  meanGsr_normal = meanGsr_normal / TIME;
  dispEmgint_normal = calculateDisp(testEmgint, meanEmgint_normal);
  dispGsr_normal = calculateDisp(testGsr, meanGsr_normal);



 
  //тестовый сбор в напряженном состоянии
  Serial.print("[КАЛИБРОВОЧНЫЙ ВОПРОС] Задайте вопрос и отправьте в консоль '1'"); Serial.print("\n");
  char con = '0';
  while (con != '1') {
      if (Serial.available()) {
       con = Serial.read();
      }
    }
  Serial.print("[КАЛИБРОВОЧНЫЙ ВОПРОС] Калибровка напряженного состояния"); Serial.print("\n");
  i = 0;
 while (i<TIME) {
            readData();
            testEmgint[i] = emgint;
            testGsr[i] = gsr;
            meanEmgint_hard += emgint;
            meanGsr_hard += gsr;
            Serial.print("Осталось: "); Serial.print(i); Serial.print("\n");
        
            i++;
            }
    
  meanEmgint_hard = meanEmgint_hard / TIME;
  meanGsr_hard = meanGsr_hard / TIME;
  dispEmgint_hard = calculateDisp(testEmgint, meanEmgint_hard);
  dispGsr_hard = calculateDisp(testGsr, meanGsr_hard);

  Serial.print("Калибровка завершена"); Serial.print("\n");






  while (true) {
  
  Serial.print("[НОВЫЙ ВОПРОС] Отправьте в консоль '1' и задайте вопрос испытуемому"); Serial.print("\n");
  char con = '0';
  while (con != '1') {
      if (Serial.available()) {
       con = Serial.read();
      }
    }
  Serial.print("[ПРОВЕРКА] Сбор данных для определения состояния испытуемого"); Serial.print("\n");
  i = 0;
  while (i < TIME) {
            readData();
            testEmgint[i] = emgint;
            testGsr[i] = gsr;
            meanEmgint_trial += emgint;
            meanGsr_trial += gsr;
            Serial.print("Осталось: "); Serial.print(i); Serial.print("\n");
            i++;
          }
    
  Serial.print("[ПРОВЕРКА] Подсчет среднего значения и дисперсии"); Serial.print("\n");
  meanEmgint_trial = meanEmgint_trial / TIME;
  meanGsr_trial = meanGsr_trial / TIME;
  dispEmgint_trial = calculateDisp(testEmgint, meanEmgint_trial);
  dispGsr_trial = calculateDisp(testGsr, meanGsr_trial);

  float trialSum = (dispEmgint_normal/dispEmgint_hard) * dispEmgint_trial + (dispGsr_normal/dispGsr_hard) * dispGsr_trial;
  float normalSum = dispEmgint_normal + dispGsr_normal;
  float hardSum = dispEmgint_hard + dispGsr_hard;
  Serial.print(normalSum); Serial.print("\n");
  Serial.print(hardSum); Serial.print("\n");
  if (trialSum > hardSum){
      Serial.print("Испытуемый лжет "); Serial.print(trialSum); Serial.print("\n");
    }
  else if (trialSum > normalSum) {
      Serial.print("Испытуемый скорее всего лжет "); Serial.print(trialSum); Serial.print("\n");
    }
  else {
      Serial.print("Испытуемый говорит правду "); Serial.print(trialSum); Serial.print("\n");
    }
  }
}

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

bool readData() {
  bool res = false;
  if (piSerial.available()) {
    char ch = char(piSerial.read());
    receivedData += ch;
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
        res = true;
      }
      receivedData = "";
    }   
  }
  
  return res;
}

double calculateDisp(double base, double mean, int TIME) {
    double disp = 0;
    for (int i = 0; i<TIME; i++) {
        disp += sqr(base[i] - mean);
      }
    return disp / TIME;
  }



void setup() {
  // 
  Serial.begin(57600);
  piSerial.begin(57600);
  int TIME = 3600;
  double testEmgint[TIME];
  double testGsr[TIME];
  double meanEmgint_normal = 0;
  double meanGsr_normal = 0;
  double dispEmgint_normal = 0;
  double dispGsr_normal = 0;

  //тестовый сбор в нормальном состоянии
  Serial.print('Начало тестового сбора данных в спокойном состоянии, приготовьтесь'); Serial.print("\n");
  for (int i = 10; i>0; i--) {
    Serial.print(i); Serial.print("\n");
    }

  for (int i = TIME-1; i>=0; i--) {
      if (readData()) {
          testEmgint[i] = emgint;
          testGsr[i] = gsr;
          meanEmgint += emgint;
          meanGsr += gsr;
          Serial.print('Осталось: '); Serial.print(i); Serial.print("\n");
        }
      meanEmgint = meanEmgint / TIME;
      meanGsr = meanGsr / TIME;
    }
  dispEmgint_normal = calculateDisp(testEmgint, meanEmgint, TIME);
  dispGsr_normal = calculateDisp(testGsr, meanGsr, TIME);

  
}


void loop() {
  // принимаем данные от Raspberry Pi и выводим их на монитор/плоттер
  if (readData()) {
    if ((emgint>150)or(gsr>40)) {
      Serial.print("Ложь");
      }
    else {
      Serial.print("Правда");
     }
    // раскомментируйте строки, чтобы вывести нужные каналы на монитор/плоттер
    //Serial.print(alpha);  Serial.print(" "); // альфа-ритм
    //Serial.print(beta);   Serial.print(" "); // бета-ритм
    Serial.print(emgint); Serial.print(" "); // ЭМГ интегральная (мышечное напряжение)
    Serial.print(gsr);    Serial.print(" "); // КГР (кожно-гальваническая реакция)  
    //Serial.print(temp); Serial.print(" "); // Температура  
    //Serial.print(rr); Serial.print(" ");  // длительность R-R-интервала в миллисекундах (пульс = 60000.0/rr)
    Serial.print("\n");
  }  
}

#include <SoftwareSerial.h>

SoftwareSerial piSerial(10, 11); // RX, TX
int a;
int led_red = 8;
int led_blue = 7;
int led_green = 4; 

void setup() {
  Serial.begin(57600);
  piSerial.begin(57600);
  pinMode(led_red, OUTPUT);
  pinMode(led_blue, OUTPUT);
  pinMode(led_green, OUTPUT);
}

void loop() { // run over and over
  if (piSerial.available()) {
    Serial.write(piSerial.read());
  }
  a = Serial.read();
  if (a == '4') {
      digitalWrite(led_green, HIGH);
      digitalWrite(led_red, LOW);
      digitalWrite(led_blue, LOW);
     }
  else if (a == '1') {
      digitalWrite(led_green, LOW);
      digitalWrite(led_red, LOW);
      digitalWrite(led_blue, HIGH);
     }
  else if (a == '2') {
      digitalWrite(led_green, LOW);
      digitalWrite(led_red, HIGH);
      digitalWrite(led_blue, LOW);
    }
  else if (a == '8') {
      digitalWrite(led_green, LOW);
      digitalWrite(led_red, LOW);
      digitalWrite(led_blue, LOW);
    }
    
    
  
}

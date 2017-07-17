#include <ESP8266WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const char* ssid = "WIFI_SID";
const char* password = "WIFI_PASSWORD";

const char* servidor = "HOST";
String updateWS = "WEB_SERVICE"; //indicar el webservice que se utilice




WiFiClient client;

int measureInterval = 1000;

volatile int flowPulse;

int pinCaudal = D3;
int pinControl = D6;

// YF-S201
float factorK = 8; //A modo de ejemplo
float volume = 0;
long t0 = 0;

String ipLocal = "";

void setup() {
  pinMode(pinControl,  OUTPUT) ;
  digitalWrite(pinControl, LOW);
  
  lcd.begin();
  lcd.backlight();
  lcd.print("conectando");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }

  pinMode(pinCaudal, INPUT);
  attachInterrupt(pinCaudal, rpm, RISING);

  reportarDatos();
  
  digitalWrite(pinControl, HIGH);
}

unsigned long currentTime;
int contador = 0;
float flow_Lmin;

float frequency;
void loop() {

  currentTime = millis();
  frequency = GetFrequency();

  // calcular caudal L/min
  flow_Lmin = frequency / factorK;
  SumVolume(flow_Lmin);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(currentTime / measureInterval);
  lcd.print(" ");
  lcd.print(flow_Lmin);
  lcd.print(" lt/min ");
  lcd.setCursor(0, 1);
  lcd.print(volume, 1);
  lcd.print(" lt ");

  if (flowPulse > 0) {
    reportarDatos();
  }
}

float GetFrequency() {
  flowPulse = 0;

  interrupts();
  delay(measureInterval);
  noInterrupts();

  return (float)flowPulse * 1000 / measureInterval;
}

void SumVolume(float dV) {
  volume += dV / 60 * (millis() - t0) / 1000.0;
  t0 = millis();
}

void reportarDatos() {
  WiFiClient client;
  if (client.connect(servidor, 80)) {

    String postStr = String(updateWS);
    postStr += "&lectura=";
    postStr += String(flow_Lmin);
    postStr += "\r\n\r\n";

    client.print(postStr);
    client.print("\r\n");
  }

  client.stop();
  delay(100);

}


void rpm() {
  flowPulse++;
}




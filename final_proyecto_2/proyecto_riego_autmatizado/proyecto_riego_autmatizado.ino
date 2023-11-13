#include <Ubidots.h>

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <time.h>
#include "secrets.h"
#include <OneWire.h>                
#include <DallasTemperature.h>

//Pines de los Sensores
const int TEMPER_PIN=4;
const int OPTO01_PIN=5;
const int HUMEDA_PIN=A0;

//Se establece el pin 2 como bus OneWire (ver mapa GPIO)
//Se declara una variable para nuestro sensor
OneWire ourWire(TEMPER_PIN);
DallasTemperature sensors(&ourWire);

Ubidots ubidots(UBIDOTS_TOKEN, UBI_HTTP);

int sensorhumeValue, valvula_riego;
float humedadValue, temper;
WiFiClientSecure net;
time_t now;
time_t nowish = 1510592825;

void NTPConnect()
{
  //Fijando la zona horaria para tomar hora correcta
  Serial.print("Setting time using SNTP");
  configTime(TIME_ZONE * 3600, 0 * 3600, "pool.ntp.org", "time.nist.gov");
  now = time(nullptr);
  while (now < nowish)
  {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("done!");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}

void WiFiConnect()
{
  delay(3000);
  //Se establece la conexión al WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD); 
  Serial.println(String("Attempting to connect to SSID: ") + String(WIFI_SSID));

  //Proceso de verificación de conectividad 
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(1000);
  }
 
  NTPConnect();
}

void setup() {
  Serial.begin(115200);
  WiFiConnect();
  sensors.begin();
  pinMode(OPTO01_PIN, OUTPUT);
  pinMode(HUMEDA_PIN, INPUT);
  //ubidots.setDebug(true);
}

void loop() {
  //Se obtiene el valor de Temperatura
  sensors.requestTemperatures();  //Se envía el comando para leer la temperatura
  temper = sensors.getTempCByIndex(0); //Se obtiene la temperatura en ºC
  Serial.print("Temperatura: ");
  Serial.print(temper);
  Serial.println(" C");

  //Se obtiene el valor de Humedad
  sensorhumeValue = analogRead(HUMEDA_PIN); //Se obtiene el valor analogico
  humedadValue = 100.0-((sensorhumeValue/1024.0)*100.0); //Se calcula el % de humedad
  Serial.print("Indice de Humedad: ");
  Serial.print(humedadValue);
  Serial.println(" %");

  if (humedadValue<60){ //El nivel de humedad para el tomate debe ser 60% - 70%
    if(temper < 28) { //Se considera que la temperatura en las noches es menor a 28º
      digitalWrite(OPTO01_PIN,HIGH);
      Serial.println("El riego se ha habiltado");
      valvula_riego = 1;
    } else {
      digitalWrite(OPTO01_PIN,LOW);
      Serial.println("El riego se ha deshabilitado");
      valvula_riego = 0;
    }
  } else {
    digitalWrite(OPTO01_PIN,LOW);
    Serial.println("El riego se ha deshabilitado");
    valvula_riego = 0;
  }

  ubidots.add("Temperatura Tº", temper);
  ubidots.add("Humedad %", humedadValue);
  ubidots.add("Válvula Riego", valvula_riego);

  bool bufferSent = false;
  bufferSent = ubidots.send();
  if(bufferSent){
    Serial.println("Los valores fueron enviados satisfactoriamente!!");
  }

  delay(1000);

}

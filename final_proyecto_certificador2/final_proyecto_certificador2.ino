#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include "secrets.h"
#include <OneWire.h>                
#include <DallasTemperature.h>
#include <IRremote.h>
#include <Arduino.h>


const int RECV_PIN=4;
const int IR_SEND_PIN=5;
const int ledPin = D0;
//Se establece el pin 4 como bus OneWire
//Se declara una variable para nuestro sensor
OneWire ourWire(RECV_PIN);
DallasTemperature sensors(&ourWire);
float t;
unsigned int wifi_attempts = 0, thing_attempts = 0, aws_attempts = 0, snp_attempts = 0;
unsigned long lastMillis = 0, previousMillis = 0;
const long interval = 5000;
 
//Se establece el topico publicador
#define AWS_IOT_SUBSCRIBE_TOPIC "idat_domotica/pub"
#define AWS_IOT_PUBLISH_TOPIC "idat_domotica/pub"
WiFiClientSecure net;
 
BearSSL::X509List cert(cacert);
BearSSL::X509List client_crt(client_cert);
BearSSL::PrivateKey key(privkey);
 
PubSubClient client(net);
 
time_t now;
String tokens[10]; // Arreglo para almacenar los tokens
String mi_string="", date_start="", time_start="", date_end="", time_end="", summary="", location="", tipo="", medio="", contenido="";
 
void NTPConnect(void)
{
  delay(1000);
  //Fijando la zona horaria para tomar hora correcta
  Serial.print("Setting time using SNTP");
  configTime(TIME_ZONE * 3600, 0 * 3600, "pool.ntp.org", "time.nist.gov");
  now = time(nullptr);
  Serial.println("done!");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}
  
void messageReceived(char* topic, byte* payload, unsigned int length)
{
  Serial.print("Message received on topic: ");
  Serial.println(topic);
  for (int i = 0; i < length; i++)
  {
    mi_string=mi_string+(char)payload[i];
  }
  
  int start_pos = mi_string.indexOf("\"message\":") + String("\"message\":").length() + 2;
  int end_pos = mi_string.lastIndexOf('"');
  String message = mi_string.substring(start_pos, end_pos);
  Serial.println();
  Serial.println(message);
  mi_string = "";
  start_pos = 0;
  end_pos = message.indexOf(',');
  int i = 0;
  while (end_pos != -1 && i < 10) {
    tokens[i++] = message.substring(start_pos, end_pos);
    start_pos = end_pos + 1;
    end_pos = message.indexOf(',', start_pos);
  }
  
  date_start=tokens[0];
  time_start=tokens[1]; 
  date_end=tokens[2];
  time_end=tokens[3]; 
  summary=tokens[4];
  location=tokens[5];
  tipo=tokens[6];
  medio=tokens[7];
  contenido=tokens[0];
}

void connectWiFi()
{
  delay(1000);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD); 
  Serial.println(String("Attempting to connect to SSID: ") + String(WIFI_SSID));
  while (WiFi.status() != WL_CONNECTED and wifi_attempts < 5)
  {
    Serial.print(".");
    delay(1000);
    wifi_attempts++;
  }
}

void connectAWS()
{ 
  net.setTrustAnchors(&cert);
  net.setClientRSACert(&client_crt, &key);
  client.setServer(MQTT_HOST, 8883);
  client.setCallback(messageReceived);
  Serial.println("Connecting to AWS IOT");
 
  while (!client.connect(THINGNAME) and thing_attempts < 5)
  {
    Serial.print(".");
    delay(1000);
    thing_attempts++;
  }
 
  if (!client.connected()) {
    Serial.println("AWS IoT Timeout!");
    return;
  }
 
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
  Serial.println("AWS IoT Connected!");
}

void publishMessage()
{
  StaticJsonDocument<200> doc;
  doc["time"] = millis();
  doc["temperature"] = 26;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client
 
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}

time_t stringToTime(const char* dateTimeString) {
    // Variables para almacenar los componentes de fecha y hora
    int year, month, day, hour, minute, second;
    // Analizar la cadena para extraer los componentes de fecha y hora
    sscanf(dateTimeString, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);
    // Crear un objeto tm con los componentes de fecha y hora
    struct tm tm;
    tm.tm_year = year - 1900;  // Año desde 1900
    tm.tm_mon = month - 1;     // Mes (0-11)
    tm.tm_mday = day;          // Día del mes (1-31)
    tm.tm_hour = hour;         // Hora (0-23)
    tm.tm_min = minute;        // Minuto (0-59)
    tm.tm_sec = second;        // Segundo (0-59)
    // Convertir el objeto tm en segundos desde el 1 de enero de 1970
    return mktime(&tm);
}
 
void setup()
{
  Serial.begin(9600);
  //IrReceiver.begin(RECV_PIN, ENABLE_LED_FEEDBACK);
  //IrSender.begin(IR_SEND_PIN);
  connectWiFi();
  NTPConnect();
  connectAWS();
  //dht.begin();
  pinMode(ledPin, OUTPUT);
}
  
void loop()
{
  now = time(nullptr);

  if (!date_start.equals("") and !time_start.equals("") and !date_end.equals("") and !time_end.equals("")){
    String inicio = date_start+" "+time_start;
    String fin = date_end+" "+time_end;
    char charArray_inicio[inicio.length() + 1];
    inicio.toCharArray(charArray_inicio, inicio.length() + 1);
    char charArray_fin[fin.length() + 1];
    fin.toCharArray(charArray_fin, fin.length() + 1);
    time_t timeSinceEpoch_inicio = stringToTime(charArray_inicio);
    time_t timeSinceEpoch_fin = stringToTime(charArray_fin);
    //Serial.println(timeSinceEpoch_inicio);
    //Serial.println(timeSinceEpoch_fin);
    //Serial.println(now);
    if (timeSinceEpoch_inicio < now and now < timeSinceEpoch_fin){
      digitalWrite(ledPin, HIGH);
    }

    if (now > timeSinceEpoch_fin){
      date_start="";
      time_start="";
      date_end="";
      time_end="";
      summary="";
      location="";
      tipo="";
      medio="";
      contenido="";
      digitalWrite(ledPin, LOW);
    }
  }

  client.loop();
}
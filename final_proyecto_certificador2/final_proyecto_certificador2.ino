#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include "secrets.h"
#include <OneWire.h>                
#include <DallasTemperature.h>
#include <IRremote.h>

const int RECV_PIN=4;
const int IR_SEND_PIN=5;

//Se establece el pin 4 como bus OneWire
//Se declara una variable para nuestro sensor
OneWire ourWire(RECV_PIN);
DallasTemperature sensors(&ourWire);

float t;
unsigned int wifi_attempts = 0, thing_attempts = 0, aws_attempts = 0;
unsigned long lastMillis = 0;
unsigned long previousMillis = 0;
const long interval = 5000;
 
//Se establece el topico publicador
#define AWS_IOT_PUBLISH_TOPIC   "idat_domotica/pub"
//Se establece el topico suscriptor
#define AWS_IOT_SUBSCRIBE_TOPIC "idat_domotica/pub"

WiFiClientSecure net;
 
BearSSL::X509List cert(cacert);
BearSSL::X509List client_crt(client_cert);
BearSSL::PrivateKey key(privkey);
 
PubSubClient client(net);
 
time_t now;
time_t nowish = 1510592825;
 
void NTPConnect(void)
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
  
void messageReceived(char* topic, byte* payload, unsigned int length)
{
  Serial.print("Message received on topic: ");
  Serial.println(topic);
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void connectWiFi()
{
  delay(3000);
  //Se establece la conexión al WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD); 
  Serial.println(String("Attempting to connect to SSID: ") + String(WIFI_SSID));

  //Proceso de verificación de conectividad 
  while (WiFi.status() != WL_CONNECTED and wifi_attempts < 3)
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
 
  while (!client.connect(THINGNAME) and thing_attempts < 3)
  {
    Serial.print(".");
    delay(1000);
    thing_attempts++;
  }
 
  if (!client.connected()) {
    Serial.println("AWS IoT Timeout!");
    return;
  }
 
  // Subscribe to a topic
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
 
void setup()
{
  Serial.begin(9600);
  //IrReceiver.begin(RECV_PIN, ENABLE_LED_FEEDBACK);
  //IrSender.begin(IR_SEND_PIN);
  connectWiFi();
  NTPConnect();
  connectAWS();
  //dht.begin();
}
  
void loop()
{

  now = time(nullptr);
  client.loop();

}
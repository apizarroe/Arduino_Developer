
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <OneWire.h>                
#include <DallasTemperature.h>

// Credenciales para WiFi
const char* ssid = "RedmiAPE";
const char* password = "asdf1234";

// Configura el cliente UDP
WiFiUDP udp;
NTPClient timeClient(udp, "pool.ntp.org", 0, 60000); // Servidor NTP, zona horaria, intervalo de actualización

// Credenciales para broker MQTT (colocar NULL si no es requerido)
const char* MQTT_username = NULL; 
const char* MQTT_password = NULL; 

// Direcccion IP del Servidor broker MQTT
const char* mqtt_server = "192.168.123.231";

// Initializes the espClient. You should change the espClient name if you have multiple ESPs running in your home automation system
WiFiClient espClient;
PubSubClient client(espClient);

// Lamparar LED
const int lamp1A = D0;
const int lamp1B = D1;
const int lamp1C = D2;
// Ventilador
const int ventiSpeed = D3;
// Sensor de TEMPERATURA
const int temper = D4;

// Initialize TEMPERATURE sensor.
OneWire ourWire(temper);
DallasTemperature sensors(&ourWire);

// Auxiliar variables
long now = millis();
long lastMeasure = 0, lastMeasuretemp = 0, lastMeasureled = 0, lastMeasuretime=0;
unsigned long epochTime = 0, salahorainicio = 0, salahorafin = 0;
int flg_ventil_autom = 0;
float temperatureC = 0.0;
const char* modoOperativo = "";
const char* modoTemp = "";
String modoLed = "";
bool ledRGBsalaP1=LOW, ledRGBsalaP2=LOW, ledRGBsalaP3=LOW;

// Función para conectar a la red WiFi
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
}

void MotorEncendidoFull()
{
  analogWrite(ventiSpeed, 255);
  //Serial.println("Alto");
}

void MotorEncendidoMedio()
{
  analogWrite(ventiSpeed, 180);
  //Serial.println("Medio");
}

void MotorApagado()
{
  analogWrite(ventiSpeed, 0);
  //Serial.println("Apagado");
}

// This function is executed when some device publishes a message to a topic that your ESP8266 is subscribed to
// Change the function below to add logic to your program, so when a device publishes a message to a topic that 
// your ESP8266 is subscribed you can actually do something
void callback(String topic, byte* message, unsigned int length) {
  String messageTemp;

  // If a message is received on the topic room/lamp, you check if the message is either on or off. Turns the lamp GPIO according to the message
  if(topic=="livingroom/lamp") {
    for (int i = 0; i < length; i++) {
      Serial.print((char)message[i]);
      messageTemp += (char)message[i];
    }
    Serial.println();
    Serial.print("Message arrived on topic: ");
    Serial.print(topic);
    Serial.print(". Message: ");
    Serial.print("Cambiando lampara de habitación a ");
    if(messageTemp == "on"){
      digitalWrite(lamp1A, HIGH);
      digitalWrite(lamp1B, HIGH);
      digitalWrite(lamp1C, HIGH);
      modoLed = messageTemp;
      Serial.println("Encendido");
    } else if (messageTemp == "cine") {
      digitalWrite(lamp1A, LOW);
      digitalWrite(lamp1B, HIGH);
      digitalWrite(lamp1C, HIGH);
      modoLed = messageTemp;
      Serial.println("Cine");
    } else if (messageTemp == "fiesta") {
      digitalWrite(lamp1A, HIGH);
      digitalWrite(lamp1B, LOW);
      digitalWrite(lamp1C, HIGH);
      modoLed = messageTemp;
      Serial.println("Fiesta");
    } else if(messageTemp == "off") {
      digitalWrite(lamp1A, LOW);
      digitalWrite(lamp1B, LOW);
      digitalWrite(lamp1C, LOW);
      modoLed = messageTemp;
      Serial.println("Apagado");
    }
  } else if(topic=="livingroom/aireauto"){
    for (int i = 0; i < length; i++) {
      Serial.print((char)message[i]);
      messageTemp += (char)message[i];
    }
    Serial.println();
    Serial.print("Message arrived on topic: ");
    Serial.print(topic);
    Serial.print(". Message: ");
    Serial.print("Cambiando funcionamiento de ventilador a ");
    if(messageTemp == "on"){
      flg_ventil_autom = 1;
      if (temperatureC <= 20){
        modoOperativo = "Apagado";
        client.publish("livingroom/aire", modoOperativo);
        MotorApagado();
      } else if (temperatureC >20 and temperatureC <= 25) {
        modoOperativo = "Medio";
        client.publish("livingroom/aire", modoOperativo);
        MotorEncendidoMedio();
      } else {
        modoOperativo = "Alto";
        client.publish("livingroom/aire", modoOperativo);
        MotorEncendidoFull();
      }
      Serial.println("Automatico");
    } else if (messageTemp == "off") {
      flg_ventil_autom = 0;
      client.publish("livingroom/aire", "Apagado");
      MotorApagado();
      Serial.println("Manual");
    } 
  } else if(topic=="livingroom/aire"){
    if(flg_ventil_autom == 0){
      for (int i = 0; i < length; i++) {
        Serial.print((char)message[i]);
        messageTemp += (char)message[i];
      }
      Serial.println();
      Serial.print("Message arrived on topic: ");
      Serial.print(topic);
      Serial.print(". Message: ");
      Serial.print("Cambiando potencia de ventilador a ");
      if(messageTemp == "Apagado"){
        MotorApagado();
        Serial.println("Apagado");
      } else if (messageTemp == "Medio") {
        MotorEncendidoMedio();
        Serial.println("Potencia Media");
      } else if (messageTemp == "Alto") {
        MotorEncendidoFull();
        Serial.println("Potencia Completa");
      }
    }
  } else if(topic=="livingroom/salahorainicio"){
    for (int i = 0; i < length; i++) {
      messageTemp += (char)message[i];
    }
    salahorainicio = messageTemp.toInt();
  } else if(topic=="livingroom/salahorafin"){
    for (int i = 0; i < length; i++) {
      messageTemp += (char)message[i];
    }
    salahorafin = messageTemp.toInt();
  }
}

// This functions reconnects your ESP8266 to your MQTT broker
// Change the function below if you want to subscribe to more topics with your ESP8266 
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    /*              
     YOU MIGHT NEED TO CHANGE THIS LINE, IF YOU'RE HAVING PROBLEMS WITH MQTT MULTIPLE CONNECTIONS
     To change the ESP device ID, you will have to give a new name to the ESP8266.
    */
    if (client.connect("ESP8266Sala", MQTT_username, MQTT_password)) {
      Serial.println("connected");  
      // Subscribe or resubscribe to a topic. You can subscribe to more topics
      client.subscribe("livingroom/lamp");
      client.subscribe("livingroom/aireauto");
      client.subscribe("livingroom/aire");
      client.subscribe("livingroom/salahorainicio");
      client.subscribe("livingroom/salahorafin");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state()); 
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

// The setup function sets your ESP GPIOs to Outputs, starts the serial communication at a baud rate of 9600
// Sets your mqtt broker and sets the callback function, what receives messages and actually controls the LEDs
void setup() {
  pinMode(lamp1A, OUTPUT);
  pinMode(lamp1B, OUTPUT);
  pinMode(lamp1C, OUTPUT);
  Serial.begin(115200);
  setup_wifi();

  timeClient.begin();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

// For this project, you don't need to change anything in the loop function. Basically it ensures that you ESP is connected to your broker
void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  //if(!client.loop())
    //client.connect("ESP8266Client");

  // Se envía el comando para leer la temperatura
  sensors.requestTemperatures();
  temperatureC= sensors.getTempCByIndex(0);

  now = millis();
  // Publishes new temperature and humidity every second 

  if (now - lastMeasuretime > 5000) {
    lastMeasuretime = now;
    timeClient.update(); // Actualiza el tiempo
    epochTime = timeClient.getEpochTime(); // Obtiene la hora en formato epoch

    if(salahorafin!=0){
      if (epochTime>salahorafin){
        client.publish("livingroom/lamp", "off");
        salahorafin = 0;
        Serial.println("Fin de Evento!");
      }
    }
  }

  if (now - lastMeasure > 1000) {
    lastMeasure = now;

    if (flg_ventil_autom == 1){
      if (temperatureC <= 20){
        modoOperativo = "Apagado";
        if(modoTemp != modoOperativo){
          client.publish("livingroom/aire", modoOperativo);
          MotorApagado();
        }
      } else if (temperatureC >20 and temperatureC <= 25) {
        modoOperativo = "Medio";
        if(modoTemp != modoOperativo){
          client.publish("livingroom/aire", modoOperativo);
          MotorEncendidoMedio();
        }
      } else {
        modoOperativo = "Alto";
        if(modoTemp != modoOperativo){
          client.publish("livingroom/aire", modoOperativo);
          MotorEncendidoFull();
        }
      }
      modoTemp = modoOperativo;
    }   
  }

  if(now - lastMeasuretemp > 5000){
    lastMeasuretemp = now;

    // Check if any reads failed and exit early (to try again).h
    if (isnan(temperatureC)) {
      Serial.println("Failed to read from TEMPERATURE sensor!");
      return;
    }

    // Publishes Temperature values
    client.publish("livingroom/temperature", String(temperatureC).c_str(), false);

    //Serial.print("Temperature: ");
    //Serial.print(temperatureC);
    //Serial.println(" ºC");
  }

  if(now - lastMeasureled > 100){
    lastMeasureled = now;
    if(modoLed == "fiesta") {
      ledRGBsalaP1 = !ledRGBsalaP1;
      ledRGBsalaP3 = !ledRGBsalaP3;
      digitalWrite(lamp1A, ledRGBsalaP1);
      digitalWrite(lamp1B, LOW);
      digitalWrite(lamp1C, ledRGBsalaP3);
    }
  }

  
} 
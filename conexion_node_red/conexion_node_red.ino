
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>                
#include <DallasTemperature.h>

// Credenciales para WiFi
const char* ssid = "Pizarro24G";
const char* password = "admin1234";

// Credenciales para broker MQTT (colocar NULL si no es requerido)
const char* MQTT_username = NULL; 
const char* MQTT_password = NULL; 

// Direcccion IP del Servidor broker MQTT
const char* mqtt_server = "192.168.1.114";

// Initializes the espClient. You should change the espClient name if you have multiple ESPs running in your home automation system
WiFiClient espClient;
PubSubClient client(espClient);

// Lamparar LED
const int lamp = D0;
// Sensor de TEMPERATURA
const int temper = D1;
// Ventilador
const int ventiSpeed = D2;

// Initialize TEMPERATURE sensor.
OneWire ourWire(temper);
DallasTemperature sensors(&ourWire);

// Timers auxiliar variables
long now = millis();
long lastMeasure = 0;

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

// This function is executed when some device publishes a message to a topic that your ESP8266 is subscribed to
// Change the function below to add logic to your program, so when a device publishes a message to a topic that 
// your ESP8266 is subscribed you can actually do something
void callback(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // If a message is received on the topic room/lamp, you check if the message is either on or off. Turns the lamp GPIO according to the message
  if(topic=="livingroom/lamp"){
      Serial.print("Changing Room lamp to ");
      if(messageTemp == "on"){
        digitalWrite(lamp, HIGH);
        Serial.print("On");
      }
      else if(messageTemp == "off"){
        digitalWrite(lamp, LOW);
        Serial.print("Off");
      }
  }
  Serial.println();
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
    if (client.connect("ESP8266Client", MQTT_username, MQTT_password)) {
      Serial.println("connected");  
      // Subscribe or resubscribe to a topic. You can subscribe to more topics
      client.subscribe("livingroom/lamp");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
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

// The setup function sets your ESP GPIOs to Outputs, starts the serial communication at a baud rate of 9600
// Sets your mqtt broker and sets the callback function, what receives messages and actually controls the LEDs
void setup() {
  pinMode(lamp, OUTPUT);   
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

}

// For this project, you don't need to change anything in the loop function. Basically it ensures that you ESP is connected to your broker
void loop() {

  if (!client.connected()) {
    reconnect();
  }
  if(!client.loop())
    client.connect("ESP8266Client");

  now = millis();
  // Publishes new temperature and humidity every second
  if (now - lastMeasure > 1000) {
    lastMeasure = now;

    // Se envía el comando para leer la temperatura
    sensors.requestTemperatures();
    float temperatureC= sensors.getTempCByIndex(0);
	
    // Check if any reads failed and exit early (to try again).h
    if (isnan(temperatureC)) {
      Serial.println("Failed to read from TEMPERATURE sensor!");
      return;
    }

    // Publishes Temperature values
    client.publish("livingroom/temperature", String(temperatureC).c_str());

    if (temperatureC <= 20){
      client.publish("livingroom/ventila", "Apagado");
      MotorApagado();
    } else if (temperatureC >20 and temperatureC <= 25) {
      client.publish("livingroom/ventila", "Medio");
      MotorEncendidoMedio();
    } else {
      client.publish("livingroom/ventila", "Alto");
      MotorEncendidoFull();
    }

    Serial.print("Temperature: ");
    Serial.print(temperatureC);
    Serial.println(" ºC");
  }
} 
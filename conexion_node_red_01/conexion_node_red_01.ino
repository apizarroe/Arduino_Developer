
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
//#include <OneWire.h>                
//#include <DallasTemperature.h>

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
const int lamp2A = D0;
const int lamp2B = D1;
const int lamp2C = D2;
// Ventilador
const int persiaSpeed = D3;
const int persiaSentidoA = D4;
const int persiaSentidoB = D5;
// Sensor de TEMPERATURA
const int finCarreraA = D6;
const int finCarreraB = D7;

// Initialize TEMPERATURE sensor.
//OneWire ourWire(temper);
//DallasTemperature sensors(&ourWire);

// Auxiliar variables
long now = millis();
long lastMeasure = 0;
int flg_persiana = 0;
String estado_persiana = "";

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

void PersianaAbrir()
{
  digitalWrite(persiaSentidoA, LOW);
  digitalWrite(persiaSentidoB, HIGH);
  analogWrite(persiaSpeed, 170);
}

void PersianaCerrar()
{
  digitalWrite(persiaSentidoA, HIGH);
  digitalWrite(persiaSentidoB, LOW);
  analogWrite(persiaSpeed, 170);
}

void PersianaApagar()
{
  digitalWrite(persiaSentidoA, LOW);
  digitalWrite(persiaSentidoB, LOW);
  analogWrite(persiaSpeed, 0);
}

// This function is executed when some device publishes a message to a topic that your ESP8266 is subscribed to
// Change the function below to add logic to your program, so when a device publishes a message to a topic that 
// your ESP8266 is subscribed you can actually do something
void callback(String topic, byte* message, unsigned int length) {
  String messageTemp;

  //Serial.println(flg_ventil_autom);

  // If a message is received on the topic room/lamp, you check if the message is either on or off. Turns the lamp GPIO according to the message
  if(topic=="bedroom/lamp") {
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
      digitalWrite(lamp2A, HIGH);
      digitalWrite(lamp2B, HIGH);
      digitalWrite(lamp2C, HIGH);
      Serial.println("Encendido");
    } else if(messageTemp == "off") {
      digitalWrite(lamp2A, LOW);
      digitalWrite(lamp2B, LOW);
      digitalWrite(lamp2C, LOW);
      Serial.println("Apagado");
    }
  } else if(topic=="bedroom/blind"){
    for (int i = 0; i < length; i++) {
      Serial.print((char)message[i]);
      messageTemp += (char)message[i];
    }
    Serial.println();
    
    Serial.print("Message arrived on topic: ");
    Serial.print(topic);
    Serial.print(". Message: ");
    Serial.print("Persiana en estado de ");
    if(messageTemp == "on"){
      if(digitalRead(finCarreraA) == HIGH){
        Serial.println("Apertura");
        flg_persiana = 1;
        estado_persiana = "apertura";
      } else if (digitalRead(finCarreraB) == HIGH){
        Serial.println("Cierre");
        flg_persiana = 1;
        estado_persiana = "cierre";
      }
    } 
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
    if (client.connect("ESP8266Client", MQTT_username, MQTT_password)) {
      Serial.println("connected");  
      // Subscribe or resubscribe to a topic. You can subscribe to more topics
      client.subscribe("bedroom/lamp");
      client.subscribe("bedroom/blind");
      client.subscribe("bedroom/blind_operation");
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
  pinMode(lamp2A, OUTPUT);
  pinMode(lamp2B, OUTPUT);
  pinMode(lamp2C, OUTPUT);
  pinMode(persiaSpeed, OUTPUT);
  pinMode(persiaSentidoA, OUTPUT);
  pinMode(persiaSentidoB, OUTPUT);
  pinMode(finCarreraA, INPUT);
  pinMode(finCarreraB, INPUT);
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

  client.loop();

  //if(!client.loop())
    //client.connect("ESP8266Client");

  // Se envía el comando para leer la temperatura
  //sensors.requestTemperatures();
  //float temperatureC= sensors.getTempCByIndex(0);

  now = millis();
  // Publishes new temperature and humidity every second

  if (flg_persiana == 1){
    if (estado_persiana == "apertura" and digitalRead(finCarreraB) == LOW) {
      PersianaAbrir();
      client.publish("bedroom/blind_operation", "Abriendo...");
    } else if (estado_persiana == "apertura" and digitalRead(finCarreraB) == HIGH) {
      PersianaApagar();
      client.publish("bedroom/blind_operation", "Inactivo");
      flg_persiana = 0;
      estado_persiana = "";
    } else if (estado_persiana == "cierre" and digitalRead(finCarreraA) == LOW) {
      PersianaCerrar();
      client.publish("bedroom/blind_operation", "Cerrando...");
    } else if (estado_persiana == "cierre" and digitalRead(finCarreraA) == HIGH) {
      PersianaApagar();
      client.publish("bedroom/blind_operation", "Inactivo");
      flg_persiana = 0;
      estado_persiana = "";
    }
  }

  if (now - lastMeasure > 1000) {
    lastMeasure = now;


    Serial.print("flg_persiana: ");
    Serial.println(flg_persiana);
    Serial.print("estado_persiana: ");
    Serial.println(estado_persiana);
    Serial.print("finCarreraA: ");
    Serial.println(digitalRead(finCarreraA));
    Serial.print("finCarreraB: ");
    Serial.println(digitalRead(finCarreraB));
  }


  /*if (now - lastMeasure > 1000) {
    lastMeasure = now;
	
    // Check if any reads failed and exit early (to try again).h
    if (isnan(temperatureC)) {
      Serial.println("Failed to read from TEMPERATURE sensor!");
      return;
    }

    // Publishes Temperature values
    client.publish("livingroom/temperature", String(temperatureC).c_str());

    //Serial.print("Temperature: ");
    //Serial.print(temperatureC);
    //Serial.println(" ºC");
  }*/
} 
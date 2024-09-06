//-------------[LIBRERIAS]----------------------------------------
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>   
#include <ArduinoJson.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Arduino.h>
//-------------------[DEFINICION PIN RFID]----------------------------------
#define SS_PIN 5
#define RST_PIN 22
MFRC522 mfrc522(SS_PIN, RST_PIN);

//------------------[CREDENCIALES WIFI]-------------------------------------

const char* ssid = "RedmiAPE";
const char* password = "asdf1234";

//------------------[CREDENCIALES PARA BROKER MQTT]------------------------

const char* MQTT_username = NULL;
const char* MQTT_password = NULL;

//------------------[CREDENCIALES TELEGRAM]---------------------------------
#define BOTtoken "7522578386:AAH2efD-dLJUIWZ9FnHn_eByA7iyvhOle8E"  
#define CHAT_ID "6125781520"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
//----------------------------- VARIABLES DE VERIFICACION MENSAJES
int botRequestDelay = 100;
unsigned long lastTimeBotRan;

//------------------[DIRECCIÓN IP DEL SERVIDOR BROKER MQTT]-----------------

const char* mqtt_server ="192.168.123.231";

//------------------[INICIALIZACION DEL espClient]--------------------------

WiFiClient espClient;
PubSubClient tclient(espClient);
//------------------------------ VARIABLE ACTIVACION DE ALARMA
bool Alarma_Activada = LOW;
//------------------------------ SENSORES Y ACTUADORES 
const int buzzer = 12; //ZUMBADOR 
const int pir = 13; //SENSOR PIR
const int SenPuerta = 2; //SENSOR DE PUERTA 
const bool led_1 = 14;
const bool led_2 = 27;
//------------------------------ VARIABLES DE PROGRAMA
bool pirState= LOW; //ESTADO PIR 
bool puertaState = HIGH; //ESTADO PUERTA
bool ctrlPuerta = LOW; // VARIABLE MENSAJE PUERTA CERRADA
bool AccesoValido = LOW; //VARIABLE ACCESO RFID
bool Intruso; //VARIABLE MENSAJE DE INTRUSO 
bool NegIntruso;//VARIABLE NEGAR ACCESO
bool Alarma_Node; //VARIABLE ACTIVA SEGURIDAD NODE


int reconectar;
int Alarma_ON; //VARIABLE ACTIVACION ALARMA
int Alarma_OFF; //VARIABLE DESACTIVACION ALARMA

//----------------------------------[FUNCION RECIBIR MENSAJES DE NODE]-------------------------------

void callback(String topic, byte* message, unsigned int length) {
  String messageTemp;

  // If a message is received on the topic room/lamp, you check if the message is either on or off. Turns the lamp GPIO according to the message
  if(topic=="room/Alarma") {
    for (int i = 0; i < length; i++) {
      Serial.print((char)message[i]);
      messageTemp += (char)message[i];
    }
    Serial.println();
    Serial.println("Message arrived on topic: ");
    Serial.println(topic);
    Serial.println(". Mensaje: ");
    Serial.println("Cambiando sistema de seguridad a ");
    
    if(messageTemp == "on"){
      Alarma_ON = 1;
      Serial.println("SIS Encendido, Server local");
    } 
    
    else if(messageTemp == "off") {
      Alarma_OFF = 1;
      Serial.println(" SIS Apagado, Server local");
    }
  } 
}

//----------------------------------[FUNCION RECONEXION A COLA MQTT]-------------------------------

void reconnect() {
  while (!tclient.connected()) {
    Serial.print("Intentando conexión MQTT");
    
    if (tclient.connect("ESP32 Seguridad", MQTT_username, MQTT_password)) {
      Serial.println("conectado");  
      
      tclient.subscribe("room/Alarma");
    } else {
      Serial.print("falló, rc=");
      Serial.print(tclient.state()); 
      Serial.println("Inténtalo de nuevo en 1 segundos");
      delay(1000);
    }
  }
}

//----------------------------------[FUNCION RECIBIR MENSAJES DE TELEGRAM]-------------------------------
// RECIBE Y LEE MENSAJES DE TELEGRAM 
void handleNewMessages(int numNewMessages) {
  Serial.println("Nuevo Mensaje: ");
  Serial.print(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Usuario No Autorizado", "");
      continue;
    }
    
    // Muestra el mensaje recibido en el serial
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String welcome = "Bienvenido, " + from_name + ".\n";
      welcome += "Usa los siguientes comandos para controlar el sistema de alarma\n\n";
      welcome += "/Alarma_ON Encender el sistema de seguridad \n";
      welcome += "/Alarma_OFF  Apagar el sistema de seguridad \n";
      welcome += "/Estado Informe en qué condición se encuentra la alarma \n\n";
      welcome += "Informacion: el sensor PIR valida la presencia de personas y animales \n";
      welcome += "SIS está encendido y se hace la apertura de la puerta, activará la alarma sonora de intrusos\n";
      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/Alarma_ON") {
      bot.sendMessage(chat_id, "Encendiendo SIS", "");
      Alarma_ON = 2;
    }
    
    if (text == "/Alarma_OFF") {
      bot.sendMessage(chat_id, "Apagando SIS", "");
      Alarma_OFF = 2;
    }
    
    if (text == "/Estado") {
      if (Alarma_Activada == HIGH){
        bot.sendMessage(chat_id, "Alarma esta activada ✅", "");
      }
      else{
        bot.sendMessage(chat_id, "Alarma esta apagada ❌", "");
      }
    }
    
    if (text == "/SI_ACCESO") {
      bot.sendMessage(chat_id, "VALIDANDO ACCESO DESDE TELEGRAM (ALARMA SEGUIRÁ ACTIVADA)", "");
      digitalWrite(buzzer,LOW); 
      Intruso = LOW;
    }
    if (text == "/NO_ACCESO") {
      bot.sendMessage(chat_id, "El ACCESO SE HA DENEGADO; LA ALARMA SONORA CONTINUARÁ ENCENDIDA.", "");
      digitalWrite(buzzer,HIGH);
      Intruso = LOW;  
      }
    if (text == "/RESET") {
       digitalWrite(buzzer,LOW);
       Alarma_Activada = LOW;
       Intruso = LOW;
      }
  }
}
//----------------------------------------------------------------------------------------------------------
void setup() 
{
  Serial.begin(9600);   // iniciar comunicacion serial

//--------------------------------------[DifinirPines de Entradas y salidas]--------------------------------
  pinMode(pir, INPUT);
  pinMode(SenPuerta, INPUT);
  pinMode(buzzer, OUTPUT);

//---------------------------------------[INICIAR SERVICIOS RFID]-------------------------------------------

  SPI.begin();      // Iniciar SPI BUS
  mfrc522.PCD_Init();   // Iniciar MFRC522

  //------------------------------------------[Conexión WIFI]------------------------------------------------

  // Conectando a Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  #ifdef ESP32
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Agregamos certificacion root para api.telegram.org
  #endif
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a WiFi..");
  }
  // IMPRIME LA DIRECCION IP DEL ESP32 A LA CUAL SE CONECTA
  Serial.println(WiFi.localIP());

  //--------------------------------------[Configuracion Servidor MQTT]--------------------------------
  tclient.setServer(mqtt_server, 1883);
  tclient.setCallback(callback);

//-------------------------------------[MENSAJE SISTEMA CONECTADO Y LISTO]------------------------------------

  bot.sendMessage(CHAT_ID, "⚠️🛑 SIS Encendido presione /start para ver las opciones de control... ", "");

}

void loop() {

//-------------------------------[CONEXION COLAS MQTT]----------------------------------------------------------

  if (!tclient.connected()) {
    for(reconectar = 0; reconectar < 5; reconectar++){
        reconnect();
    }
  }
  tclient.loop();
  
//-------------------------------[VERIFICACION DE MENSAJES #(PERIODICAMENTE VERIFICA SI HAY MENSAJES NUEVOS)]----------------------------------------------------------
    if (millis() > lastTimeBotRan + botRequestDelay)  {
      int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

      while(numNewMessages) {
        Serial.println("Obtuve Respuesta");
        handleNewMessages(numNewMessages);
        numNewMessages = bot.getUpdates(bot.last_message_received + 1);
      }
    lastTimeBotRan = millis();
    }
    

  if (Alarma_Activada == LOW && Alarma_ON == 1) {
    Alarma_Activada = HIGH;
    
    if (Alarma_Activada) {
      bot.sendMessage(CHAT_ID, "💻 SIS ENCENDIDO - LOCAL", "");
    }
      Alarma_ON = 0; 
  }

  if (Alarma_Activada == HIGH && Alarma_OFF == 1 ) {
    Alarma_Activada = LOW;
    
    if (Alarma_Activada == LOW) {
      bot.sendMessage(CHAT_ID, "💻🔕 SIS APAGADA - LOCAL ", "");
    }
      Alarma_OFF = 0; 
  }




  if (Alarma_Activada == LOW && Alarma_ON == 2) {
    Alarma_Activada = HIGH;
    
    if (Alarma_Activada) {
      bot.sendMessage(CHAT_ID, "📱✅ SIS ENCENDIDO - TELEGRAM", "");
    }
      Alarma_ON = 0; 
  }

  if (Alarma_Activada == HIGH && Alarma_OFF == 2 ) {
    Alarma_Activada = LOW;
    
    if (Alarma_Activada == LOW) {
      bot.sendMessage(CHAT_ID, "📱🔕 SIS APAGADO - TELEGRAM ", "");
    }
      Alarma_OFF = 0; 
  }

 
  if (AccesoValido == HIGH) {

    Alarma_Activada = !Alarma_Activada;
    if (Alarma_Activada) {
      bot.sendMessage(CHAT_ID, "🛜✅ SIS ENCENDIDO - RFID", "");
    } 
    else {
      bot.sendMessage(CHAT_ID, "🛜🔕 SIS APAGADO - RFID", "");
    }
    AccesoValido = LOW; 
  }

//---------------------------------[ACTIVACION ALARMA - ESTADO SENSORES ]---------------------------------

    puertaState = digitalRead(SenPuerta);
    pirState = digitalRead(pir);

if (Alarma_Activada){

    if (puertaState == LOW) {
    digitalWrite(buzzer, HIGH); 
    Serial.println("Puerta Abierta.");
    bot.sendMessage(CHAT_ID, "¡¡ATENCION!! La Puerta ha sido Abierta", "");
    ctrlPuerta = HIGH;
    
    }

    if (ctrlPuerta == HIGH && puertaState == HIGH) {
      Serial.println("Puerta se ha Cerrado");
      bot.sendMessage(CHAT_ID, " ¡¡ATENCION!! 🚪 Se ha Cerrado", "");
      Intruso = HIGH;
      ctrlPuerta = LOW;
    }

    if (Intruso){
      bot.sendMessage(CHAT_ID, "¡¡ATENCION INTRUSO!!🚷🚷🚷 \n", "");
      bot.sendMessage(CHAT_ID, "⚠️⚠️⚠️ Validar Acceso \n\n /SI_ACCESO - /NO_ACCESO", "");
    }
    if (pirState == HIGH ) {
     bot.sendMessage(CHAT_ID, "¡ATENCION! SE HA DETECTADO PRESENCIA 💂", "");
     delay(200);
      }
    }else{
      digitalWrite(buzzer,LOW);

    }

    

  //-----------------------------[LECTOR DE RFID]------------------------------------------

  // ver si hay nuevas tarjetas
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    return;
  }
  // Seleccionar una de las tarjetas
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }


  //VER EL UID en el monitor serial
  Serial.print("UID tag :");
  String content= "";
  byte letter;
 

  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("Message : ");
  content.toUpperCase();

  
   //if (content.substring(1) == "21 88 83 26")
   if (content.substring(1) == "BE BB 1A 5B")
  {
    Serial.println("CREDENCIAL ACEPTADA");
    Serial.println("ENVIANDO NOTIFICACION A TELEGRAM");
    Serial.println(); 
    AccesoValido = HIGH;
    Intruso = LOW;
    
  }else   {
    Serial.println(" CREDENCIAL DENEGADO");
    Serial.println("ENVIANDO NOTIFICACION A TELEGRAM");
    bot.sendMessage(CHAT_ID, "¡¡ALERTA!! SE HA DETECTADO UNA NUEVA TARJETA RFID ¡¡INTENTO DENEGADO!!", "");
    }
}

 

     
 

/*
     CÓDIGO:  Q0589-Master
     AUTOR:   BrincandoComIdeias
     LINK:    https://www.youtube.com/brincandocomideias ; https://cursodearduino.net/ ; https://cursoderobotica.net
     COMPRE:  https://www.arducore.com.br/
     SKETCH:  Integrando Variaveis entre 2 Arduinos - Esp01 Master
     DATA:    26/01/2020
*/

// INCLUSÃO DE BIBLIOTECAS
#include <A2a.h>
#include "config.h"

// DEFINIÇÕES
#define endereco 0x08
#define tempoAtualizacao 10000

// INSTANCIANDO OBJETOS
AdafruitIO_Feed *sensor = io.feed("sensor");
AdafruitIO_Feed *displayLED = io.feed("displayLED");

A2a arduinoSlave;

// DECLARAÇÃO DE FUNÇÕES
void configuraMQTT();
void retornoDisplayLED(AdafruitIO_Data *data);
bool monitoraSensor();

// DECLARAÇÃO DE VARIÁVEIS
unsigned long controleTempo = 0;
bool comandoRecebido = false;
unsigned int valorSensor;

void setup() {
  Serial.begin(9600);
  while (! Serial);

  configuraMQTT();

  arduinoSlave.begin(0, 2);

  Serial.println("Atualizando valor do Display de LED");
  
  displayLED->get();
  io.run();
  
  Serial.println("Fim Setup");
}

void loop() {
  io.run();

  //Monitora o sensor 
  if (millis() > controleTempo + tempoAtualizacao) {
    if (monitoraSensor()) {
      controleTempo = millis();
      sensor->save(valorSensor);
    }
  }
}

// IMPLEMENTO DE FUNÇÕES
void configuraMQTT() {
  Serial.print("Conectando ao Adafruit IO");
  io.connect();

  displayLED->onMessage(retornoDisplayLED);

  while (io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.println(io.statusText());
}

void retornoDisplayLED(AdafruitIO_Data *data) {
  Serial.print("Controle Recebido <- ");  
  Serial.println(data->value());
  
  arduinoSlave.varWireWrite(endereco, 2, byte(data->toInt()));
}

bool monitoraSensor() {
  static int leituraAnt;
  
  byte byte1 = arduinoSlave.varWireRead(endereco, 0);
  byte byte2 = arduinoSlave.varWireRead(endereco, 1);
  unsigned int leitura = byte1 << 8 | byte2;
  
  if (leitura != leituraAnt) {

    valorSensor = leitura;
    
    leituraAnt = leitura;
    return true;
  } else {
    return false;
  }
}

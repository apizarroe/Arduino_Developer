#include <OneWire.h>                
#include <DallasTemperature.h>
#include <IRremote.h>
 
OneWire ourWire(2);                //Se establece el pin 2  como bus OneWire
DallasTemperature sensors(&ourWire); //Se declara una variable u objeto para nuestro sensor

const int RECV_PIN=7;
const int IR_SEND_PIN=5;

void setup() {
  Serial.begin(9600);
  IrReceiver.begin(RECV_PIN, ENABLE_LED_FEEDBACK);
  IrSender.begin(IR_SEND_PIN);
}
 
void loop() {

  sensors.requestTemperatures();   //Se envía el comando para leer la temperatura
  float temp= sensors.getTempCByIndex(0); //Se obtiene la temperatura en ºC
  Serial.print("Temperatura= ");
  Serial.print(temp);
  Serial.println(" C");
  
  if (temp >= 35) {
    IrSender.sendNEC(61184, 3, 32);
  } else {
    IrSender.sendNEC(61184, 2, 32);
  }

  /*if (IrReceiver.decode()){
    Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);
    Serial.println(IrReceiver.decodedIRData.protocol);
    Serial.println(IrReceiver.decodedIRData.address);
    Serial.println(IrReceiver.decodedIRData.command);
    IrReceiver.resume();
  }*/

  delay(1000);
  
}
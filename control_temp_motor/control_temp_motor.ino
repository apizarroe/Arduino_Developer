// C++ code
//

int led=8;
int relemotor=7;
float sensorT=A5;
float valorT;

void setup()
{
  pinMode(led, OUTPUT);
  pinMode(relemotor, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  
  valorT=analogRead(sensorT);
  valorT=((5 * valorT * 100.0)/1024)-49.71;
  
  if(valorT>40){
  	digitalWrite(relemotor,HIGH); //encender
    digitalWrite(led,HIGH);
  } else {
  	digitalWrite(relemotor,LOW); //apagar
    digitalWrite(led,LOW);
  }
  
  Serial.println(valorT);

}

//Programa: Control de Parpadeo de un LED
//Datos: el LED esta conectado el puerto 13 digital

int led=13;
float valortemperatura;
float lm35=A0;

void setup() { //configurar puertos (Entrada/Salida)
  pinMode(led, OUTPUT); //El puerto 13 es Salida
}

void loop() { //el cuerpo del programa, lo que hace el programa
  // capturar el dato del puerto analogo
  // almacenar el dato en variable valor
  
  valortemperatura=analogRead(lm35);
  valortemperatura=(5 * valortemperatura * 100.0)/1024;
  
  if(valortemperatura>35){
  	digitalWrite(led,HIGH); //encender
  } else {
  	digitalWrite(led,LOW); //apagar
  }
  
}

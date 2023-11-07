//Programa: Control de Parpadeo de un LED
//Datos: el LED esta conectado el puerto 13 digital

int led=13;
float valor;
float potenciometro=A0;

void setup() { //configurar puertos (Entrada/Salida)
  pinMode(led, OUTPUT); //El puerto 13 es Salida
}

void loop() { //el cuerpo del programa, lo que hace el programa
  // capturar el dato del puerto analogo
  // almacenar el dato en variable valor
  valor=analogRead(potenciometro);
  if(valor>500){
  	digitalWrite(led,HIGH);
  } else {
  	digitalWrite(led,LOW);
  }
  
  //digitalWrite(led,HIGH); //led encendido
  //delay(1000);           //espera 1 segundo
  //digitalWrite(led,LOW);  //led apagado
  //delay(1000);           //espera 1 segundo
}
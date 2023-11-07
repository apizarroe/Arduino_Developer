//Programa: Control de Parpadeo de un LED
//Datos: el LED esta conectado el puerto 13 digital

void setup() { //configurar puertos (Entrada/Salida)
  pinMode(13, OUTPUT); //El puerto 13 es Salida
}

void loop() { //el cuerpo del programa, lo que hace el programa
  digitalWrite(13,HIGH); //led encendido
  delay(1000);           //espera 1 segundo
  digitalWrite(13,LOW);  //led apagado
  delay(1000);           //espera 1 segundo
}

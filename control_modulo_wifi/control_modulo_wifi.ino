void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while(!Serial){
    ; // Espera a que el puerto se conecte. Necesario para los puertos USB nativos
  }

  Serial.println("Comenzando...");

  Serial1.begin(115200);
  Serial1.write("AT\r\n"); // Envio AT
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial1.available()) {
    Serial.write(Serial1.read());
  }

  if (Serial.available()) {
    Serial1.write(Serial.read());
  }
}

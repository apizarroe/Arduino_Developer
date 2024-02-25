int VelocidadMotorA = 8; 
int VelocidadMotorB = 9;

int entrada1MotorA = 2;
int entrada2MotorA = 3;
int entrada1MotorB = 7;
int entrada2MotorB = 6;

int infraPinD = 4;    
int infraPinI = 5;

int valorInfraI = 0;  
int valorInfraD = 0;  

void setup() {
  Serial.begin(9600);
  delay(1000);
  pinMode(infraPinD, INPUT);    
  pinMode(infraPinI, INPUT);

  pinMode(entrada1MotorA, OUTPUT);
  pinMode(entrada2MotorA, OUTPUT);
  pinMode(entrada1MotorB, OUTPUT);
  pinMode(entrada2MotorB, OUTPUT);

  pinMode(VelocidadMotorA, OUTPUT);
  pinMode(VelocidadMotorB, OUTPUT);
  analogWrite(VelocidadMotorA, 180); 
  analogWrite(VelocidadMotorB, 180 );

  digitalWrite(entrada1MotorA, LOW);
  digitalWrite(entrada2MotorA, LOW);
  digitalWrite(entrada1MotorB, LOW);
  digitalWrite(entrada2MotorB, LOW);
}

void loop() {
  valorInfraD = digitalRead(infraPinD);   
  //valorInfraI = digitalRead(infraPinI);

  Serial.println(valorInfraD);
  Serial.println(valorInfraI);

 // Cuatro escenarios: De frente      
  if(valorInfraD == 0 && valorInfraI == 0){
    Serial.println("Ninguno en linea");
    
    // Modificamos sentido de giro de los motores
    digitalWrite(entrada1MotorA, HIGH);
    digitalWrite(entrada1MotorB, HIGH);
    delay(25);                      
    digitalWrite(entrada1MotorA, LOW);
    digitalWrite(entrada1MotorB,LOW);
    delay(25);                     
  }

  // El robot encuentra línea negra con el infrarrojo derecho y hay que corregir girando a la derecha
  if(valorInfraD == 1 && valorInfraI == 0){  
    Serial.println("Derecho en linea");
    
    // Modificamos sentido de giro de los motores
    digitalWrite(entrada1MotorA, LOW);
    digitalWrite(entrada1MotorB,LOW);
    delay(25);
    digitalWrite(entrada1MotorA, HIGH);
    digitalWrite(entrada1MotorB,LOW);
    delay(25);
  }

  // El robot encuentra línea negra con el infrarrojo izquierdo y hay que corregir girando a la izquierda
  if(valorInfraD == 0 && valorInfraI == 1){ 
    Serial.println("Izquierdo en linea");
    
    // Modificamos sentido de giro de los motores
    digitalWrite(entrada1MotorA,LOW);
    digitalWrite(entrada1MotorB, LOW);
    delay(25);
    digitalWrite(entrada1MotorA,LOW);
    digitalWrite(entrada1MotorB, HIGH);
    delay(25);
  }

  // Los dos sensores infrarrojos encuentran una línea negra, el final del circuito
  if(valorInfraD == 1 && valorInfraI == 1){ 
    Serial.println("Ambos en linea");
    
    // Paramos los motores 
    digitalWrite(entrada1MotorA, LOW);
    digitalWrite(entrada2MotorA, LOW);
    digitalWrite(entrada1MotorB, LOW);
    digitalWrite(entrada2MotorB, LOW);

    //delay(500);
  }
}
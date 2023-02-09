const int enPin   = 3; // PWM se conecta al pin 1 del puente-H
const int in1Pin  = 4; // Entrada 2 del puente-H
const int in2Pin  = 5; // Entrada 7 del puente-H
 
 
 
void setup() {
 
 
 
Serial.begin(9600); //Iniciamos la comunicacion serial
 
pinMode(in1Pin, OUTPUT); 
 
pinMode(in2Pin, OUTPUT); 
 
Serial.println("Para seleccionar la velocidad del motor (0-9). Para seleccionar la direccion introduce + o -"); //Imprime en la consola puerto serie
 
}
 
 
 
void loop() {
 
 
 
  if ( Serial.available()) { //Comprobamos que el puerto serial esta disponible para la comunicacion
 
    char ch = Serial.read(); //Leemos las variables que introducimos desde el teclado por el monitor serial
 
    if(ch >= '0' && ch <= '9') {
 
      int speed = map(ch, '0', '9', 0, 255); // Adaptamos el numero a una escala de 0 a 255
 
      analogWrite(enPin, speed);
 
      Serial.println("El valor de velocidad intruducido es:");
 
      Serial.println(speed); //Escribe el valor analogico PWM enPin
 
    }
 
    else if (ch == '+') {
 
      Serial.println("Girando en sentido horario...");
 
      digitalWrite(in1Pin,LOW);
 
      digitalWrite(in2Pin,HIGH);
 
    }
 
    else if (ch == '-') { 
 
      Serial.println("Girando en sentido anti-horario...");
 
      digitalWrite(in1Pin,HIGH);
 
      digitalWrite(in2Pin,LOW);
 
    }
 
 
  }
 
}

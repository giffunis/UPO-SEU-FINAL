/* Arduino tutorial - Buzzer / Piezo Speaker
   More info and circuit: http://www.ardumotive.com/how-to-use-a-buzzer-en.html
   Dev: Michalis Vasilakis // Date: 9/6/2015 // www.ardumotive.com */
#include <Servo.h>

const int buzzer = 8; //buzzer to arduino pin 9
int pinRojo = 7;
int pinVerde = 9;
int llegaTren = 0;
Servo myservo;  // create servo object to control a servo
int pos = 90;    // variable to store the servo position                      //Change to your talkback ID


void setup(){
  pinMode(buzzer, OUTPUT); // Set buzzer - pin 9 as an output
  pinMode(pinRojo,OUTPUT);
  pinMode(pinVerde,OUTPUT);
  myservo.attach(5);  // attaches the servo on pin 9 to the servo object
  // Inicialización de LEDs (todos encendidos durante 5 segundos para saber que empieza aplicación)    
  digitalWrite(pinRojo,HIGH);
  digitalWrite(pinVerde,HIGH);
  delay(1000);
  digitalWrite(pinRojo,LOW);
  digitalWrite(pinVerde,LOW);
}

void loop(){
  if (llegaTren == 1) {
      digitalWrite(pinRojo,HIGH);
      digitalWrite(pinVerde,LOW);
      analogWrite(buzzer, 2);
      bajarBarrera(); 
  }
  else{
    digitalWrite(pinRojo,LOW);
    digitalWrite(pinVerde,HIGH);
    noTone(buzzer);     // Stop sound...
    levantarBarrera();
  }
}

void alarma(){
  tone(buzzer, 1000); // Send 1KHz sound signal...
  delay(100);        // ...for 1 sec
  noTone(buzzer);     // Stop sound...
  delay(100);        // ...for 1sec
}

void levantarBarrera(){
  if (pos==0){
for (pos = 0; pos <= 90; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(100);         
    }
  }
}

void bajarBarrera(){
  if(pos==90){
  for (pos = 90; pos >= 0; pos -= 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(100);         
    } 
  }

}
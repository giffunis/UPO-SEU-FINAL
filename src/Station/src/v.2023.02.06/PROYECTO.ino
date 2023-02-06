//                                                                      //
//  Sistemas Empotrados y Oblicuos                                      //
//  Máster en Ingeniería Informática - Universidad Pablo de Olavide     //
//                                                                      //
//  Estación de tren                                                    //
//  Febrero 2023                                                        //
//                                                                      //


//Librerías
#include <LiquidCrystal.h>
#include <SPI.h>
#include <WiFi101.h>
//Includes
#include "pitches.h"
#include "arduino_secrets.h" 


//Definición de sensores y actuadores
    //Pantalla LCD
    const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
    LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
    //LED rojo
    int pinRojo = 0;
    //Sensor de temperatura
    int pinTempSensor = A0;
    //Aviso acústico
    int pinAlarma = 8;
    int melody[] = {NOTE_C4, NOTE_E4, NOTE_G4, NOTE_C4, NOTE_E4, NOTE_G4};
    int noteDurations[] = {4, 4, 2, 4, 4, 2};

//ThingSpeak API
//#define TIMEOUT  5000                               // Timeout for server response.
//#define UPDATE_TIME_INTERVAL 5000                   // Time interval in milliseconds to update ThingSpeak (number of seconds * 1000 = interval)
//#define NUM_FIELDS 3                                // To update more fields, increase this number and add a field label below.
//#define TEMPERATURE_FIELD 1                         // ThingSpeak field for temperature measurement.
//#define WIFI_SIGNAL_FIELD 2                         // ThingSpeak Field for WiFi Signal measurement.
//#define BUTTON_COUNT_FIELD 3                        // ThingSpeak Field for button
//#define THING_SPEAK_ADDRESS "api.thingspeak.com"
//#define TALKBACK_ID 47583                       
//String writeAPIKey= SECRET_WRITE_API_KEY;           
//String talkbackAPIKey = SECRET_TALKBACK_API_KEY;    

//WiFi
//char ssid[] = SECRET_SSID;                          // your network SSID (name)
//char pass[] = SECRET_PASS;                          // your network password (use for WPA, or use as key for WEP)
//int keyIndex = 0;                                   // your network key Index number (needed only for WEP)
//int status = WL_IDLE_STATUS;
//WiFiClient client; 

//Variables propias
int   llegaTren = 0;
long  horaLlegada = millis() + 120000;   //Al tren le quedan 2 minutos para llegar
long  tiempoRestante = 0;

void setup() 
{
// put your setup code here, to run once:

  lcd.begin(16, 2);
  lcd.print("Estacion de tren");
  
  // Configuración de pines digitales como salida
  pinMode(pinRojo,OUTPUT);

  // Inicialización de LEDs (todos encendidos durante 5 segundos para saber que empieza aplicación)    
  digitalWrite(pinRojo,HIGH);
  delay(5000);
  digitalWrite(pinRojo,LOW);

  lcd.clear();
  
  // Inicialización comunicación serie PC-Arduino
  Serial.begin(9600);
}


void loop()
{
// put your main code here, to run repeatedly:

  // Calculamos el tiempo restante
  tiempoRestante = calcularTiempo();

char minRest[4];
sprintf(minRest,"%lu min",tiempoRestante);

  // Imprimimos el tiempo por pantalla
  lcd.setCursor(0, 0);
  lcd.print("Tiempo restante:");
  lcd.setCursor(0, 1);
  lcd.print(minRest);

  //lcd.print(minRest);

  if (tiempoRestante < 1) {
    //El tren va a llegar
    llegaTren = 1;
  }

  // Si el tren llega a la estación, se desencadena la respuesta
  if (llegaTren == 1) {
      trenEnEstacion();
  }
  
  // Esperar 1 s hasta la siguiente lectura
  delay(1000);

}

//Función para imprimir el tiempo pendiente de llegada
long calcularTiempo()
{
    long minRestantes = (horaLlegada - millis())/60000;

        return minRestantes;
}

//Función para desencadenar acciones cuando el tren llegue a la estación
int trenEnEstacion()
{
    //El LED rojo se enciende para avisar lumínicamente
    digitalWrite(pinRojo, HIGH);

    //La pantalla avisa
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ATENCION!");
    lcd.setCursor(0, 1);
    lcd.print("Tren en estacion");

    //Suena una señal sonora
    for (int thisNote = 0; thisNote < 6; thisNote++) {
  
      int noteDuration = 1000 / noteDurations[thisNote];
      tone(pinAlarma, melody[thisNote], noteDuration);
  
      int pauseBetweenNotes = noteDuration * 1.30;
      delay(pauseBetweenNotes);
      // stop the tone playing:
      noTone(pinAlarma);
    }
}

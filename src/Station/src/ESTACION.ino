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
#include <PubSubClient.h>
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

//ThingSpeak & MQTT API
#define channelID 2028262
#define mqttPort 1883
#define UPDATE_TIME_INTERVAL  5000                  // Time interval in milliseconds to update ThingSpeak (number of seconds * 1000 = interval)
#define POS_TRAIN             1                     // ThingSpeak field for train position
#define POS_CROSS             2                     // ThingSpeak Field for level crossing position
#define POS_STATION           3                     // ThingSpeak Field for station position
#define TRAIN_SPEED           4                     // ThingSpeak Field for train speed
#define SPEED_CONTROL         5                     // ThingSpeak Field for speed control
const char mqttUserName[] = SECRET_MQTT_USERNAME; 
const char clientID[]     = SECRET_MQTT_CLIENT_ID;
const char mqttPass[]     = SECRET_MQTT_PASSWORD;
const char* server        = "mqtt3.thingspeak.com";
long lastPublishMillis = 0;


//WiFi
char ssid[] = SECRET_SSID;                          // your network SSID (name)
char pass[] = SECRET_PASS;                          // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;                                   // your network key Index number (needed only for WEP)
int status = WL_IDLE_STATUS;
WiFiClient client; 
PubSubClient mqttClient( client );


//Variables propias
int    enviarVelocidad = 0;
long   tiempoRestante = 0;
long   distanciaRestante = 0;
int    posEstacion = 10000;
String respuesta = "";
String posTren = "";
String velocTren = "";


void setup() 
{

      // Connect to Wi-Fi network.
      connectWifi();
      // Configure the MQTT client
      mqttClient.setServer( server, mqttPort ); 
      // Set the MQTT message handler function for subscription
      mqttClient.setCallback( mqttSubscriptionCallback );
      // Set the buffer to handle the returned JSON. NOTE: A buffer overflow of the message buffer will result in your callback not being invoked.
      mqttClient.setBufferSize( 2048 );


      // INICIALIZAMOS LA ESTACION
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

    // Reconnect to WiFi if it gets disconnected.
      if (WiFi.status() != WL_CONNECTED) {
          connectWifi();
      }
      
      // Connect if MQTT client is not connected and resubscribe to channel updates.
      if (!mqttClient.connected()) {
         mqttConnect(); 
         //Publicamos solo una vez la posición de la estacion
         mqttPublish( channelID, POS_STATION, posEstacion);        
         //We subscribe to the topic corresponding to all fields of the channel
         mqttSubscribe( channelID );
      }
      
      // Call the loop to maintain connection to the server.
      mqttClient.loop();                 
        
}

//Función para comprobar el tiempo restante
void gestionarTren(String posTren, String velocTren){

  char minRest[10];
  char distRest[10];
  
        // Calculamos el tiempo restante
        tiempoRestante = calcularTiempo(posTren,velocTren);
          sprintf(minRest,"%lu",tiempoRestante);

        // Calculamos la distancia restante
        distanciaRestante = calcularDistancia(posTren);
          sprintf(distRest,"%lu",distanciaRestante);
        
        // Imprimimos el tiempo por pantalla
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Tiempo restante:");
        lcd.setCursor(0, 1);
        lcd.print(minRest);
        lcd.print(" min");
      
        // Si el tren está llegando a la estación, se desencadena la respuesta
        if (distanciaRestante < 1000 && distanciaRestante > 10) {
            trenEnEstacion();
        }

        // Cuando el tren esté parado, se informa. Ya ha llegado
        if (distanciaRestante <= 10 || posTren.toInt() == 0) {
            trenParado();
        }
}

//Función para imprimir el tiempo restante hasta la llegada
int calcularTiempo(String posTren, String velocTren)
{

    float kmRestantes  = (posEstacion - posTren.toInt()) / 1000;
    int   velocidad    = velocTren.toInt();
    int   minRestantes = kmRestantes / velocidad * 60;

    if (minRestantes > 20) {
      minRestantes = 20;
    }
    
        return minRestantes;
}

//Función para calcular la distancia que queda
int calcularDistancia(String posTren)
{
    int distancia = posEstacion - posTren.toInt();

    if (distancia > 10000) {
      distancia = 10000;
    }

        return distancia;
}

//Función para desencadenar acciones cuando el tren llegue a la estación
int trenEnEstacion()
{
    if (enviarVelocidad == 0) {
        //Cuando se acerque a la estación se establece la velocidad del tren a 40
        mqttPublish( channelID, SPEED_CONTROL, 40.00);
        Serial.println( " - Control de velocidad activado!" );
        enviarVelocidad = 1;        
    }

    //El LED rojo se enciende para avisar lumínicamente
    digitalWrite(pinRojo, HIGH);

    //La pantalla avisa
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ATENCION!");
    lcd.setCursor(0, 1);
    lcd.print("Tren llegando...");

    //Cuando le queden menos de 500 metros activo la señal acústica
    if (calcularDistancia(posTren) < 500) {
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
}


//Función para desencadenar acciones cuando el tren se pare
int trenParado()
{
    //El LED rojo se enciende para avisar lumínicamente
    digitalWrite(pinRojo, HIGH);

    //La pantalla avisa
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ATENCION!");
    lcd.setCursor(0, 1);
    lcd.print("Tren estacionado");

    // No suena nada
    noTone(pinAlarma);
    
}

// Connect to WiFi.
void connectWifi()
{
  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    while (true);       // don't continue
  }

  // attempt to connect to WiFi network:
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);                   // print the network name (SSID);

    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
  }

  printWiFiStatus();                        // you're connected now, so print out the status
}

// Connect to MQTT broker.
void mqttConnect() {
  // Loop until connected.
  while ( !mqttClient.connected() )
  {
    // Connect to the MQTT broker.
    if ( mqttClient.connect( clientID, mqttUserName, mqttPass ) ) {
      Serial.print( "MQTT to " );
      Serial.print( server );
      Serial.print (" at port ");
      Serial.print( mqttPort );
      Serial.println( " successful." );
    } else {
      Serial.print( "MQTT connection failed, rc = " );
      // See https://pubsubclient.knolleary.net/api.html#state for the failure code explanation.
      Serial.print( mqttClient.state() );
      Serial.println( " Will try again in a few seconds" );
      delay( 10000 );
    }
  }
}

// Function to handle messages from MQTT subscription.
void mqttSubscriptionCallback( char* topic, byte* payload, unsigned int length ) {
  // Print the details of the message that was received to the serial monitor. See that it is a JSON-formatted message in this case
  //Clasificamos la respuesta, solo nos interesa el campo 1 y 4
  respuesta = "";
  if (String(topic).lastIndexOf("fields/field1") > 0) {
    for (int i = 0; i < length; i++) {
      respuesta += (char)payload[i];  //almacenamos la respuesta
    }
    posTren = respuesta;
    //Controlo acciones asociadas al tren
    gestionarTren(posTren,velocTren);
    Serial.println("Posicion del tren: " + posTren);
  }
  if (String(topic).lastIndexOf("fields/field4") > 0) {
    for (int i = 0; i < length; i++) {
      respuesta += (char)payload[i];  //almacenamos la respuesta
    }
    velocTren = respuesta;
    //Controlo acciones asociadas al tren
    gestionarTren(posTren,velocTren);
    Serial.println("Velocidad del tren: " + velocTren);
  }
    
}


// Subscribe to ThingSpeak channel for updates.
void mqttSubscribe( long subChannelID ){

  //Create the topic name according to the ThingSpeak organization of topics
  String myTopic = "channels/"+String( subChannelID )+"/subscribe/fields/+";   //me suscribo a todos los campos

  //Use MQTT client to subscribe
  mqttClient.subscribe(myTopic.c_str());
  
}


// Publish messages to a particular field of a ThingSpeak channel.
void mqttPublish(long pubChannelID, int fieldNumber, float value) {

  //Create the topic name according to the ThingSpeak organization of topics
  String topicString ="channels/" + String( pubChannelID ) + "/publish/fields/field" + String( fieldNumber );

  //Message content
  String payload = String(value);
  
  mqttClient.publish( topicString.c_str(), payload.c_str() );

}

void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  // print where to go in a browser:
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}

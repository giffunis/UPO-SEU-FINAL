#include <WiFi101.h>
#include <PubSubClient.h>
#include <LiquidCrystal.h>
#include "arduino_secrets.h"
#include "mqtt_settings.h"


// Change arduino_secrets.h accordingly
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;        // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;                 // your network key Index number (needed only for WEP)

//Change mqtt_settings.h accordingly
const char mqttUserName[] = SECRET_MQTT_USERNAME; 
const char clientID[] = SECRET_MQTT_CLIENT_ID;
const char mqttPass[] = SECRET_MQTT_PASSWORD;
const int channelID = CHANNEL_ID;
const int updateTimeInterval = UPDATE_TIME_INTERVAL;
const int mqttPort = MQTT_PORT;
const int trainFields = TRAIN_FIELDS;
const int trainPosField = TRAIN_POS_FIELD;
const int trainSpeedField = TRAIN_SPEED_FIELD;
const int trainSpeedControlField = TRAIN_SPEED_CONTROL_FIELD;


// Variables de la aplicación
const char* server = "mqtt3.thingspeak.com";
int status = WL_IDLE_STATUS;
long lastPublishMillis = 0; // Variable que se utiliza para controlar el tiempo de publicación en ThingSpeak
long lastPosCalcMillis = 0; // Variable que almacena la última vez que se calculó el tiempo
int _speed = 0; // Variable que almacena la velocidad en m/s
int _pos = 0;   // Variable que almacena la posición del tren en m


// Objetos
WiFiClient client; 
PubSubClient mqttClient(client);

// Declaración de los pines del motor
const int enPin   = 3; // PWM se conecta al pin 1 del puente-H
const int in1Pin  = 4; // Entrada 2 del puente-H
const int in2Pin  = 5; // Entrada 7 del puente-H

// Declaración de los pines del LCD
const int rs = 12, en = 11, d4 = 10, d5 = 9, d6 = 2, d7 = 1;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() 
{
  Serial.begin(9600); //Iniciamos la comunicacion serial

  // Llamamos a las funciones que hacen el setup de los componentes
  dcMotorSetup();
  lcdSetup();

  // Connect to Wi-Fi network.
  connectWifi();
  
  // Configure the MQTT client
  mqttClient.setServer( server, mqttPort ); 
  
  // Set the MQTT message handler function for subscription
  mqttClient.setCallback( mqttSubscriptionCallback );
  
  // Set the buffer to handle the returned JSON. NOTE: A buffer overflow of the message buffer will result in your callback not being invoked.
  mqttClient.setBufferSize( 2048 );

}


void loop() {
 
   // Reconnect to WiFi if it gets disconnected.
  if (WiFi.status() != WL_CONNECTED) {
      connectWifi();
  }
  
  // Connect if MQTT client is not connected and resubscribe to channel updates.
  if (!mqttClient.connected()) {
     mqttConnect(); 

     // C9 - We subscribe only to the temperature field
     mqttSubscribe(channelID, trainSpeedControlField);
  }

  // Call the loop to maintain connection to the server.
  mqttClient.loop();

  // Calculamos la pos cada segundo
  if(abs(millis() - lastPosCalcMillis) > 1000){
    updatePos();
  }

    // Update ThingSpeak channel periodically. The update results in the message to the subscriber.
  if ( abs(millis() - lastPublishMillis) > updateTimeInterval) {
    
    int fieldsLength = trainFields;
    int fields[] = {trainPosField, trainSpeedField};
    String payloads[] = {String(_pos), String(getSpeedInKmH(_speed))};
    mqttPublishChannelFeed(channelID, fields, payloads, fieldsLength);
    
    // Guardamos el momento de la última publicación.
    lastPublishMillis = millis();
  }
  
}

/**
 * Función que configura la conexión WIFI
 */
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

/**
 * Connect to MQTT broker.
 */
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

/**
 * Subscribe to channel updates from specific field of channel with MQTT
 */
void mqttSubscribe(long subChannelID, int fieldNumber ){

  //Create the topic name according to the ThingSpeak organization of topics
  String myTopic = "channels/" + String( subChannelID ) + "/subscribe/fields/field" + String (fieldNumber);

  //Use MQTT client to subscribe
  mqttClient.subscribe(myTopic.c_str());
  
}

/**
 * Función para publicar todos los campos en el canal
 */
void mqttPublishChannelFeed(long pubChannelID, int* fieldNumbers, String* payloads, int fields) {
  
  String topicString ="channels/" + String( pubChannelID ) + "/publish";
  String payload = "";
  
  for(int n = 0; n < fields; n++){
    payload += "field" + String(fieldNumbers[n]) + "=" + String(payloads[n])+ "&";
  }
  
  payload += "status=MQTTPUBLISH";
  mqttClient.publish( topicString.c_str(), payload.c_str() );
  Serial.println(payload);
}

// Function to handle messages from MQTT subscription.
void mqttSubscriptionCallback( char* topic, byte* payload, unsigned int length ) {
  // Print the details of the message that was received to the serial monitor. See that it is a JSON-formatted message in this case
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  String topicString = String ( topic );
  String field = "field";
  
  if(topicString.indexOf(String(field + String(trainSpeedControlField))) >= 0){
    int newSpeed = payloadToInt(payload, length);
    setTrainSpeed(newSpeed);
  }
}


/**
 * Recibimos la velocidad en Km/h, lo mapeamos a un rango 0 - 255 para
 * enviarle un valor analógico a la electrónica.
 */
int setTrainSpeed(int newSpeed)
{
  //Calculamos la posición alcanzada a la velocidad anterior
  updatePos();
  
  // Actualizamos la variable de velocidad en m/s
  _speed = getSpeedInMS(newSpeed);
  int speedPWM = map(_speed, 0, 28, 0, 255); // Adaptamos el número a una escala de 0 a 255
  analogWrite(enPin, speedPWM);
 
  Serial.print("Modificada la velocidad del tren a ");
  Serial.print(newSpeed);
  Serial.print(" Km/h o ");
  Serial.print(_speed);
  Serial.println(" m/s");
}

/**
 * Setea el modo de funcionamiento de los pines del L293DNE
 * y establece el sentido de giro 'horario'
 */
void dcMotorSetup(){
  // Definimos el modo de los pines
  pinMode(in1Pin, OUTPUT);
  pinMode(in2Pin, OUTPUT);

  // Establecemos el sentido del giro 'horario'
  digitalWrite(in1Pin,LOW);
  digitalWrite(in2Pin,HIGH);

  // Establecemos el sentido del giro 'anti-horario'
  digitalWrite(in1Pin,HIGH);
  digitalWrite(in2Pin,LOW);

  // Inicializamos la velocidad de inicio.
  analogWrite(enPin, _speed);
}

void lcdSetup(){
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  // Print a message to the LCD.
  lcd.print("hello, world!");
}

/**
 * Imprime el estado de la conexión WIFI
 */ 
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

/**
 * Parsea el payload y devuelve un valor de tipo int
 */
int payloadToInt( byte* payload, unsigned int length ){
  char input[length];
  for (int i = 0; i < length; i++) {
    input[i] = ((char)payload[i]);
  }
  return atoi(input);
}

float getSpeedInKmH(int speedInMS)
{
  return (speedInMS * 18.0) / 5;
}

float getSpeedInMS(int speedInKmH)
{
  return (speedInKmH * 5.0) / 18;
}

float getPosInKm(int posInM)
{
  return posInM * 1.0 / 1000;
}

void updatePos()
{
  float seconds = ((millis() - lastPosCalcMillis) * 1.0 / 1000);
  _pos = _pos + _speed * seconds; // S = V(m/s) * T(s) + Pos_(t-1)
  lastPosCalcMillis = millis();
  Serial.print("Pos en metros: ");
  Serial.println(_pos);
}

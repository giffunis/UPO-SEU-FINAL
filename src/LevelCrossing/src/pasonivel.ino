
#include <SPI.h>
#include <WiFi101.h>
#include <PubSubClient.h>
#include <Servo.h>



#include "arduino_secrets.h" 
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;                 // your network key Index number (needed only for WEP)
int trainPos=0;



// Ensure that the credentials here allow you to publish and subscribe to the ThingSpeak channel.

//Change the channel to your ChannelID
#define channelID 2028262

//Change arduino_secrets.h accordingly
const char mqttUserName[] = SECRET_MQTT_USERNAME; 
const char clientID[] = SECRET_MQTT_CLIENT_ID;
const char mqttPass[] = SECRET_MQTT_PASSWORD;
const int buzzer = 8; //buzzer to arduino pin 9
int pinRojo = 7;
int pinVerde = 9;
int llegaTren = 0;
int pos = 90;
Servo myservo;
int subeVelocidad=0;
int bajaVelocidad=0;
//Unsecure MQTT
#define mqttPort 1883


#define UPDATE_TIME_INTERVAL 1000                // Time interval in milliseconds to update ThingSpeak (number of seconds * 1000 = interval)

// ThingSpeak information.
#define TEMPERATURE_FIELD 1                      // ThingSpeak field for temperature measurement.
#define WIFI_SIGNAL_FIELD 2                       // ThingSpeak Field for WiFi Signal measurement.

const char* server = "mqtt3.thingspeak.com";

long lastPublishMillis = 0;

int status = WL_IDLE_STATUS;

WiFiClient client; 

//Cliente MQTT
PubSubClient mqttClient( client );


//Helper functions

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
  trainPos=0;
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    trainPos+=((char)payload[i]-'0')*pow(10,length-1-i);

  }
  Serial.println(topic);
if(strcmp(topic,"channels/2028262/subscribe/fields/field1")==0) {
    if(trainPos>=4000 && trainPos<=5100){
    llegaTren=1;      
    }
    else{
    llegaTren=0;
    }
}
}


// Subscribe to ThingSpeak channel for updates.
void mqttSubscribe( long subChannelID, int fieldNumber ){

  //Create the topic name according to the ThingSpeak organization of topics
  String myTopic = "channels/"+String( subChannelID )+"/subscribe/fields/field" + String(fieldNumber);

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

  Serial.print( "Topic = " );
  Serial.println( topicString );
  Serial.print( "Payload = " );
  Serial.println( payload );
}


void setup() {
  Serial.begin(115200);      // initialize serial communication
  pinMode(buzzer, OUTPUT); // Set buzzer - pin 9 as an output
  pinMode(pinRojo,OUTPUT);
  pinMode(pinVerde,OUTPUT);
  myservo.attach(5);  // attaches the servo on pin 9 to the servo object
  digitalWrite(pinRojo,HIGH);
  digitalWrite(pinVerde,HIGH);
  delay(1000);
  digitalWrite(pinRojo,LOW);
  digitalWrite(pinVerde,LOW);
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
     //We subscribe to the topic corresponding to all fields of the channel
     mqttSubscribe( channelID , 1);
     mqttPublish( channelID, 2, 5000);
  }
  
  // Call the loop to maintain connection to the server.
  mqttClient.loop(); 
  
  // Update ThingSpeak channel periodically. The update results in the message to the subscriber.
  if ( abs(millis() - lastPublishMillis) > UPDATE_TIME_INTERVAL) {
    //Send temperature to field 1
    if(trainPos>=4000 && bajaVelocidad==0){
    mqttPublish( channelID, 5, 50);
    bajaVelocidad=1;
    }
    if (trainPos>5100 && subeVelocidad==0){
    subeVelocidad=1;
    mqttPublish( channelID, 5, 100);      
    }
    lastPublishMillis = millis();
  }

  delay(50);

  if (llegaTren == 1) {
      digitalWrite(pinRojo,HIGH);
      digitalWrite(pinVerde,LOW);
      analogWrite(buzzer, 5);
      bajarBarrera(); 
  }
  else{
    digitalWrite(pinRojo,LOW);
    digitalWrite(pinVerde,HIGH);
    analogWrite(buzzer, 0);
    levantarBarrera();
  }
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


void levantarBarrera(){
  if (pos==-1){
for (pos = 0; pos <= 90; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(100);         
    }
  }
}

void bajarBarrera(){
  if(pos>=90){
  for (pos = 90; pos >= 0; pos -= 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(100);         
    } 
  }

}
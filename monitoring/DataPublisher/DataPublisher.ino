/*
 * This node (ESP6288 v12) is responcible for read data from the sensors controller (arduino node)
 * And to broadcast it via MQTT, The broad cast message will be handled by a MQTT subsriber and persisted in db
 * The MQTT sybsriber and Server will be on one node (RaspebberyPi) and the db is MongoDB
 * MQTT subsriber is a different project placed here: 
 */
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "DataMessage.h"

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "Animal5"
#define WLAN_PASS       "ToM40&LiLa"
/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "192.168.1.103"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "Jakim"
#define AIO_KEY         ""

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT);

/****************************** Feeds ***************************************/

// Setup a feed called 'riserriser' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish riserriser = Adafruit_MQTT_Publish(&mqtt, "/feeds/riserriser");

/*************************** Sketch Code ************************************/

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();


void setup() {
  Serial.println("Enter in Setup function\n");
  
  Serial.begin(115200);
  delay(10);

  // Connect to WiFi access point.
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();

  Serial.println("WiFi connected");
  Serial.print("IP address: "); Serial.println(WiFi.localIP());
}

char message[] = "Hello World";
void loop() {
  char incomingByte;
  int dataSize = sizeof(dataMessage);
  char buffer[dataSize];
  int i = 0;
  
  dataMessage myData;
  
  int receivedBytes = Serial.available();
  Serial.print("\nWe got bytes: ");
  Serial.println(receivedBytes);
  Serial.print("\nWe are expecting: ");
  Serial.println(dataSize);

  if(receivedBytes > 0){
    Serial.readBytes(buffer, dataSize);
    
    Serial.println("\nDone receiving: ");
    
    memcpy(&myData, buffer, dataSize);

    Serial.print("Header is: ");
    Serial.println(myData.header);
    Serial.print("Humidity is: ");
    Serial.println(myData.sensData.hummidity);
    Serial.print("Temperature is: ");
    Serial.println(myData.sensData.temperature);
    Serial.print("Checks sum is: ");
    Serial.println(myData.checkSum);

      // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  char *message = sensorDataToJSON(myData);
  //char message[] = "{\"sensor\":2, \"data\":{\"temperature\":22, \"hummidity\":18 }}"; 
  // this is our 'wait for incoming subscription packets' busy subloop
  // try to spend your time here

  // Broadcast data
  Serial.print(F("\nSending riserver val ")); Serial.print(message); Serial.print("...");
  
  if (! riserriser.publish(message)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }

  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds
 
   mqtt.disconnect();
   free(message); message = NULL;
  }
  else
  {
    Serial.println("Waiting for input..."); //Print Hello word every one second
  }
  
  delay(3000);
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}

char *sensorDataToJSON(dataMessage msg)
{
  char *convertedInt = (char *) malloc(sizeof(int));
  
  char *tmpMsg = (char *)  malloc(80);
  strcpy(tmpMsg, "{\"header\" : \"");
  
  strcat(tmpMsg, msg.header); 
  strcat(tmpMsg, "\", \"data\" :{\"temperature\" : " );
  itoa(msg.sensData.temperature, convertedInt, 10);
  strcat(tmpMsg, convertedInt );
  
  tmpMsg = strcat(tmpMsg, ", \"hummidity\" : " );
  itoa(msg.sensData.hummidity, convertedInt, 10);
  tmpMsg = strcat(tmpMsg, convertedInt );

  tmpMsg = strcat(tmpMsg, ", \"mq2\" : " );
  itoa(msg.sensData.mq2, convertedInt, 10);
  tmpMsg = strcat(tmpMsg, convertedInt );

  tmpMsg = strcat(tmpMsg, "}}");  

  return tmpMsg;
  }

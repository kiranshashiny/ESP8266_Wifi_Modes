#include <WiFi.h>
#include <PubSubClient.h>
#include <string>
#include <iostream>
//*****************************************
//// AP + STA MODE TOGETHER………


// Local Access point Ip Address……
IPAddress local_IP(192,168,4,15);
IPAddress subnet(255,255,255,0);

// Cloud connection details
WiFiClient espClient;
PubSubClient pubsubclient(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
const char *ssid ="JioFiber-ahGu7";
const char *password = "welcome2ibm";
const char* mqtt_server = "m12.cloudmqtt.com";

IPAddress staticIP(192,168,4,15);
IPAddress subnet2(255,255,255,0);

WiFiServer server(80);
void callback(char* topic, byte* payload, unsigned int length) {
}
int pos =0;

void reconnect() {
  //this only applies to the cloud connection.
  // Loop until we're reconnected
  // 28 Aug, changed 'while() to if()' this way I am not blocked
  
  if (!pubsubclient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (pubsubclient.connect(clientId.c_str(), "oxefqvkn", "uTM7RdarxTPA")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      pubsubclient.publish("outTopic", "Creative Controllers");
      // ... and resubscribe
      pubsubclient.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(pubsubclient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{

    Serial.begin(9600);
        
    Serial.println ("Setting Soft-AP ") ;    
    WiFi.softAP ("Wemos_AP", "Wemos_comm");
    WiFi.softAPConfig(local_IP, local_IP, subnet);
    
    Serial.println(WiFi.softAPIP());
    server.begin();

    ////////////// . Part 2    /////////////
    
    Serial.println();
    
    WiFi.begin(ssid, password);
    
    //WiFi.config(staticIP, staticIP, subnet2);
    
    while (WiFi.status() != WL_CONNECTED) 
    {
      delay(500);
      Serial.print ("." );      
    }
    
    Serial.println("Connected to cloud successfully");
    Serial.println(WiFi.localIP());
    pubsubclient.setServer(mqtt_server, 19757);
    pubsubclient.setCallback(callback);
}

void publish_mqtt_cloud ( String pubstr ) {
  
      int firstComma = pubstr.indexOf(',');
      String topicStr = pubstr.substring( 0, firstComma);
      Serial.println(" .  extracted topicStr: ["+ topicStr + "]");
      
      String valueStr = pubstr.substring(firstComma +1 );
      Serial.println(" .  extracted valueStr: [" + valueStr + "]");
    
      pubsubclient.publish ( topicStr.c_str() , valueStr.c_str() );
    
}
 
void loop() {

  // Part 1 // Local Access Point, Get data from Clients.
  WiFiClient client = server.available();
  if (!client) {return;}
  String request = client.readStringUntil('\r');
  Serial.println("********** Got Something from Station **********************");
  Serial.println("From the station: [" + request + "]" );
  client.flush();
  //Serial.print("Byte sent to the station: ");
  //Serial.println(client.println(request + "__received" + "\r"));


  // Part 2 .  Forward it to Remote Cloud Point, Cloudmqtt.com//

  if (!pubsubclient.connected()) {
    reconnect();
  }
  pubsubclient.loop();

  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    
    // Link to String functions in Arduino, 
    // https://www.arduino.cc/reference/en/language/variables/data-types/string/functions/substring/
    // returns a "-1" if not found.
    
    // Get the ribbon of information Pipe Char separated from Clients, publish to the cloud.
    
    while (  (pos = request.indexOf( "|" ) ) != -1) {
 
           // it is not string.substr(), instead string.substring()
           String sub = request.substring( 0,pos );
           Serial.println ("Extracted substr after removing first part =  "  + sub );
           // it is not string.erase(), it is string.remove() instead.
           request.remove ( 0, pos+1);
           Serial.println ("request is now chopped to  " + request );
           publish_mqtt_cloud(sub );
    }
    // there will be the last string that will contain the topic after the while(), handle it.
    publish_mqtt_cloud(request );
  }
}

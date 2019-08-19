/*  Accesspoint - station communication without router
 *  see: https://github.com/esp8266/Arduino/blob/master/doc/esp8266wifi/station-class.rst
 *  Works with: accesspoint_bare_01.ino
 */

#include <ESP8266WiFi.h>
#include<dht.h>
dht DHT;
#define DHT11_PIN D7


char ssid[] = "Wemos_AP";           // SSID of your AP
char pass[] = "Wemos_comm";         // password of your AP
char msg[50];


IPAddress server(192,168,4,15);     // IP address of the AP
WiFiClient client;

void setup() {
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);           // connects to the WiFi AP
  Serial.println();
  Serial.println("Connection to the AP");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.println("Connected");
  Serial.println("station_bare_01.ino");
  Serial.print("LocalIP:"); Serial.println(WiFi.localIP());
  Serial.println("MAC:" + WiFi.macAddress());
  Serial.print("Gateway:"); Serial.println(WiFi.gatewayIP());
  Serial.print("AP MAC:"); Serial.println(WiFi.BSSIDstr());
}

void loop() {
  int chk = DHT.read11(DHT11_PIN); 

  Serial.print("Temperature:");
  Serial.print(DHT.temperature, 2);
  Serial.print("    ");
  Serial.print("Humidity:");
  Serial.println(DHT.humidity, 2);
  delay(500);
  
  client.connect(server, 80);
  Serial.println("********************************");
  Serial.println("Byte sent to the AP: ");

  // Temperature
  float t = DHT.temperature;
  snprintf (msg, 75, "tempr, %0.2f C\r", t);

  Serial.print(">>Publish message: ");
  Serial.println(msg);
  client.print(msg);
  
  String answer = client.readStringUntil('\r');
  Serial.println("From the AP: " + answer);
  client.flush();
  delay(1000);
  //Humidity
  float h = DHT.humidity;
  snprintf (msg, 75, "hmdty, %0.2f \%% \r", h);

  Serial.print(">>Publish message: ");
  Serial.println(msg);
  client.print(msg);
  
  answer = client.readStringUntil('\r');
  Serial.println("<<From the AP: " + answer);
  client.flush();
  
  client.stop();
  delay(3000);

}


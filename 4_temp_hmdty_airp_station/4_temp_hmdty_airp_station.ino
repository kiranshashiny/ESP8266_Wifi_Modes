/*  Accesspoint - station communication without router
 *  see: https://github.com/esp8266/Arduino/blob/master/doc/esp8266wifi/station-class.rst
 *  Works with: accesspoint_bare_01.ino
 */

#include <ESP8266WiFi.h>
#include<dht.h>
#include <SFE_BMP180.h> //Including BMP180 library

dht DHT;
#define DHT11_PIN D7


char ssid[] = "Wemos_AP";           // SSID of your AP
char pass[] = "Wemos_comm";         // password of your AP
char msg[50];
char newmsg[50];

#define ALTITUDE 920.00 //Altitude where I live (change this to your altitude)

SFE_BMP180 pressure; //Creating an object

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

  // Init the Pressure sensor
  if (pressure.begin()) //If initialization was successful, continue
    Serial.println("BMP180 init success");
  else {
    Serial.println("BMP180 init failed !!!!");
  }
}

void loop() {
  double T, P, p0;
  char status;
  
  int chk = DHT.read11(DHT11_PIN); 

  Serial.print("Temperature:");
  Serial.print(DHT.temperature, 2);
  Serial.print("    ");
  Serial.print("Humidity:");
  Serial.println(DHT.humidity, 2);
  delay(500);
  
  client.connect(server, 80);
  Serial.println("********************************");
  
  // Temperature
  float t = DHT.temperature;
  snprintf (msg, 75, "tempr, %0.2f C\r", t);
  client.print(msg);
  Serial.println(msg);
  //client.flush();
  
  //String answer = client.readStringUntil('\r');
  //Serial.println("From the AP: " + answer);
  client.flush();
  client.stop();
  delay(2000);
  
  // To send the second sensor data....
  // Strange !  I have to connect(server..) again to send data.
  // or else it wont go.

  //Humidity  
  client.connect(server, 80);  
  float h = DHT.humidity;
  sprintf (newmsg, "hmdty, %0.2f \%%\r", h);
  client.print(newmsg);
  Serial.println (newmsg);
  client.flush();
  client.stop();
  delay(3000);

  
  // Air Pressure
  //Serial.print("You provided altitude: ");
  //Serial.print(ALTITUDE, 0);
  //Serial.println(" meters");

  status = pressure.startPressure(3);

  if (status != 0) {
      delay(status);

      status = pressure.getPressure(P, T);
      if (status != 0) {
          //Serial.print("Pressure measurement: ");
          //Serial.print(P);
          //Serial.println(" hPa (Pressure measured using temperature)");

          p0 = pressure.sealevel(P, ALTITUDE);
          //Serial.print("Relative (sea-level) pressure: ");
          //Serial.print(p0);
          //Serial.println("hPa");
          
          client.connect(server, 80);
          long ap = (long) p0;  
          sprintf (newmsg, "airp, %ld hPa\r", ap);
          client.print(newmsg);
          Serial.println (newmsg);
          client.flush();
          client.stop();
          delay(3000);
      }
  }
}

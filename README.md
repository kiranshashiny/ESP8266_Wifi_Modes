# ESP8266_Wifi_Modes
Access Point and Station Mode examples

1st Example:
Code:

#### AP
"WiFi_SoftAP" is the Access Point - central
Install this on a ESP8266, this will sit and monitor for incoming requests.

#### Station

"WiFi_Station" is the end point that connects to the AP.
Install this on a different ESP8266 - this connects to the AP above.

This example was from 
https://circuits4you.com/2019/01/05/connecting-esp8266-to-wifi-hotspot-example/

2nd Example:
https://www.instructables.com/id/Accesspoint-Station-Communication-Between-Two-ESP8/

#### AP

This sends data back and forth between the AP and the Station.
On one hand is the AP - that has configured the Ap, with fixed IP address

#### Station

On the other end is the station that sends just the string to the AP and receives data back.

	e.g:
	********************************
	Byte sent to the AP: 5
	From the AP: Anyoca
	********************************



Different WiFi modes explained.

What is Soft-AP

Soft-AP means : it does NOT have access to Wired Network !

![image](https://user-images.githubusercontent.com/14288989/63208550-10072380-c0f3-11e9-9bc3-7b0fede23355.png)

https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/readme.html



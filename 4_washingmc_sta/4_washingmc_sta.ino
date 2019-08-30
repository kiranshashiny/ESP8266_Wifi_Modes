/*
 * 
 * This code integrates the debounce mechanism where it ignores any 
 * pulses that are triggered when any other wall switches are turned on
 * or off.
 * This reduces the false pulses thereby records the water flow almost
 * proprotionately.
 * The references for debounce  are from 
 * https://forum.arduino.cc/index.php?topic=417406.0
 * https://www.bc-robotics.com/tutorials/using-a-flow-sensor-with-arduino/#comment-87670
 * 
 * Code is meant for Wemos D1-R1 mini microcontroller.
 * 
 */

#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

char ssid[] = "Wemos_AP";           // SSID of your AP
char pass[] = "Wemos_comm";         // password of your AP

IPAddress server(192,168,4,15);     // IP address of the AP
WiFiClient client;


// for LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// The amount of time (in milliseconds) between tests
#define WASHMC_DELAY         1000
#define WASHMC_PUBLISH_DELAY 3
int publish_delay_count = 0;

byte connect_flag = false;

byte sensorInterrupt = D5;  // 0 = digital pin 2

// The hall-effect flow sensor outputs approximately 4.5 pulses per second per
// litre/minute of flow.
float calibrationFactor = 4.5;

volatile byte pulseCount;  

float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;

unsigned long oldTime;

// Update these with values suitable for your network.
volatile int buttonState = 0;         // variable for reading the pushbutton status

WiFiClient espClient;
long lastMsg = 0;
char msg[75];
int value = 0;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  lcd.clear();
  lcd.print ("Connecting ...");
  Serial.print("Connecting to ");
  Serial.println(ssid);

  //WiFi.begin(ssid, password);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);    

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected");
  Serial.println("station_bare_01.ino");
  Serial.print("LocalIP:"); Serial.println(WiFi.localIP());
  Serial.println("MAC:" + WiFi.macAddress());
  Serial.print("Gateway:"); Serial.println(WiFi.gatewayIP());
  Serial.print("AP MAC:"); Serial.println(WiFi.BSSIDstr());
  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting connection to AP:  ");
    
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    
    // Attempt to connect
    Serial.println (clientId);

    if (client.connect(server, 80)){

      Serial.println("connected");
      break;
    
    } else {
      Serial.print("failed, rc=");
      //Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  // Initialize a serial connection for reporting values to the host
  Serial.begin(9600);
  // LCD 
  lcd.begin();
  // Turn on the blacklight and print a message.
  lcd.backlight();
  
  setup_wifi();
 
  pulseCount        = 0;
  flowRate          = 0.0;
  flowMilliLitres   = 0;
  totalMilliLitres  = 0;
  oldTime           = 0;

  // The Hall-effect sensor is connected to pin 2 which uses interrupt 0.
  // Configured to trigger on a FALLING state change (transition from HIGH
  // state to LOW state)
  attachInterrupt(digitalPinToInterrupt(sensorInterrupt), pulseCounter, FALLING);

}

/**
 * Main program loop
 */
void loop()
{
   
  //client.loop();
  if((millis() - oldTime) > WASHMC_DELAY)    // Only process counters once per second
  { 
    // Disable the interrupt while calculating flow rate and sending the value to
    // the host
    detachInterrupt(digitalPinToInterrupt(sensorInterrupt));
        
    // Because this loop may not complete in exactly 1 second intervals we calculate
    // the number of milliseconds that have passed since the last execution and use
    // that to scale the output. We also apply the calibrationFactor to scale the output
    // based on the number of pulses per second per units of measure (litres/minute in
    // this case) coming from the sensor.
    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
    
    // Note the time this processing pass was executed. Note that because we've
    // disabled interrupts the millis() function won't actually be incrementing right
    // at this point, but it will still return the value it was set to just before
    // interrupts went away.
    oldTime = millis();
    
    // Divide the flow rate in litres/minute by 60 to determine how many litres have
    // passed through the sensor in this 1 second interval, then multiply by 1000 to
    // convert to millilitres.
    flowMilliLitres = (flowRate / 60) * 1000;
    
    // Add the millilitres passed in this second to the cumulative total
    totalMilliLitres += flowMilliLitres;
    
    //float litresvar = totalMilliLitres/1000;
    float  x = totalMilliLitres;
    x = x /1000 ;
    
    if ( publish_delay_count > WASHMC_PUBLISH_DELAY ) { 
        if (!client.connected()) {
          reconnect();
          delay(10);
        }

       //snprintf (msg, 75, "hello world #%ld", value);
       snprintf (msg, 75, "/WhiteHouse/C1-702/washingmc, %0.2f L\r", x);

       Serial.print("Publish message: ");
       Serial.println(msg);

       client.print(msg);
       publish_delay_count = 0;
       Serial.println ("Resetting count ");
    } else {
       publish_delay_count++;
    }
    lcd.clear();
    // display on LCD
    lcd.setCursor ( 0,0);
    lcd.print("Usage ");
    
    lcd.print(  x );
    lcd.print ( " L " );

    lcd.setCursor ( 0,1 );

    lcd.print ( int( totalMilliLitres) );
    lcd.print ( "mL");
      
    // Reset the pulse counter so we can start incrementing again
    pulseCount = 0;
    // Enable the interrupt again now that we've finished sending output
    attachInterrupt(digitalPinToInterrupt(sensorInterrupt), pulseCounter, FALLING);
  }
}

/*
Interrupt Service Routine
 */
void pulseCounter()
{
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  unsigned long debounceInterval = 10;
  
  if (interrupt_time - last_interrupt_time > debounceInterval) //declare the debounce/block out interval in setup
  {
    pulseCount++;
  }
  last_interrupt_time = interrupt_time;
 
}

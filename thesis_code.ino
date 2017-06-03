/*
  Fraunhofer UMSICHT - Environmental Logging
  LEDs Grow Quality
  ruju
  
  
  Using open source code partially developed by: 
  
(CO2)
  Basic Arduino example for K-Series sensor
  Created by Jason Berger
  Co2meter.com

(Humidity and Temperature)
  An Arduino library for reading the DHT family of temperature and humidity sensors.
  Written by Mark Ruys, mark@paracas.nl.

(Light)
  #Adafruit TCS34725 Color Sensor Driver #
  Adafruit invests time and resources providing this open source code, please support Adafruit and open-source hardware by purchasing products from Adafruit!
  Written by Kevin (KTOWN) Townsend for Adafruit Industries. BSD license, check license.txt for more information All text above must be included in any redistribution

(MQTT Connection)
  Written by Schröder, Andreas. Fraunhofer UMSICHT.
  and 
  Adafruit MQTT Library ESP8266 Adafruit IO SSL/TLS example
  Written by Tony DiCola for Adafruit Industries.
  SSL/TLS additions by Todd Treece for Adafruit Industries.
*/

#include <ESP8266WiFi.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <Wire.h> 
#include "SoftwareSerial.h"
#include "Adafruit_TCS34725.h"
#include "DHT.h"

DHT dht;


SoftwareSerial K_30_Serial(14,12);  //Sets up a virtual serial port
                                    //Using Digital pins 5 for Rx and pin 6 for Tx in NodeMCU Board


byte readCO2[] = {0xFE, 0X44, 0X00, 0X08, 0X02, 0X9F, 0X25};  //Command packet to read Co2 (see app note)
byte response[] = {0,0,0,0,0,0,0};  //create an array to store the response

//multiplier for value. default is 1. set to 3 for K-30 3% and 10 for K-33 ICB
int valMultiplier = 1;

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_1X);    //Light Sensor Settings

#define INTERVAL 10000 // time between measurements

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "****"
#define WLAN_PASS       "****"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "******"
#define AIO_SERVERPORT  *****                   // 8883 for MQTTS
#define AIO_USERNAME    "*****"
#define AIO_KEY         "******"
#define TOPIC_PREFIX    "/ruju/thesis/"
/************ Global State (you don't need to change this!) ******************/

// WiFiFlientSecure for SSL/TLS support
WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// io.adafruit.com SHA1 fingerprint
const char* fingerprint = "26 96 1C 2A 51 07 FD 15 80 96 93 AE F7 32 CE B9 0D 01 55 C4";

long lastMeasure = 0; // timestamp of last measurement

/****************************** Feeds ***************************************/
 
// Setup a feed(s)for publishing and subscribing.

String Thing = "esp32-"+ String(ESP.getChipId());

String uptimeTopic = TOPIC_PREFIX+Thing+"/uptime";
Adafruit_MQTT_Publish mqtt_uptime = Adafruit_MQTT_Publish(&mqtt, uptimeTopic.c_str());
 
String co2Topic = TOPIC_PREFIX+Thing+"/co2";
Adafruit_MQTT_Publish mqtt_co2 = Adafruit_MQTT_Publish(&mqtt, co2Topic.c_str());

String luxTopic = TOPIC_PREFIX+Thing+"/lux";
Adafruit_MQTT_Publish mqtt_lux = Adafruit_MQTT_Publish(&mqtt, luxTopic.c_str());
 
String colortempTopic = TOPIC_PREFIX+Thing+"/colortemp";
Adafruit_MQTT_Publish mqtt_colortemp = Adafruit_MQTT_Publish(&mqtt, colortempTopic.c_str());
 
String redTopic = TOPIC_PREFIX+Thing+"/red";
Adafruit_MQTT_Publish mqtt_red = Adafruit_MQTT_Publish(&mqtt, redTopic.c_str());
 
String greenTopic = TOPIC_PREFIX+Thing+"/green";
Adafruit_MQTT_Publish mqtt_green = Adafruit_MQTT_Publish(&mqtt, greenTopic.c_str());
 
String blueTopic = TOPIC_PREFIX+Thing+"/blue";
Adafruit_MQTT_Publish mqtt_blue = Adafruit_MQTT_Publish(&mqtt, blueTopic.c_str());
 
String clearTopic = TOPIC_PREFIX+Thing+"/clear";
Adafruit_MQTT_Publish mqtt_clear = Adafruit_MQTT_Publish(&mqtt, clearTopic.c_str());

String temperatureTopic = TOPIC_PREFIX+Thing+"/temperature";
Adafruit_MQTT_Publish mqtt_temperature = Adafruit_MQTT_Publish(&mqtt, temperatureTopic.c_str());

String humidityTopic = TOPIC_PREFIX+Thing+"/humidity";
Adafruit_MQTT_Publish mqtt_humidity = Adafruit_MQTT_Publish(&mqtt, humidityTopic.c_str());


void setup() 
{
  // put your setup code here, to run once:
  Serial.begin(9600);         //Opens the main serial port to communicate with the computer
  K_30_Serial.begin(9600);    //Opens the virtual serial port with a baud of 9600
  Wire.begin (); 
  dht.setup(13);              //Humidity and Temperature sensor is connected to Digital pin 7
  if (tcs.begin()) {
    Serial.println(F("TCS34725 sensor found"));
  } else {
    Serial.println(F("No TCS34725 sensor found ... check your connections"));
    while (1);
  }

    // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print(F("Connecting to WiFi SSID \""));
  Serial.print(WLAN_SSID);
  Serial.println(F("\""));
 
  delay(500);
 
  WiFi.begin(WLAN_SSID, WLAN_PASS);   //Starts WiFi connection
  delay(500);
 
  while (WiFi.status() != WL_CONNECTED) {   //Keeps on trying until WiFi connection is established
    delay(500);
    Serial.print(".");
  }
  delay(1000);

   MQTT_connect();                          //Establish MQTT connection
   
  Serial.println();
}

void loop() 
{
  MQTT_connect();
  uint32_t now = millis();

  if (now -lastMeasure > INTERVAL) {
    lastMeasure=now;
    uint16_t r, g, b, c, colorTemp, lux;         //Light Sensor Variables
    static char buffer[256];

        // Measurement
    Serial.println(F("\nMeasurement:"));
 
    Serial.print(F("  Uptime:")); Serial.println(now);
  
  sendRequest(readCO2);
 int valCO2 = getValue(response);
  Serial.println(" ");
  Serial.print("Co2 ppm = ");
  Serial.println(valCO2);
/****************************** Light ***************************************/
    tcs.getRawData(&r, &g, &b, &c);
    colorTemp = tcs.calculateColorTemperature(r, g, b);
    lux = tcs.calculateLux(r, g, b);
   
    Serial.print("  Color Temp: "); Serial.print(colorTemp, DEC); Serial.print(" K - ");
    Serial.print("Lux: "); Serial.print(lux, DEC); Serial.print(" - ");
    Serial.print("R: "); Serial.print(r, DEC); Serial.print(" ");
    Serial.print("G: "); Serial.print(g, DEC); Serial.print(" ");
    Serial.print("B: "); Serial.print(b, DEC); Serial.print(" ");
    Serial.print("C: "); Serial.print(c, DEC); Serial.print(" ");
    Serial.println(" ");

/****************************** Humidity and Temperature ***************************************/
  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();
  
  Serial.print(dht.getStatusString());
  Serial.print("\t");
  Serial.print(humidity, 1);
  Serial.print("\t\t");
  Serial.print(temperature, 1);
  Serial.print("\t\t");

  
/****************************** Publishing ***************************************/
    Serial.println(F("\nSending values via mqtt:"));
   
    sprintf(buffer,"  val: %d \ttopic: %-50s %s", now, uptimeTopic.c_str(), (mqtt_uptime.publish(now) ? " OK" : " Failed!"));
    Serial.println(buffer);
 
    sprintf(buffer,"  val: %d \ttopic: %-50s %s", colorTemp, colortempTopic.c_str(), (mqtt_colortemp.publish(colorTemp) ? " OK" : " Failed!"));
    Serial.println(buffer);
 
    sprintf(buffer,"  val: %d \ttopic: %-50s %s", lux, luxTopic.c_str(), (mqtt_lux.publish(lux) ? " OK" : " Failed!"));
    Serial.println(buffer);
 
    sprintf(buffer,"  val: %d \ttopic: %-50s %s", r, redTopic.c_str(), (mqtt_red.publish(r) ? " OK" : " Failed!"));
    Serial.println(buffer);
 
    sprintf(buffer,"  val: %d \ttopic: %-50s %s", g, greenTopic.c_str(), (mqtt_green.publish(g) ? " OK" : " Failed!"));
    Serial.println(buffer);
 
    sprintf(buffer,"  val: %d \ttopic: %-50s %s", b, blueTopic.c_str(), (mqtt_blue.publish(b) ? " OK" : " Failed!"));
    Serial.println(buffer);
 
    sprintf(buffer,"  val: %d \ttopic: %-50s %s", c, clearTopic.c_str(), (mqtt_clear.publish(c) ? " OK" : " Failed!"));
    Serial.println(buffer);

    sprintf(buffer,"  val: %d \ttopic: %-50s %s", valCO2, co2Topic.c_str(), (mqtt_co2.publish(valCO2) ? " OK" : " Failed!"));
    Serial.println(buffer);

        // trick to sprintf float!
    static char fl_str[15],fl_str2[15];
    dtostrf(humidity,5,3, fl_str);
    dtostrf(temperature,5,3, fl_str2);
    sprintf(buffer,"  val: %s \ttopic: %-50s %s", fl_str, humidityTopic.c_str(), (mqtt_humidity.publish(humidity, 3) ? " OK" : " Failed!"));
    Serial.println(buffer);
    sprintf(buffer,"  val: %s \ttopic: %-50s %s", fl_str2, temperatureTopic.c_str(), (mqtt_temperature.publish(temperature, 3) ? " OK" : " Failed!"));
    Serial.println(buffer);
  }
  yield();
  
}

void sendRequest(byte packet[])
{
  while(!K_30_Serial.available())  //keep sending request until we start to get a response
  {
    K_30_Serial.write(readCO2,7);
    delay(50);
  }
  
  int timeout=0;  //set a timeoute counter
  while(K_30_Serial.available() < 7 ) //Wait to get a 7 byte response
  {
    timeout++;  
    if(timeout > 10)    //if it takes to long there was probably an error
      {
        while(K_30_Serial.available())  //flush whatever we have
          K_30_Serial.read();
          
          break;                        //exit and try again
      }
      delay(50);
  }
  
  for (int i=0; i < 7; i++)
  {
    response[i] = K_30_Serial.read();
  }  
}

unsigned long getValue(byte packet[])
{
    int high = packet[3];                        //high byte for value is 4th byte in packet in the packet
    int low = packet[4];                         //low byte for value is 5th byte in the packet

  
    unsigned long val = high*256 + low;                //Combine high byte and low byte with this formula to get value
    return val* valMultiplier;
}
void verifyFingerprint() {

  const char* host = AIO_SERVER;

  Serial.print("Connecting to ");
  Serial.println(host);

  if (! client.connect(host, AIO_SERVERPORT)) {
    Serial.println("Connection failed. Halting execution.");
    while(1);
  }

  if (client.verify(fingerprint, host)) {
    Serial.println("Connection secure.");
  } else {
    Serial.println("Connection insecure! Halting execution.");
    while(1);
  }

}

// ------------------------------ mQTT Connect --------------
// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.

void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 5;
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




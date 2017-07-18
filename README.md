# Bachelor Thesis

Welcome to the repository of the Bachelor Thesis of Juan David Ruiz (Environment and Energy)

The Project is to build an environmental data logger (carbon dioxide, humidity and temperature, light intensity and color and soil moisture).

The components used were:


Carbon Dioxide Sensor     K-30 by Sense Air (http://www.senseair.com/products/oem-modules/k30/)

Humidity and Temperature  DHT22 by Adafruit (https://www.adafruit.com/product/385)

Soil Moisture             VH400 by Vegetronix (http://www.vegetronix.com/Products/VH400/)

Light Sensor              TCS34725 by Adafruit (https://www.adafruit.com/product/1334)

Microcontroller           ESP8266 by NodeMCU (http://nodemcu.com/index_en.html)

Power Supply 5V 1000 mA
waterproof Case.
jumper wires, resistors, micro-usb cable, breadboard, PCB.

Special Tools:

Arduino IDE and electronics soldering station

Libraries used:

#include <ESP8266WiFi.h> (https://github.com/ekstrand/ESP8266wifi)

#include <Adafruit_MQTT.h> (https://github.com/adafruit/Adafruit_MQTT_Library/blob/master/Adafruit_MQTT.h)

#include <Adafruit_MQTT_Client.h> (https://github.com/adafruit/Adafruit_MQTT_Library/blob/master/Adafruit_MQTT_Client.h)

#include <Adafruit_TCS34725.h> (https://github.com/adafruit/Adafruit_TCS34725)

#include <DHT.h> (https://github.com/adafruit/DHT-sensor-library)


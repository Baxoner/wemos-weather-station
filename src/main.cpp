// code made by Baxoner
// https://github.com/Baxoner/wemos-weather-station
#include <Arduino.h> //including some required libraries
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include "wifi_credentials.h"

const char *ssid = WIFI_SSID;
const char *wifipassword = WIFI_PASSWORD;

Adafruit_BME280 bme; // I2C

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, server, port, username, password);
Adafruit_MQTT_Publish temperature = Adafruit_MQTT_Publish(&mqtt, "weatherStation/temperature", MQTT_QOS_1); //defining topics and Quality Of Service
Adafruit_MQTT_Publish humidity = Adafruit_MQTT_Publish(&mqtt, "weatherStation/humidity", MQTT_QOS_1);
Adafruit_MQTT_Publish pressure = Adafruit_MQTT_Publish(&mqtt, "weatherStation/pressure", MQTT_QOS_1);

unsigned long previousMilliseconds = 0;
const long interval = 60000;

void setup()
{
  Serial.begin(115200); //starting serial
  WiFi.mode(WIFI_STA);  //connecting to the network
  WiFi.begin(ssid, wifipassword);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  bme.begin(0x76); //init BME280 at address 0x76
  pinMode(D5, OUTPUT);
  ArduinoOTA.setHostname("wemos");   // configuring ota
  ArduinoOTA.setPassword("station"); // enabling authentication
  ArduinoOTA.begin();                // OTA is now ready
}

void weatherPublish(); //defining functions before they called
void MQTT_connect();   //because PlatformIO compiler needs that

void loop()
{
  ArduinoOTA.handle();
  unsigned long now = millis(); //some additional code that replace delays
  if (now - previousMilliseconds >= interval)
  {
    previousMilliseconds = now;
    MQTT_connect();   // calling function to connect to the MQTT broker and checking if server is alive
    weatherPublish(); // preparing message and sending it over MQTT
  }
}

void MQTT_connect()
{
  uint8_t ret;
  if (mqtt.connected())
  {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) //now microcontroller tries to connect to the broker
  {
    Serial.println(mqtt.connectErrorString(ret)); //if it can't connect 3 times, then it does nothing and requires a restart to try again
    Serial.println("Retrying MQTT connection in 5 seconds...");
    delay(5000);
    retries--;
    if (retries == 0)
    {
      while (1)
        ;
    }
  }
  Serial.println("MQTT connected");
}

void weatherPublish()
{
  temperature.publish(bme.readTemperature()); //sending sensor readings to mqtt
  if (humidity.publish(bme.readHumidity())) // if board can publish data, function will return true, if not then false
    Serial.println("Succesfully published data");
  else
    Serial.println("Data publish failure");
  pressure.publish(bme.readPressure() / 100.0F);
  Serial.println(bme.readTemperature()); //send it over serial to test if BME280 works correctly
  Serial.println(bme.readHumidity());
  Serial.println(bme.readPressure() / 100.0F);
}

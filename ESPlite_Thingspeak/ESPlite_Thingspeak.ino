/******************************************************************************
  Project  : ESPlite Thingspeak
  Compiler : Arduino 1.6.7
  Board    : ESPresso Lite V1
  Device   : DHT22
  Author   : Chiang Mai Maker Club
*******************************************************************************/


#include <Arduino.h>
#include <CMMC_Manager.h>
//#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "DHT.h"

#define BUTTON_INPUT_PIN 16
CMMC_Manager manager(BUTTON_INPUT_PIN, LED_BUILTIN);

//const char* ssid     = "ESPERT-3020";  // Change your ssid wifi
//const char* password = "espertap";  // Change your password wifi
String api_key = "SGUH5EDVHXIALPHO";   //  Change your api key

#define DHTPIN 12
#define DHTTYPE DHT22

WiFiClient client;
DHT dht(DHTPIN, DHTTYPE);
uint32_t pevmillis = 0;
void init_wifi();
void init_hardware();
void doHttpGet(float, float);

void setup() {
  init_wifi();
  init_hardware();
  Serial.println("Init done...");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    uint32_t conMillis = millis();

    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor!");
      delay(500);
      return;
    }

    if (conMillis - pevmillis >= 15000)  {
      doHttpGet(t, h);
      pevmillis = conMillis;
    }

    Serial.print("Temperature = ");
    Serial.print(t);
    Serial.print("\t");
    Serial.print("Humidity = ");
    Serial.println(h);
    delay(5000);
  } else  {
    Serial.println("connection lost, reconnect...");
    delay(500);
  }
}

/******************* initial loop ***********************************/
void init_wifi() {
  Serial.begin(115200);
  delay(200);
  pinMode(BUTTON_INPUT_PIN, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  manager.start();
  //  if (WiFi.begin(ssid, password)) {
  //    while (WiFi.status() != WL_CONNECTED) {
  //      delay(500);
  //      Serial.print(".");
  //    }
  //  }
  //  Serial.println("WiFi connected");
  //  Serial.println("IP address: ");
  //  Serial.println(WiFi.localIP());
}

void init_hardware() {
  dht.begin();
}

void doHttpGet(float t, float h) {
  HTTPClient http;
  Serial.print("[HTTP] begin...\n");

  // http://api.thingspeak.com/update?api_key=5T4WXGZFE1PZPS2K&field1=0
  http.begin("http://api.thingspeak.com/update?api_key=" + api_key + "&field1=" + String(t) + "&field2=" + String(h)); //HTTP

  // start connection and send HTTP header
  int httpCode = http.GET();

  // httpCode will be negative on error
  if (httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);
    Serial.print("[CONTENT]\n");

    // file found at server
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println(payload);
    }
  } else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();
}

// Copyright Nat Weerawan 2015-2016
// Chiang Mai Maker Club
#include <ESPert.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiConnector.h>
#include <ESP8266HTTPClient.h>

ESPert espert;

#ifndef WIFI_SSID
#define WIFI_SSID       "ESPERT-3020"   //   แก้ไข ssid 
#define WIFI_PASSPHRASE "espertap"
#endif

String apikey = "5T4WXGZFE1PZPS2K"; // แก้ไข api keys thingspeak
float t, h;
int wificount = 0;
const int sleepTimeS = 300; // 300 = 30นาที

WiFiConnector wifi(WIFI_SSID, WIFI_PASSPHRASE);

void init_hardware()  {
  espert.init();
  espert.dht.init();
  espert.oled.init();
  delay(2000);
  Serial.begin(115200);
  WiFi.disconnect(true);
  delay(1000);
}

void init_wifi() {
  wifi.init();
  wifi.on_connected([&](const void* message)  {
    Serial.print("WIFI CONNECTED WITH IP: ");
    Serial.println(WiFi.localIP());
  });

  wifi.on_connecting([&](const void* message) {
    Serial.print("Connecting to ");
    Serial.println(wifi.get("ssid") + ", " + wifi.get("password"));
    delay(200);
    if (wificount >= 10) {    // หากยังเชื่อมต่อไวไฟไม่ได้เกิน 10 ครั้ง จะเข้าโหมด sleep 30 นาที
      espert.oled.clear();
      ESP.deepSleep(sleepTimeS * 6000000);
      wificount = 0;
    }
  });
}


void doHttpGet() {  // ฟังก์ชันสำหรับส่งค่าขึ้น Thingspeak
  HTTPClient http;
  Serial.print("[HTTP] begin...\n");
  http.begin("http://api.thingspeak.com/update?api_key=" + apikey + "&field1=" + String(t) + "&field2=" + String(h)); //HTTP
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

void setup()  {
  init_hardware();
  init_wifi();
  wifi.connect();
}

void loop() {
  wifi.loop();
  if (wifi.connected()) {
    bool isFarenheit = false;
    t = espert.dht.getTemperature(isFarenheit); // อ่านค่าอุณหภูมิ
    h = espert.dht.getHumidity(); // อ่านค่าความชื้นในอากาศ

    String dht = "Temperature: " + String(t) + (isFarenheit ?  " F" : " C") + "\n";
    dht += " Humidity   : " + String(h) + " %\n";
    espert.oled.clear();
    espert.oled.println("   Weather Station ");
    espert.oled.println("");
    espert.oled.println(dht);
    espert.oled.println("Chiang Mai Maker Club");
    espert.oled.update();

    doHttpGet();  //  ส่งค่าขึ้น Thingspeak โดยใช้รูปแบบ GET
    delay(5000);

  } else {
    bool isFarenheit = false;
    t = espert.dht.getTemperature(isFarenheit); // อ่านค่าอุณหภูมิ
    h = espert.dht.getHumidity(); // อ่านค่าความชื้นในอากาศ
    if (!isnan(t) && !isnan(h)) {
      String dht = "Temperature: " + String(t) + (isFarenheit ?  " F" : " C") + "\n";
      dht += " Humidity   : " + String(h) + " %\n";
      espert.oled.clear();
      espert.oled.println("   Weather Station ");
      espert.oled.println("");
      espert.oled.println(dht);
      espert.oled.println("Wifi is not connect !");
      espert.oled.update();
      espert.println(dht);
      delay(5000);
      wificount += 1; // นับเวลา หากยังเชื่อมต่อไวไฟไม่ได้เกิน 10 ครั้งจะเข้า sleep mode 30 นาที
    }
  }
}


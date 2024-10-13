#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include "time.h"

// WiFi Credentials
#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""

// Firebase Things Realtime Database
#define DATABASE_URL "https://upcpre202402si572wv71-default-rtdb.firebaseio.com/.json"
// HTTP Client
HTTPClient client;

// HC-SR04 Pin Configuration
#define TRIGGER_PIN 5
#define ECHO_PIN 18

// LED Configuration
#define GREEN_LED 4
#define RED_LED 2

// Liquid Crystal Display
LiquidCrystal_I2C LCD = LiquidCrystal_I2C(0x27, 16, 2);

// Variables
int prev = 0;
String tempText = "";
String payload = "";
String sensorID = "HC001";
char timeStringBuff[20];
String greenLedBool;
String redLedBool;
String safeDistBool;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  // Initial message
  LCD.init();
  LCD.backlight();
  LCD.setCursor(0, 0);
  LCD.print("Connecting to ");
  LCD.setCursor(0, 1);
  LCD.print("WiFi ");

  // WiFi Setup

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());

  // Firebase connection

  LCD.clear();
  LCD.setCursor(0, 0);
  LCD.println("Online");
  delay(500);
  LCD.clear();
  LCD.setCursor(0, 0);
  LCD.println("Connecting to");
  LCD.setCursor(0, 1);
  LCD.println("Firebase...");
  Serial.println("connecting...");

  client.begin(DATABASE_URL);
  int httpResponseCode=client.GET();

  if (httpResponseCode>0) {
    LCD.clear();
    LCD.setCursor(0, 0);
    LCD.println("Connected");
    Serial.println("connected, Firebase payload:");
    payload = client.getString();
    Serial.println(payload);
    Serial.println();
  }

  // Component setup
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  Serial.println("Green = Safe");
  Serial.println("Red = Unsafe");

  // Indicate Transmission Start
  for (int i=0; i<5; i++) {
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, HIGH);
    delay(200);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, LOW);
    delay(200);
  }

  // NTP Time Setup
  configTime(-9000, -9000, "1.south-america.pool.ntp.org");
}

void loop() {
  long sure, mesafe;
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, LOW);
  sure = pulseIn(ECHO_PIN, HIGH);
  mesafe=(sure/2)/29.1;

  if(prev!=mesafe) {
    if(mesafe>200 || mesafe < 0) {
      digitalWrite(GREEN_LED, HIGH);
      digitalWrite(RED_LED, LOW);
      tempText = "Safe distance: ";
      greenLedBool = "true";
      redLedBool = "false";
      safeDistBool = "true";
    } else {
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(RED_LED, HIGH);
      tempText = "Unsafe distance: ";
      greenLedBool = "false";
      redLedBool = "true";
      safeDistBool = "false";
    }

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
      Serial.println("Time Err");
    }
    strftime(timeStringBuff, sizeof(timeStringBuff), "%d/%m/%Y %H:%M", &timeinfo);
    Serial.println(String(timeStringBuff));

    client.PATCH("{\"Status/Sensors/time\":\""  + String(timeStringBuff) + "\"}");
    client.PATCH("{\"Status/Led/LedV\":" + greenLedBool + "}");
    client.PATCH("{\"Status/Led/LedR\":" + redLedBool + "}");
    client.PATCH("{\"Status/Sensors/Distance\":" + String(mesafe) + "}");
    client.PATCH("{\"Status/Sensors/safeDistance\":" + safeDistBool + "}");
    client.PATCH("{\"Status/Sensors/id\":\"" + sensorID + "\"}");


    LCD.clear();
    LCD.setCursor(0, 0);
    LCD.print(tempText);
    LCD.setCursor(0, 1);
    LCD.print(mesafe);
  }
  prev = mesafe;
  delay(500);

}

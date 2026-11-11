/*
  Lab 3: ESP32 + DHT11 Data Logging to Google Sheets & Looker Studio
  Pin Setup: DHT11 Data Pin connected to GPIO16
*/

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

// WiFi credentials
const char* ssid = "Denie Aidil_2.4G";
const char* password = "asdfghjkl456";

// Google Apps Script Web App URL 
const char* scriptURL = "https://script.google.com/macros/s/AKfycbxrjNeMNDDri_W9Jve3LFx_4iEiiyeiBysBRylft1dFbFIguHkZLscAvSZ-Vq8zEG0D/exec";

// DHT11 Sensor Configuration
#define DHTPIN 16
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// Interval timer for sending data (10 seconds)
const unsigned long sendInterval = 10000; 
unsigned long previousMillis = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Starting ESP32 DHT11 Data Logger...");

  // Initialize DHT sensor
  dht.begin();

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected to WiFi! IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  unsigned long currentMillis = millis();

  // Trigger data collection every 10 seconds without blocking code
  if (currentMillis - previousMillis >= sendInterval) {
    previousMillis = currentMillis;

    // Read values from DHT11 sensor
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();

    // Check if readings failed
    if (isnan(humidity) || isnan(temperature)) {
      Serial.println("Error: Failed to read from DHT sensor!");
      return;
    }

    // Print sensor values to Serial Monitor
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.print(" °C, Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");

    // Send values to Google Sheets
    sendDataToGoogleSheets(temperature, humidity);
  }
}

void sendDataToGoogleSheets(float temperature, float humidity) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected!");
    return;
  }

  HTTPClient http;
  
  // Begin HTTP connection to Google Apps Script endpoint
  http.begin(scriptURL);
  
  // Disable automatic redirects to prevent HTTP 400/302 conversion errors
  http.setFollowRedirects(HTTPC_DISABLE_REDIRECTS);
  http.addHeader("Content-Type", "application/json");

  // Create JSON object payload
  StaticJsonDocument<200> doc;
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;

  String jsonString;
  serializeJson(doc, jsonString);

  // Send HTTP POST request
  int httpResponseCode = http.POST(jsonString);

  // Check HTTP response (Status 200 or 302 indicates success on Google Apps Script)
  if (httpResponseCode == 200 || httpResponseCode == 302) {
    Serial.println("-> Data successfully sent to Google Sheets! Code: " + String(httpResponseCode));
  } else {
    Serial.println("-> Error sending data. HTTP Code: " + String(httpResponseCode));
  }

  http.end();
}
#include <WiFi.h>
#include "Esp32MQTTClient.h"
#include "ArduinoJson.h"

const char *ssid = "-- enter value --";
const char *password = "-- enter value --";

static const char *connectionString = "-- enter value --";

static bool hasIoTHub = false;

int startPin = 26;
int lane1Pin = 34;
int debounceMs = 200;

int raceStartMs = 0;
int lane1EndMs = 0;

void report_race_results(int time)
{
  DynamicJsonDocument doc(1024);

  doc["deviceId"] = "robert-esp32";
  doc["key"] = "lane1";
  doc["hasLocation"] = false;
  doc["body"]["ms"] = time;

  String output;

  serializeJson(doc, output);

  Esp32MQTTClient_SendEvent(output.c_str());
}

void startRace()
{
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  if (interrupt_time - last_interrupt_time > debounceMs)
  {
    Serial.println("Start Race");
    raceStartMs = millis();
  }
  last_interrupt_time = interrupt_time;
}

void setup()
{
  pinMode(startPin, INPUT_PULLDOWN);
  // pinMode(lane1Pin, INPUT_PULLDOWN);
  // pinMode(lane1Pin, INPUT);

  Serial.begin(115200);

  Serial.println("Starting connecting WiFi.");
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  if (!Esp32MQTTClient_Init((const uint8_t *)connectionString))
  {
    hasIoTHub = false;
    Serial.println("Initializing IoT hub failed.");
    return;
  }

  hasIoTHub = true;

  attachInterrupt(digitalPinToInterrupt(startPin), startRace, FALLING);
}

void loop()
{
  int val = analogRead(lane1Pin);

  if (raceStartMs != 0 && val <= 10)
  {
    int lane1Ms = millis() - raceStartMs;
    Serial.println(lane1Ms);
    // report_race_results(lane1Ms);
    raceStartMs = 0;
  }

  // Serial.println("loop");
  // Serial.println(analogRead(lane1Pin));

  // delay(1000);
}
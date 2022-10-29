#include "ArduinoJson.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "Arduino.h"

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLEServer *pServer;
BLEService *pService;
BLECharacteristic *pCharacteristic;

int startPin = 26;

int lane1Pin = 34;
int lane2Pin = 25;
int lane3Pin = 39;
int lane4Pin = 36;
int lane5Pin = 4;
int lane6Pin = 13;

int debounceMs = 200;

int raceStartMs = 0;
int lane1EndMs = 0;

void report_race_results(int time)
{
  DynamicJsonDocument doc(1024);

  doc["key"] = "lane1";
  doc["body"]["ms"] = time;

  String output;
  serializeJson(doc, output);

  pCharacteristic->setValue(output.c_str());
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
  pinMode(lane1Pin, INPUT_PULLUP);
  Serial.begin(115200);

  BLEDevice::init("Derby Track");
  pServer = BLEDevice::createServer();
  pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_WRITE);

  pService->start();
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  attachInterrupt(digitalPinToInterrupt(startPin), startRace, FALLING);
}

void loop()
{
  int val = analogRead(lane6Pin);

  if (raceStartMs != 0 && val <= 1000)
  {
    int lane1Ms = millis() - raceStartMs;
    Serial.println("Report Race Results");
    report_race_results(lane1Ms);
    raceStartMs = 0;
  }
}
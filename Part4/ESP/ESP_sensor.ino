#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <NewPing.h>
#include <math.h>

#define SCREEN_MOSI 19
#define SCREEN_SCLK 18
#define SCREEN_CS 5
#define SCREEN_DC 16
#define SCREEN_RST 23
#define SCREEN_BL 4
Adafruit_ST7789 screen = Adafruit_ST7789(SCREEN_CS, SCREEN_DC, SCREEN_MOSI, SCREEN_SCLK, SCREEN_RST);

#define B1 0
#define B2 35
int inputMode = B1;

#define SHARP_PIN 36
#define POTENT_PIN 37

#define TRIG_PIN 12
#define ECHO_PIN 13
#define MAX_DISTANCE 400
NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE);

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHAR_DIST_UUID "2A1E"
#define CHAR_TEMP_UUID "2A1C"

BLEServer* pServer = NULL;
BLECharacteristic* pDistCharacteristic = NULL;
BLECharacteristic* pTempCharacteristic = NULL;

bool deviceConnected = false;
bool oldDeviceConnected = false;
uint8_t distance;
uint8_t temp;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      BLEDevice::startAdvertising();
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

void setup(void) {
  Serial.begin(9600); 
  pinMode(SCREEN_BL, OUTPUT);
  digitalWrite(SCREEN_BL, HIGH);
  screen.init(135, 240);
  screen.setTextWrap(true);
  screen.setTextSize(4);
  Serial.println(F("Initialized"));
  
  BLEDevice::init("CoudraisGirardin");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);
  pDistCharacteristic = pService->createCharacteristic(
                      CHAR_DIST_UUID,
                      BLECharacteristic::PROPERTY_READ
                    );
  pDistCharacteristic->addDescriptor(new BLE2902());
  pTempCharacteristic = pService->createCharacteristic(
                      CHAR_TEMP_UUID,
                      BLECharacteristic::PROPERTY_READ
                    );
  pTempCharacteristic->addDescriptor(new BLE2902());
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {
  if (analogRead(B1) == 0){
    inputMode = B1;
  } else {
    if (analogRead(B2) == 0){
      inputMode = B2;
    }
  }

  if(inputMode == B1){
    SharpRead();
  }
  if(inputMode == B2){
    UltrasonRead();
  }

  temp = analogRead(POTENT_PIN) / 128;
  if (deviceConnected) {
    pTempCharacteristic->setValue((uint8_t*)&temp, 1);
    pTempCharacteristic->notify();
  }

  if (!deviceConnected && oldDeviceConnected) {
      pServer->startAdvertising();
      Serial.println("start advertising");
      oldDeviceConnected = deviceConnected;
  }
  if (deviceConnected && !oldDeviceConnected) {
      oldDeviceConnected = deviceConnected;
  }
}
void SharpRead(){
  float volt = analogRead(SHARP_PIN) / 1000.0;
  if(volt > 2.8 || volt < 0.435){
    SharpWrite("Out of range");
  }else{
    distance = 60.374 * pow(volt, -1.16);
    SharpWrite(String(distance));
    if (deviceConnected) {
      pDistCharacteristic->setValue((uint8_t*)&distance, 1);
      pDistCharacteristic->notify();
    }
  }
}
void UltrasonRead(){
  distance = sonar.ping_cm();
  UltrasonWrite(distance);
  if (deviceConnected) {
    pDistCharacteristic->setValue((uint8_t*)&distance, 1);
    pDistCharacteristic->notify();
  }
}

void SharpWrite(String toPrint){
  screen.fillScreen(ST77XX_BLACK);
  screen.setCursor(0, 0);
  screen.setTextColor(ST77XX_RED);
  screen.println("SHARP");
  screen.println(toPrint);
  screen.println("TEMPERATURE");
  screen.println(temp);
  delay(1000);
}

void UltrasonWrite(uint8_t toPrint){
  screen.fillScreen(ST77XX_BLACK);
  screen.setCursor(0, 0);
  screen.setTextColor(ST77XX_YELLOW);
  screen.println("ULTRASON");
  screen.println(toPrint);
  screen.println("TEMPERATURE");
  screen.println(temp);
  delay(1000);
}
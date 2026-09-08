#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// =====================================================
// BLE UUIDs
// =====================================================

#define SERVICE_UUID        "12345678-1234-1234-1234-1234567890AB"
#define CHARACTERISTIC_UUID "12345678-1234-1234-1234-1234567890AC"

// =====================================================
// POTENTIOMETER
// =====================================================

const int potPin = 15;

// Maximum resistance of student potentiometer
const float MAX_RESISTANCE = 5000.0;

// =====================================================
// BLE
// =====================================================

BLECharacteristic *resistanceCharacteristic;

bool deviceConnected = false;

// =====================================================
// BLE SERVER CALLBACKS
// =====================================================

class MyServerCallbacks : public BLEServerCallbacks {

  void onConnect(BLEServer *pServer) {

    deviceConnected = true;

    Serial.println();
    Serial.println("================================");
    Serial.println("BLE CONNECTED");
    Serial.println("================================");
  }

  void onDisconnect(BLEServer *pServer) {

    deviceConnected = false;

    Serial.println();
    Serial.println("================================");
    Serial.println("BLE DISCONNECTED");
    Serial.println("================================");

    // Start advertising again
    delay(500);

    pServer->getAdvertising()->start();

    Serial.println("BLE advertising restarted");
  }
};

// =====================================================
// ADC READING
// =====================================================

int readADC() {

  long sum = 0;

  for (int i = 0; i < 20; i++) {

    sum += analogRead(potPin);

    delay(2);
  }

  return sum / 20;
}

// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(115200);

  delay(1000);

  Serial.println();
  Serial.println("================================");
  Serial.println("REMOTE LAB - STUDENT KIT");
  Serial.println("================================");

  // ADC configuration
  analogSetAttenuation(ADC_11db);

  // ---------------------------------------------------
  // BLE INITIALIZATION
  // ---------------------------------------------------

  Serial.println("Starting BLE...");

  BLEDevice::init("RemoteLab Student Kit");

  // Create BLE server
  BLEServer *pServer = BLEDevice::createServer();

  pServer->setCallbacks(new MyServerCallbacks());

  // Create BLE service
  BLEService *pService =
      pServer->createService(SERVICE_UUID);

  // Create characteristic
  resistanceCharacteristic =
      pService->createCharacteristic(
          CHARACTERISTIC_UUID,
          BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_NOTIFY
      );

  // Add notification descriptor
  resistanceCharacteristic->addDescriptor(
      new BLE2902()
  );

  // Initial value
  resistanceCharacteristic->setValue("0");

  // Start service
  pService->start();

  // ---------------------------------------------------
  // BLE ADVERTISING
  // ---------------------------------------------------

  BLEAdvertising *pAdvertising =
      BLEDevice::getAdvertising();

  pAdvertising->addServiceUUID(SERVICE_UUID);

  pAdvertising->setScanResponse(true);

  BLEDevice::startAdvertising();

  Serial.println("BLE STARTED");
  Serial.println("Device Name:");
  Serial.println("RemoteLab Student Kit");
  Serial.println();
  Serial.println("Waiting for Bluetooth connection...");
}

// =====================================================
// LOOP
// =====================================================

void loop() {

  // Read ADC
  int adc = readADC();

  // Convert ADC to resistance
  float resistance =
      ((float)adc * MAX_RESISTANCE) / 4095.0;

  // Limit value
  resistance = constrain(
      resistance,
      0.0,
      MAX_RESISTANCE
  );

  // Print to Serial Monitor
  Serial.print("ADC: ");
  Serial.print(adc);

  Serial.print(" | Resistance: ");
  Serial.print(resistance, 2);

  Serial.print(" ohms");

  Serial.print(" | BLE: ");

  if (deviceConnected) {
    Serial.println("CONNECTED");
  } else {
    Serial.println("WAITING");
  }

  // ---------------------------------------------------
  // SEND RESISTANCE THROUGH BLE
  // ---------------------------------------------------

  if (deviceConnected) {

    char valueString[20];

    dtostrf(
        resistance,
        0,
        2,
        valueString
    );

    resistanceCharacteristic->setValue(valueString);

    resistanceCharacteristic->notify();

    Serial.print("BLE Sent: ");
    Serial.println(valueString);
  }

  // Send every 200 ms
  delay(200);
}
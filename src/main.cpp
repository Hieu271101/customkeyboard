#include <Arduino.h>
#include <NimBLEDevice.h>

#define BTN_PIN 4

static NimBLECharacteristic *inputChar;
static bool connected = false;

/* HID Report Map (Keyboard) */
static const uint8_t hidReportMap[] = {
    0x05, 0x01, // Usage Page (Generic Desktop)
    0x09, 0x06, // Usage (Keyboard)
    0xA1, 0x01, // Collection (Application)

    0x05, 0x07,  // Usage Page (Key Codes)
    0x19, 0x0E0, // Usage Minimum (224)
    0x29, 0x0E7, // Usage Maximum (231)
    0x15, 0x00,
    0x25, 0x01,
    0x75, 0x01,
    0x95, 0x08,
    0x81, 0x02,

    0x95, 0x01,
    0x75, 0x08,
    0x81, 0x01,

    0x95, 0x05,
    0x75, 0x01,
    0x05, 0x08,
    0x19, 0x01,
    0x29, 0x05,
    0x91, 0x02,

    0x95, 0x01,
    0x75, 0x03,
    0x91, 0x01,

    0x95, 0x06,
    0x75, 0x08,
    0x15, 0x00,
    0x25, 0x65,
    0x05, 0x07,
    0x19, 0x00,
    0x29, 0x65,
    0x81, 0x00,

    0xC0};

/* HID Service UUID */
#define HID_SERVICE "1812"
#define REPORT_CHAR "2A4D"
#define REPORT_MAP_CHAR "2A4B"
#define INFO_CHAR "2A4A"
#define HID_CONTROL_POINT_CHAR "2A4C"

void sendKey(uint8_t key)
{
  uint8_t report[8] = {
      0x00, // modifier
      0x00, // reserved
      key,  // keycode
      0x00,
      0x00,
      0x00,
      0x00,
      0x00};

  inputChar->setValue(report, sizeof(report));
  inputChar->notify();

  delay(20);

  // release
  memset(report, 0, sizeof(report));
  inputChar->setValue(report, sizeof(report));
  inputChar->notify();
}

class ServerCallbacks : public NimBLEServerCallbacks
{
  void onConnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo) override
  {
    connected = true;
    Serial.println("Client connected");
  }

  void onDisconnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo, int reason) override
  {
    connected = false;
    Serial.println("Client disconnected");
    NimBLEDevice::startAdvertising();
  }
};

void setupBLE()
{
  Serial.println("BLE RESET...");
  NimBLEDevice::deinit(true);
  delay(800);

  NimBLEDevice::init("ESP32-S3-HID");
  NimBLEDevice::setPower(ESP_PWR_LVL_P9);

  NimBLEServer *server = NimBLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());

  NimBLEService *hid = server->createService(HID_SERVICE);

  // HID Report characteristic (Input Report) — required for keyboard
  inputChar = hid->createCharacteristic(
      REPORT_CHAR,
      NIMBLE_PROPERTY::NOTIFY);

  NimBLEDescriptor *reportRef = inputChar->createDescriptor("2908");
  uint8_t reportRefData[2] = {0x01, 0x01};
  reportRef->setValue(reportRefData, 2);

  // HID Information characteristic — required by HID spec
  uint8_t hidInfoData[4] = {0x11, 0x01, 0x00, 0x03};
  hid->createCharacteristic(INFO_CHAR, NIMBLE_PROPERTY::READ)
      ->setValue(hidInfoData, 4);

  // HID Report Map characteristic — required by HID spec
  hid->createCharacteristic(REPORT_MAP_CHAR, NIMBLE_PROPERTY::READ)
      ->setValue((uint8_t *)hidReportMap, sizeof(hidReportMap));

  // HID Control Point characteristic — required by HID spec
  hid->createCharacteristic(HID_CONTROL_POINT_CHAR, NIMBLE_PROPERTY::WRITE_NR);

  NimBLEAdvertising *adv = NimBLEDevice::getAdvertising();

  adv->addServiceUUID(HID_SERVICE);

  // setAdvertisementType(BLE_HCI_ADV_TYPE_ADV_IND) → setConnectableMode in NimBLE 2.5.0
  adv->setConnectableMode(BLE_GAP_CONN_MODE_UND);
  // setScanResponse(true) → enableScanResponse in NimBLE 2.5.0
  adv->enableScanResponse(true);
  // Set general discoverable mode (advertising flags) - critical for visibility
  adv->setDiscoverableMode(BLE_GAP_DISC_MODE_GEN);
  // Set appearance as HID Keyboard (961) - helps identification
  adv->setAppearance(961);

  adv->setMinInterval(0x20);
  adv->setMaxInterval(0x40);

  // Small delay to let BLE stack settle before advertising
  delay(50);

  Serial.println("START ADV...");
  adv->start();
  Serial.println("ADV SHOULD BE VISIBLE NOW");
}

bool lastState = HIGH;

void setup()
{
  Serial.begin(115200);

  pinMode(BTN_PIN, INPUT_PULLUP);

  setupBLE();
}

void loop()
{
  bool state = digitalRead(BTN_PIN);

  if (connected)
  {
    if (lastState == HIGH && state == LOW)
    {
      Serial.println("Send A");
      sendKey(0x04); // HID keycode for 'A'
    }
  }

  lastState = state;
  delay(10);
}
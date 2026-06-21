---
name: esp32-nimble-hid-keyboard
description: ESP32-S3 NimBLE BLE HID Keyboard project skill. Use this skill whenever working on this custom keyboard project — editing main.cpp, debugging BLE advertising/visibility, modifying HID reports, adding key bindings, or making any changes to the PlatformIO/Arduino/NimBLE firmware. Also use when troubleshooting Bluetooth discovery issues with nRF Connect, or when the user mentions BLE, HID, keyboard, NimBLE, ESP32-S3, or Bluetooth not visible/discoverable.
---

# ESP32-S3 NimBLE HID Keyboard Skill

## Project Overview

Custom Bluetooth Low Energy (BLE) HID Keyboard running on ESP32-S3 DevKitM-1 using NimBLE-Arduino library. A physical button on GPIO 4 sends HID keypress 'A' to connected devices.

**Hardware:** ESP32-S3 DevKitM-1, button on GPIO 4 (INPUT_PULLUP), active-low (press = LOW)

**Key files:**
- `src/main.cpp` — All firmware code (single file project)
- `platformio.ini` — Build configuration (board: esp32-s3-devkitm-1, framework: arduino)
- `.github/workflows/build.yml` — CI build workflow
- `.agents/skills/esp32-nimble-hid-keyboard/SKILL.md` — This skill

## NimBLE 2.5.0 API — Critical Version-Specific Facts

This project uses **NimBLE-Arduino 2.5.0**. Many older tutorials, ChatGPT answers, and even the original task guide reference APIs that **DO NOT EXIST** in this version. Here are the exact mappings:

### Methods that DO NOT EXIST in NimBLE 2.5.0

| ❌ Old/Invalid API | ✅ NimBLE 2.5.0 Equivalent |
|---|---|
| `adv->setAdvertisementType(BLE_HCI_ADV_TYPE_ADV_IND)` | `adv->setConnectableMode(BLE_GAP_CONN_MODE_UND)` |
| `adv->setAdvertisementType(ADV_TYPE_IND)` | `adv->setConnectableMode(BLE_GAP_CONN_MODE_UND)` |
| `adv->setScanResponse(true)` | `adv->enableScanResponse(true)` |
| `NimBLEDevice::setPower(ESP_PWR_LVL_P9, ...)` (multi-arg) | `NimBLEDevice::setPower(ESP_PWR_LVL_P9)` (single arg) |
| `NimBLEService::start()` | Deprecated/no-op — services start automatically with server |

### Advertising API Available in 2.5.0

```
adv->addServiceUUID(uuid)           // ✅ Add service UUID to advertisement
adv->setConnectableMode(mode)       // ✅ BLE_GAP_CONN_MODE_UND (undirected connectable)
adv->enableScanResponse(bool)       // ✅ Enable/disable scan response data
adv->setDiscoverableMode(mode)      // ✅ BLE_GAP_DISC_MODE_GEN (general discoverable)
adv->setAppearance(uint16_t)        // ✅ 961 = HID Keyboard
adv->setMinInterval(uint16_t)       // ✅ Min advertising interval
adv->setMaxInterval(uint16_t)       // ✅ Max advertising interval
adv->start()                        // ✅ Start advertising
```

### ServerCallbacks API in 2.5.0

```cpp
class ServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo) override { }
  void onDisconnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo, int reason) override { }
};
```

Note: `onConnect`/`onDisconnect` take `NimBLEConnInfo&` (not `uint16_t connId`).

## BLE Advertising — Visibility Fixes

The ESP32-S3 can fail to appear in BLE scanners (nRF Connect) without these critical settings in `setupBLE()`:

### 1. BLE Stack Reset (Required)
```cpp
NimBLEDevice::deinit(true);  // Force full BLE stack reset
delay(800);                   // Wait for radio to settle
NimBLEDevice::init("ESP32-S3-HID");
NimBLEDevice::setPower(ESP_PWR_LVL_P9);  // Set power RIGHT AFTER init
```

### 2. Advertising Configuration (All Required)
```cpp
NimBLEAdvertising *adv = NimBLEDevice::getAdvertising();
adv->addServiceUUID(HID_SERVICE);                    // Include HID service UUID
adv->setConnectableMode(BLE_GAP_CONN_MODE_UND);      // Connectable + scannable
adv->enableScanResponse(true);                        // Respond to active scans
adv->setDiscoverableMode(BLE_GAP_DISC_MODE_GEN);     // General discoverable flags
adv->setAppearance(961);                              // HID Keyboard appearance
adv->setMinInterval(0x20);                           // ~20ms min interval
adv->setMaxInterval(0x40);                           // ~40ms max interval
delay(50);                                            // Let stack settle
adv->start();
```

### 3. Why These Matter (Root Causes)
- **Missing `setConnectableMode`**: Device broadcasts but isn't connectable — scanners see packets but don't list it
- **Missing `setDiscoverableMode`**: Advertising flags lack "General Discoverable" bit — some scanners filter these out
- **Missing `enableScanResponse`**: Active scans get no response — device appears and immediately disappears
- **Missing `deinit(true)`**: Previous BLE session leaves stale state — new advertising starts on bad state
- **Power set too late**: Setting power during advertise phase causes incorrect packet timing

## HID Report Map

The keyboard uses a standard 8-byte HID report:
- Byte 0: Modifier keys (Ctrl, Shift, Alt, GUI — 8 bits)
- Byte 1: Reserved (0x00)
- Bytes 2-7: Keycodes (up to 6 simultaneous keys)
- Report ID 0x01 with Report Reference Descriptor (0x2908)

### Keycode Lookup (HID Usage Tables)
```
A=0x04  B=0x05  C=0x06  D=0x07  E=0x08  F=0x09  G=0x0A
H=0x0B  I=0x0C  J=0x0D  K=0x0E  L=0x0F  M=0x10  N=0x11
O=0x12  P=0x13  Q=0x14  R=0x15  S=0x16  T=0x17  U=0x18
V=0x19  W=0x1A  X=0x1B  Y=0x1C  Z=0x1D
0=0x1E  1=0x1F  2=0x20  3=0x21  4=0x22  5=0x23
6=0x24  7=0x25  8=0x26  9=0x27
ENTER=0x28  ESC=0x29  BACKSPACE=0x2A  TAB=0x2B
SPACE=0x2C  MINUS=0x2D  EQUAL=0x2E  LBRACKET=0x2F
RBRACKET=0x30  BACKSLASH=0x31  SEMICOLON=0x33
APOSTROPHE=0x34  GRAVE=0x35  COMMA=0x36  PERIOD=0x37
SLASH=0x38  CAPSLOCK=0x39  F1=0x3A  F2=0x3B ... F12=0x45
```

### Modifier Byte Values
```
Left Ctrl=0x01  Left Shift=0x02  Left Alt=0x04  Left GUI=0x08
Right Ctrl=0x10  Right Shift=0x20  Right Alt=0x40  Right GUI=0x80
```

**Example: Ctrl+C** → modifier=0x01, keycode=0x06
**Example: Shift+A** → modifier=0x02, keycode=0x04

## sendKey() Pattern

```cpp
void sendKey(uint8_t key) {
  uint8_t report[8] = {0x00, 0x00, key, 0x00, 0x00, 0x00, 0x00, 0x00};
  inputChar->setValue(report, sizeof(report));
  inputChar->notify();
  delay(20);
  // Key release
  memset(report, 0, sizeof(report));
  inputChar->setValue(report, sizeof(report));
  inputChar->notify();
}
```

Always send key-down then key-up (release) with a small delay between.

## BLE Service UUIDs

| UUID | Name |
|------|------|
| 0x1812 | HID Service |
| 0x2A4D | HID Report Characteristic |
| 0x2A4B | HID Report Map Characteristic |
| 0x2A4A | HID Information Characteristic |
| 0x2A4C | HID Control Point Characteristic |
| 0x2908 | Report Reference Descriptor |

## build_command

```bash
cd /Users/123a/Documents/PlatformIO/Projects/keyboard && pio run
```

Or for upload:
```bash
cd /Users/123a/Documents/PlatformIO/Projects/keyboard && pio run --target upload
```

## Common Pitfalls

1. **Compilation: "taking address of temporary array"** — ESP32-S3 compiler rejects `(uint8_t[]){...}` compound literals. Use named arrays: `uint8_t data[4] = {0x11, 0x01, 0x00, 0x03};`
2. **Compilation: implicit 'this' capture in lambda** — Use explicit capture: `[this](NimBLEAdvertising *adv)` instead of `[&]`
3. **Deprecated: `hid->start()`** — Services start automatically with server in NimBLE 2.5.0; `hid->start()` compiles but emits warning
4. **Device not visible in nRF Connect** — See "BLE Advertising — Visibility Fixes" section above
5. **Device visible but can't connect** — Missing `setConnectableMode(BLE_GAP_CONN_MODE_UND)`
6. **NimBLECharacteristic::setValue(const NimBLEAttValue&)** — In newer NimBLE, use `setValue(data, len)` with explicit pointer + size, not `setValue(attValue)`
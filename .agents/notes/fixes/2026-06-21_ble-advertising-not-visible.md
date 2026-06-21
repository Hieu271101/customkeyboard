# Fix: BLE Advertising Not Visible in nRF Connect

**Date**: 2026-06-21
**Context**: ESP32-S3 NimBLE HID keyboard not discoverable via nRF Connect app
**Severity**: Critical — device completely invisible to BLE scanners

## Problem
The ESP32-S3 was not showing up in nRF Connect or other BLE scanner apps. The original code only called `addServiceUUID()` and `start()` on the advertising object, which is insufficient.

## Root Cause
Four missing configuration steps:
1. No `setConnectableMode(BLE_GAP_CONN_MODE_UND)` — device not marked as connectable
2. No `enableScanResponse(true)` — device doesn't respond to active scans
3. No `setDiscoverableMode(BLE_GAP_DISC_MODE_GEN)` — missing general discoverable flags in advertising packet
4. No `NimBLEDevice::deinit(true)` before init — stale BLE stack state

## Solution
```cpp
void setupBLE()
{
  NimBLEDevice::deinit(true);  // Force reset BLE stack
  delay(800);

  NimBLEDevice::init("ESP32-S3-HID");
  NimBLEDevice::setPower(ESP_PWR_LVL_P9);  // Max power, set right after init

  NimBLEServer *server = NimBLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());

  NimBLEService *hid = server->createService(HID_SERVICE);
  // ... create characteristics ...

  NimBLEAdvertising *adv = NimBLEDevice::getAdvertising();
  adv->addServiceUUID(HID_SERVICE);
  adv->setConnectableMode(BLE_GAP_CONN_MODE_UND);    // Make connectable
  adv->enableScanResponse(true);                      // Respond to scans
  adv->setDiscoverableMode(BLE_GAP_DISC_MODE_GEN);    // General discoverable flags
  adv->setAppearance(961);                            // HID Keyboard appearance
  adv->setMinInterval(0x20);
  adv->setMaxInterval(0x40);

  delay(50);  // Let BLE stack settle
  adv->start();
}
```

## NimBLE 2.5.0 API Mappings
| Old API (pre-2.5.0) | NimBLE 2.5.0 API |
|---|---|
| `setAdvertisementType(ADV_TYPE_IND)` | `setConnectableMode(BLE_GAP_CONN_MODE_UND)` |
| `setScanResponse(true)` | `enableScanResponse(true)` |
| `service->start()` | Deprecated/no-op, removed |

## Prevention
- Always include all four advertising configuration calls
- Set power level immediately after `init()`, not during advertising phase
- Use `deinit(true)` before `init()` to clear stale state
- Add small delay (50ms) before `adv->start()` to let stack settle

## Verification
1. Upload firmware to ESP32-S3
2. Open nRF Connect app
3. Scan for BLE devices
4. "ESP32-S3-HID" should appear as a connectable device
5. Connect and verify HID service (0x1812) is visible

"""
Relay: ESP32 button (GPIO4) → Android emulator key via ADB.
Reads COM3 serial output and fires ADB keyevents on button press.
"""

import serial
import subprocess
import sys
import time

SERIAL_PORT = "COM3"
BAUD_RATE   = 115200
ADB_DEVICE  = "emulator-5554"

# Android keycodes to send per trigger word
# KEYCODE_ENTER=66  → j2me-loader KEY_FIRE (-5) → center/fire/OK in game canvases
TRIGGERS = {
    "Send OK": "66",      # KEY_FIRE = OK / fire / center in J2ME games
}


def adb_keyevent(keycode: str):
    subprocess.Popen(
        ["adb", "-s", ADB_DEVICE, "shell", "input", "keyevent", keycode],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )


def open_serial():
    """Open COM port, wait for ESP32 boot to settle after USB-CDC reset."""
    ser = serial.Serial(
        SERIAL_PORT, BAUD_RATE,
        timeout=1,
        dsrdtr=False,
        rtscts=False,
    )
    ser.dtr = False
    ser.rts = False
    time.sleep(2)           # let ESP32 finish USB-CDC reset / boot
    ser.reset_input_buffer()
    return ser


def main():
    print(f"Opening {SERIAL_PORT} @ {BAUD_RATE} baud …")
    ser = None
    try:
        ser = open_serial()
    except serial.SerialException as e:
        print(f"ERROR: {e}")
        print("Close PlatformIO monitor first (only one app can own the COM port).")
        sys.exit(1)

    print(f"Relaying to ADB device: {ADB_DEVICE}")
    print("Press Ctrl+C to stop.\n")

    try:
        while True:
            try:
                raw = ser.readline()
                if not raw:
                    continue
                line = raw.decode("utf-8", errors="ignore").strip()
                if line:
                    print(f"  serial: {line}")
                for trigger, keycode in TRIGGERS.items():
                    if trigger in line:
                        print(f"  → adb keyevent {keycode} ({trigger})")
                        adb_keyevent(keycode)
                        break
            except serial.SerialException as e:
                err = str(e)
                if "ClearCommError" in err:
                    # ESP32-S3 USB-CDC glitch — don't reopen, just retry
                    time.sleep(0.1)
                    continue
                print(f"Serial error: {e}. Reconnecting in 3 s …")
                try:
                    ser.close()
                except Exception:
                    pass
                time.sleep(3)
                try:
                    ser = open_serial()
                    print("Reconnected.")
                except serial.SerialException as e2:
                    print(f"Reconnect failed: {e2}")
    except KeyboardInterrupt:
        print("\nStopped.")
    finally:
        if ser and ser.is_open:
            ser.close()


if __name__ == "__main__":
    main()

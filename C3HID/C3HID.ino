#include <NimBLEDevice.h>
#include <NimBLEHIDDevice.h>
#include <NimBLEUtils.h>
#include <driver/uart.h>

// HID report descriptor for keyboard and mouse
static const uint8_t hidReportDescriptor[] = {
    // Mouse
    0x05, 0x01, // Usage Page (Generic Desktop Ctrls)
    0x09, 0x02, // Usage (Mouse)
    0xA1, 0x01, // Collection (Application)
    0x09, 0x01, //   Usage (Pointer)
    0xA1, 0x00, //   Collection (Physical)
    0x85, 0x01, //     Report ID (1)
    0x05, 0x09, //     Usage Page (Button)
    0x19, 0x01, //     Usage Minimum (0x01)
    0x29, 0x03, //     Usage Maximum (0x03)
    0x15, 0x00, //     Logical Minimum (0)
    0x25, 0x01, //     Logical Maximum (1)
    0x95, 0x03, //     Report Count (3)
    0x75, 0x01, //     Report Size (1)
    0x81, 0x02, //     Input (Data,Var,Abs)
    0x95, 0x01, //     Report Count (1)
    0x75, 0x05, //     Report Size (5)
    0x81, 0x03, //     Input (Const,Var,Abs)
    0x05, 0x01, //     Usage Page (Generic Desktop Ctrls)
    0x09, 0x30, //     Usage (X)
    0x09, 0x31, //     Usage (Y)
    0x09, 0x38, //     Usage (Wheel)
    0x15, 0x81, //     Logical Minimum (-127)
    0x25, 0x7F, //     Logical Maximum (127)
    0x75, 0x08, //     Report Size (8)
    0x95, 0x03, //     Report Count (3)
    0x81, 0x06, //     Input (Data,Var,Rel)
    0xC0,       //   End Collection
    0xC0,       // End Collection

    // Keyboard
    0x05, 0x01, // Usage Page (Generic Desktop Ctrls)
    0x09, 0x06, // Usage (Keyboard)
    0xA1, 0x01, // Collection (Application)
    0x85, 0x02, //   Report ID (2)
    0x05, 0x07, //   Usage Page (Kbrd/Keypad)
    0x19, 0xE0, //   Usage Minimum (0xE0)
    0x29, 0xE7, //   Usage Maximum (0xE7)
    0x15, 0x00, //   Logical Minimum (0)
    0x25, 0x01, //   Logical Maximum (1)
    0x75, 0x01, //   Report Size (1)
    0x95, 0x08, //   Report Count (8)
    0x81, 0x02, //   Input (Data,Var,Abs)
    0x95, 0x01, //   Report Count (1)
    0x75, 0x08, //   Report Size (8)
    0x81, 0x03, //   Input (Const,Var,Abs)
    0x95, 0x06, //   Report Count (6)
    0x75, 0x08, //   Report Size (8)
    0x15, 0x00, //   Logical Minimum (0)
    0x25, 0x65, //   Logical Maximum (101)
    0x05, 0x07, //   Usage Page (Kbrd/Keypad)
    0x19, 0x00, //   Usage Minimum (0x00)
    0x29, 0x65, //   Usage Maximum (0x65)
    0x81, 0x00, //   Input (Data,Array)
    0xC0        // End Collection
};

NimBLEHIDDevice* hid;
NimBLECharacteristic* inputMouse;
NimBLECharacteristic* inputKeyboard;

void setup() {
    Serial.begin(115200);
    Serial.println("Starting BLE work!");

    NimBLEDevice::init("ESP32C3 Supermini MK");
    NimBLEServer* pServer = NimBLEDevice::createServer();

    hid = new NimBLEHIDDevice(pServer);
    inputMouse = hid->inputReport(1); // report ID 1
    inputKeyboard = hid->inputReport(2); // report ID 2

    hid->manufacturer()->setValue("MyCompany");
    hid->pnp(0x02, 0x1234, 0x5678, 0x0110);
    hid->hidInfo(0x00, 0x01);

    hid->reportMap((uint8_t*)hidReportDescriptor, sizeof(hidReportDescriptor));
    hid->startServices();
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->setAppearance(HID_KEYBOARD);
    pAdvertising->addServiceUUID(hid->hidService()->getUUID());
    pAdvertising->start();
    hid->setBatteryLevel(7);
}

void loop() {
    if (Serial.available()) {
        uint8_t identifier = Serial.read(); // Read the identifier first

        if (identifier == 0x01) {
            // Mouse data
            uint8_t buffer[4];
            Serial.readBytes(buffer, 4);

            // Print received data for debugging
            // Serial.print("Received mouse data: ");
            // Serial.print(identifier, HEX);
            // Serial.print(" ");
            // for (int i = 0; i < 4; i++) {
            //     Serial.print(buffer[i], HEX);
            //     Serial.print(" ");
            // }
            // Serial.println();

            int8_t mouseMoveX = buffer[1];
            int8_t mouseMoveY = buffer[2];
            int8_t scrollY = buffer[3];
            uint8_t buttons = buffer[0];

            uint8_t mouseReport[5] = { buttons, mouseMoveX, mouseMoveY, scrollY, 0 };
            inputMouse->setValue(mouseReport, sizeof(mouseReport));
            inputMouse->notify();
        } else if (identifier == 0x02) {
            // Keyboard data
            uint8_t buffer[7];
            Serial.readBytes(buffer, 7);

            // Print received data for debugging
            // Serial.print("Received keyboard data: ");
            // Serial.print(identifier, HEX);
            // Serial.print(" ");
            // for (int i = 0; i < 7; i++) {
            //     Serial.print(buffer[i], HEX);
            //     Serial.print(" ");
            // }
            // Serial.println();

            uint8_t modifiers = buffer[0];
            uint8_t keys[6] = { buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6] };

            uint8_t keyboardReport[8] = { modifiers, 0, keys[0], keys[1], keys[2], keys[3], keys[4], keys[5] };
            inputKeyboard->setValue(keyboardReport, sizeof(keyboardReport));
            inputKeyboard->notify();
        }
    }
}

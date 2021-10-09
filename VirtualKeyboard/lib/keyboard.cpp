#include "Arduino.h"
#include "Adafruit_TinyUSB.h"

// HID report descriptor using TinyUSB's template
// Single Report (no ID) descriptor
// HID report descriptor using TinyUSB's template
uint8_t const desc_hid_report[] = {
    TUD_HID_REPORT_DESC_KEYBOARD()};
Adafruit_USBD_HID usb_hid;

bool hasKeyPressed = false;

void wakeupUSB();

void setup()
{
    Serial.begin(115200);

    usb_hid.setPollInterval(2);
    usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
    usb_hid.begin();

    // wait until device mounted
    while (!USBDevice.mounted())
        delay(1);

    Serial.println("USB Device Ready!");
}

void keyboardAction()
{
    // Only send KeyRelease if previously pressed to avoid sending
    // multiple keyRelease reports (that consume memory and bandwidth)
    if (hasKeyPressed)
    {
        hasKeyPressed = false;
        usb_hid.keyboardRelease(0);
        // Delay a bit after a report
        delay(5);
    }

    if (!Serial.available())
        return;

    char ch = (char)Serial.read();

    // echo
    Serial.write(ch);

    if (ch == '\n')
        return;

    wakeupUSB();

    if (!usb_hid.ready())
        return;

    usb_hid.keyboardPress(0, ch);
    hasKeyPressed = true;

    // Delay a bit after a report
    delay(5);
}

void loop()
{
    delay(5);
    keyboardAction();
}

void wakeupUSB()
{
    // Remote wakeup
    if (USBDevice.suspended())
    {
        // Wake up host if we are in suspend mode
        // and REMOTE_WAKEUP feature is enabled by host
        USBDevice.remoteWakeup();
    }
}
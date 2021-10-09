#include "Arduino.h"
#include "Adafruit_TinyUSB.h"

const int pin = BUTTON_MODE; // UserSw
bool activeState = false;

// Report ID
enum
{
    RID_KEYBOARD = 1,
    RID_MOUSE
};

// HID report descriptor using TinyUSB's template
// Single Report (no ID) descriptor
// HID report descriptor using TinyUSB's template
uint8_t const desc_hid_report[] =
    {
        TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(RID_KEYBOARD), ),
        TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(RID_MOUSE), ),
};
Adafruit_USBD_HID usb_hid;

bool hasKeyPressed = false;

void wakeupUSB();

void setup()
{
    pinMode(pin, activeState ? INPUT_PULLDOWN : INPUT_PULLUP);

    usb_hid.setPollInterval(2);
    usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
    usb_hid.begin();

    Serial.begin(115200);
    while (!Serial)
        yield();

    // wait until device mounted
    while (!USBDevice.mounted())
        delay(1);

    Serial.println("USB Device Ready!");
}

void mouseAction()
{
    // nothing to do if button is not pressed
    if (digitalRead(pin) != activeState)
        return;

    wakeupUSB();

    if (!usb_hid.ready())
        return;

    int8_t const delta = 5;
    usb_hid.mouseMove(RID_MOUSE, delta, delta); // no ID: right + down
}

void keyboardAction()
{
    // Only send KeyRelease if previously pressed to avoid sending
    // multiple keyRelease reports (that consume memory and bandwidth)
    if (hasKeyPressed)
    {
        hasKeyPressed = false;
        usb_hid.keyboardRelease(RID_KEYBOARD);
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

    usb_hid.keyboardPress(RID_KEYBOARD, ch);
    hasKeyPressed = true;

    // Delay a bit after a report
    delay(5);
}

void loop()
{
    delay(5);
    mouseAction();
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
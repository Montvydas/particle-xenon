#include "Arduino.h"
#include "Adafruit_TinyUSB.h"

const int pin = BUTTON_MODE; // UserSw
bool activeState = false;

// HID report descriptor using TinyUSB's template
// Single Report (no ID) descriptor
uint8_t const desc_hid_report[] =
    {
        TUD_HID_REPORT_DESC_MOUSE()};

// USB HID object
Adafruit_USBD_HID usb_hid;

// the setup function runs once when you press reset or power the board
void setup()
{
  Serial.begin(115200);

  // Set up button, pullup opposite to active state
  pinMode(pin, activeState ? INPUT_PULLDOWN : INPUT_PULLUP);

  usb_hid.setPollInterval(2);
  usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
  usb_hid.begin();

  // wait until device mounted
  while (!USBDevice.mounted())
    delay(1);

  Serial.println("Adafruit TinyUSB HID Mouse example");
}

void loop()
{
  // poll gpio once each 10 ms
  delay(10);

  // Whether button is pressed
  bool btn_pressed = (digitalRead(pin) == activeState);

  // nothing to do if button is not pressed
  if (!btn_pressed)
    return;

  // Remote wakeup
  if (USBDevice.suspended())
  {
    // Wake up host if we are in suspend mode
    // and REMOTE_WAKEUP feature is enabled by host
    USBDevice.remoteWakeup();
  }

  if (usb_hid.ready())
  {
    int8_t const delta = 5;
    usb_hid.mouseMove(0, delta, delta); // no ID: right + down
  }
}

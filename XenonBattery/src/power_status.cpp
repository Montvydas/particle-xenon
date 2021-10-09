#include <Arduino.h>

void switch_callback(void)
{
  digitalToggle(LED_RED);
}

void setup()
{
  Serial.begin(115200);

  pinMode(PIN_PWR, INPUT);
  pinMode(PIN_CHG, INPUT);

  pinMode(LED_RED, OUTPUT);
  pinMode(BUTTON_MODE, INPUT_PULLUP);   // Configure the switch pin as an input with internal pull-up register enabled.
  attachInterrupt(BUTTON_MODE, switch_callback, FALLING);
  suspendLoop();

  while (!Serial) delay(10);   // for nrf52840 with native usb
}

void loop()
{
  char buf[128];
  snprintf(buf, sizeof(buf), "PWR=%d CHG=%d", digitalRead(PIN_PWR), digitalRead(PIN_CHG));
  Serial.println(buf);

  bool hasUsbPower = digitalRead(PIN_PWR);
  bool isCharging = (hasUsbPower && !digitalRead(PIN_CHG));
  bool onBatteryPower = !hasUsbPower;
}

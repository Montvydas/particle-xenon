#include "Arduino.h"
#include "Adafruit_TinyUSB.h"
#include <bluefruit.h>

BLEDis  bledis;  // device information
BLEDfu  bledfu;  // OTA DFU service
BLEHidAdafruit blehid; // BLE keyboard
BLEUart bleuart;       // uart over ble

uint8_t const desc_hid_report[] = {TUD_HID_REPORT_DESC_KEYBOARD()};
Adafruit_USBD_HID usb_hid; // USB keyboard

bool hasKeyPressed = false;

// ------------------------------- signatures ------------------------
void setupUsbKeyboard(void);
void wakeupUSB(void);
void hidReportCallback(uint8_t, hid_report_type_t, uint8_t const *, uint16_t);
void setupBleKeyboard(void);
void startAdvertising(void);
void setKeyboardLed(uint16_t, uint8_t);
void connect_callback(uint16_t);
void disconnect_callback(uint16_t, uint8_t);
// ------------------------------- END -------------------------------

void setup()
{
    Serial.begin(115200);

    setupUsbKeyboard();
    setupBleKeyboard();

    Serial.println("BLE and USB Keyboards Ready!");
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
    // delay(5);
    // keyboardAction();

    while (bleuart.available())
    {
        uint8_t ch;
        ch = (uint8_t)bleuart.read();
        Serial.write(ch);
    }
}

void setupUsbKeyboard(void)
{
    usb_hid.setPollInterval(2);
    usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
    usb_hid.setReportCallback(NULL, hidReportCallback);
    usb_hid.begin();

    // wait until device mounted
    while (!USBDevice.mounted())
        delay(1);
}

// Output report callback for LED indicator such as Caplocks
void hidReportCallback(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize)
{
    // LED indicator is output report with only 1 byte length
    if (report_type != HID_REPORT_TYPE_OUTPUT)
        return;

    // The LED bit map is as follows: (also defined by KEYBOARD_LED_* )
    // Kana (4) | Compose (3) | ScrollLock (2) | CapsLock (1) | Numlock (0)
    uint8_t ledIndicator = buffer[0];

    // turn on LED if caplock is set
    digitalWrite(LED_BUILTIN, ledIndicator & KEYBOARD_LED_CAPSLOCK);
}

void wakeupUSB(void)
{
    // Remote wakeup
    if (USBDevice.suspended())
    {
        // Wake up host if we are in suspend mode
        // and REMOTE_WAKEUP feature is enabled by host
        USBDevice.remoteWakeup();
    }
}

void setupBleKeyboard(void)
{
    // Setup the BLE LED to be disabled on CONNECT as Xenon only has one LED
    // And we need want to use it to report transmission
    Bluefruit.autoConnLed(false);
    // Config the peripheral connection with maximum bandwidth
    Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);

    Bluefruit.begin();
    Bluefruit.setTxPower(4); // Check bluefruit.h for supported values
    Bluefruit.setName("BLE-USB-Keyboard");

    Bluefruit.Periph.setConnectCallback(connect_callback);
    Bluefruit.Periph.setDisconnectCallback(disconnect_callback);

    // To be consistent OTA DFU should be added first if it exists
    bledfu.begin();

    // Configure and Start Device Information Service
    bledis.setManufacturer("Adafruit Industries");
    bledis.setModel("Bluefruit Feather52");
    bledis.begin();

    // Configure and Start BLE Uart Service
    bleuart.begin();

    /* Start BLE HID
   * Note: Apple requires BLE device must have min connection interval >= 20m
   * ( The smaller the connection interval the faster we could send data).
   * However for HID and MIDI device, Apple could accept min connection interval 
   * up to 11.25 ms. Therefore BLEHidAdafruit::begin() will try to set the min and max
   * connection interval to 11.25  ms and 15 ms respectively for best performance.
   */
    blehid.begin();

    // Set callback for set LED from central
    blehid.setKeyboardLedCallback(setKeyboardLed);

    /* Set connection interval (min, max) to your perferred value.
   * Note: It is already set by BLEHidAdafruit::begin() to 11.25ms - 15ms
   * min = 9*1.25=11.25 ms, max = 12*1.25= 15 ms 
   */
    /* Bluefruit.Periph.setConnInterval(9, 12); */

    // Set up and start advertising
    startAdvertising();
}

void startAdvertising(void)
{
    // Advertising packet
    Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
    Bluefruit.Advertising.addTxPower();
    Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_HID_KEYBOARD);

    // Include bleuart 128-bit uuid
    Bluefruit.Advertising.addService(bleuart);

    // Include BLE HID service
    Bluefruit.Advertising.addService(blehid);

    // There is enough room for the dev name in the advertising packet
    // Bluefruit.Advertising.addName();
    // Secondary Scan Response packet (optional)
    // Since there is no room for 'Name' in Advertising packet
    Bluefruit.ScanResponse.addName();

    /* Start Advertising
   * - Enable auto advertising if disconnected
   * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
   * - Timeout for fast mode is 30 seconds
   * - Start(timeout) with timeout = 0 will advertise forever (until connected)
   * 
   * For recommended advertising interval
   * https://developer.apple.com/library/content/qa/qa1931/_index.html   
   */
    Bluefruit.Advertising.restartOnDisconnect(true);
    Bluefruit.Advertising.setInterval(32, 244); // in unit of 0.625 ms
    Bluefruit.Advertising.setFastTimeout(30);   // number of seconds in fast mode
    Bluefruit.Advertising.start(0);             // 0 = Don't stop advertising after n seconds
}

void setKeyboardLed(uint16_t conn_handle, uint8_t led_bitmap)
{
    (void)conn_handle;

    // light up Red Led if any bits is set
    if (led_bitmap)
    {
        ledOn(LED_RED);
    }
    else
    {
        ledOff(LED_RED);
    }
}

// callback invoked when central connects
void connect_callback(uint16_t conn_handle)
{
    // Get the reference to current connection
    BLEConnection *connection = Bluefruit.Connection(conn_handle);

    char central_name[32] = {0};
    connection->getPeerName(central_name, sizeof(central_name));

    Serial.print("Connected to ");
    Serial.println(central_name);
}

/**
 * Callback invoked when a connection is dropped
 * @param conn_handle connection where this event happens
 * @param reason is a BLE_HCI_STATUS_CODE which can be found in ble_hci.h
 */
void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
    (void)conn_handle;
    (void)reason;

    Serial.println();
    Serial.print("Disconnected, reason = 0x");
    Serial.println(reason, HEX);
}

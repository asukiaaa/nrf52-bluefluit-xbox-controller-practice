#include <Arduino.h>

// https://github.com/adafruit/Adafruit_nRF52_Arduino/blob/master/libraries/Bluefruit52Lib/examples/Central/central_scan/central_scan.ino
// https://github.com/adafruit/Adafruit_nRF52_Arduino/blob/master/libraries/Bluefruit52Lib/examples/Central/central_scan_advanced/central_scan_advanced.ino
// https://github.com/adafruit/Adafruit_nRF52_Arduino/blob/master/libraries/Bluefruit52Lib/examples/Central/central_custom_hrm/central_custom_hrm.ino
// https://github.com/adafruit/Adafruit_nRF52_Arduino/blob/master/libraries/Bluefruit52Lib/examples/Central/central_hid/central_hid.ino

#define EXAMPLE_SCAN_ADVANCE
#ifdef EXAMPLE_SCAN_ADVANCE
/*********************************************************************
 This is an example for our nRF52 based Bluefruit LE modules

 Pick one up today in the adafruit shop!

 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/

#include <bluefruit.h>

/* For a list of EIR data types see:
 *    https://www.bluetooth.com/specifications/assigned-numbers/generic-access-profile
 *    Matching enum: cores/nRF5/SDK/components/softdevice/s132/headers/ble_gap.h
 */

// BLEClientService        hrms(UUID16_SVC_HEART_RATE);
// BLEClientCharacteristic hrmc(UUID16_CHR_HEART_RATE_MEASUREMENT);
// BLEClientCharacteristic bslc(UUID16_CHR_BODY_SENSOR_LOCATION);

BLEClientService serviceBattery(UUID16_SVC_BATTERY);
BLEClientCharacteristic charaBattery(UUID16_CHR_BATTERY_LEVEL);
BLEClientService serviceHid(UUID16_SVC_HUMAN_INTERFACE_DEVICE);
BLEClientCharacteristic charaHidInfo(UUID16_CHR_HID_INFORMATION);
BLEClientCharacteristic charaReport(UUID16_CHR_REPORT);
BLEClientCharacteristic charaReportMap(UUID16_CHR_REPORT_MAP);

BLEClientHidAdafruit hidAdafruit;

void printUuid16List(uint8_t* buffer, uint8_t len) {
  Serial.printf("%14s ", "16-Bit UUID");
  for (int i = 0; i < len; i += 2) {
    uint16_t uuid16;
    memcpy(&uuid16, buffer + i, 2);
    Serial.printf("%04X ", uuid16);
  }
  // Serial.printf(" len%d ", len);
  // for (int i = 0; i < len; ++i) {
  //   Serial.print(buffer[i]);
  //   Serial.print(" ");
  // }
  Serial.println();
}

void printUuid128List(uint8_t* buffer, uint8_t len) {
  (void)len;
  Serial.printf("%14s %s", "128-Bit UUID");

  // Print reversed order
  for (int i = 0; i < 16; i++) {
    const char* fm = (i == 4 || i == 6 || i == 8 || i == 10) ? "-%02X" : "%02X";
    Serial.printf(fm, buffer[15 - i]);
  }

  Serial.println();
}

void scan_callback(ble_gap_evt_adv_report_t* report) {
  PRINT_LOCATION();
  Bluefruit.Central.connect(report);
  return;

  uint8_t len = 0;
  uint8_t buffer[32];
  memset(buffer, 0, sizeof(buffer));

  // if (!Bluefruit.Scanner.parseReportByType(report,
  //                                          BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME,
  //                                          buffer, sizeof(buffer))) {
  //   Bluefruit.Scanner.resume();
  //   return;
  // }

  /* Display the timestamp and device address */
  if (report->type.scan_response) {
    Serial.printf("[SR%10d] Packet received from ", millis());
  } else {
    Serial.printf("[ADV%9d] Packet received from ", millis());
  }
  // MAC is in little endian --> print reverse
  Serial.printBufferReverse(report->peer_addr.addr, 6, ':');
  Serial.print("\n");

  /* Raw buffer contents */
  Serial.printf("%14s %d bytes\n", "PAYLOAD", report->data.len);
  if (report->data.len) {
    Serial.printf("%15s", " ");
    Serial.printBuffer(report->data.p_data, report->data.len, '-');
    Serial.println();
  }

  /* RSSI value */
  Serial.printf("%14s %d dBm\n", "RSSI", report->rssi);

  /* Adv Type */
  Serial.printf("%14s ", "ADV TYPE");
  if (report->type.connectable) {
    Serial.print("Connectable ");
  } else {
    Serial.print("Non-connectable ");
  }

  if (report->type.directed) {
    Serial.println("directed");
  } else {
    Serial.println("undirected");
  }

  /* Shortened Local Name */
  if (Bluefruit.Scanner.parseReportByType(
          report, BLE_GAP_AD_TYPE_SHORT_LOCAL_NAME, buffer, sizeof(buffer))) {
    Serial.printf("%14s %s\n", "SHORT NAME", buffer);
    memset(buffer, 0, sizeof(buffer));
  }

  /* Complete Local Name */
  if (Bluefruit.Scanner.parseReportByType(report,
                                          BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME,
                                          buffer, sizeof(buffer))) {
    Serial.printf("%14s %s\n", "COMPLETE NAME", buffer);
    memset(buffer, 0, sizeof(buffer));
  }

  /* TX Power Level */
  if (Bluefruit.Scanner.parseReportByType(
          report, BLE_GAP_AD_TYPE_TX_POWER_LEVEL, buffer, sizeof(buffer))) {
    Serial.printf("%14s %i\n", "TX PWR LEVEL", buffer[0]);
    memset(buffer, 0, sizeof(buffer));
  }

  /* Check for UUID16 Complete List */
  len = Bluefruit.Scanner.parseReportByType(
      report, BLE_GAP_AD_TYPE_16BIT_SERVICE_UUID_COMPLETE, buffer,
      sizeof(buffer));
  if (len) {
    printUuid16List(buffer, len);
  }

  /* Check for UUID16 More Available List */
  len = Bluefruit.Scanner.parseReportByType(
      report, BLE_GAP_AD_TYPE_16BIT_SERVICE_UUID_MORE_AVAILABLE, buffer,
      sizeof(buffer));
  if (len) {
    printUuid16List(buffer, len);
  }

  /* Check for UUID128 Complete List */
  len = Bluefruit.Scanner.parseReportByType(
      report, BLE_GAP_AD_TYPE_128BIT_SERVICE_UUID_COMPLETE, buffer,
      sizeof(buffer));
  if (len) {
    printUuid128List(buffer, len);
  }

  /* Check for UUID128 More Available List */
  len = Bluefruit.Scanner.parseReportByType(
      report, BLE_GAP_AD_TYPE_128BIT_SERVICE_UUID_MORE_AVAILABLE, buffer,
      sizeof(buffer));
  if (len) {
    printUuid128List(buffer, len);
  }

  /* Check for BLE UART UUID */
  if (Bluefruit.Scanner.checkReportForUuid(report, BLEUART_UUID_SERVICE)) {
    Serial.printf("%14s %s\n", "BLE UART", "UUID Found!");
  }

  /* Check for DIS UUID */
  if (Bluefruit.Scanner.checkReportForUuid(report,
                                           UUID16_SVC_DEVICE_INFORMATION)) {
    Serial.printf("%14s %s\n", "DIS", "UUID Found!");
  }

  /* Check for Manufacturer Specific Data */
  len = Bluefruit.Scanner.parseReportByType(
      report, BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA, buffer,
      sizeof(buffer));
  if (len) {
    Serial.printf("%14s ", "MAN SPEC DATA");
    Serial.printBuffer(buffer, len, '-');
    Serial.println();
    memset(buffer, 0, sizeof(buffer));
  }

  Serial.println();

  // For Softdevice v6: after received a report, scanner will be paused
  // We need to call Scanner resume() to continue scanning
  Bluefruit.Scanner.resume();
}

void read_and_print_chara(uint16_t conn_handle,
                          BLEClientCharacteristic* chara) {
  uint16_t dataLen = 300;
  uint8_t dataArr[dataLen];
  uint16_t receivedLen = chara->read(dataArr, dataLen);
  for (int i = 0; i < receivedLen; ++i) {
    Serial.printf("%02x", dataArr[i]);
    Serial.print(" ");
  }
  Serial.println();
}

void handle_service_hid(uint16_t conn_handle) {
  Serial.println("hid info");
  read_and_print_chara(conn_handle, &charaHidInfo);

  Serial.print("Discovering characteristic report map ... ");
  if (!charaReportMap.discover()) {
    // Measurement chr is mandatory, if it is not found (valid), then disconnect
    Serial.println("not found !!!");
    Bluefruit.disconnect(conn_handle);
    return;
  }
  Serial.println("Found it");
  read_and_print_chara(conn_handle, &charaReportMap);

  Serial.print("Discovering hid info ... ");
  if (!charaHidInfo.discover()) {
    // Measurement chr is mandatory, if it is not found (valid), then disconnect
    Serial.println("not found !!!");
    Bluefruit.disconnect(conn_handle);
    return;
  }
  Serial.println("Found it");

  Serial.print("Discovering characteristic report ... ");
  if (!charaReport.discover()) {
    // Measurement chr is mandatory, if it is not found (valid), then disconnect
    Serial.println("not found !!!");
    Bluefruit.disconnect(conn_handle);
    return;
  }
  Serial.println("Found it");

  if (charaReport.enableNotify()) {
    Serial.println("Ready to receive report value");
  } else {
    Serial.println(
        "Couldn't enable notify for report. Increase DEBUG LEVEL for "
        "troubleshooting");
  }
}

void handle_service_battery(uint16_t conn_handle) {
  Serial.print("Discovering characteristic battery ... ");
  if (!charaBattery.discover()) {
    Serial.println("not found !!!");
    return;
  } else {
    Serial.println("Found it");
    charaBattery.enableNotify();
  }
}

void connect_callback(uint16_t conn_handle) {
  Serial.println("Connected");
  // Serial.print("Discovering Service ... ");

  BLEConnection* conn = Bluefruit.Connection(conn_handle);

  if (serviceHid.discover(conn_handle)) {
    Serial.println("Found HID service");
    conn->requestPairing();
    return;
  }
  Serial.println("Cannot found target service");
}

void disconnect_callback(uint16_t conn_handle, uint8_t reason) {
  (void)conn_handle;
  (void)reason;

  Serial.print("Disconnected, reason = 0x");
  Serial.println(reason, HEX);
}

void callbackBattery(BLEClientCharacteristic* chr, uint8_t* data,
                     uint16_t len) {
  Serial.println("callbackBattery");
  for (int i = 0; i < len; ++i) {
    Serial.print(data[i]);
    Serial.print(" ");
  }
  Serial.println();
}

void callbackHIDReport(BLEClientCharacteristic* chr, uint8_t* data,
                       uint16_t len) {
  Serial.println("callbackHIDReport");
  for (int i = 0; i < len; ++i) {
    Serial.printf("%02x ", data[i]);
  }
  Serial.println();
}

void connection_secured_callback(uint16_t conn_handle) {
  BLEConnection* conn = Bluefruit.Connection(conn_handle);

  if (!conn->secured()) {
    Serial.println("not secured");
    // It is possible that connection is still not secured by this time.
    // This happens (central only) when we try to encrypt connection using
    // stored bond keys but peer reject it (probably it remove its stored key).
    // Therefore we will request an pairing again --> callback again when
    // encrypted
    conn->requestPairing();
  } else {
    Serial.println("Secured");

    // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.characteristic.hid_information.xml
    uint8_t hidInfo[4];
    charaHidInfo.read(hidInfo, 4);

    Serial.printf("HID version: %d.%d\n", hidInfo[0], hidInfo[1]);
    Serial.print("Country code: ");
    Serial.println(hidInfo[2]);
    Serial.printf("HID Flags  : 0x%02X\n", hidInfo[3]);

    handle_service_hid(conn_handle);

    if (serviceBattery.discover(conn_handle)) {
      Serial.println("Found battery service");
      // handle_service_battery(conn_handle);
      handle_service_battery(conn_handle);
    }

    // // BLEClientHidAdafruit currently only supports Boot Protocol Mode
    // // for Keyboard and Mouse. Let's set the protocol mode on prph to Boot
    // Mode hid.setBootMode(true);

    // // Enable Keyboard report notification if present on prph
    // if (hid.keyboardPresent()) hid.enableKeyboard();

    // // Enable Mouse report notification if present on prph
    // if (hid.mousePresent()) hid.enableMouse();

    // Serial.println("Ready to receive from peripheral");
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);  // for nrf52840 with native usb

  Serial.println("Bluefruit52 Central ADV Scan Example");
  Serial.println("------------------------------------\n");

  charaBattery.setNotifyCallback(callbackBattery);
  charaReport.setNotifyCallback(callbackHIDReport);

  serviceHid.begin();
  charaHidInfo.begin();
  charaReport.begin();
  charaReportMap.begin();
  serviceBattery.begin();
  charaBattery.begin();

  // Initialize Bluefruit with maximum connections as Peripheral = 0, Central =
  // 1 SRAM usage required by SoftDevice will increase dramatically with number
  // of connections
  Bluefruit.begin(0, 1);
  Bluefruit.setTxPower(4);  // Check bluefruit.h for supported values

  /* Set the device name */
  Bluefruit.setName("Bluefruit52");

  /* Set the LED interval for blinky pattern on BLUE LED */
  Bluefruit.setConnLedInterval(250);

  Bluefruit.Central.setConnectCallback(connect_callback);
  Bluefruit.Central.setDisconnectCallback(disconnect_callback);
  Bluefruit.Security.setSecuredCallback(connection_secured_callback);

  /* Start Central Scanning
   * - Enable auto scan if disconnected
   * - Filter out packet with a min rssi
   * - Interval = 100 ms, window = 50 ms
   * - Use active scan (used to retrieve the optional scan response adv packet)
   * - Start(0) = will scan forever since no timeout is given
   */
  Bluefruit.Scanner.setRxCallback(scan_callback);
  Bluefruit.Scanner.restartOnDisconnect(true);
  Bluefruit.Scanner.filterUuid(serviceHid.uuid, serviceBattery.uuid);
  // Bluefruit.Scanner.filterUuid(serviceBattery.uuid);
  Bluefruit.Scanner.filterRssi(-80);
  // Bluefruit.Scanner.filterUuid(BLEUART_UUID_SERVICE); // only invoke callback
  // if detect bleuart service
  Bluefruit.Scanner.setInterval(160, 80);  // in units of 0.625 ms
  Bluefruit.Scanner.useActiveScan(true);   // Request scan response data
  Bluefruit.Scanner.start(0);  // 0 = Don't stop scanning after n seconds

  Serial.println("Scanning ...");
}

void loop() {
  // nothing to do
}
#endif

#ifdef EXAMPLE_SCAN_BASIC
#include <bluefruit.h>

void scan_callback(ble_gap_evt_adv_report_t* report) {
  Serial.println("Timestamp Addr              Rssi Data");

  Serial.printf("%09d ", millis());

  // MAC is in little endian --> print reverse
  Serial.printBufferReverse(report->peer_addr.addr, 6, ':');
  Serial.print(" ");

  Serial.print(report->rssi);
  Serial.print("  ");

  Serial.printBuffer(report->data.p_data, report->data.len, '-');
  Serial.println();

  // Check if advertising contain BleUart service
  if (Bluefruit.Scanner.checkReportForUuid(report, BLEUART_UUID_SERVICE)) {
    Serial.println("                       BLE UART service detected");
  }

  Serial.println();

  // For Softdevice v6: after received a report, scanner will be paused
  // We need to call Scanner resume() to continue scanning
  Bluefruit.Scanner.resume();
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);  // for nrf52840 with native usb

  Serial.println("Bluefruit52 Central Scan Example");
  Serial.println("--------------------------------\n");

  // Initialize Bluefruit with maximum connections as Peripheral = 0, Central =
  // 1 SRAM usage required by SoftDevice will increase dramatically with number
  // of connections
  Bluefruit.begin(0, 1);
  Bluefruit.setTxPower(4);  // Check bluefruit.h for supported values
  Bluefruit.setName("Bluefruit52");

  // Start Central Scan
  Bluefruit.setConnLedInterval(250);
  Bluefruit.Scanner.setRxCallback(scan_callback);
  Bluefruit.Scanner.start(0);

  Serial.println("Scanning ...");
}

void loop() {
  // nothing to do
}
#endif
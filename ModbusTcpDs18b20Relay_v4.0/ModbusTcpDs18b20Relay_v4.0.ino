#include "DualSerial/DualSerial.h"
//#include "RandomMac.h"
//#include "EthernetSetup.h"
#include <ModbusTCPSlave.h>
#include <ModbusRTUSlave.h>
#include "DS18B20_INT.h"
#include <Ethernet.h>

#ifndef USBCON
#if defined(ARDUINO_UNOR4_MINIMA) || defined(ARDUINO_UNOWIFIR4) || defined(ARDUINO_MINIMA) || defined(ARDUINO_UNOR4_WIFI)
// This is an UNO R4 board with native USB

#define USBCON
#endif
#endif

//Remove serial port1, if it doesnt exist
//This means hte compiler will remove serial1 and we dont have to do it in the program
#if defined(HAVE_HWSERIAL1) || defined(Serial1)

#else
#define Serial1 \
  if (false) Serial
#endif

// Create a global instance that writes to both Serial and Serial1
DualSerial Serials(Serial, Serial1);

//---- Connected hardware ---- Connected hardware ---- Connected hardware ---- Connected hardware ---- Connected hardware

//define the onewire
#define ONE_WIRE_BUS1 2
#define ONE_WIRE_BUS2 3
DeviceAddress sensorAddress1;
DeviceAddress sensorAddress2;

OneWire oneWire1(ONE_WIRE_BUS1);
OneWire oneWire2(ONE_WIRE_BUS2);

DS18B20_INT sensor1(&oneWire1);
DS18B20_INT sensor2(&oneWire2);


//define the relay pins, also adding the rx/tx led for the r4
#pragma message("Compiling for: " ARDUINO_BOARD)
#if defined(ARDUINO_UNOR4_MINIMA) || defined(ARDUINO_UNOWIFIR4) || defined(ARDUINO_MINIMA) || defined(ARDUINO_UNOR4_WIFI)
constexpr uint8_t relayPins[] = { 4, 5, 6, 7, LED_TX, LED_RX };
#else
// For all other boards
constexpr uint8_t relayPins[] = { 4, 5, 6, 7 };
#endif

constexpr uint8_t numRelays = sizeof(relayPins) / sizeof(relayPins[0]);

//\---- Connected hardware ---- Connected hardware ---- Connected hardware ---- Connected hardware ---- Connected hardware




//----- Enums ----- Enums ----- Enums ----- Enums ----- Enums ----- Enums ----- Enums ----- Enums
enum eNetWorkSettings : int8_t {
  using_FixedIp = 0,
  using_DHCP,
  using_FallbackIp,
  NoEthernetHardware,
  NetWorkSettings_Count
};
//\----- Enums ----- Enums ----- Enums ----- Enums ----- Enums ----- Enums ----- Enums ----- Enums


//----- structs ----- structs ----- structs ----- structs ----- structs ----- structs ----- structs
struct StEEPROM {
  byte mac[6];
  eNetWorkSettings NetWorkSettings;
  IPAddress ip;
};

//\----- structs ----- structs ----- structs ----- structs ----- structs ----- structs ----- structs

StEEPROM EEPROMdata;

//---- setup ---- setup ---- setup ---- setup ---- setup ---- setup ---- setup ---- setup ---- setup
void StartSerial() {
  //Start the serial debugging
  Serial.begin(9600);
  Serial1.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  while ((!Serial || !Serial1) && (millis() < 10000)) {
    digitalWrite(LED_BUILTIN, HIGH);  //Only write to led whiie not using Ethernet
    delay(1500);
    digitalWrite(LED_BUILTIN, LOW);  //Only write to led whiie not using Ethernet
    delay(500);
  }
  Serials.clear();

  pinMode(LED_BUILTIN, INPUT);
  Serials.println(F("Starting"));
}
void SetupRelays() {
  for (uint8_t i = 0; i < numRelays; i++) {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], HIGH);  // Default OFF
  }
  Serials.println(F("Relays defined"));
}

void printAddress(String Sensorname, DeviceAddress deviceAddress) {
  Serials.print(Sensorname);
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serials.print(deviceAddress[i], HEX);
  }
  Serials.println();
}


void StartTemperatureSensors() {
  sensor1.begin();
  sensor2.begin();
  if (sensor1.getAddress(sensorAddress1)) {
    Serials.print(F("Temperature sensor 1 id: "));
    printAddress("sensorAddress1: ", sensorAddress1);
  } else {
    Serials.println(F("Sensor 1 not found"));
  }
  if (sensor2.getAddress(sensorAddress1)) {
    Serials.print(F("Temperature sensor 2 id: "));
    printAddress("sensorAddress2: ", sensorAddress2);
  } else {
    Serials.println(F("Sensor 2 not found"));
  }
}

void setup() {
  StartSerial();
  SetupRelays();
  StartTemperatureSensors();
  ReadEEPROM(0, EEPROMdata);
  EthernetSetup();
}

//\---- setup ---- setup ---- setup ---- setup ---- setup ---- setup ---- setup ---- setup ---- setup

void loop() {
  // put your main code here, to run repeatedly:
  if (true) {
    Serials.println(F("Reboot in bootloadermode"));

  }
}

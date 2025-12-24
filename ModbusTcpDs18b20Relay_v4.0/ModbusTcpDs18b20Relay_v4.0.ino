#include "DualSerial/DualSerial.h"
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

enum eHoldingRegs : uint16_t {
  HREG_CMD = 40001,
  HREG_SETTINGS,
  HREG_IPADDRESS_12,
  HREG_IPADDRESS_34,
  HREG_RELAYS,
  HREG_COUNT
};

enum eInputRegs : uint16_t {
  IREG_PRGSTATE = 30001,
  IREG_SETTINGS,
  IREG_IPADDRESS_12,
  IREG_IPADDRESS_34,
  IREG_BUTTONS,
  IREG_TEMP1,
  IREG_TEMP2,
  IREG_COUNT
};


//----- Enums ----- Enums ----- Enums ----- Enums ----- Enums ----- Enums ----- Enums ----- Enums
constexpr int8_t eNo_Stored_Data = -1;

enum eNetWorkSettings : int8_t {
  eUsing_FixedIp = 0,
  eUsing_DHCP,
  eUsing_FallbackIp,
  eUse_Rtu,
  eNoEthernetHardware,
  eNetWorkSettings_Count
};
//\----- Enums ----- Enums ----- Enums ----- Enums ----- Enums ----- Enums ----- Enums ----- Enums


//----- structs ----- structs ----- structs ----- structs ----- structs ----- structs ----- structs
struct StEEPROM {
  byte mac[6];
  eNetWorkSettings NetWorkSettings;
  IPAddress ip;
  unsigned long DhcpTimeout;
};

//\----- structs ----- structs ----- structs ----- structs ----- structs ----- structs ----- structs

//----- Variables ----- Instances ----- Unions ----- Variables ----- Instances ----- Unions

union {
  uint16_t stSettings;  // 16-bit value, perfect for a Modbus register
  struct {
    uint16_t UseTcp : 1;          //using tcp not rtu
    uint16_t Using_DHCP : 1;      //using dhcp
    uint16_t Using_StoredIp : 1;  //using the ip from config, not the fallback
    uint16_t TempSensor1Active : 1;
    uint16_t TempSensor2Active : 1;
    uint16_t bit5 : 1;
    uint16_t bit6 : 1;
    uint16_t free : 9;
  };
} Settings;

DualSerial Serials(Serial, Serial1);  // Instance that writes to both Serial and Serial1
//ModbusTCPSlave mbTCP;  // Declare ModbusTCP instance
//ModbusRTUSlave modbus(MODBUS_SERIAL, dePin);       // Declare ModbusTCP instance
ModbusSlaveLogic Modbus;  //Declare the modbus slave, that we use for anything but setup of the modbus

StEEPROM EEPROMdata;  //Instance of saved EEPROM data

IPAddress FallbackIp(192, 168, 1, 254);
//----- Variables ----- Instances ----- Unions ----- Variables ----- Instances ----- Unions



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
  if (sensor2.getAddress(sensorAddress2)) {
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
  if (Settings.UseTcp = 0) {
    //StartRtuModbus();
  }
}

//\---- setup ---- setup ---- setup ---- setup ---- setup ---- setup ---- setup ---- setup ---- setup

void loop() {
  // put your main code here, to run repeatedly:
  /*if (true) {
    Serials.println(F("Reboot in bootloadermode"));

  }
  */
}

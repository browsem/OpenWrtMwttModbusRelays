
#include <SPI.h>
#include <Ethernet.h>
#include <ModbusTCPSlave.h>
#include <ModbusRTUSlave.h>
#include "DS18B20_INT.h"
#include "RandomMac.h"
#include "txtStrings.h"
#include "SerPrint.h"


#ifndef USBCON
#if defined(ARDUINO_UNOR4_MINIMA) || defined(ARDUINO_UNOWIFIR4) || defined(ARDUINO_MINIMA) || defined(ARDUINO_UNOR4_WIFI)
// This is an UNO R4 board with native USB

#define USBCON
#endif
#endif

//Remove serial port1, if it doesnt exist
#if defined(HAVE_HWSERIAL1) || defined(Serial1)

#else
#define Serial1 \
  if (false) Serial
#endif

//define the onewire/temperature sensor
#define ONE_WIRE_BUS1 7
#define ONE_WIRE_BUS2 8


OneWire oneWire1(ONE_WIRE_BUS1);
OneWire oneWire2(ONE_WIRE_BUS2);

DS18B20_INT sensor1(&oneWire1);
DS18B20_INT sensor2(&oneWire2);


//define the modbus
//RTU
#define MODBUS_SERIAL Serial
#define MODBUS_BAUD 9600
#define MODBUS_CONFIG SERIAL_8N1
#define MODBUS_UNIT_ID 1
#define dePin A3
ModbusRTUSlave modbusRTU(MODBUS_SERIAL, dePin);

//TCP
EthernetServer server(MODBUS_TCP_SLAVE_DEFAULT_PORT);
ModbusTCPSlave modbusTCP;

//Dual, going for the gold here
ModbusSlaveLogic* mb = nullptr;

//Default ip adress for configuration
constexpr byte DefaultIP1[] = { 192, 168, 1, 1 };
IPAddress CurrentIP(0, 0, 0, 0);  // Static fallback IP


// The IP address will be dependent on your local network:
byte mac[6];

//define the relays
#pragma message("Compiling for: " ARDUINO_BOARD)
#if defined(ARDUINO_UNOR4_MINIMA) || defined(ARDUINO_UNOWIFIR4) || defined(ARDUINO_MINIMA) || defined(ARDUINO_UNOR4_WIFI)
constexpr uint8_t relayPins[] = { 4, 5, 6, 7, LED_TX, LED_RX };
#else
// For all other boards
constexpr uint8_t relayPins[] = { 4, 5, 6, 7 };
#endif

constexpr uint8_t numRelays = sizeof(relayPins) / sizeof(relayPins[0]);

//-------- Enums --------  Enums --------  Enums --------  Enums --------  Enums --------  Enums --------

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

enum eUseDHCP : uint8_t {
  using_FixedIp = 0,
  using_DHCP,
  using_FallbackIp,
  NoEthernetHardware,
  eUseDHCP_Count
};

//\-------- Enums --------  Enums --------  Enums --------  Enums --------  Enums --------  Enums --------

constexpr uint8_t nrOfeHoldingRegs = HREG_COUNT - HREG_CMD;
constexpr uint8_t nrOfeInputRegs = IREG_COUNT - IREG_PRGSTATE;

uint16_t holdingRegisters[nrOfeHoldingRegs];
uint16_t inputRegisters[nrOfeInputRegs];

struct StEEPROM {
  int8_t UseDHCP;
  IPAddress ip;
};


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

constexpr uint32_t blinkTime = 5000;
uint32_t blinkStartTime = millis() - blinkTime;

//------- Functions ---- Functions ---- Functions ---- Functions ---- Functions ---- Functions ---- Functions ---- Functions

void IpToModbus() {
  //run the send/recive modbus thinghy
  IPAddress ip = CurrentIP;
  if (Settings.UseTcp) {
    IPAddress ip = Ethernet.localIP();
  }
  inputRegisters[IREG_IPADDRESS_12] = (ip[0] << 8) | ip[1];
  inputRegisters[IREG_IPADDRESS_34] = (ip[2] << 8) | ip[3];
}


//-------- SETUP -------- SETUP -------- SETUP -------- SETUP -------- SETUP -------- SETUP -------- SETUP -------- SETUP

void configurRegisters() {
  mb->configureHoldingRegisters(holdingRegisters, nrOfeHoldingRegs);
  mb->configureInputRegisters(inputRegisters, nrOfeInputRegs);
}


void InitTCP() {
  SerPrintFromRomln(eMsg_configuration_of_tcp_started);
  //Get mac adress from rom
  GetMyMac();
  //get the stored values from the eeprom
  StEEPROM loaded;
  //cheking settings from eeprom
  int8_t NetworkSetup = using_DHCP;
  if ((0<=loaded.UseDHCP) && (loaded.UseDHCP<eUseDHCP_Count) ) {
    NetworkSetup = loaded.UseDHCP;
  } else {
    //Start by trying dhcp if nothing else works
    NetworkSetup = using_DHCP; 
  }
  SerPrint(F("NetworkSetup from eeprom: "));
  SerPrintln(NetworkSetup);
  Settings.Using_DHCP = 0;
  Settings.Using_StoredIp = 0;
  if (NetworkSetup == using_DHCP) {
    //we start by trying dhcp, if thats in the cards
    if (Ethernet.begin(mac)) {
      //Found adress
      Settings.Using_DHCP = 1;
      SerPrintFromRomln(eMsg_ip_from_dhcp);
      CurrentIP = Ethernet.localIP();
    } else {
      NetworkSetup = using_FallbackIp;
    }

   }
  
  SerPrint(F("NetworkSetup as found: "));
  SerPrint(Settings.Using_DHCP);
  SerPrintFromRom(eMsg_SpaceCommaSpace);
  SerPrintln(Settings.Using_StoredIp);
  // print your local IP address:
}
/*

  //Use modbus tcp
  mb= &modbusTCP;

  
  configurRegisters();

  
  //start the ethernet
  if (!Ethernet.begin(mac)){
    //No mac adress, fall back to default IP
    IPAddress FixedIP=DefaultIP1[];
    Ethernet.begin(mac,FixedIP);
  }
  server.begin();

  Flags.UseTcp = true;
  
  delay(1000);     // give the Ethernet shield a second to initialize
  mbTCP.server();  // Act as Modbus TCP server

  mbAddRegs();
  //define the callbacks
  mbTCP.onSetHreg(HREG_RELAYS, SetRelays);
  mbTCP.onGetHreg(HREG_RELAYS, GetRelays);
  mbTCP.onGetIreg(IREG_TEMP1, getTemperature);
  mbTCP.onGetIreg(IREG_TEMP2, getTemperature);
  mbTCP.onGetIreg(IREG_IPADDRESS_12, IpToModbus);
  mbTCP.onGetIreg(IREG_IPADDRESS_34, IpToModbus);
  
}

/*
void InitRTU() {
  mb= &modbusRTU;
  //start the serial
  
  mbRTU.begin(&Serial);
  mbRTU.slave(1);



  mbAddRegs();
  //define the callbacks
  mbRTU.onSetHreg(HREG_RELAYS, SetRelays);
  mbRTU.onGetHreg(HREG_RELAYS, GetRelays);
  mbRTU.onGetIreg(IREG_TEMP1, getTemperature);
  mbRTU.onGetIreg(IREG_TEMP2, getTemperature);
  mbRTU.onGetIreg(IREG_IPADDRESS_12, IpToModbus);
  mbRTU.onGetIreg(IREG_IPADDRESS_34, IpToModbus);
  
}
*/


void SerialStartup() {
  Settings.UseTcp = 1;
  Serial.begin(9600);

#if defined(USBCON)
#pragma message("Usb connected serial detected")
  unsigned long start = millis();
  while (!Serial && (millis() - start < 5000)) {
    // wait up to 2 seconds
  }
#endif
  Serial.println(F("Serial Started"));

  Serial1.begin(9600);

  SerPrintln(F("Serial1 Started"));
  SerPrintFromRomln(eMsg_Booting);
}

void setup() {
  SerialStartup();
  //setup the relays as outputs
  for (uint8_t i = 0; i < numRelays; i++) {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], HIGH);  // Default OFF
  }
  SerPrintln(F("Relays are setup"));



  // Here we start reading the temperature sensor
  Settings.TempSensor1Active = sensor1.setResolution(9);
  Settings.TempSensor2Active = sensor2.setResolution(9);

  //now we see if ethernet is present

  // setup the ethernet, and see if its present
  InitTCP();
}
/*
  } else {
    InitRTU();
  }
  
}
*/
void loop() {
  // put your main code here, to run repeatedly:
}

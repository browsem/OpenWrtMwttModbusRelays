// put your setup code here, to run once:
#include <SPI.h>
#include <Ethernet.h>
#include <ModbusRTU.h>
#include <ModbusEthernet.h>
#include "DS18B20_INT.h"

//define the modbus
ModbusEthernet mbTCP;  // Declare ModbusTCP instance
ModbusRTU mbRTU;       // Declare ModbusTCP instance

//define the onewire
#define ONE_WIRE_BUS1 7
#define ONE_WIRE_BUS2 8

OneWire oneWire1(ONE_WIRE_BUS1);
OneWire oneWire2(ONE_WIRE_BUS2);

DS18B20_INT sensor1(&oneWire1);
DS18B20_INT sensor2(&oneWire2);



// The IP address will be dependent on your local network:
byte mac[] = { 0xF4, 0x18, 0x4C, 0xEB, 0xD2, 0x52 };

//define the relays
constexpr uint8_t relayPins[] = { 2, 3, 4, 6, LED_BUILTIN };
constexpr uint8_t numRelays = sizeof(relayPins) / sizeof(relayPins[0]);

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


union {
  uint16_t stSettings;  // 16-bit value, perfect for a Modbus register
  struct {
    uint16_t Using_DHCP : 1;
    uint16_t bit1 : 1;
    uint16_t bit2 : 1;
    uint16_t bit3 : 1;
    uint16_t bit4 : 1;
    uint16_t bit5 : 1;
    uint16_t bit6 : 1;
    uint16_t free : 9;
  };
} Settings;


union {
  uint8_t byte;
  struct {
    uint8_t UseTcp : 1;
    uint8_t TempSensor1 : 1;
    uint8_t TempSensor2 : 1;
    uint8_t free : 5;
  };
} Flags;

//-------- call back-------- call back-------- call back-------- call back-------- call back-------- call back-------- call back
uint16_t SetRelays(TRegister* reg, uint16_t val) {

}
uint16_t GetRelays(TRegister* reg, uint16_t val) {

}
uint16_t getTemperature(TRegister* reg, uint16_t val) {

}
uint16_t IpToModbus(TRegister* reg, uint16_t val) {

}



//-------- SETUP -------- SETUP -------- SETUP -------- SETUP -------- SETUP -------- SETUP -------- SETUP -------- SETUP
void mbAddRegs() {  //define the adress ranges

  uint8_t nrOfeHoldingRegs = HREG_COUNT - HREG_CMD;
  uint8_t nrOfeInputRegs = IREG_COUNT - IREG_PRGSTATE;

  if (Flags.UseTcp) {
    mbTCP.addHreg(HREG_CMD, 0, nrOfeHoldingRegs);
    mbTCP.addIreg(IREG_PRGSTATE, 0, nrOfeInputRegs);
  } else {
    mbRTU.addHreg(HREG_CMD, 0, nrOfeHoldingRegs);
    mbRTU.addIreg(IREG_PRGSTATE, 0, nrOfeInputRegs);
  }

}

void InitTCP() {
  // print your local IP address:
  //start the ethernet
  Ethernet.begin(mac);
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


void InitRTU() {
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


void setup() {
  //setup the relays as outputs
  for (uint8_t i = 0; i < numRelays; i++) {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], LOW);  // Default OFF
  }
  // Here we start reading the temperature sensor
  Flags.TempSensor1 = sensor1.setResolution(9);
  Flags.TempSensor2 = sensor2.setResolution(9);

  //now we see if ethernet is present

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() != EthernetNoHardware) {
    InitTCP();
  } else {
    InitRTU();
  }
}

//-------- loop -------- loop -------- loop -------- loop -------- loop -------- loop -------- loop -------- loop
void loop() {
  // put your main code here, to run repeatedly:
    if (Flags.UseTcp) {
    mbTCP.task();
  } else {
    mbRTU.task();
  }
}

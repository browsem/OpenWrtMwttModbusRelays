/*
Modbus server to control 4x relays
Reading 2x ds18b20 sensors and possibly a couple of switches
*/

//-------- Ethernet and modbus setup -----------------------------

#include <SPI.h>
#include <Ethernet.h>
#include <ModbusRTU.h>
#include <ModbusEthernet.h>



uint32_t LastBlink = 0;
uint32_t LastUpdate = 0;
int16_t Temperature[] = { 0, 0 };
uint16_t IPADDRESS_12Word = 0;
uint16_t IPADDRESS_34Word = 0;

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield


// The IP address will be dependent on your local network:
byte mac[] = { 0xF4, 0x18, 0x4C, 0xEB, 0xD2, 0x52 };

//IPAddress ip(192, 168, 40, 146);


ModbusEthernet mbTCP;  // Declare ModbusTCP instance
ModbusRTU mbRTU;       // Declare ModbusTCP instance


struct Word_example {
  uint16_t bit0 : 1;
  uint16_t bit1 : 1;
  uint16_t bit2 : 1;
  uint16_t bit3 : 1;
  uint16_t bit4 : 1;
  uint16_t bit5 : 1;
  uint16_t bit6 : 1;
  uint16_t bit7 : 1;
  uint16_t bit8 : 1;
  uint16_t bit9 : 1;
  uint16_t bit10 : 1;
  uint16_t bit11 : 1;
  uint16_t bit12 : 1;
  uint16_t bit13 : 1;
  uint16_t bit14 : 1;
  uint16_t bit15 : 1;
};

struct stSettings {
  uint16_t Using_DHCP : 1;
  uint16_t bit1 : 1;
  uint16_t bit2 : 1;
  uint16_t bit3 : 1;
  uint16_t bit4 : 1;
  uint16_t bit5 : 1;
  uint16_t bit6 : 1;
  uint16_t bit7 : 1;
  uint16_t bit8 : 1;
  uint16_t bit9 : 1;
  uint16_t bit10 : 1;
  uint16_t bit11 : 1;
  uint16_t bit12 : 1;
  uint16_t bit13 : 1;
  uint16_t bit14 : 1;
  uint16_t bit15 : 1;
};


enum eModbusCmd {
  Task = 1,
  AddHreg,
  AddIreg,
  SetHreg,
  SetIreg,
  GetHreg,
  GetIreg,
};

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



//-------- \Ethernet and modbus setup -----------------------------

//-------- Relay setup -----------------------------

//definition of relay pins
const uint8_t relayPins[] PROGMEM = { 2, 3, 4, 6, LED_BUILTIN };  // Relay pins
uint8_t numRelays = sizeof(relayPins) / sizeof(relayPins[0]);

//-------- \Relay setup -----------------------------

//-------- Temperature sensor setup -----------------------------

#include "DS18B20_INT.h"

#define ONE_WIRE_BUS1 7
#define ONE_WIRE_BUS2 8

OneWire oneWire1(ONE_WIRE_BUS1);
OneWire oneWire2(ONE_WIRE_BUS2);
DS18B20_INT sensor1(&oneWire1);
DS18B20_INT sensor2(&oneWire2);

//-------- \debug serial setup -----------------------------


HardwareSerial* debugSerial = nullptr;

//-------- \debug serial setup-----------------------------

struct stFlags {
  uint8_t TempSensor1 : 1;
  uint8_t TempSensor2 : 1;
  uint8_t UseTcp : 1;
  uint8_t free : 5;
};

stFlags Flags;



int16_t readSensor(DS18B20_INT& sensor) {

  uint32_t LclStartTime = millis();
  sensor.requestTemperatures();

  while (!sensor.isConversionComplete() && millis() - LclStartTime < 750) {
  }
  uint32_t SpentTime = millis() - LclStartTime;

  debugSerial->print(F("time: "));
  debugSerial->println(SpentTime);

  if ((SpentTime >= 750)) {
    debugSerial->println("Bad temp: ");
    return -127;
  }

  int16_t t = sensor.getTempC();

  debugSerial->print(F("temp: "));

  debugSerial->println(t);


  return t;
}

void PrintDeb(uint16_t RegisterAdress = 0, uint16_t Value = 0) {
  debugSerial->print(F(" RegisterAdress: "));
  debugSerial->print(RegisterAdress);
  debugSerial->print(F(" Value : "));
  debugSerial->println(Value);
}

/*
uint16_t fctmb(eModbusCmd cmd, uint16_t RegisterAdress = 0, uint16_t Value = 0) {
  bool Retval = false;

  if (Flags.UseTcp) {
    switch (cmd) {
      case Task:
        mbTCP.task();
        Retval = true;
        break;
      case AddHreg:
        PrintDeb(RegisterAdress, Value);
        Retval = mbTCP.addHreg(RegisterAdress, 1630, Value);
        break;
      case AddIreg:
        PrintDeb(RegisterAdress, Value);
        Retval = mbTCP.addIreg(RegisterAdress, 1630, Value);
        break;
      case SetHreg:
        Retval = mbTCP.Hreg(RegisterAdress, Value);
        break;
      case SetIreg:
        Retval = mbTCP.Ireg(RegisterAdress, Value);
        break;
      case GetHreg:
        Retval = mbTCP.Hreg(RegisterAdress);
        PrintDeb(RegisterAdress, Retval);
        break;
      case GetIreg:
        Retval = mbTCP.Ireg(RegisterAdress);
        PrintDeb(RegisterAdress, Retval);
        break;
    }
  } else {
    switch (cmd) {
      case Task:
        mbRTU.task();
        Retval = true;
        break;
      case AddHreg:
        Retval = mbRTU.addHreg(RegisterAdress, 0, Value);
        break;
      case AddIreg:
        Retval = mbRTU.addIreg(RegisterAdress, 0, Value);
        break;
      case SetHreg:
        Retval = mbRTU.Hreg(RegisterAdress, Value);
        break;
      case SetIreg:
        Retval = mbRTU.Ireg(RegisterAdress, Value);
        break;
      case GetHreg:
        Retval = mbRTU.Hreg(RegisterAdress);
        PrintDeb(RegisterAdress, Retval);
        break;
      case GetIreg:
        Retval = mbRTU.Ireg(RegisterAdress);
        PrintDeb(RegisterAdress, Retval);
        break;
    }
  }
  return Retval;
}
*/
uint16_t HandleRelays(TRegister* reg, uint16_t val) {

  for (int i = 0; i < numRelays; i++) {
    bool state = val & (1 << i);
    digitalWrite(relayPins[i], state ? HIGH : LOW);
    if (debugSerial) {
      debugSerial->print(F("RelayNum: "));
      debugSerial->print(i);
      debugSerial->print(F("  RelayPinNum: "));
      debugSerial->print(relayPins[i]);
      debugSerial->print(F("  State: "));
      debugSerial->println(state);
    }
  }
  return val;
}

//-------- CALLBACKS -------- CALLBACKS -------- CALLBACKS -------- CALLBACKS -------- CALLBACKS -------- CALLBACKS



uint16_t GetRelays(TRegister* reg, uint16_t val) {
  debugSerial->println("Reading status: ");
  val = HandleRelays(reg, val);
  return val;
}
uint16_t SetRelays(TRegister* reg, uint16_t val) {
  debugSerial->println("Setting relays: ");
  val = HandleRelays(reg, val);
}

uint16_t IpToModbus(TRegister* reg, uint16_t val) {
  GetIp();
  uint16_t addr = reg->address.address;
  if (addr == IREG_IPADDRESS_12) val = IPADDRESS_12Word;
  if (addr == IREG_IPADDRESS_34) val = IPADDRESS_34Word;
  debugSerial->println(val);
  return val;
}

uint16_t getTemperature(TRegister* reg, uint16_t val) {
  uint16_t addr = reg->address.address;
  if (addr == IREG_TEMP1 && Flags.TempSensor1) {
    val = readSensor(sensor1);
  } else if (addr == IREG_TEMP2 && Flags.TempSensor2) {
    val = readSensor(sensor2);
  } else {
    val = -128;
  }
  debugSerial->println(val);
  return val;
}


void GetIp() {
  //run the send/recive modbus thinghy
  IPAddress ip = Ethernet.localIP();
  IPADDRESS_12Word = (ip[0] << 8) | ip[1];
  IPADDRESS_34Word = (ip[2] << 8) | ip[3];
}

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


void InitRTU() {
  debugSerial->println(F(" Use serial for modbus "));
  debugSerial->end();
  debugSerial = nullptr;
  //Startup the modbus

  Serial.begin(9600, SERIAL_8N1);
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }
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

void InitTCP() {
  // print your local IP address:

  debugSerial->print(F(" My IP address: "));
  debugSerial->println(Ethernet.localIP());

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





//-------- SETUP -------- SETUP -------- SETUP -------- SETUP -------- SETUP -------- SETUP -------- SETUP -------- SETUP
void setup() {
  debugSerial = &Serial;
  // Open serial communications and wait for port to open:
  debugSerial->begin(9600);
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
    delay(1);
  }
  debugSerial->println(F(" Seriel port started "));

  //setup the relays as outputs
  for (uint8_t i = 0; i < numRelays; i++) {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], LOW);  // Default OFF
  }

  // Here we start reading the temperature sensor

  Flags.TempSensor1 = sensor1.setResolution(9);
  Flags.TempSensor2 = sensor2.setResolution(9);

  //***here we could search for fixedip from sdcard
  //***start the ethernet, using fixes or dhcpd

  // start the Ethernet connection and the server:
  Ethernet.begin(mac);

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {

    debugSerial->println(F(" Ethernet not found"));
    Flags.UseTcp = false;
    InitRTU();

  } else {
    Flags.UseTcp = true;
    InitTCP();
  }

  debugSerial->println(F(" Init done "));
  // configure the LED
}



//-------- loop -------- loop -------- loop -------- loop -------- loop -------- loop -------- loop -------- loop


void loop() {

  uint32_t CurrentTime = millis();
  uint32_t timeForNextCycle = CurrentTime + 50;  //Next scan in 50 ms
  if (CurrentTime - LastBlink > 1000) {
    LastBlink = CurrentTime;
  }
  if (CurrentTime - LastUpdate > 10000) {
    LastUpdate = CurrentTime;
  }



  /*
  int n = 0;
  

  sensor1.requestTemperatures();
  while (!sensor1.isConversionComplete()) n++;
  int t = sensor1.getTempC();
  stop = millis();
  
  }
*/
  if (Flags.UseTcp) {
    mbTCP.task();
  } else {
    mbRTU.task();
  }
  CurrentTime = millis();
  if (timeForNextCycle > CurrentTime) {
    delay(timeForNextCycle - CurrentTime);
  }
}
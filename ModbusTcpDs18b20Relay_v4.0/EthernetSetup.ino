//#include <Ethernet.h>
bool ConnectionFound;


bool TestFixedIp() {
  if (EEPROMdata.NetWorkSettings != eUsing_FixedIp) {
    return false;
  }
  Serials.println(F("Trying stored Ip"));
  Ethernet.begin(EEPROMdata.mac, EEPROMdata.ip);
  if ((Ethernet.localIP() == IPAddress(0, 0, 0, 0)) || (Ethernet.localIP() == IPAddress(255, 255, 255, 255))) {
    Serials.print(F("Stored ip is incorrect: "));
    Serials.println(Ethernet.localIP());
    return false;
  }
  if (Ethernet.hardwareStatus() != EthernetNoHardware) {
    Serials.println(F("Using stored ip"));
    Settings.Using_StoredIp = 1;
    return true;
  }
  return false;
}

bool TestDhcp() {
  if (EEPROMdata.NetWorkSettings != eUsing_DHCP) {
    return false;
  }
  EEPROMdata.DhcpTimeout = min(EEPROMdata.DhcpTimeout, 60000);
  EEPROMdata.DhcpTimeout = max(EEPROMdata.DhcpTimeout, 5000);
  Serials.print(F("Trying dhcp with timeout: "));
  Serials.println(EEPROMdata.DhcpTimeout);
  if (Ethernet.begin(EEPROMdata.mac, EEPROMdata.DhcpTimeout) == 0) {
    return false;
  }
  Settings.Using_DHCP = 1;
  return true;
}

bool TestFallBackIp() {
  Serials.print(F("Trying fallbackIp: "));
  Serials.println(FallbackIp);
  Ethernet.begin(EEPROMdata.mac, FallbackIp);
  if (Ethernet.hardwareStatus() != EthernetNoHardware) {
    Serials.println(F("Using fallback ip"));
    return true;
  }
  return false;
}


void EthernetSetup() {
  Settings.Using_DHCP = 0;
  Settings.Using_StoredIp = 0;
  if (EEPROMdata.NetWorkSettings == eUse_Rtu) {
    Settings.UseTcp = 0;
    return;
  }
  // Initialize SPI with CS pin (usually 10 on Uno)
  Ethernet.init(10);
  Settings.UseTcp = 1;
  //Get the random generated mac addres
  GetMyMac();

  ConnectionFound = TestFixedIp();
  if (!ConnectionFound) ConnectionFound = TestDhcp();
  if (!ConnectionFound) ConnectionFound = TestFallBackIp();
  // Detect hardware
  auto status = Ethernet.hardwareStatus();
  if (status == EthernetNoHardware) {
    Serials.println("No Ethernet hardware detected!");
    Settings.UseTcp = 0;
    return;
  } else if (status == EthernetW5100) {
    Serials.println("W5100 detected");
  } else if (status == EthernetW5200) {
    Serials.println("W5200 detected");
  } else if (status == EthernetW5500) {
    Serials.println("W5500 detected");
  }
  //we have the functional ethernet chip
  Settings.UseTcp = 1;
  Serials.print(F("Using ip adress: "));
  Serials.println(Ethernet.localIP());
  //Starting Ethernet modbus
  ModbusTCPSlave mbTCP;  // Declare ModbusTCP instance
}
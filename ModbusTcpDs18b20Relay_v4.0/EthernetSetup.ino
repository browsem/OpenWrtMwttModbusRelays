#include <Ethernet.h>


void EthernetSetup() {
  // Initialize SPI with CS pin (usually 10 on Uno)
  Ethernet.init(10);
  //Get the random generated mac addres
  GetMyMac();


  //Look for stored settings

  //look for dhcpd 

  //Look use fallback ip



  // Detect hardware
  auto status = Ethernet.hardwareStatus();

  if (status == EthernetNoHardware) {
    Serials.println("No Ethernet hardware detected!");
  } else if (status == EthernetW5100) {
    Serials.println("W5100 detected");
  } else if (status == EthernetW5200) {
    Serials.println("W5200 detected");
  }
}
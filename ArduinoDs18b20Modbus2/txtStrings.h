
const char str0[] PROGMEM = "IREG_SETTINGS:";
const char str1[] PROGMEM = "IREG_IPADDRESS_12: ";
const char str2[] PROGMEM = "IREG_IPADDRESS_34: ";
const char str3[] PROGMEM = "IREG_TEMP1: ";
const char str4[] PROGMEM = "IREG_TEMP2: ";
const char str5[] PROGMEM = "HREG_CMD: ";
const char str6[] PROGMEM = "HREG_SETTINGS: ";
const char str7[] PROGMEM = "HREG_IPADDRESS_12: ";
const char str8[] PROGMEM = "HREG_IPADDRESS_34: ";
const char str9[] PROGMEM = "IREG_BUTTONS: ";
const char str10[] PROGMEM = "HREG_RELAYS: ";
const char str11[] PROGMEM = "Should use TCP";
const char str12[] PROGMEM = "Should use RTU";
const char str13[] PROGMEM = "configuration of tcp started";
const char str14[] PROGMEM = "Result from eeprom";
const char str15[] PROGMEM = "using ip from flash: ";
const char str16[] PROGMEM = "Testing dhcpd ...";
const char str17[] PROGMEM = "ip from dhcp: ";
const char str18[] PROGMEM = "using_FallbackIp: ";
const char str19[] PROGMEM = "configuration of rtu started";
const char str20[] PROGMEM = "Configuration done";
const char str21[] PROGMEM = "Server Running";
const char str22[] PROGMEM = "Booting";
const char str23[] PROGMEM = " , ";


//pointerArray
const char* const MessageLst[] PROGMEM = {
  str0,
  str1,
  str2,
  str3,
  str4,
  str5,
  str6,
  str7,
  str8,
  str9,
  str10,
  str11,
  str12,
  str13,
  str14,
  str15,
  str16,
  str17,
  str18,
  str19,
  str20,
  str21,
  str22,
  str23,
  }; /*
  str24,
  str25,
  str26,
  str27,
  str28,
  str29,
};
*/

enum eMessageLst : uint8_t {
  eMsg_IREG_SETTINGS = 0,
  eMsg_IREG_IPADDRESS_12,
  eMsg_IREG_IPADDRESS_34,
  eMsg_IREG_BUTTONS,
  eMsg_IREG_TEMP1,
  eMsg_IREG_TEMP2,
  eMsg_HREG_CMD,
  eMsg_HREG_SETTINGS,
  eMsg_HREG_IPADDRESS_12,
  eMsg_HREG_IPADDRESS_34,
  eMsg_HREG_RELAYS,
  eMsg_Should_use_TCP,
  eMsg_Should_use_RTU,
  eMsg_configuration_of_tcp_started,
  eMsg_Result_from_eeprom,
  eMsg_using_ip_from_flash,
  eMsg_Testing_dhcpd,
  eMsg_ip_from_dhcp,
  eMsg_using_FallbackIp,
  eMsg_configuration_of_rtu_started,
  eMsg_Configuration_done,
  eMsg_Server_Running,
  eMsg_Booting,
  eMsg_SpaceCommaSpace,

};
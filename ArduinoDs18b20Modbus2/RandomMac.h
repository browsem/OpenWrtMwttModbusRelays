#include <EEPROM.h>
#include "SerPrint.h"
extern byte mac[6];


void generateRandomMAC() {
  // First byte: 0x02 → locally administered, unicast
  mac[0] = 0x02;
  for (int i = 1; i < 6; i++) {
    mac[i] = random(0, 256);
  }
}

void saveMACtoEEPROM() {
  for (int i = 0; i < 6; i++) {
    EEPROM.write(i, mac[i]);
  }
}

void readMACfromEEPROM() {
  for (int i = 0; i < 6; i++) {
    mac[i] = EEPROM.read(i);
  }
}

bool isMACValid() {
  // Check if EEPROM contains a non‑zero MAC
  for (int i = 0; i < 6; i++) {
    if (mac[i] != 0xFF && mac[i] != 0x00) return true;
  }
  return false;
}

void PrintMac() {
  // Print MAC for debugging
  //Serial.print("Generated MAC: ");
  for (int i = 0; i < 6; i++) {
    if (mac[i] < 0x10) Serial.print("0");
    //Serial.print(mac[i], HEX);
    SerPrint(mac[i], HEX);
    if (i < 5) Serial.print(":");
  }
  //Serial.println();
  //SerPrintln();
}


void GetMyMac() {

  // Try to read MAC from EEPROM
  readMACfromEEPROM();
  if (!isMACValid()) {
    // Seed random generator with analog noise
    randomSeed(analogRead(A0));
   // SerPrintln(F("No valid MAC found, generating new one..."));
    //Serial.println("No valid MAC found, generating new one...");
    generateRandomMAC();
    saveMACtoEEPROM();
  } else {
    //Serial.println("Using stored MAC from EEPROM.");
    //SerPrintln(F("Using stored MAC from EEPROM."));
  }
  PrintMac();
}
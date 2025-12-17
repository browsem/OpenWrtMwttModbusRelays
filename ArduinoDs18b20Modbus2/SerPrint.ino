#include <Arduino.h>

// Generic template that forwards any type to Serial
template<typename T>
void SerPrint(const T &msg) {
    if (Settings.UseTcp == 1) {
        Serial.print(msg);
    }
    Serial1.print(msg);
   
}

template<typename T>
void SerPrintln(const T &msg) {
    if (Settings.UseTcp == 1) {
        Serial.println(msg);
    }    
    Serial1.println(msg);
    
}

void SerPrintFromRom(uint8_t StrName_in) {
    //get the string from flash
    char buffer[50];
    SerPrint(MessageLst[StrName_in]);
    //strcpy_P(buffer, (PGM_P)pgm_read_word(&(MessageLst[StrName_in])));
    //SerPrint(buffer);
}

void SerPrintFromRomln(uint8_t StrName_in) {
    //get the string from flash
   SerPrintFromRom(StrName_in);
   SerPrintln("");
}

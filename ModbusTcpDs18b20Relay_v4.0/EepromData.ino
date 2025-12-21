#include <EEPROM.h>

 void ReadEEPROM(int address,StEEPROM &cfg) {
    EEPROM.get(address, cfg);
}


void saveEEPROMIfChanged(int address, const StEEPROM &data) {
    StEEPROM existing;

    // Read current EEPROM contents
    EEPROM.get(address, existing);

    // Compare memory blocks
    if (memcmp(&existing, &data, sizeof(StEEPROM)) != 0) {
        // Something changed → write new struct
        EEPROM.put(address, data);
        Serials.println("EEPROM updated");
    } else {
        Serials.println("No change, EEPROM not written");
    }
}
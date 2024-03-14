#include "Pool_Management.h"
#include <EEPROM.h>
#include "CRC8.h"

// Persistent settings like calibrations and runtime settings.
persistent_settings persistentSettings = {};
runtime_settings runtimeSettings = {};


void setup() {
#if DEBUG == 1
  Serial.begin(115200);
#endif

  // Initialize the EEPROM
  // Header size + ((Persistent Settings size + 1 crc bit) * 2 copies of the settings data for backup)
  uint8_t settingsSize = 2 + ((sizeof(persistentSettings) + 1) * 2);
  logger_line("Requested EEPROM size: %d", settingsSize);
  EEPROM.begin(settingsSize);

  // Try to load the app settings from the EEPROM
  // First read the header and check it
  char readedHeader[2];
  EEPROM.readBytes(0, readedHeader, 2);

  // If the readed header is equal to the program header, then settings were stored. Try to read it.
  if (readedHeader[0] == header[0] && readedHeader[1] == header[1]) {
    logger_line("There are settings stored into the EEPROM. Loading it...");
    // Read the two settings data first before
    persistent_settings settingsReaded[2] = {};
    bool settingsOK[2] = {};

    // Read the settings copies into a temporal array
    logger_line("Reading the settings.");
    for (uint8_t i = 0; i < 2; i++) {
      EEPROM.readBytes(2 + (sizeof(persistent_settings) * i), &settingsReaded[i], sizeof(persistent_settings));
    }

    // Check one by one the settings copies
    logger_line("Checking both settings copies.");
    for (uint8_t i = 0; i < 2; i++) {
      CRC8 calcCRC = CRC8();
      calcCRC.add(reinterpret_cast<uint8_t *>(&settingsReaded[i]), sizeof(persistent_settings) - 1);
      if (calcCRC.calc() == settingsReaded[i].crc) {
        settingsOK[i] = true;
        logger_line("The settings %d are correct.", i);
      } else {
        settingsOK[i] = false;
        logger_line("The settings %d are damaged.", i);
      }
    }

    bool restored = false;
    // if the main settings are OK, load it...
    if (settingsOK[0]) {
      logger_line("Using the main settings as source.");
      memcpy(&persistentSettings, &settingsReaded[0], sizeof(persistent_settings));

      // If the backup settings were damaged, try to recover it
      if (!settingsOK[1]) {
        logger_line("The backup settings were damaged. Trying to restore it...");
        EEPROM.writeBytes(2 + sizeof(persistent_settings), &settingsReaded[0], sizeof(persistent_settings));
        restored = true;
      }
    } else if (settingsOK[1]) {
      logger_line("The main settings are damaged, so the backup will be used.");
      memcpy(&persistentSettings, &settingsReaded[1], sizeof(persistent_settings));

      // Try to restore the main settings
      logger_line("Trying to restore the main settings...");
      EEPROM.writeBytes(2, &settingsReaded[1], sizeof(persistent_settings));
      restored = true;
    } else {
      // All the settings are damaged, this is bad...
      logger_line("All the stored settings are damaged... Maybe the EEPROM is damaged.");
    }

    // If any of the settings was restored, check both again.
    if (restored) {
      logger_line("Some settings were restored. Checking it again...");
      // Read the settings copies into a temporal array
      for (uint8_t i = 0; i < 2; i++) {
        EEPROM.readBytes(2 + (sizeof(persistent_settings) * i), &settingsReaded[i], sizeof(persistent_settings));
      }

      // Check one by one the settings copies
      for (uint8_t i = 0; i < 2; i++) {
        CRC8 calcCRC = CRC8();
        calcCRC.add(reinterpret_cast<uint8_t *>(&settingsReaded[i]), sizeof(persistent_settings) - 1);
        if (calcCRC.calc() == settingsReaded[i].crc) {
          settingsOK[i] = true;
          logger_line("The settings %d are correct.", i);
        } else {
          settingsOK[i] = false;
          logger_line("The settings %d are damaged.", i);
        }
      }

      if (!settingsOK[0] || !settingsOK[1]) {
        // At this point, the EEPROM can be damaged because the data cannot be restored correctly
        logger_line("The EEPROM seems to be damnaged. It is recommended to change the start address.");
      }
    }
  }

  // Now that the settings were loaded, calculate the ph ranges if there were settings...
  if (persistentSettings.phCalibrationValueMin == 0 || persistentSettings.phCalibrationValueMed == 0 || persistentSettings.phCalibrationValueMax == 0) {
    // The probe is not well calibrated, so must be calibrated.
    logger_line("The PH probe is not calibrated so the PH cannot be measured correctly. Please, calibrate it.");

    // ToDo: Set the calibration for an standard probe
  } else {
    // Calculate the probe ranges
    logger_line("Calculating the PH probe calibration.");
    // The low range (between min and med calibrations). The mv per every 0.01 PH difference.
    runtimeSettings.phCalibrationLow = (persistentSettings.phCalibrationValueMed - persistentSettings.phCalibrationValueMin) / (persistentSettings.phCalibrationPhMed - persistentSettings.phCalibrationPhMin);
    // The low range (between med and max calibrations). The mv per every 0.01 PH difference.
    runtimeSettings.phCalibrationHigh = (persistentSettings.phCalibrationValueMax - persistentSettings.phCalibrationValueMed) / (persistentSettings.phCalibrationPhMax - persistentSettings.phCalibrationPhMed);
    logger_line("The calibrations settings are: Low = %d, High = %d. (must be similar)", runtimeSettings.phCalibrationLow, runtimeSettings.phCalibrationHigh);

    // ToDo: Check if both are consistentm by comparing them (I have to do some tests first).
  }
}

void loop() {
  // put your main code here, to run repeatedly:
}

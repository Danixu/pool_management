
#define DEBUG 1

#if DEBUG == 1
void logger(const char* format, ...) {
  va_list argptr;
  va_start(argptr, format);
  char strBuf[200];
  sprintf(strBuf, format, argptr);
  Serial.print(strBuf);
}
void logger_line(const char* format, ...) {
  va_list argptr;
  va_start(argptr, format);
  char strBuf[200];
  sprintf(strBuf, format, argptr);
  Serial.println(strBuf);
}
#else
#define logger(...) ;
#define logger_line(...) ;
#endif

// Pack the struct tight
#pragma pack(push, 1)
struct persistent_settings {
  // PH Calibration settings: voltage value of the reference calibration solutions in ADC value
  uint16_t phCalibrationValueMin = 0;
  uint16_t phCalibrationValueMed = 0;
  uint16_t phCalibrationValueMax = 0;
  // PH Calibration settings: Corresponding PH value of those calibration solutions * 100 -> 6.86 * 100 = 686.
  uint16_t phCalibrationPhMin = 0;
  uint16_t phCalibrationPhMed = 0;
  uint16_t phCalibrationPhMax = 0;

  // ORP calibration settings: The ADC value of the reference 256mv calibration solution
  uint16_t orpCalibration = 0;

  // Temperature correction
  uint16_t temperatureCorrection = 0;

  // CRC of the data
  uint8_t crc = 0;
};
#pragma pack(pop)

struct runtime_settings {
  // Calculated ph calibration data
  uint16_t phCalibrationLow = 0;
  uint16_t phCalibrationHigh = 0;
};

// EEPROM header used to determine if data was written
char header[2] = { 'P', 'M' };

// Endpoints
#define ESPZB_EP_BASIC 1
#define ESPZB_PUMP_SWITCH 2
#define ESPZB_PH_SENSOR 3
#define ESPZB_CHLORINE_SENSOR 4
#define ESPZB_ALGAECIDE_SENSOR 5

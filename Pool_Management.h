#include "zb_sensor.h"

#define TAG "Pool Management"

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

// Values storage
struct sensor_data {
  uint16_t value = 0;
  uint16_t targetLevel = 0;
  uint16_t depositLevel = 0;
  bool enabled = false;
};
struct global_data {
  bool enabled = false;
  bool pump = false;
  int32_t temperature = 2300;
  sensor_data algaecide;
  sensor_data chlorine;
  sensor_data ph;

  zb_sensor pump_sensor = zb_sensor();
  zb_sensor algaecide_sensor = zb_sensor();
  zb_sensor chlorine_sensor = zb_sensor();
  zb_sensor ph_sensor = zb_sensor();
};

// EEPROM header used to determine if data was written
char header[2] = { 'P', 'M' };

// Endpoints
#define ESPZB_EP_BASIC 1
#define ESPZB_EP_PUMP_SWITCH 2
#define ESPZB_EP_PH_SENSOR 3
#define ESPZB_EP_CHLORINE_SENSOR 4
#define ESPZB_EP_ALGAECIDE_SWITCH 5

// ClustersID
#define ESPZB_CID_CHLORINE_VALUE 0xfd10U
#define ESPZB_CID_CHLORINE_TARGET 0xfd11U
#define ESPZB_CID_CHLORINE_DEPOSIT 0xfd12U
#define ESPZB_CID_PH_VALUE 0xfd20U
#define ESPZB_CID_PH_TARGET 0xfd21U
#define ESPZB_CID_PH_DEPOSIT 0xfd22U

/********************* Global Variables ***************************/
global_data globalData;
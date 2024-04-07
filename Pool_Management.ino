/**
 * @brief This example demonstrates simple Zigbee light bulb.
 * 
 * The example demonstrates how to use ESP Zigbee stack to create a end device light bulb.
 * The light bulb is a Zigbee end device, which is controlled by a Zigbee coordinator.
 * 
 * Proper Zigbee mode must be selected in Tools->Zigbee mode 
 * and also the correct partition scheme must be selected in Tools->Partition Scheme.
 * 
 * Please check the README.md for instructions and more detailed description.
 */

#include "Pool_Management.h"
#include "zigbee_ed_functions.h"

#ifndef ZIGBEE_MODE_ED
#error "Zigbee end device mode is not selected in Tools->Zigbee mode"
#endif

#include "esp_zigbee_core.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#define LED_PIN RGB_BUILTIN

/* Zigbee configuration */
#define INSTALLCODE_POLICY_ENABLE false /* enable the install code policy for security */
#define ED_AGING_TIMEOUT ESP_ZB_ED_AGING_TIMEOUT_64MIN
#define ED_KEEP_ALIVE 3000                                               /* 3000 millisecond */
#define ESP_ZB_PRIMARY_CHANNEL_MASK ESP_ZB_TRANSCEIVER_ALL_CHANNELS_MASK /* Zigbee primary channel mask use in the example */

/* Default End Device config */
#define ESP_ZB_ZED_CONFIG() \
  { \
    .esp_zb_role = ESP_ZB_DEVICE_TYPE_ED, \
    .install_code_policy = INSTALLCODE_POLICY_ENABLE, \
    .nwk_cfg = { \
      .zed_cfg = { \
        .ed_timeout = ED_AGING_TIMEOUT, \
        .keep_alive = ED_KEEP_ALIVE, \
      }, \
    }, \
  }

#define ESP_ZB_DEFAULT_RADIO_CONFIG() \
  { \
    .radio_mode = RADIO_MODE_NATIVE, \
  }

#define ESP_ZB_DEFAULT_HOST_CONFIG() \
  { \
    .host_connection_mode = HOST_CONNECTION_MODE_NONE, \
  }

#define HA_ESP_SENSOR_ENDPOINT 1


/********************* Zigbee functions **************************/
static void esp_zb_task(void *pvParameters) {
  // Define cluster attributes
  char manufname[] = { 6, 'D', 'a', 'n', 'i', 'x', 'u' };
  char modelid[] = { 15, 'P', 'o', 'o', 'l', '.', 'M', 'a', 'n', 'a', 'g', 'e', 'm', 'e', 'n', 't' };

  int tempValue = 2300;
  int tempMin = 0;
  int tempMax = 32000;

  uint16_t phValue = 700;
  uint16_t phMin = 0;
  uint16_t phMax = 1400;

  uint16_t chlorineValue = 700;
  uint16_t chlorineMin = 0;
  uint16_t chlorineMax = 1400;


  // Initialize Zigbee stack
  esp_zb_cfg_t zb_nwk_cfg = ESP_ZB_ZED_CONFIG();
  esp_zb_init(&zb_nwk_cfg);


  // Create genBasic cluster/attribute list
  esp_zb_attribute_list_t *esp_zb_basic_cluster = esp_zb_zcl_attr_list_create(ESP_ZB_ZCL_CLUSTER_ID_BASIC);
  esp_zb_basic_cluster_add_attr(esp_zb_basic_cluster, ESP_ZB_ZCL_ATTR_BASIC_MANUFACTURER_NAME_ID, &manufname[0]);
  esp_zb_basic_cluster_add_attr(esp_zb_basic_cluster, ESP_ZB_ZCL_ATTR_BASIC_MODEL_IDENTIFIER_ID, &modelid[0]);


  // Create temperature cluster/attribute list
  esp_zb_attribute_list_t *esp_zb_temperature_cluster = esp_zb_zcl_attr_list_create(ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT);
  esp_zb_temperature_meas_cluster_add_attr(esp_zb_temperature_cluster, ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_VALUE_ID, &tempValue);
  esp_zb_temperature_meas_cluster_add_attr(esp_zb_temperature_cluster, ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_MIN_VALUE_ID, &tempMin);
  esp_zb_temperature_meas_cluster_add_attr(esp_zb_temperature_cluster, ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_MAX_VALUE_ID, &tempMax);


  // Create custom cluster for PH
  esp_zb_attribute_list_t *esp_zb_ph_cluster = esp_zb_zcl_attr_list_create(0xfd09U);
  esp_zb_cluster_add_attr(esp_zb_ph_cluster, 0xfd09U, 0x0000, ESP_ZB_ZCL_ATTR_TYPE_U16, ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY | ESP_ZB_ZCL_ATTR_ACCESS_REPORTING, &phValue);
  esp_zb_cluster_add_attr(esp_zb_ph_cluster, 0xfd09U, 0x0001, ESP_ZB_ZCL_ATTR_TYPE_U16, ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY | ESP_ZB_ZCL_ATTR_ACCESS_REPORTING, &phMax);
  esp_zb_cluster_add_attr(esp_zb_ph_cluster, 0xfd09U, 0x0002, ESP_ZB_ZCL_ATTR_TYPE_U16, ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY | ESP_ZB_ZCL_ATTR_ACCESS_REPORTING, &phMin);


  // Create custom cluster for Chlorine
  esp_zb_attribute_list_t *esp_zb_chlorine_cluster = esp_zb_zcl_attr_list_create(0xfd1aU);
  esp_zb_cluster_add_attr(esp_zb_chlorine_cluster, 0xfd1aU, 0x0000, ESP_ZB_ZCL_ATTR_TYPE_U16, ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY | ESP_ZB_ZCL_ATTR_ACCESS_REPORTING, &chlorineValue);
  esp_zb_cluster_add_attr(esp_zb_chlorine_cluster, 0xfd1aU, 0x0001, ESP_ZB_ZCL_ATTR_TYPE_U16, ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY | ESP_ZB_ZCL_ATTR_ACCESS_REPORTING, &chlorineMax);
  esp_zb_cluster_add_attr(esp_zb_chlorine_cluster, 0xfd1aU, 0x0002, ESP_ZB_ZCL_ATTR_TYPE_U16, ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY | ESP_ZB_ZCL_ATTR_ACCESS_REPORTING, &chlorineMin);


  // Create cluster list
  esp_zb_cluster_list_t *esp_zb_cluster_list = esp_zb_zcl_cluster_list_create();
  esp_zb_cluster_list_add_basic_cluster(esp_zb_cluster_list, esp_zb_basic_cluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
  esp_zb_cluster_list_add_temperature_meas_cluster(esp_zb_cluster_list, esp_zb_temperature_cluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
  esp_zb_cluster_list_add_custom_cluster(esp_zb_cluster_list, esp_zb_ph_cluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
  esp_zb_cluster_list_add_custom_cluster(esp_zb_cluster_list, esp_zb_chlorine_cluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);


  // Create endpoint list
  esp_zb_ep_list_t *esp_zb_ep_list = esp_zb_ep_list_create();
  // esp_zb_ep_list_add_ep(esp_zb_ep_list, esp_zb_cluster_list, HA_ESP_SENSOR_ENDPOINT, ESP_ZB_AF_HA_PROFILE_ID, ESP_ZB_HA_TEMPERATURE_SENSOR_DEVICE_ID);
  esp_zb_ep_list_add_ep(esp_zb_ep_list, esp_zb_cluster_list, HA_ESP_SENSOR_ENDPOINT, ESP_ZB_AF_HA_PROFILE_ID, ESP_ZB_HA_CUSTOM_ATTR_DEVICE_ID);


  // Register endpoint list
  esp_zb_device_register(esp_zb_ep_list);

  //esp_zb_factory_reset();

  // Error check, and start zigbee main loop
  ESP_ERROR_CHECK(esp_zb_start(true));
  esp_zb_main_loop_iteration();
}

esp_err_t zb_update_temperature(int32_t temperature) {
  /* Update temperature attribute */
  esp_err_t state = esp_zb_zcl_set_attribute_val(
    HA_ESP_SENSOR_ENDPOINT,
    ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT,
    ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
    ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_VALUE_ID,
    &temperature,
    false);

  /* Error check */
  if (state != ESP_ZB_ZCL_STATUS_SUCCESS) {
    logger_line("Updating temperature attribute failed!");
    return ESP_FAIL;
  }

  /* Report temperature attribute */
  static esp_zb_zcl_report_attr_cmd_t temperature_cmd_req = {
    { NULL, NULL, 1 },
    ESP_ZB_APS_ADDR_MODE_DST_ADDR_ENDP_NOT_PRESENT,
    ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT,
    ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
    ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_VALUE_ID
  };
  state = esp_zb_zcl_report_attr_cmd_req(&temperature_cmd_req);

  /* Error check */
  if (state != ESP_ZB_ZCL_STATUS_SUCCESS) {
    logger_line("Reporting temperature attribute failed!");
    return ESP_FAIL;
  }

  return ESP_OK;
}


esp_err_t zb_update_ph(int32_t ph) {
  /* Update ph attribute */
  esp_err_t state = esp_zb_zcl_set_attribute_val(
    HA_ESP_SENSOR_ENDPOINT,
    0xfd09U,
    ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
    0x0000,
    &ph,
    false);

  /* Error check */
  if (state != ESP_ZB_ZCL_STATUS_SUCCESS) {
    logger_line("Updating ph attribute failed!");
    return ESP_FAIL;
  }

  /* Report ph attribute */
  static esp_zb_zcl_report_attr_cmd_t ph_cmd_req = {
    { NULL, NULL, 1 },
    ESP_ZB_APS_ADDR_MODE_DST_ADDR_ENDP_NOT_PRESENT,
    0xfd09U,
    ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
    0x0000
  };
  state = esp_zb_zcl_report_attr_cmd_req(&ph_cmd_req);

  /* Error check */
  if (state != ESP_ZB_ZCL_STATUS_SUCCESS) {
    logger_line("Reporting ph attribute failed!");
    return ESP_FAIL;
  }

  return ESP_OK;
}

esp_err_t zb_update_chlorine(int32_t chlorine) {
  /* Update chlorine attribute */
  esp_err_t state = esp_zb_zcl_set_attribute_val(
    HA_ESP_SENSOR_ENDPOINT,
    0xfd1aU,
    ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
    0x0000,
    &chlorine,
    false);

  /* Error check */
  if (state != ESP_ZB_ZCL_STATUS_SUCCESS) {
    logger_line("Updating chlorine attribute failed!");
    return ESP_FAIL;
  }

  /* Report chlorine attribute */
  static esp_zb_zcl_report_attr_cmd_t chlorine_cmd_req = {
    { NULL, NULL, 1 },
    ESP_ZB_APS_ADDR_MODE_DST_ADDR_ENDP_NOT_PRESENT,
    0xfd1aU,
    ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
    0x0000
  };
  state = esp_zb_zcl_report_attr_cmd_req(&chlorine_cmd_req);

  /* Error check */
  if (state != ESP_ZB_ZCL_STATUS_SUCCESS) {
    logger_line("Reporting chlorine attribute failed!");
    return ESP_FAIL;
  }

  return ESP_OK;
}

/********************* Arduino functions **************************/
void setup() {
#if DEBUG == 1
  Serial.begin(115200);
#endif

  // Init Zigbee
  esp_zb_platform_config_t config = {
    .radio_config = ESP_ZB_DEFAULT_RADIO_CONFIG(),
    .host_config = ESP_ZB_DEFAULT_HOST_CONFIG(),
  };
  ESP_ERROR_CHECK(esp_zb_platform_config(&config));
  // Start Zigbee task
  xTaskCreate(esp_zb_task, "Zigbee_main", 4096, NULL, 5, NULL);
}

uint64_t last_run = 0;

void loop() {
  uint64_t new_run = millis() / 5000;
  //empty, zigbee running in task
  if (new_run != last_run) {
    Serial.println("Updating tempearure.");
    last_run = new_run;
    zb_update_temperature(random(1000, 4000));
    zb_update_ph(random(1000, 4000));
    zb_update_chlorine(random(1000, 4000));
  }
}


/*
#include "Pool_Management.h"
#include "zigbee_ed_functions.h"
#include <EEPROM.h>
#include "CRC8.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Persistent settings like calibrations and runtime settings.
persistent_settings persistentSettings = {};
runtime_settings runtimeSettings = {};

void setup() {
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
*/
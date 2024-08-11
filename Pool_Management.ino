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
#include "zb_sensor.h"

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
    .radio_mode = ZB_RADIO_MODE_NATIVE, \
  }

#define ESP_ZB_DEFAULT_HOST_CONFIG() \
  { \
    .host_connection_mode = ZB_HOST_CONNECTION_MODE_NONE, \
  }


/********************* Zigbee functions **************************/
static esp_err_t common_action_handler(esp_zb_core_action_callback_id_t callback_id, const void *message) {
  // This handlers cannot be registered directly because they are not static,
  // so we will use this "caller" function.
  globalData.temperature_sensor.actionHandler(callback_id, message);
  globalData.pump_sensor.actionHandler(callback_id, message);
  globalData.ph_sensor.actionHandler(callback_id, message);
  globalData.chlorine_sensor.actionHandler(callback_id, message);
  globalData.algaecide_sensor.actionHandler(callback_id, message);
  return ESP_OK;
}


static void esp_zb_task(void *pvParameters) {
  // Create the endpoints list
  esp_zb_ep_list_t *esp_zb_ep_list = esp_zb_ep_list_create();

  // Initialize Zigbee stack
  esp_zb_cfg_t zb_nwk_cfg = ESP_ZB_ZED_CONFIG();
  esp_zb_init(&zb_nwk_cfg);

  //
  // ------------------------------ Cluster BASIC ------------------------------
  //

  // Basic cluster configuration
  esp_zb_basic_cluster_cfg_t basic_cluster_cfg = {
    .zcl_version = ESP_ZB_ZCL_BASIC_ZCL_VERSION_DEFAULT_VALUE,
    .power_source = 0x01,
  };
  uint32_t ApplicationVersion = 0x0001;
  uint32_t StackVersion = 0x0002;
  uint32_t HWVersion = 0x0001;
  DEFINE_PSTRING(ManufacturerName, "Danixu");
  DEFINE_PSTRING(ModelIdentifier, "Pool Management");
  DEFINE_PSTRING(DateCode, "20240805");

  // Create the basic clusters list
  esp_zb_cluster_list_t *esp_zb_basic_cluster_list = esp_zb_zcl_cluster_list_create();
  // Create genBasic cluster/attributes
  esp_zb_attribute_list_t *esp_zb_basic_cluster = esp_zb_basic_cluster_create(&basic_cluster_cfg);
  esp_zb_basic_cluster_add_attr(esp_zb_basic_cluster, ESP_ZB_ZCL_ATTR_BASIC_APPLICATION_VERSION_ID, &ApplicationVersion);
  esp_zb_basic_cluster_add_attr(esp_zb_basic_cluster, ESP_ZB_ZCL_ATTR_BASIC_STACK_VERSION_ID, &StackVersion);
  esp_zb_basic_cluster_add_attr(esp_zb_basic_cluster, ESP_ZB_ZCL_ATTR_BASIC_HW_VERSION_ID, &HWVersion);
  esp_zb_basic_cluster_add_attr(esp_zb_basic_cluster, ESP_ZB_ZCL_ATTR_BASIC_MANUFACTURER_NAME_ID, (void *)&ManufacturerName);
  esp_zb_basic_cluster_add_attr(esp_zb_basic_cluster, ESP_ZB_ZCL_ATTR_BASIC_MODEL_IDENTIFIER_ID, (void *)&ModelIdentifier);
  esp_zb_basic_cluster_add_attr(esp_zb_basic_cluster, ESP_ZB_ZCL_ATTR_BASIC_DATE_CODE_ID, (void *)&DateCode);
  // Add the cluster to the basic clusters list
  esp_zb_cluster_list_add_basic_cluster(esp_zb_basic_cluster_list, esp_zb_basic_cluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);

  // Create the time cluster/attributes
  esp_zb_attribute_list_t *esp_zb_time_cluster = esp_zb_zcl_attr_list_create(ESP_ZB_ZCL_CLUSTER_ID_TIME);
  uint32_t current_time = 0;
  esp_zb_time_cluster_add_attr(esp_zb_time_cluster, ESP_ZB_ZCL_ATTR_TIME_TIME_ID, &current_time);
  // Add the cluster to the basic list
  esp_zb_cluster_list_add_time_cluster(esp_zb_basic_cluster_list, esp_zb_time_cluster, ESP_ZB_ZCL_CLUSTER_CLIENT_ROLE);

  // Create temperature cluster/attributes
  globalData.temperature_sensor.init(esp_zb_ep_list, ESPZB_EP_BASIC, true, false, false, esp_zb_basic_cluster_list);


  //
  // ------------------------------ Cluster Pump Switch ------------------------------
  //
  globalData.pump_sensor.init(esp_zb_ep_list, ESPZB_EP_PUMP_SWITCH, false, false, false);


  //
  // ------------------------------ Cluster PH ------------------------------
  //
  globalData.ph_sensor.init(esp_zb_ep_list, ESPZB_EP_PH_SENSOR, true, true, true);


  //
  // ------------------------------ Cluster Chlorine ------------------------------
  //
  globalData.chlorine_sensor.init(esp_zb_ep_list, ESPZB_EP_CHLORINE_SENSOR, true, true, true);


  //
  // ------------------------------ Cluster Algaecide Switch ------------------------------
  //
  globalData.algaecide_sensor.init(esp_zb_ep_list, ESPZB_EP_ALGAECIDE_SWITCH, false, false, true);


  //
  // Register the endpoint list in the device and start the ZB stack.
  //

  // Register endpoint list
  esp_zb_device_register(esp_zb_ep_list);

  // Add the action handler to manage the changes made to the attributes and more.
  esp_zb_core_action_handler_register(zb_action_handler);
  esp_zb_core_action_handler_register(common_action_handler);

  // Set the primary network channel mask
  esp_zb_set_primary_network_channel_set(ESP_ZB_PRIMARY_CHANNEL_MASK);

  // Erase NVRAM before creating connection to new Coordinator
  //esp_zb_nvram_erase_at_start(true);  //Comment out this line to erase NVRAM data if you are conneting to new Coordinator
  // Factory reset if the above doesn't work
  //esp_zb_factory_reset();

  // Error check, and start zigbee main loop
  ESP_ERROR_CHECK(esp_zb_start(true));
  esp_zb_main_loop_iteration();
}


/********************* Arduino functions **************************/
void setup() {
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
    log_d("Updating data.");
    last_run = new_run;
    //zb_update_temperature(random(1000, 4000));
    globalData.temperature_sensor.setCurrentValue(random(1000, 4000));
    globalData.ph_sensor.setCurrentValue(random(1000, 1400));
    globalData.ph_sensor.setDepositValue(random(0, 100));
    globalData.chlorine_sensor.setCurrentValue(random(1000, 1400));
    globalData.chlorine_sensor.setDepositValue(random(0, 100));
    globalData.algaecide_sensor.setDepositValue(random(0, 100));

    globalData.temperature_sensor.report();
    globalData.ph_sensor.report();
    globalData.chlorine_sensor.report();
    globalData.algaecide_sensor.report();

    // Report the status
    log_v("Global enabled: %d - Pump: %d", globalData.temperature_sensor.isEnabled(), globalData.pump_sensor.isEnabled());
    log_v("Temperature: %0.2f", (float)globalData.temperature_sensor.getCurrentValue() / 100);
    log_v("Chlorine: %0.2f - Enabled: %d - Deposit Level: %d - Target Level %0.2f", (float)globalData.chlorine_sensor.getCurrentValue() / 100, globalData.chlorine_sensor.isEnabled(), globalData.chlorine_sensor.getDepositValue(), (float)globalData.chlorine_sensor.getTargetValue() / 100);
    log_v("PH: %0.2f - Enabled: %d - Deposit Level: %d - Target Level %0.2f", (float)globalData.ph_sensor.getCurrentValue() / 100, globalData.ph_sensor.isEnabled(), globalData.ph_sensor.getDepositValue(), (float)globalData.ph_sensor.getTargetValue() / 100);
    log_v("Algaecide Enabled: %d - Deposit Level: %d", globalData.algaecide_sensor.isEnabled(), globalData.algaecide_sensor.getDepositValue());
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
  D_println("Requested EEPROM size: %d", settingsSize);
  EEPROM.begin(settingsSize);

  // Try to load the app settings from the EEPROM
  // First read the header and check it
  char readedHeader[2];
  EEPROM.readBytes(0, readedHeader, 2);

  // If the readed header is equal to the program header, then settings were stored. Try to read it.
  if (readedHeader[0] == header[0] && readedHeader[1] == header[1]) {
    D_println("There are settings stored into the EEPROM. Loading it...");
    // Read the two settings data first before
    persistent_settings settingsReaded[2] = {};
    bool settingsOK[2] = {};

    // Read the settings copies into a temporal array
    D_println("Reading the settings.");
    for (uint8_t i = 0; i < 2; i++) {
      EEPROM.readBytes(2 + (sizeof(persistent_settings) * i), &settingsReaded[i], sizeof(persistent_settings));
    }

    // Check one by one the settings copies
    D_println("Checking both settings copies.");
    for (uint8_t i = 0; i < 2; i++) {
      CRC8 calcCRC = CRC8();
      calcCRC.add(reinterpret_cast<uint8_t *>(&settingsReaded[i]), sizeof(persistent_settings) - 1);
      if (calcCRC.calc() == settingsReaded[i].crc) {
        settingsOK[i] = true;
        D_println("The settings %d are correct.", i);
      } else {
        settingsOK[i] = false;
        D_println("The settings %d are damaged.", i);
      }
    }

    bool restored = false;
    // if the main settings are OK, load it...
    if (settingsOK[0]) {
      D_println("Using the main settings as source.");
      memcpy(&persistentSettings, &settingsReaded[0], sizeof(persistent_settings));

      // If the backup settings were damaged, try to recover it
      if (!settingsOK[1]) {
        D_println("The backup settings were damaged. Trying to restore it...");
        EEPROM.writeBytes(2 + sizeof(persistent_settings), &settingsReaded[0], sizeof(persistent_settings));
        restored = true;
      }
    } else if (settingsOK[1]) {
      D_println("The main settings are damaged, so the backup will be used.");
      memcpy(&persistentSettings, &settingsReaded[1], sizeof(persistent_settings));

      // Try to restore the main settings
      D_println("Trying to restore the main settings...");
      EEPROM.writeBytes(2, &settingsReaded[1], sizeof(persistent_settings));
      restored = true;
    } else {
      // All the settings are damaged, this is bad...
      D_println("All the stored settings are damaged... Maybe the EEPROM is damaged.");
    }

    // If any of the settings was restored, check both again.
    if (restored) {
      D_println("Some settings were restored. Checking it again...");
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
          D_println("The settings %d are correct.", i);
        } else {
          settingsOK[i] = false;
          D_println("The settings %d are damaged.", i);
        }
      }

      if (!settingsOK[0] || !settingsOK[1]) {
        // At this point, the EEPROM can be damaged because the data cannot be restored correctly
        D_println("The EEPROM seems to be damnaged. It is recommended to change the start address.");
      }
    }
  }

  // Now that the settings were loaded, calculate the ph ranges if there were settings...
  if (persistentSettings.phCalibrationValueMin == 0 || persistentSettings.phCalibrationValueMed == 0 || persistentSettings.phCalibrationValueMax == 0) {
    // The probe is not well calibrated, so must be calibrated.
    D_println("The PH probe is not calibrated so the PH cannot be measured correctly. Please, calibrate it.");

    // ToDo: Set the calibration for an standard probe
  } else {
    // Calculate the probe ranges
    D_println("Calculating the PH probe calibration.");
    // The low range (between min and med calibrations). The mv per every 0.01 PH difference.
    runtimeSettings.phCalibrationLow = (persistentSettings.phCalibrationValueMed - persistentSettings.phCalibrationValueMin) / (persistentSettings.phCalibrationPhMed - persistentSettings.phCalibrationPhMin);
    // The low range (between med and max calibrations). The mv per every 0.01 PH difference.
    runtimeSettings.phCalibrationHigh = (persistentSettings.phCalibrationValueMax - persistentSettings.phCalibrationValueMed) / (persistentSettings.phCalibrationPhMax - persistentSettings.phCalibrationPhMed);
    D_println("The calibrations settings are: Low = %d, High = %d. (must be similar)", runtimeSettings.phCalibrationLow, runtimeSettings.phCalibrationHigh);

    // ToDo: Check if both are consistentm by comparing them (I have to do some tests first).
  }
}
*/
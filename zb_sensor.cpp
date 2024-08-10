#include "zb_sensor.h"

zb_sensor::zb_sensor() {
}


void zb_sensor::init(esp_zb_ep_list_t *endpointList, uint8_t endpoint, uint16_t clusterId, bool sensor, bool targetLevel, bool depositLevel, esp_zb_cluster_list_t *clusterList) {
  // Create the Cluster List if it's not provided
  if (clusterList == nullptr) {
    _clusterList = esp_zb_zcl_cluster_list_create();
  } else {
    _clusterList = clusterList;
  }


  // Create the switch cluster/attributes
  _switchCluster = esp_zb_zcl_attr_list_create(ESP_ZB_ZCL_CLUSTER_ID_ON_OFF);
  esp_zb_on_off_cluster_add_attr(_switchCluster, ESP_ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID, &_enabled);
  // Add the switch cluster to the list
  esp_zb_cluster_list_add_on_off_cluster(_clusterList, _switchCluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);


  // Create custom cluster for the sensor
  if (sensor) {
    _clusterId = clusterId;
    _sensorCluster = esp_zb_zcl_attr_list_create(clusterId);
    esp_zb_cluster_add_attr(_sensorCluster, clusterId, 0x0000, ESP_ZB_ZCL_ATTR_TYPE_U16, ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY | ESP_ZB_ZCL_ATTR_ACCESS_REPORTING, &_minValue);
    esp_zb_cluster_add_attr(_sensorCluster, clusterId, 0x0001, ESP_ZB_ZCL_ATTR_TYPE_U16, ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY | ESP_ZB_ZCL_ATTR_ACCESS_REPORTING, &_maxValue);
    esp_zb_cluster_add_attr(_sensorCluster, clusterId, 0x0002, ESP_ZB_ZCL_ATTR_TYPE_U16, ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY | ESP_ZB_ZCL_ATTR_ACCESS_REPORTING, &_minValue);
    // Create Chlorine cluster
    esp_zb_cluster_list_add_custom_cluster(_clusterList, _sensorCluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
  }


  // Create the level client cluster (Target level for the sensor)
  if (targetLevel) {
    _sensorTargetLevelCluster = esp_zb_zcl_attr_list_create(ESP_ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL);
    esp_zb_cluster_list_add_level_cluster(_clusterList, _sensorTargetLevelCluster, ESP_ZB_ZCL_CLUSTER_CLIENT_ROLE);
  }


  // Create the level server cluster (Desposit level reporter)
  if (depositLevel) {
    _depositCurrentLevelCluster = esp_zb_zcl_attr_list_create(ESP_ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL);
    esp_zb_cluster_list_add_level_cluster(_clusterList, _depositCurrentLevelCluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
  }

  if (endpoint != nullptr) {
    // Add the endpoint to the list
    esp_zb_endpoint_config_t endpoint_config = {
      .endpoint = endpoint,
      .app_profile_id = ESP_ZB_AF_HA_PROFILE_ID,
      .app_device_id = ESP_ZB_HA_CUSTOM_ATTR_DEVICE_ID,
      .app_device_version = 0
    };
    esp_zb_ep_list_add_ep(endpointList, _clusterList, endpoint_config);
  }
}

esp_err_t zb_sensor::report(uint16_t newValue) {
  _currentValue = newValue;

  // Update the attribute
  esp_err_t state = esp_zb_zcl_set_attribute_val(
    _endpoint,
    _clusterId,
    ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
    0x0000,
    &_currentValue,
    false);

  // Error check
  if (state != ESP_ZB_ZCL_STATUS_SUCCESS) {
    log_e("Updating sensor attribute failed!");
    return ESP_FAIL;
  }

  // Report ph attribute
  static esp_zb_zcl_report_attr_cmd_t cmd_req = {
    { NULL, NULL, _endpoint },
    ESP_ZB_APS_ADDR_MODE_DST_ADDR_ENDP_NOT_PRESENT,
    _clusterId,
    ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
    0x0000
  };
  state = esp_zb_zcl_report_attr_cmd_req(&cmd_req);

  // Error check
  if (state != ESP_ZB_ZCL_STATUS_SUCCESS) {
    log_e("Reporting sensor attribute failed!");
    return ESP_FAIL;
  }

  return ESP_OK;
}
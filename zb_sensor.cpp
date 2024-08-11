#include "zb_sensor.h"

zb_sensor::zb_sensor(uint16_t valueClusterID, uint16_t targetValueClusterID, uint16_t depositLevelClusterID) {
  _valueClusterID = valueClusterID;
  _targetValueClusterID = targetValueClusterID;
  _depositValueClusterID = depositLevelClusterID;
}


void zb_sensor::init(esp_zb_ep_list_t *endpointList, uint8_t endpoint, bool sensor, bool targetValue, bool depositLevel, esp_zb_cluster_list_t *clusterList) {
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
    if (_valueClusterID == ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT) {
      esp_zb_attribute_list_t *_sensorCluster = esp_zb_zcl_attr_list_create(_valueClusterID);
      esp_zb_temperature_meas_cluster_add_attr(_sensorCluster, ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_VALUE_ID, &_currentValue);
      esp_zb_temperature_meas_cluster_add_attr(_sensorCluster, ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_MIN_VALUE_ID, &_minValue);
      esp_zb_temperature_meas_cluster_add_attr(_sensorCluster, ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_MAX_VALUE_ID, &_maxValue);
      // Add the cluster to the list
      esp_zb_cluster_list_add_temperature_meas_cluster(_clusterList, _sensorCluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
    } else {
      _sensorCluster = esp_zb_zcl_attr_list_create(_valueClusterID);
      esp_zb_cluster_add_attr(_sensorCluster, _valueClusterID, 0x0000, ESP_ZB_ZCL_ATTR_TYPE_U16, ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY | ESP_ZB_ZCL_ATTR_ACCESS_REPORTING, &_currentValue);
      esp_zb_cluster_add_attr(_sensorCluster, _valueClusterID, 0x0001, ESP_ZB_ZCL_ATTR_TYPE_U16, ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY | ESP_ZB_ZCL_ATTR_ACCESS_REPORTING, &_minValue);
      esp_zb_cluster_add_attr(_sensorCluster, _valueClusterID, 0x0002, ESP_ZB_ZCL_ATTR_TYPE_U16, ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY | ESP_ZB_ZCL_ATTR_ACCESS_REPORTING, &_maxValue);
      // Create Chlorine cluster
      esp_zb_cluster_list_add_custom_cluster(_clusterList, _sensorCluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
    }
  }

  // Create the level client cluster (Target level for the sensor)
  if (_targetValueClusterID) {
    _sensorTargetLevelCluster = esp_zb_zcl_attr_list_create(_targetValueClusterID);
    esp_zb_cluster_add_attr(_sensorTargetLevelCluster, _targetValueClusterID, 0x0000, ESP_ZB_ZCL_ATTR_TYPE_U16, ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE | ESP_ZB_ZCL_ATTR_ACCESS_REPORTING, &_targetValue);
    esp_zb_cluster_list_add_custom_cluster(_clusterList, _sensorTargetLevelCluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
  }

  // Create the level server cluster (Desposit level reporter)
  if (_depositValueClusterID) {
    _depositCurrentLevelCluster = esp_zb_zcl_attr_list_create(_depositValueClusterID);
    esp_zb_cluster_add_attr(_depositCurrentLevelCluster, _depositValueClusterID, 0x0000, ESP_ZB_ZCL_ATTR_TYPE_U8, ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY | ESP_ZB_ZCL_ATTR_ACCESS_REPORTING, &_depositValue);
    esp_zb_cluster_list_add_custom_cluster(_clusterList, _depositCurrentLevelCluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
  }

  if (endpoint != 0) {
    _endpoint = endpoint;
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

esp_err_t zb_sensor::report() {
  esp_err_t state = ESP_OK;
  log_v("Reporting the sensor changes");
  // Update the current value attribute
  if (_valueClusterID) {
    log_v("Reporting the sensor value changes of the cluster 0x%x", _valueClusterID);
    state = esp_zb_zcl_set_attribute_val(
      _endpoint,
      _valueClusterID,
      ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
      0x0000,
      &_currentValue,
      false);

    // Error check
    if (state != ESP_ZB_ZCL_STATUS_SUCCESS) {
      log_e("Updating sensor value attribute failed!");
      return ESP_FAIL;
    }
  }

  if (_targetValueClusterID) {
    log_v("Reporting the sensor target value changes of the cluster 0x%x", _targetValueClusterID);
    state = esp_zb_zcl_set_attribute_val(
      _endpoint,
      _targetValueClusterID,
      ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
      0x0000,
      &_targetValue,
      false);

    // Error check
    if (state != ESP_ZB_ZCL_STATUS_SUCCESS) {
      log_e("Updating sensor target value attribute failed!");
      return ESP_FAIL;
    }
  }

  // Update the deposit value attribute
  if (_depositValueClusterID) {
    log_v("Reporting the sensor deposit value changes of the cluster 0x%x", _depositValueClusterID);
    state = esp_zb_zcl_set_attribute_val(
      _endpoint,
      _depositValueClusterID,
      ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
      0x0000,
      &_depositValue,
      false);

    // Error check
    if (state != ESP_ZB_ZCL_STATUS_SUCCESS) {
      log_e("Updating deposit value attribute failed!");
      return ESP_FAIL;
    }
  }

  /*
  // Report the attribute
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
  */

  return ESP_OK;
}

esp_err_t zb_sensor::actionHandler(esp_zb_core_action_callback_id_t callback_id, const void *message) {
  if (callback_id == ESP_ZB_CORE_SET_ATTR_VALUE_CB_ID) {
    const esp_zb_zcl_set_attr_value_message_t *attr_message = (esp_zb_zcl_set_attr_value_message_t *)message;
    esp_err_t ret = ESP_OK;

    if (!attr_message) {
      log_e("Empty message");
    }
    if (attr_message->info.status != ESP_ZB_ZCL_STATUS_SUCCESS) {
      log_e("Received message: error status(%d)", attr_message->info.status);
    }
    if (attr_message->info.dst_endpoint == _endpoint) {
      // Basic endpoint
      log_d("Updating the sensor data");
      log_v(
        "Received message: endpoint(%d), cluster(0x%x), attribute(0x%x), data size(%d)",
        attr_message->info.dst_endpoint, attr_message->info.cluster, attr_message->attribute.id,
        attr_message->attribute.data.size);

      if (attr_message->info.cluster == ESP_ZB_ZCL_CLUSTER_ID_ON_OFF) {
        // Switch cluster
        _enabled = attr_message->attribute.data.value ? *(bool *)attr_message->attribute.data.value : 0;
        log_v("Switch sets to %s", _enabled ? "On" : "Off");
      }

      if (attr_message->info.cluster == _targetValueClusterID) {
        // Target value cluster
        _targetValue = attr_message->attribute.data.value ? *(uint16_t *)attr_message->attribute.data.value : 0;
        log_v("Target Value sets to %0.2f", _targetValue / 100.0);
      }
    }

    return ret;
  }
}
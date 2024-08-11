#include "esp32-hal-log.h"
#include "esp_zigbee_core.h"

#ifndef _ZB_SENSOR_H_
#define _ZB_SENSOR_H_

class zb_sensor {
public:
  zb_sensor(uint16_t valueClusterID = 0, uint16_t targetValueClusterID = 0, uint16_t depositLevelClusterID = 0);
  void init(esp_zb_ep_list_t *endpointList, uint8_t endpoint, bool sensor = true, bool targetLevel = true, bool depositLevel = true, esp_zb_cluster_list_t *clusterList = nullptr);
  esp_err_t report();
  esp_err_t actionHandler(esp_zb_core_action_callback_id_t callback_id, const void *message);
  void setEnabled(bool state) { _enabled = state; };
  esp_zb_cluster_list_t *getEndpoint() { return _clusterList; }
  uint16_t getCurrentValue() { return _currentValue; }
  uint16_t getDepositValue() { return _depositValue; }
  uint16_t getTargetValue() { return _targetValue; }
  void setCurrentValue(uint16_t value) { _currentValue = value; }
  void setDepositValue(uint8_t value) { _depositValue = value; }
  bool isEnabled() { return _enabled; }

private:
  // Values variables
  uint16_t _minValue = 0;
  uint16_t _maxValue = 0;
  uint16_t _currentValue = 0;
  uint16_t _targetValue = 720;
  uint8_t _depositValue = 0;
  bool _enabled = false;

  // Zigbee variables
  esp_zb_cluster_list_t *_clusterList = nullptr;
  esp_zb_attribute_list_t *_switchCluster = nullptr;
  esp_zb_attribute_list_t *_sensorCluster = nullptr;
  esp_zb_attribute_list_t *_sensorTargetLevelCluster = nullptr;
  esp_zb_attribute_list_t *_depositCurrentLevelCluster = nullptr;
  uint16_t _valueClusterID = 0;
  uint16_t _targetValueClusterID = 0;
  uint16_t _depositValueClusterID = 0;
  uint8_t _endpoint = 0;
};

#endif // _ZB_SENSOR_H_
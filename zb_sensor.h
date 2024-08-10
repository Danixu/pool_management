#include "esp_zigbee_core.h"

#ifndef _ZB_SENSOR_H_
#define _ZB_SENSOR_H_

class zb_sensor {
public:
  zb_sensor();
  void init(esp_zb_ep_list_t *endpointList, uint8_t endpoint, uint16_t clusterId = 0, bool sensor = true, bool targetLevel = true, bool depositLevel = true, esp_zb_cluster_list_t *clusterList = nullptr);
  esp_err_t report(uint16_t newValue);
  esp_zb_cluster_list_t *getEndpoint() { return _clusterList; }

private:
  // Values variables
  uint16_t _minValue = 0;
  uint16_t _maxValue = 0;
  uint16_t _currentValue = 0;
  uint16_t _targetLevel = 0;
  uint16_t _depositLevel = 0;
  bool _enabled = false;

  // Zigbee variables
  esp_zb_cluster_list_t *_clusterList = nullptr;
  esp_zb_attribute_list_t *_switchCluster = nullptr;
  esp_zb_attribute_list_t *_sensorCluster = nullptr;
  esp_zb_attribute_list_t *_sensorTargetLevelCluster = nullptr;
  esp_zb_attribute_list_t *_depositCurrentLevelCluster = nullptr;
  uint16_t _clusterId = 0;
  uint8_t _endpoint = nullptr;
};

#endif // _ZB_SENSOR_H_
#include "esp_zigbee_core.h"
#include "esp_check.h"
#include "esp_err.h"

#ifndef ZIGBEE_MODE_ED
#error "Zigbee end device mode is not selected in Tools->Zigbee mode"
#endif

/* Default End Device config */
#define ESP_ZB_ZED_CONFIG() \
  { \
    .esp_zb_role = ESP_ZB_DEVICE_TYPE_ED, \
    .install_code_policy = false, \
    .nwk_cfg = { \
      .zed_cfg = { \
        .ed_timeout = ESP_ZB_ED_AGING_TIMEOUT_64MIN, \
        .keep_alive = 3000, \
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


/********************* Zigbee functions **************************/
static void bdb_start_top_level_commissioning_cb(uint8_t mode_mask) {
  if (esp_zb_bdb_start_top_level_commissioning(mode_mask) != ESP_OK) {
    logger_line("Failed to start Zigbee bdb commissioning");
  }
}

void esp_zb_app_signal_handler(esp_zb_app_signal_t *signal_struct) {
  uint32_t *p_sg_p = signal_struct->p_app_signal;
  esp_err_t err_status = signal_struct->esp_err_status;
  esp_zb_app_signal_type_t sig_type = (esp_zb_app_signal_type_t)*p_sg_p;
  esp_zb_zdo_signal_leave_params_t *leave_params = NULL;
  switch (sig_type) {
    case ESP_ZB_BDB_SIGNAL_DEVICE_FIRST_START:
    case ESP_ZB_BDB_SIGNAL_DEVICE_REBOOT:
    case ESP_ZB_BDB_SIGNAL_STEERING:
      if (err_status != ESP_OK) {
        logger_line("Stack %s failure with %s status, steering", esp_zb_zdo_signal_to_string(sig_type), esp_err_to_name(err_status));
        esp_zb_scheduler_alarm((esp_zb_callback_t)bdb_start_top_level_commissioning_cb, ESP_ZB_BDB_MODE_NETWORK_STEERING, 1000);
      } else {
        esp_zb_ieee_addr_t extended_pan_id;
        esp_zb_get_extended_pan_id(extended_pan_id);
        logger_line("Joined network successfully (Extended PAN ID: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x, PAN ID: 0x%04hx, Channel:%d, Short Address: 0x%04hx)",
                 extended_pan_id[7], extended_pan_id[6], extended_pan_id[5], extended_pan_id[4],
                 extended_pan_id[3], extended_pan_id[2], extended_pan_id[1], extended_pan_id[0],
                 esp_zb_get_pan_id(), esp_zb_get_current_channel(), esp_zb_get_short_address());
      }
      break;
    case ESP_ZB_ZDO_SIGNAL_LEAVE:
      leave_params = (esp_zb_zdo_signal_leave_params_t *)esp_zb_app_signal_get_params(p_sg_p);
      if (leave_params->leave_type == ESP_ZB_NWK_LEAVE_TYPE_RESET) {
        logger_line("Reset device");
        // Perform a factory reset and restart the device
        esp_zb_factory_reset();
      }
      break;
    default:
    #if DEBUG==1
      char strBuf[100];
      sprintf(strBuf, "ZDO signal: %s (0x%x), status: %s", esp_zb_zdo_signal_to_string(sig_type), sig_type,
              esp_err_to_name(err_status));
      Serial.println(strBuf);
    #endif
      break;
  }
}

static esp_err_t zb_attribute_reporting_handler(const esp_zb_zcl_report_attr_message_t *message)
{
    ESP_RETURN_ON_FALSE(message, ESP_FAIL, TAG, "Empty message");
    ESP_RETURN_ON_FALSE(message->status == ESP_ZB_ZCL_STATUS_SUCCESS, ESP_ERR_INVALID_ARG, TAG, "Received message: error status(%d)",
                        message->status);
    logger_line("Received report from address(0x%x) src endpoint(%d) to dst endpoint(%d) cluster(0x%x)", message->src_address.u.short_addr,
             message->src_endpoint, message->dst_endpoint, message->cluster);
    logger_line("Received report information: attribute(0x%x), type(0x%x), value(%d)\n", message->attribute.id, message->attribute.data.type,
             message->attribute.data.value ? *(uint8_t *)message->attribute.data.value : 0);
    return ESP_OK;
}

static esp_err_t zb_read_attr_resp_handler(const esp_zb_zcl_cmd_read_attr_resp_message_t *message)
{
    ESP_RETURN_ON_FALSE(message, ESP_FAIL, TAG, "Empty message");
    ESP_RETURN_ON_FALSE(message->info.status == ESP_ZB_ZCL_STATUS_SUCCESS, ESP_ERR_INVALID_ARG, TAG, "Received message: error status(%d)",
                        message->info.status);

    esp_zb_zcl_read_attr_resp_variable_t *variable = message->variables;
    while (variable) {
        logger_line("Read attribute response: status(%d), cluster(0x%x), attribute(0x%x), type(0x%x), value(%d)", variable->status,
                 message->info.cluster, variable->attribute.id, variable->attribute.data.type,
                 variable->attribute.data.value ? *(uint8_t *)variable->attribute.data.value : 0);
        variable = variable->next;
    }

    return ESP_OK;
}

static esp_err_t zb_configure_report_resp_handler(const esp_zb_zcl_cmd_config_report_resp_message_t *message)
{
    ESP_RETURN_ON_FALSE(message, ESP_FAIL, TAG, "Empty message");
    ESP_RETURN_ON_FALSE(message->info.status == ESP_ZB_ZCL_STATUS_SUCCESS, ESP_ERR_INVALID_ARG, TAG, "Received message: error status(%d)",
                        message->info.status);

    esp_zb_zcl_config_report_resp_variable_t *variable = message->variables;
    while (variable) {
        logger_line("Configure report response: status(%d), cluster(0x%x), attribute(0x%x)", message->info.status, message->info.cluster,
                 variable->attribute_id);
        variable = variable->next;
    }

    return ESP_OK;
}

static esp_err_t zb_action_handler(esp_zb_core_action_callback_id_t callback_id, const void *message)
{
    esp_err_t ret = ESP_OK;
    switch (callback_id) {
    case ESP_ZB_CORE_REPORT_ATTR_CB_ID:
        ret = zb_attribute_reporting_handler((esp_zb_zcl_report_attr_message_t *)message);
        break;
    case ESP_ZB_CORE_CMD_READ_ATTR_RESP_CB_ID:
        ret = zb_read_attr_resp_handler((esp_zb_zcl_cmd_read_attr_resp_message_t *)message);
        break;
    case ESP_ZB_CORE_CMD_REPORT_CONFIG_RESP_CB_ID:
        ret = zb_configure_report_resp_handler((esp_zb_zcl_cmd_config_report_resp_message_t *)message);
        break;
    default:
        logger_line("Receive Zigbee action(0x%x) callback", callback_id);
        break;
    }
    return ret;
}


static esp_err_t zb_attribute_handler(const esp_zb_zcl_set_attr_value_message_t *message) {
  esp_err_t ret = ESP_OK;
  bool light_state = 0;

  if (!message) {
    logger_line("Empty message");
  }
  if (message->info.status != ESP_ZB_ZCL_STATUS_SUCCESS) {
    logger_line("Received message: error status(%d)", message->info.status);
  }

  logger_line("Received message: endpoint(%d), cluster(0x%x), attribute(0x%x), data size(%d)", message->info.dst_endpoint, message->info.cluster,
          message->attribute.id, message->attribute.data.size);
  /*
    if (message->info.dst_endpoint == HA_ESP_LIGHT_ENDPOINT) {
        if (message->info.cluster == ESP_ZB_ZCL_CLUSTER_ID_ON_OFF) {
            if (message->attribute.id == ESP_ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID && message->attribute.data.type == ESP_ZB_ZCL_ATTR_TYPE_BOOL) {
                light_state = message->attribute.data.value ? *(bool *)message->attribute.data.value : light_state;
                sprintf(strBuf, "Light sets to %s", light_state ? "On" : "Off");
                Serial.println(strBuf);
                neopixelWrite(LED_PIN,255*light_state,255*light_state,255*light_state); // Toggle light
            }
        }
    }
    */
  return ret;
}
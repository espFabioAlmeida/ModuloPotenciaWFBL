
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "esp_log.h"
#include "esp_err.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_gatt_common_api.h"

#define BLE_DEVICE_NAME       "MOTOR_001"

#define GATTS_APP_ID          0

#define SERVICE_UUID          0xFF00
#define CHAR_UUID_RX          0xFF01
#define CHAR_UUID_TX          0xFF02

#define RX_MAX_LEN            128
#define TX_MAX_LEN            128

static const char *TAG = "BLE_MOTOR";

// --------------------------------------------------
// Handles GATT
// --------------------------------------------------

static uint16_t service_handle;
static uint16_t rx_handle;
static uint16_t tx_handle;
static uint16_t cccd_handle;

static uint16_t conn_id;
static esp_gatt_if_t gatts_interface;

static bool gatt_ready = false;
static bool adv_ready = false;
static bool notify_enabled = false;
static bool link_secured = false;

// --------------------------------------------------
// Valores das characteristics
// --------------------------------------------------

static uint8_t rx_value[RX_MAX_LEN];

static uint8_t tx_value[TX_MAX_LEN] = "READY";

static esp_attr_value_t rx_attr = {
    .attr_max_len = RX_MAX_LEN,
    .attr_len = 0,
    .attr_value = rx_value
};

static esp_attr_value_t tx_attr = {
    .attr_max_len = TX_MAX_LEN,
    .attr_len = 5,
    .attr_value = tx_value
};

// --------------------------------------------------
// UUIDs
// --------------------------------------------------

static esp_gatt_srvc_id_t service_id;

static esp_bt_uuid_t rx_uuid = {
    .len = ESP_UUID_LEN_16,
    .uuid.uuid16 = CHAR_UUID_RX
};

static esp_bt_uuid_t tx_uuid = {
    .len = ESP_UUID_LEN_16,
    .uuid.uuid16 = CHAR_UUID_TX
};

static esp_bt_uuid_t cccd_uuid = {
    .len = ESP_UUID_LEN_16,
    .uuid.uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG
};

// --------------------------------------------------
// Advertising
// --------------------------------------------------

static esp_ble_adv_params_t adv_params = {
    .adv_int_min       = 0x20,
    .adv_int_max       = 0x40,
    .adv_type          = ADV_TYPE_IND,
    .own_addr_type     = BLE_ADDR_TYPE_PUBLIC,
    .channel_map       = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

static esp_ble_adv_data_t adv_data = {
    .set_scan_rsp    = false,
    .include_name    = true,
    .include_txpower = true,
    .flag            = ESP_BLE_ADV_FLAG_GEN_DISC |
                       ESP_BLE_ADV_FLAG_BREDR_NOT_SPT,
};

// --------------------------------------------------
// Inicia advertising quando tudo estiver pronto
// --------------------------------------------------

static void iniciaAdvertising(void)
{
    if (gatt_ready && adv_ready)
    {
        ESP_LOGI(TAG, "Iniciando advertising");

        esp_err_t ret =
            esp_ble_gap_start_advertising(&adv_params);

        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "Erro advertising: %s",
                     esp_err_to_name(ret));
        }
    }
}

// --------------------------------------------------
// Configuração de segurança
// --------------------------------------------------

static void configuraSegurancaBLE(void)
{
    uint32_t passkey = 123456;

    uint8_t auth_req = ESP_LE_AUTH_REQ_SC_MITM_BOND;
    uint8_t iocap = ESP_IO_CAP_OUT;
    uint8_t key_size = 16;

    ESP_ERROR_CHECK(
        esp_ble_gap_set_security_param(
            ESP_BLE_SM_SET_STATIC_PASSKEY,
            &passkey,
            sizeof(passkey)
        )
    );

    ESP_ERROR_CHECK(
        esp_ble_gap_set_security_param(
            ESP_BLE_SM_AUTHEN_REQ_MODE,
            &auth_req,
            sizeof(auth_req)
        )
    );

    ESP_ERROR_CHECK(
        esp_ble_gap_set_security_param(
            ESP_BLE_SM_IOCAP_MODE,
            &iocap,
            sizeof(iocap)
        )
    );

    ESP_ERROR_CHECK(
        esp_ble_gap_set_security_param(
            ESP_BLE_SM_MAX_KEY_SIZE,
            &key_size,
            sizeof(key_size)
        )
    );

    ESP_LOGI(TAG, "Seguranca BLE configurada");
}

void enviaRespostaBLE(const char *mensagem)
{
    if (!link_secured || !notify_enabled)
    {
        ESP_LOGW(TAG, "BLE nao seguro ou notificacao desabilitada");
        return;
    }

    esp_err_t ret = esp_ble_gatts_send_indicate(
        gatts_interface,
        conn_id,
        tx_handle,
        strlen(mensagem),
        (uint8_t *)mensagem,
        false
    );

    if (ret == ESP_OK)
    {
        ESP_LOGI(TAG, "Resposta BLE enviada: %s", mensagem);
    }
    else
    {
        ESP_LOGE(TAG, "Erro ao enviar resposta BLE: %s",
                 esp_err_to_name(ret));
    }
}

// --------------------------------------------------
// Callback GAP
// --------------------------------------------------

static void gap_callback(
    esp_gap_ble_cb_event_t event,
    esp_ble_gap_cb_param_t *param)
{
    switch (event)
    {
        case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:

            if (param->adv_data_cmpl.status == ESP_BT_STATUS_SUCCESS)
            {
                ESP_LOGI(TAG, "Advertising data configurado");

                adv_ready = true;
                iniciaAdvertising();
            }
            else
            {
                ESP_LOGE(TAG, "Erro ao configurar advertising data");
            }

            break;

        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:

            if (param->adv_start_cmpl.status == ESP_BT_STATUS_SUCCESS)
            {
                ESP_LOGI(TAG, "Advertising iniciado");
            }
            else
            {
                ESP_LOGE(TAG, "Falha ao iniciar advertising");
            }

            break;

        case ESP_GAP_BLE_PASSKEY_NOTIF_EVT:

            ESP_LOGI(TAG, "Passkey: %06" PRIu32,
                     param->ble_security.key_notif.passkey);

            break;

        case ESP_GAP_BLE_SEC_REQ_EVT:

            ESP_LOGI(TAG, "Solicitacao de seguranca recebida");

            esp_ble_gap_security_rsp(
                param->ble_security.ble_req.bd_addr,
                true
            );

            break;

        case ESP_GAP_BLE_AUTH_CMPL_EVT:

            if (param->ble_security.auth_cmpl.success)
            {
                link_secured = true;
                ESP_LOGI(TAG, "Pareamento realizado com sucesso");
            }
            else
            {
                ESP_LOGE(TAG, "Falha no pareamento: 0x%x",
                         param->ble_security.auth_cmpl.fail_reason);
                link_secured = false;
            }

            break;

        default:
            break;
    }
}

// --------------------------------------------------
// Callback GATTS
// --------------------------------------------------

static void gatts_callback(
    esp_gatts_cb_event_t event,
    esp_gatt_if_t gatts_if,
    esp_ble_gatts_cb_param_t *param)
{
    esp_err_t ret;

    switch (event)
    {
        // ------------------------------------------
        // Registro da aplicação GATT
        // ------------------------------------------

        case ESP_GATTS_REG_EVT:

            if (param->reg.status != ESP_GATT_OK)
            {
                ESP_LOGE(TAG, "Erro no registro GATT");
                break;
            }

            gatts_interface = gatts_if;

            ESP_LOGI(TAG, "Aplicacao GATT registrada");

            service_id.is_primary = true;
            service_id.id.inst_id = 0;
            service_id.id.uuid.len = ESP_UUID_LEN_16;
            service_id.id.uuid.uuid.uuid16 = SERVICE_UUID;

            ret = esp_ble_gatts_create_service(
                gatts_if,
                &service_id,
                8
            );

            ESP_ERROR_CHECK(ret);

            break;

        // ------------------------------------------
        // Serviço criado
        // ------------------------------------------

        case ESP_GATTS_CREATE_EVT:

            ESP_LOGI(TAG, "Servico criado");

            service_handle = param->create.service_handle;

            ret = esp_ble_gatts_start_service(service_handle);
            ESP_ERROR_CHECK(ret);

            // Characteristic RX
            ret = esp_ble_gatts_add_char(
                service_handle,
                &rx_uuid,
                ESP_GATT_PERM_WRITE_ENC_MITM,
                ESP_GATT_CHAR_PROP_BIT_WRITE,
                &rx_attr,
                NULL
            );

            ESP_ERROR_CHECK(ret);

            break;

        // ------------------------------------------
        // Characteristic criada
        // ------------------------------------------

        case ESP_GATTS_ADD_CHAR_EVT:

            if (param->add_char.status != ESP_GATT_OK)
            {
                ESP_LOGE(TAG, "Erro ao criar characteristic");
                break;
            }

            if (param->add_char.char_uuid.uuid.uuid16 == CHAR_UUID_RX)
            {
                rx_handle = param->add_char.attr_handle;

                ESP_LOGI(TAG, "Characteristic RX criada");

                // Characteristic TX
                ret = esp_ble_gatts_add_char(
                    service_handle,
                    &tx_uuid,
                    ESP_GATT_PERM_READ_ENC_MITM,
                    ESP_GATT_CHAR_PROP_BIT_READ |
                    ESP_GATT_CHAR_PROP_BIT_NOTIFY,
                    &tx_attr,
                    NULL
                );

                ESP_ERROR_CHECK(ret);
            }
            else if (param->add_char.char_uuid.uuid.uuid16 == CHAR_UUID_TX)
            {
                tx_handle = param->add_char.attr_handle;

                ESP_LOGI(TAG, "Characteristic TX criada");

                // Descriptor CCCD para habilitar notificacoes
                ret = esp_ble_gatts_add_char_descr(
                    service_handle,
                    &cccd_uuid,
                    ESP_GATT_PERM_READ |
                    ESP_GATT_PERM_WRITE,
                    NULL,
                    NULL
                );

                ESP_ERROR_CHECK(ret);
            }

            break;

        // ------------------------------------------
        // Descriptor criado
        // ------------------------------------------

        case ESP_GATTS_ADD_CHAR_DESCR_EVT:

            cccd_handle = param->add_char_descr.attr_handle;

            ESP_LOGI(TAG, "Descriptor CCCD criado");

            gatt_ready = true;

            iniciaAdvertising();

            break;

        // ------------------------------------------
        // Conexão BLE
        // ------------------------------------------

        case ESP_GATTS_CONNECT_EVT:

            conn_id = param->connect.conn_id;
            notify_enabled = false;

            ESP_LOGI(TAG, "Celular conectado");

            // Solicita criptografia com autenticacao MITM
            ret = esp_ble_set_encryption(
                param->connect.remote_bda,
                ESP_BLE_SEC_ENCRYPT_MITM
            );

            if (ret != ESP_OK)
            {
                ESP_LOGE(TAG, "Erro ao solicitar criptografia: %s",
                         esp_err_to_name(ret));
            }

            break;

        // ------------------------------------------
        // Desconexão BLE
        // ------------------------------------------

        case ESP_GATTS_DISCONNECT_EVT:

            ESP_LOGI(TAG, "Celular desconectado");

            notify_enabled = false;
            link_secured  = false;

            iniciaAdvertising();

            break;

        // ------------------------------------------
        // Leitura de characteristic
        // ------------------------------------------

        case ESP_GATTS_READ_EVT:

            ESP_LOGI(TAG, "Characteristic lida");

            break;

        // ------------------------------------------
        // Escrita de characteristic
        // ------------------------------------------

        case ESP_GATTS_WRITE_EVT:

            if (param->write.handle == rx_handle)
            {
                ESP_LOGI(TAG, "Dados recebidos pela RX:");
                /*ESP_LOG_BUFFER_HEX(TAG,
                                   param->write.value,
                                   param->write.len);*/
                
                //aqui recebe os dados
                ESP_LOG_BUFFER_CHAR(TAG,
                                param->write.value,
                                param->write.len);

                enviaRespostaBLE("Teste 1234");
            }

            else if (param->write.handle == cccd_handle)
            {
                uint16_t cccd_value = 0;

                if (param->write.len >= 2)
                {
                    cccd_value =
                        param->write.value[0] |
                        (param->write.value[1] << 8);
                }

                notify_enabled = (cccd_value & 0x0001) != 0;

                ESP_LOGI(TAG, "Notificacoes: %s",
                         notify_enabled ? "HABILITADAS" : "DESABILITADAS");

                if (notify_enabled)
                {
                    const uint8_t msg[] = "BLE_READY";

                    esp_ble_gatts_send_indicate(
                        gatts_interface,
                        conn_id,
                        tx_handle,
                        sizeof(msg) - 1,
                        (uint8_t *)msg,
                        false
                    );
                }
            }

            if(param->write.need_rsp == true) {
                esp_err_t ret = esp_ble_gatts_send_response(
                    gatts_if,
                    param->write.conn_id,
                    param->write.trans_id,
                    ESP_GATT_OK,
                    NULL
                );

                if (ret != ESP_OK)
                {
                    ESP_LOGE(TAG,
                            "Erro ao responder WRITE: %s",
                            esp_err_to_name(ret));
                }
                else
                {
                    ESP_LOGI(TAG, "WRITE GATT ACK enviado");
                }
            }

            break;

        default:
            break;
    }
}

void configuraBlee(void) {
    esp_err_t ret;

    // Controlador Bluetooth
    esp_bt_controller_config_t bt_cfg =
        BT_CONTROLLER_INIT_CONFIG_DEFAULT();

    ret = esp_bt_controller_init(&bt_cfg);
    ESP_ERROR_CHECK(ret);

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    ESP_ERROR_CHECK(ret);

    // Bluedroid
    ret = esp_bluedroid_init();
    ESP_ERROR_CHECK(ret);

    ret = esp_bluedroid_enable();
    ESP_ERROR_CHECK(ret);

    // Callbacks
    ret = esp_ble_gap_register_callback(gap_callback);
    ESP_ERROR_CHECK(ret);

    ret = esp_ble_gatts_register_callback(gatts_callback);
    ESP_ERROR_CHECK(ret);

    // Segurança
    configuraSegurancaBLE();

    // Nome
    ret = esp_ble_gap_set_device_name(BLE_DEVICE_NAME);
    ESP_ERROR_CHECK(ret);

    // Registro da aplicação GATT
    ret = esp_ble_gatts_app_register(GATTS_APP_ID);
    ESP_ERROR_CHECK(ret);

    // Advertising
    ret = esp_ble_gap_config_adv_data(&adv_data);
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "Inicializacao BLE solicitada");
}

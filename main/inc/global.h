#define MOTOR_PWM 4
#define IN1 9
#define LED_CPU 38
#define LED_COM 37
#define LED_IN  36

#define BUFFER_RX_SIZE 255
#define BUFFER_TX_SIZE 255
#define WIFI_SSID_SIZE 64
#define WIFI_PASS_SIZE 64

enum MODO_OPERACAO_WIFI {
    MODO_ESTACAO = 1,
    MODO_AP
};

enum NUMERO_MOTOR {
    MOTOR_NAO_CONFIGURADO,
    MOTOR_01,
    MOTOR_02,
    MOTOR_03,
    MOTOR_04
};

extern bool 
    flagLedCOM,
    flagLedIN;

extern uint8_t 
    modoOperacaoWifi,
    numeroMotor;

extern char 
    bufferBleeRX[BUFFER_RX_SIZE],
    bufferBleeTX[BUFFER_TX_SIZE],

    wifiSSID[WIFI_SSID_SIZE],
    wifiPass[WIFI_PASS_SIZE];
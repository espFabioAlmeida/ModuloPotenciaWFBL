#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "nvs_flash.h"
#include "esp_err.h"
#include "esp_log.h"


#include "inc/global.h"
#include "Scheduler.c"
#include "utilities/UtilityGPIO.c"
#include "utilities/UtilityPWM.c"
#include "utilities/UtilityBlee.c"


bool 
    flagLedCOM = false,
    flagLedIN = false;


void taskPrincipal(void * param) {
    while(1) {
        updatePWM(2000);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void taskBluetooth(void * param) {
    while(1) {

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }   
}

void taskWiFi(void * param) {
    while(1) {

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }   
}

void timer10ms(TimerHandle_t xTimer) {
    tarefas10ms();
}

void timer100ms(TimerHandle_t xTimer) {
    tarefas100ms();
}

void timer1s(TimerHandle_t xTimer) {
    tarefas1s();
}

void iniciaNVSFlash() {
    esp_err_t ret = nvs_flash_init();

    if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {       
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);
}

void app_main(void) {
    printf("Sistema iniciado\r\n");
    esp_rom_gpio_pad_select_gpio(MOTOR_PWM);
    gpio_set_direction(MOTOR_PWM, GPIO_MODE_OUTPUT);
    esp_rom_gpio_pad_select_gpio(LED_CPU);
    gpio_set_direction(LED_CPU, GPIO_MODE_OUTPUT);
    esp_rom_gpio_pad_select_gpio(LED_COM);
    gpio_set_direction(LED_COM, GPIO_MODE_OUTPUT);
    esp_rom_gpio_pad_select_gpio(LED_IN);
    gpio_set_direction(LED_IN, GPIO_MODE_OUTPUT);
    esp_rom_gpio_pad_select_gpio(IN1);
    gpio_set_direction(IN1, GPIO_MODE_INPUT);
    gpio_set_pull_mode(IN1, GPIO_PULLUP_ONLY);
    iniciaNVSFlash();
    pwmInit();
    configuraBlee();

    xTaskCreatePinnedToCore(&taskPrincipal, "Principal", 2048, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(&taskBluetooth, "Bluetooth", 2048, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(&taskWiFi, "WiFi", 2048, NULL, 1, NULL, 1);
    TimerHandle_t xTimer10ms = xTimerCreate("timer10ms", pdMS_TO_TICKS(10), true, NULL, timer10ms);
    xTimerStart(xTimer10ms, 0);
    TimerHandle_t xTimer100ms = xTimerCreate("timer100ms", pdMS_TO_TICKS(100), true, NULL, timer100ms);
    xTimerStart(xTimer100ms, 0);
    TimerHandle_t xTimer1s = xTimerCreate("timer1s", pdMS_TO_TICKS(1000), true, NULL, timer1s);
    xTimerStart(xTimer1s, 0);
}
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_system.h"
#include "Scheduler.c"

void taskPrincipal(void * param) {
    while(1) {

        vTaskDelay(1 / portTICK_PERIOD_MS);
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

void timer1ms(TimerHandle_t xTimer) {
    tarefas1ms();
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

void app_main(void) {
    printf("Sistema iniciado\r\n");
    xTaskCreatePinnedToCore(&taskPrincipal, "Principal", 2048, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(&taskBluetooth, "Bluetooth", 2048, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(&taskWiFi, "WiFi", 2048, NULL, 1, NULL, 1);
    TimerHandle_t xTimer1ms = xTimerCreate("timer1ms", pdMS_TO_TICKS(1), true, NULL, timer1ms);
    xTimerStart(xTimer1ms, 0);
    TimerHandle_t xTimer10ms = xTimerCreate("timer10ms", pdMS_TO_TICKS(10), true, NULL, timer10ms);
    xTimerStart(xTimer10ms, 0);
    TimerHandle_t xTimer100ms = xTimerCreate("timer100ms", pdMS_TO_TICKS(100), true, NULL, timer100ms);
    xTimerStart(xTimer100ms, 0);
    TimerHandle_t xTimer1s = xTimerCreate("timer1s", pdMS_TO_TICKS(1000), true, NULL, timer1s);
    xTimerStart(xTimer1s, 0);
}

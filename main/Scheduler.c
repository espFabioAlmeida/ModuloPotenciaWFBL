#include "inc/global.h"
#include "inc/UtilityGPIO.h"

void tarefas10ms() {
    
}

void tarefas100ms() {
    static bool flagLedCPU = false;
    flagLedCPU = !flagLedCPU;
    setPin(LED_CPU, flagLedCPU);

    if(flagLedCOM) {
        off(LED_COM);
        flagLedCOM = false;
    }
    else {
        on(LED_COM);
    }
}

void tarefas1s() {

}
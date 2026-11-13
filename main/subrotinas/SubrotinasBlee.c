#include <stdio.h>
#include "inc/global.h"
#include "inc/Utils.h"

void protocoloBluetoothBlee() {
    signed int index = indexOf(bufferBleeRX, "$,");
    if(index >= 0) {
        uint8_t comando = charToByte(bufferBleeRX[index + 2]);
        comando *= 10;
        comando += charToByte(bufferBleeRX[index + 3]);

        switch(comando) {
            case 1:
                // Comando 0: Configurar modo de operação WiFi
                if(bufferBleeRX[index + 4] == '0') {
                    modoOperacaoWifi = MODO_ESTACAO;
                } else if(bufferBleeRX[index + 4] == '1') {
                    modoOperacaoWifi = MODO_AP;
                }
                break;

            case 2:
                // Comando 2: Configurar ssid e senha do WiFi
                for(uint16_t i = index + 5; i < BUFFER_RX_SIZE; i ++) {
                    if(bufferBleeRX[i] == ',') {
                        wifiSSID[i] = '\0';
                        index = i + 1;
                        break;
                    }
                    wifiSSID[i] = bufferBleeRX[i];
                }

                for(uint16_t i = index; i < BUFFER_RX_SIZE; i ++) {
                    if(bufferBleeRX[i] == ',') {
                        wifiPass[i] = '\0';
                        index = i + 1;
                        break;
                    }
                    wifiPass[i] = bufferBleeRX[i];
                }

                break;

            case 3:
                // Comando 3: Configurar número do motor
                numeroMotor = charToByte(bufferBleeRX[index + 4]);
                break;

            default:
                // Comando desconhecido
                break;
        }
    }
}
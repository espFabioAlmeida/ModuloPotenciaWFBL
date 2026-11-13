#include <stdio.h>
#include <string.h>
#include "inc/global.h"

signed int indexOf(char stringIn[], char stringComparacao[]) {
	bool encontrado = false;
	for(uint16_t i = 0; i < strlen(stringIn); i ++) {
		if(stringIn[i] == stringComparacao[0]) {
			encontrado = true;
			for(uint16_t j = 0; j < strlen(stringComparacao); j ++) {
				if(stringIn[i + j] != stringComparacao[j]) {
					encontrado = false;
				}
			}
			if(encontrado) {
				return i;
			}
		}
	}
	return -1;
}

uint8_t chatToByte(char data) {
	switch(data) {
		default:
		case '0':
			return 0;
		case '1':
			return 1;
		case '2':
			return 2;
		case '3':
			return 3;
		case '4':
			return 4;
		case '5':
			return 5;
		case '6':
			return 6;
		case '7':
			return 7;
		case '8':
			return 8;
		case '9':
			return 9;
	}
}
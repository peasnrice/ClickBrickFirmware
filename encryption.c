/*
 * encryption.c
 *
 * Created: 3/10/2013 10:54:44 AM
 *  Author: Andrew Evans & Shawn Stoute
 */ 
#include <avr/io.h>
#include "encryption.h"
#include "EEPROM.h"

uint16_t encryptNumber(uint32_t num){
	uint16_t e = getDeviceID();
	uint32_t n = getModulus();
	uint32_t result = 1;
	
	for(uint16_t j = 0; j < e; j++){
		result = (result * num % n);
	}
	result = (result % n);	
	return (uint16_t)result;
}
/*
 * external.h
 *
 * Created: 3/6/2013 1:36:09 PM
 *  Author: Andrew Evans
 */ 


#ifndef EXTERNAL_H_
#define EXTERNAL_H_

#include <avr/io.h>

uint8_t researchPeriod;
uint8_t invertColours;
uint8_t modifiedTimeFlag;
volatile uint8_t breakFromUSARTFlag;

#endif /* EXTERNAL_H_ */
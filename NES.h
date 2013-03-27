/*
 * NES.h
 *
 * Created: 2/22/2013 8:26:21 PM
 *  Author: Andrew Evans
 */ 


#ifndef NES_H
#define NES_H

#define CLOCK 5
#define LATCH 6
#define DATA 7

#define A 0
#define B 1
#define SELECT 2
#define START 3
#define NORTH 4
#define SOUTH 5
#define WEST 6
#define EAST 7

void initNES(void);
uint8_t NESRead(void);
uint8_t checkForNESButtons(void);

uint8_t buttonsPressed;
uint8_t nesButtons;

#endif /* NES_H_ */
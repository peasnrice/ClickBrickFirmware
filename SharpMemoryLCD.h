/*
 * SharpMemoryLCD.h
 *
 * Created: 1/10/2013 3:35:23 PM
 *  Author: Andrew Evans
 */ 


#ifndef SHARPMEMORYLCD_H
#define SHARPMEMORYLCD_H

#define SCREENWIDTH 144
#define SCREENHEIGHT 168

#define SCLK 1 //PB1
#define SI 2 //PB2
#define SCS 5 //PH5
#define EXTC 6 //PH6
#define DISP 4 //PB4
#define EXTM 5 //PB5
#define SS 0 //PB0

#define TIRED 0
#define EXHAUSTED 1
#define CONFUSED 2
#define MISERABLE 3
#define HAPPY 4
#define SAD 5
#define MEH 6
#define ANGRY 7

#define output_low(port,pin) port &= ~(1<<pin)
#define output_high(port,pin) port |= (1<<pin)
#define set_input(portdir,pin) portdir &= ~(1<<pin)
#define set_output(portdir,pin) portdir |= (1<<pin)

extern uint8_t invertColours;

void initLCD(void);
void clearLCD(void);
void printTimeLineIsEmpty(void);
void printTimeSelectionOnLine(uint8_t line, uint8_t timeSelectionHour, uint8_t timeSelectionMinute, uint8_t invert, uint8_t currentSelection);
void printDateSelectionOnLine(uint8_t line, uint8_t dateSelectionDay, uint8_t dateSelectionMonth, uint8_t dateSelectionYear, uint8_t invert, uint8_t currentSelection);
void printStringOnLine(uint8_t line, char lineText[], uint8_t invert, uint8_t option);	//prints 18 character long string

void printTimelineStringOnLine(uint8_t line, char lineText[], uint8_t invert, uint8_t face);	//prints 16 characters + face of current mood
void printBigTime(uint8_t line, char lineText[], uint8_t invert);

void printImage(void);


#endif /* SHARPMEMORYLCD_H_ */
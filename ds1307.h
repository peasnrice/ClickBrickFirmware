#ifndef DS1307_H
#define DS1307_H

#include <avr/io.h>

uint8_t DS1307Read(uint8_t address,uint8_t *data);
uint8_t DS1307Write(uint8_t address,uint8_t data);
uint8_t getTimeYear();
uint8_t getTimeMonth();
uint8_t getTimeDate();
uint8_t getTimeDay();
uint8_t getTimeHour();
uint8_t getTimeMinute();
uint8_t getTimeSecond();
void setTime(uint8_t hour,uint8_t minute,uint8_t second,uint8_t dow,uint8_t day,uint8_t month,uint8_t year);
void setTimeFromString(char* timeString);
void setTimeYear(uint8_t year);
void setTimeMonth(uint8_t month);
void setTimeDay(uint8_t day);
void setTimeDOW(uint8_t dow);
void setTimeHour(uint8_t hour);
void setTimeMinute(uint8_t minute);
void setTimeSecond(uint8_t second);
void startClock();
void getTimeString(char *Time);

#endif

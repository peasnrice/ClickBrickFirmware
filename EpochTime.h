/*
* EpochTime.h
*
* Created: 2/14/2013 2:57:52 PM
*  Author: Andrew Evans
*  Contributor : Peter schmelzer, Oliver Kraus
*  Date       : 2010-11-01
*  Version    : 1.00
*  License    : cc-by-sa-3.0
*
*  Description:
*  The DS1307new Library
*
*
*  Notes on the date calculation procedures
*  Written 1996/97 by Oliver Kraus
*  Published by Heinz Heise Verlag 1997 (c't 15/97)
*  Completly rewritten and put under GPL 2011 by Oliver Kraus
*
*/


#ifndef EPOCHTIME_H
#define EPOCHTIME_H

//Determines if leap year
uint8_t is_leap_year(uint16_t year);
//Calculate number of days within the year
uint16_t calculateYDN(uint16_t year, uint8_t month, uint8_t day);
//Calculate century day number
uint16_t calculateCDN(uint16_t year, uint16_t ydn);
//convert time to epoch time for comparing age of records with time
void convertToEpoch(uint16_t time[], uint32_t *t);
//Takes 2 dates/times and returns a 0 if both are identical, 1 if the first is larger and a 2 if the 2nd date is larger
uint8_t compareTimes(uint16_t times_1[], uint16_t times_2[]);

uint8_t fortyEightHoursApart(uint16_t times_1[], uint16_t times_2[]);

int8_t calculateDOW(uint8_t currentDayOfMonth, uint8_t currentMonth, uint8_t currentYear);

#endif /* EPOCHTIME_H_ */
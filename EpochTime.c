/*
* EpochTime.c
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

#include <avr/io.h>
#include <stdint.h> 
#include "EpochTime.h"
#include "usart.h"


//Determines if leap year
uint8_t is_leap_year(uint16_t year){
	if (((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0))
		return 1;
	else
		return 0;
}
//Calculate number of days within the year
uint16_t calculateYDN(uint16_t year, uint8_t month, uint8_t day){
	uint8_t tmp1;
	uint16_t tmp2;
	tmp1 = 0;
	if ( month >= 3 )
		tmp1++;
	tmp2 = month;
	tmp2 +=2;
	tmp2 *=611;
	tmp2 /= 20;
	tmp2 += day;
	tmp2 -= 91;
	tmp1 <<=1;
	tmp2 -= tmp1;
	if ( tmp1 != 0 )
		tmp2 += is_leap_year(year);
	return tmp2;
}
//Calculate century day number
uint16_t calculateCDN(uint16_t year, uint16_t ydn){
	uint16_t cdn = ydn;
	cdn--;
	while(year > 2000)
	{
		year--;
		cdn += 365;
		cdn += is_leap_year(year);
	}
	return cdn;
}
//convert time to epoch time for comparing age of records with time
void convertToEpoch(uint16_t time[], uint32_t *t){
	uint16_t yr = time[0]+2000;
	*t = calculateCDN(yr, calculateYDN(yr, time[1], time[2]));
	//USART_Sendbyte(yr);
	*t *= 24;
	*t += time[3];
	*t *= 60;
	*t += time[4];
	*t *= 60;
	*t += time[5];
	//USART_Sendbyte(*t>>24);
	//USART_Sendbyte(*t>>16);
	//USART_Sendbyte(*t>>8);
	//USART_Sendbyte(*t);
}

uint8_t compareTimes(uint16_t times_1[],uint16_t times_2[]){
	
	//for(uint8_t i = 0; i < 6; i++)
		//USART_Sendbyte(times_1[i]);
	//for(uint8_t i = 0; i < 6; i++)
		//USART_Sendbyte(times_2[i]);
		
	uint32_t *time_1;
	uint32_t *time_2;
	time_1 = (uint32_t *)malloc(sizeof(uint32_t));
	time_2 = (uint32_t *)malloc(sizeof(uint32_t));
	*time_1 = 0;
	*time_2 = 0;
	
	convertToEpoch(times_1,time_1);
	
	convertToEpoch(times_2,time_2);
	
	//USART_Sendbyte(*time_1>>24);
	//USART_Sendbyte(*time_1>>16);
	//USART_Sendbyte(*time_1>>8);
	//USART_Sendbyte(*time_1);
	//
	//USART_Sendbyte(*time_2>>24);
	//USART_Sendbyte(*time_2>>16);
	//USART_Sendbyte(*time_2>>8);
	//USART_Sendbyte(*time_2);
	
	
	if(*time_1 < *time_2){								
		//USART_Sendbyte(1);
		free(time_1);
		free(time_2);
		return 1;
	}		
	else if(*time_1 > *time_2){
		//USART_Sendbyte(2);
		free(time_1);
		free(time_2);
		return 2;
	}		
	else{
		//USART_Sendbyte(0);	
		free(time_1);
		free(time_2);
		return 0;
	}		
}

uint8_t fortyEightHoursApart(uint16_t times_1[], uint16_t times_2[]){

	uint32_t *time_1;
	uint32_t *time_2;
	time_1 = (uint32_t *)malloc(sizeof(uint32_t));
	time_2 = (uint32_t *)malloc(sizeof(uint32_t));
	*time_1 = 0;
	*time_2 = 0;
	

			
	convertToEpoch(times_1,time_1);
	convertToEpoch(times_2,time_2);

	int32_t time_diff = *time_1 - *time_2;
	
	if(time_diff >= 172800){
		return 1;
	}		
	else
		return 0;
}
/************************************************************

Libray fuctions to access the popular DS1307 RTC IC with AVR
Microcontroller.

The libray has just two functions. One reads the register whose
address is provided. Other writes to the given register with
given data.Please see DS1307 Datasheet for more info on
the registers.

Requires eXtreme Electronics Low Level I2C Libray.

PLEASE SEE WWW.EXTREMEELECTRONICS.CO.IN FOR DETAILED
SCHEMATICS,USER GUIDE AND VIDOES.

COPYRIGHT (C) 2008-2009 EXTREME ELECTRONICS INDIA

************************************************************/

#include <avr/io.h>

#include "I2C.h"
#include "ds1307.h"

uint8_t data;

// Convert Binary Coded Decimal (BCD) to Decimal
uint8_t bcd2dec(char num){
	return ((num/16 * 10) + (num % 16));
}

// Convert Decimal to Binary Coded Decimal (BCD)
uint8_t dec2bcd(uint8_t num){
	return ((num/10 * 16) + (num % 10));
}


/***************************************************

Function To Read Internal Registers of DS1307
---------------------------------------------

address : Address of the register
data: value of register is copied to this.


Returns:
0= Failure
1= Success
***************************************************/

uint8_t DS1307Read(uint8_t address,uint8_t *data)
{
	uint8_t res;	//result
	
	//Start
	I2CStart();
	
	//SLA+W (for dummy write to set register pointer)
	res=I2CWriteByte(0b11010000);	//DS1307 address + W
	
	//Error
	if(!res)	return FALSE;
	
	//Now send the address of required register
	res=I2CWriteByte(address);
	
	//Error
	if(!res)	return FALSE;
	
	//Repeat Start
	I2CStart();
	
	//SLA + R
	res=I2CWriteByte(0b11010001);	//DS1307 Address + R
	
	//Error
	if(!res)	return FALSE;
	
	//Now read the value with NACK
	res=I2CReadByte(data,0);
	
	//Error
	if(!res)	return FALSE;
	
	//STOP
	I2CStop();
	
	return TRUE;
}

/***************************************************

Function To Write Internal Registers of DS1307
---------------------------------------------

address : Address of the register
data: value to write.


Returns:
0= Failure
1= Success
***************************************************/

uint8_t DS1307Write(uint8_t address,uint8_t data)
{
	uint8_t res;	//result
	
	//Start
	I2CStart();
	
	//SLA+W
	res=I2CWriteByte(0b11010000);	//DS1307 address + W
	
	//Error
	if(!res)	return FALSE;
	
	//Now send the address of required register
	res=I2CWriteByte(address);
	
	//Error
	if(!res)	return FALSE;
	
	//Now write the value
	res=I2CWriteByte(data);
	
	//Error
	if(!res)	return FALSE;
	
	//STOP
	I2CStop();
	
	return TRUE;
}

/*
 * Additional functions
 *
 * Created: 1/12/2013 8:26:38 PM
 * Authors: Andrew Evans & Shawn Stoute
 */ 

void setTime(uint8_t hour,uint8_t minute,uint8_t second,uint8_t dow,uint8_t day,uint8_t month,uint8_t year){
	DS1307Write(0x00,dec2bcd(second));	//set seconds
	DS1307Write(0x01,dec2bcd(minute));	//set minutes
	DS1307Write(0x02,dec2bcd(hour) & 0x3F);	//set hours
	DS1307Write(0x03,dec2bcd(dow));	//set dow
	DS1307Write(0x04,dec2bcd(day));	//set date
	DS1307Write(0x05,dec2bcd(month));	//set month
	DS1307Write(0x06,dec2bcd(year));	//set year
}

void setTimeFromString(char* timeString){
	uint8_t i = 0;
	char *integerStringPointer = strtok(timeString, ",");
	while(integerStringPointer != 0x00){
		switch (i)
		{
			case 0:
				setTimeMonth(atoi(integerStringPointer));
				break;
			case 1:
				setTimeDay(atoi(integerStringPointer));
				break;				
			case 2:
				setTimeYear(atoi(integerStringPointer));
				break;
			case 3:
				setTimeHour(atoi(integerStringPointer));
				break;	
			case 4:
				setTimeMinute(atoi(integerStringPointer));
				break;		
			case 5:
				setTimeSecond(atoi(integerStringPointer));
				break;
			case 6:
				setTimeDOW(atoi(integerStringPointer));
				break;
		}
		i++;
		integerStringPointer = strtok(0x00,",");
	}	
}

void setTimeYear(uint8_t year){
	DS1307Write(0x06,dec2bcd(year));	//set year
}

void setTimeMonth(uint8_t month){
	DS1307Write(0x05,dec2bcd(month));	//set month	
}

void setTimeDay(uint8_t day){
	DS1307Write(0x04,dec2bcd(day));	//set date	
}

void setTimeDOW(uint8_t dow){
	DS1307Write(0x03,dec2bcd(dow));	//set dow	
}

void setTimeHour(uint8_t hour){
	DS1307Write(0x02,dec2bcd(hour) & 0x3F);	//set hours	
}

void setTimeMinute(uint8_t minute){
	DS1307Write(0x01,dec2bcd(minute));	//set minutes	
}

void setTimeSecond(uint8_t second){
	DS1307Write(0x00,dec2bcd(second));	//set seconds	
}

void startClock(void){
	DS1307Write(0x00,dec2bcd(getTimeSecond()) | 0x7F);	//set seconds
}

uint8_t getTimeYear(){
	DS1307Read(0x06, &data);
	return bcd2dec(data & 0xFF);
}

uint8_t getTimeMonth(){
	DS1307Read(0x05, &data);
	return bcd2dec(data & 0x1F);
}

uint8_t getTimeDate(){
	DS1307Read(0x04, &data);
	return bcd2dec(data & 0x3F);
}

uint8_t getTimeDay(){
	DS1307Read(0x03, &data);
	return bcd2dec(data & 0x0F);
}

uint8_t getTimeHour(){
	DS1307Read(0x02, &data);
	return bcd2dec(data & 0x3F);
}

uint8_t getTimeMinute(){
	DS1307Read(0x01, &data);
	return bcd2dec(data & 0xFF);
}

uint8_t getTimeSecond(){
	DS1307Read(0x00, &data);
	return bcd2dec(data & 0xFF);
}

void getTimeString(char *Time){
	
	/*
	DS1307Read(0x00,&data);
	Time[8]='\0';
	Time[7]=48+(data & 0b00001111);
	Time[6]=48+((data & 0b01110000)>>4);
	Time[5]=':';
	*/
	DS1307Read(0x01,&data);
	
	Time[4]=48+(data & 0b00001111);
	Time[3]=48+((data & 0b01110000)>>4);
	Time[2]=':';
	
	DS1307Read(0x02,&data);
	
	Time[1]=48+(data & 0b00001111);
	Time[0]=48+((data & 0b00110000)>>4);
	
	//return Time;
}





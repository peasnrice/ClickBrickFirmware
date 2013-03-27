/*
 * EEPROM.c
 *
 * Created: 2/14/2013 3:01:43 PM
 *  Author: Andrew Evans & Shawn Stoute
 */ 

#include <avr/eeprom.h>
#include <util/delay.h>
#include "usart.h"
#include "SharpMemoryLCD.h"
#include "EEPROM.h"

//Storage for configuration tool to set custom activities/moods/faces
//moods and faces are stored as strings
//faces are stored as integers
uint8_t EEMEM NonVolatileActivityString[306]; //(16 activities * 18 chars long)+ 15 commas + "\n\r" + 0x00 chars
uint8_t EEMEM NonVolatileMoodString[306];
uint8_t EEMEM NonVolatileFaceInts[16];
//A memory map of record memory locations, will be sorted by time contained within recorded data
uint16_t EEMEM memoryMapExtEEPROM[40];
//as the memory will require constant adding and removal of data entries, we need queue functionality
//we want to minimise writes so a circular array seems to be the best option
uint8_t EEMEM memoryMapStartAddress;
uint8_t EEMEM memoryMapEndAddress;
uint8_t EEMEM memoryMapActiveAddress;
//These pointers point to the next available address to write to for encrypted/unencrypted records
uint16_t EEMEM unencryptedRecordPointer;
uint16_t EEMEM encryptedRecordPointer;
//Keeping track of how many encrypted/unencrypted records have been recorded to make sorting and retrieving easier
uint8_t EEMEM unencrpytedRecordCount;
uint16_t EEMEM encryptedRecordCount;
//Start and end date is decided by the configuration tool and 
uint8_t EEMEM startDate[3];
uint8_t EEMEM endDate[3];
//Device ID and Prime Numbers
uint16_t EEMEM deviceID;
uint16_t EEMEM modulus;
//settings
uint8_t EEMEM invertColourSetting;

uint8_t SRAMstring[306];

//Writes a byte of data to a specified address in external eeprom
void extEEPROMWrite (uint16_t addr, uint8_t data)
{
	I2CStart();
	I2CWriteByte(eepromwrite);
	I2CWriteByte(addr>>8);
	I2CWriteByte(addr);
	I2CWriteByte(data);
	I2CStop();
	_delay_ms(5);
}

//Reads a byte of data to from a specified address in external eeprom
uint8_t extEEPROMRead (uint16_t addr)
{
	uint8_t* data = 0x00;
	I2CStart();
	I2CWriteByte(eepromwrite);
	I2CWriteByte(addr>>8);
	I2CWriteByte(addr);
	I2CStart();
	I2CWriteByte(eepromread);
	I2CReadByte(data,0);
	I2CStop();
	_delay_ms(5);
	return *data;
}

//Stores the string of activities delimited by ',' to the internal eeprom.
//Used when activities have been sent to the clickbrick via the configuration tool
void ActivitiesToEEPROM(char* charPtr){
	uint16_t ActStringSize = sizeof(NonVolatileActivityString)/sizeof(NonVolatileActivityString[0]);
	eeprom_update_block (( const void *)charPtr , (const void *)NonVolatileActivityString , ActStringSize) ;
	NonVolatileActivityString[ActStringSize] = 0x00;
}

//Stores the string of moods delimited by ',' to the internal eeprom.
//Used when moods have been sent to the clickbrick via the configuration tool
void MoodsToEEPROM(char* charPtr){
	uint16_t MoodStringSize = sizeof(NonVolatileMoodString)/sizeof(NonVolatileMoodString[0]);
	eeprom_update_block (( const void *)charPtr , (const void *)NonVolatileMoodString , MoodStringSize) ;
	NonVolatileMoodString[MoodStringSize] = 0x00;
}

//Stores the string of face integers delimited by ',' to the internal eeprom.
//Used when faces have been sent to the clickbrick via the configuration tool
//Each face integer within this string should pair up with the mood within the mood string
void FacesToEEPROM(char* charPtr){
	uint8_t i = 0;
	char *integerStringPointer = strtok(charPtr, ",");
	while(integerStringPointer != 0x00){
		eeprom_update_byte(&NonVolatileFaceInts[i], atoi(integerStringPointer));
		i++;
		integerStringPointer = strtok(0x00,",");
	}
}

//Returns a string of what is contained in the Activity, mood or face address space
//Used when loading activities and moods from the eeprom when the device is turned on
//when the activities/moods are loaded into SRAM.
//The face string is also available but only for debugging perposes.
void ActOrMoodArrayFromEEPROM(uint8_t arrType, char** Arr[], uint8_t arrSize){
	memset(Arr,0,arrSize * sizeof(Arr));
	uint8_t i = 0;
	uint16_t SRAMstringSize = 306;
	
	if(arrType == ACTIVITY)
		eeprom_read_block (( void *) SRAMstring , ( const void *) NonVolatileActivityString , SRAMstringSize);
	else if(arrType == MOOD)
		eeprom_read_block (( void *) SRAMstring , ( const void *) NonVolatileMoodString , SRAMstringSize);
		
	char* wordPointer = strtok(SRAMstring,","); //split string with ',' as divider
	
	while(wordPointer != 0x00){
		Arr[i] = calloc(strlen(wordPointer)+1,1);
		strcpy(Arr[i], wordPointer);	
		i++;
		wordPointer = strtok(0x00,",");
	}
	arrSize = i+1;
}

//Reads back Integers stored in EEPROM
//This is used for obtaining the faces values that coincide with the moods
//Range of integers should correlate to number of faces stored in PROGMEM under clickbrick.h
void FaceArrayFromEEPROM(uint8_t arrType, uint8_t *Arr, uint8_t arrSize){
	memset(Arr,0,arrSize * sizeof(Arr));
	for(uint8_t i = 0; i < 16; i++){
		Arr[i] = eeprom_read_byte(&NonVolatileFaceInts[i]);
		arrSize = i+1;
	}
}

//Saves data records to external eeprom and updates memory map
//Note that we currently only ever store 40 unencrypted records at a time
//based on the assumption most users will never record more than 20 logs/day
void saveDataRecordToEEPROM(uint16_t address, uint8_t cell, uint8_t data){
	extEEPROMWrite(address-cell, data);
}

//checks if memory map location is available or taken, returns address in taken, 0 if not taken
uint16_t getMemMapElement(uint8_t index){
	uint16_t SRAMchar;
	SRAMchar = eeprom_read_word(&memoryMapExtEEPROM[index]);
	return (uint16_t)SRAMchar;
}

//updates base and end address of mem map array
void setMemMapStartAddress(uint8_t address){
	eeprom_update_byte(&memoryMapStartAddress, address);
}

void setMemMapEndAddress(uint8_t address){
	eeprom_update_byte(&memoryMapEndAddress, address);
}

void setMemMapActiveAddress(uint8_t address){
	eeprom_update_byte(&memoryMapActiveAddress, address);
}

//returns the base address of the mem map circular array
uint8_t getMemMapStartAddress(void){
	return eeprom_read_byte(&memoryMapStartAddress);
}

uint8_t getMemMapEndAddress(void){
	return eeprom_read_byte(&memoryMapEndAddress);
}

uint8_t getMemMapActiveAddress(void){
	return eeprom_read_byte(&memoryMapActiveAddress);
}

//saves address to memory map in location specified by index 
//used when a new record has been saved and we want to update the memory map
void setMemMapElement(uint8_t index, uint16_t data){
	eeprom_update_word(&memoryMapExtEEPROM[index], data);
}

//setters for Record pointers
void setUnencryptedRecordPointer(uint16_t address){
	eeprom_update_word (& unencryptedRecordPointer , address );
}

void setEncryptedRecordPointer(uint16_t address){
	eeprom_update_word (& encryptedRecordPointer , address );
}

//getters for record pointers
uint16_t getUnencryptedRecordPointer(void){
	uint16_t SRAMchar;
	SRAMchar = eeprom_read_word (& unencryptedRecordPointer );
	return (uint16_t)SRAMchar;
}

uint16_t getEncryptedRecordPointer(void){
	uint16_t SRAMchar;
	SRAMchar = eeprom_read_word (& encryptedRecordPointer );	
	return (uint16_t)SRAMchar;
}

//setters for record counts
void setUnencryptedRecordCount( uint8_t count){
	eeprom_update_byte (& unencrpytedRecordCount, count);
}

void setEncrpytedRecordCount( uint16_t count){
	eeprom_update_word (& encryptedRecordCount, count );
}

//getters for record counts
uint8_t getUnencrpytedRecordCount( void ){
	uint16_t SRAMchar;
	SRAMchar = eeprom_read_byte (& unencrpytedRecordCount );
	return (uint16_t)SRAMchar;	
}

uint16_t getEncryptedRecordCount( void ){
	uint16_t SRAMchar;
	SRAMchar = eeprom_read_word (& encryptedRecordCount );
	return (uint16_t)SRAMchar;
}

//setters for device ID and modulus
void setDeviceID(char* device_id_string){
	uint16_t deviceID_int = atoi(device_id_string);
	eeprom_update_word(& deviceID, deviceID_int );
}

void setModulus(char* modulus_string){
	uint16_t mod_int = atoi(modulus_string);
	eeprom_update_word(& modulus, mod_int );
}

//getters for device ID and modulus
uint16_t getDeviceID(void){
	uint16_t SRAMchar;
	SRAMchar = eeprom_read_word (& deviceID );
	return (uint16_t)SRAMchar;	
}

uint16_t getModulus(void){
	uint16_t SRAMchar;
	SRAMchar = eeprom_read_word (& modulus );
	return (uint16_t)SRAMchar;	
}

void setStartDate(char* start){
	uint8_t i = 0;
	char *integerStringPointer = strtok(start, ",");
	while(integerStringPointer != 0x00){
		eeprom_update_byte(&startDate[i], atoi(integerStringPointer));
		i++;
		integerStringPointer = strtok(0x00,",");
	}	
}

void setEndDate(char* end){
	uint8_t i = 0;
	char *integerStringPointer = strtok(end, ",");
	while(integerStringPointer != 0x00){
		eeprom_update_byte(&endDate[i], atoi(integerStringPointer));
		i++;
		integerStringPointer = strtok(0x00,",");
	}	
}

uint8_t getStartDateYear(void){
	return eeprom_read_byte (& startDate[2] );	
}

uint8_t getStartDateMonth(void){
	return eeprom_read_byte (& startDate[1] );
}

uint8_t getStartDateDay(void){
	return eeprom_read_byte (& startDate[0] );
}

uint8_t getEndDateYear(void){
	return eeprom_read_byte (& endDate[2] );	
}

uint8_t getEndDateMonth(void){
	return eeprom_read_byte (& endDate[1] );	
}

uint8_t getEndDateDay(void){
	return eeprom_read_byte (& endDate[0] );	
}

//modify/retrieve users saved settings in eeprom
uint8_t getInvertColourSetting(void){
	return eeprom_read_byte (& invertColourSetting );
}

void setInvertColourSetting(uint8_t invertSetting){
	eeprom_update_byte (& invertColourSetting, invertSetting);
}

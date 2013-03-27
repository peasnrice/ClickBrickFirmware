/*
* DataHandler.c
*
* Created: 2/14/2013 2:57:52 PM
*  Author: Andrew Evans
*/

#include <avr/io.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h> 
#include "dataHandler.h"
#include "EEPROM.h"
#include "EpochTime.h"
#include "encryption.h"

static char **activityArray[16];
static char **moodArray[16];

uint8_t actArrSize = 16;
uint8_t moodArrSize = 16;

uint8_t maxNumberOfRecords = 40;

uint32_t dataArray[9];

uint8_t *stringBuffer;

//0's the memory map, esentially clearing all records
void resetRecords(){
	for (uint8_t index = 0; index < maxNumberOfRecords; index++){
		setMemMapElement(index,0x0000);
	}
	setUnencryptedRecordCount(0);
	setEncrpytedRecordCount(0);
	setEncryptedRecordPointer(0);
	setMemMapStartAddress(0);
	setMemMapEndAddress(0);
	setUnencryptedRecordPointer(0x7FFF);
}

//removes record from memory map
//if record being deleted is located at the base address of mem map, 
//move base address and set mem location to 0 in mem map
//otherwise, delete record and shift other records into its place
void deleteRecord(uint16_t memMapIndex){
	uint8_t end = getMemMapEndAddress();
	
	setMemMapElement(memMapIndex, 0x0000);
	
	memMapIndex = (memMapIndex + 1) % maxNumberOfRecords;
	
	while(memMapIndex != end){
		if(memMapIndex != 0)
			setMemMapElement(memMapIndex-1,getMemMapElement(memMapIndex));							
		else
			setMemMapElement(maxNumberOfRecords-1,getMemMapElement(memMapIndex));	
		memMapIndex = (memMapIndex + 1) % maxNumberOfRecords;				
	}
	if(end != 0){
		setMemMapEndAddress(end-1);
		setMemMapElement(end-1,0x0000);
	}		
	else{
		setMemMapEndAddress(maxNumberOfRecords-1);
		setMemMapElement(maxNumberOfRecords-1,0x0000);
	}		
	setUnencryptedRecordCount(getUnencrpytedRecordCount()-1);
}

void deleteOldestRecord(void){
	uint8_t start = getMemMapStartAddress();
	setEncryptedRecord(start);
	setMemMapElement(start,0x0000);
	start = (start + 1) % maxNumberOfRecords;
	setMemMapStartAddress(start);
	setUnencryptedRecordCount(getUnencrpytedRecordCount()-1);
}

//sets a record at a given memory location
//we only ever call this function when we want to edit a record
//therefore it will exist in the memory map
void setRecordByAddress(uint16_t address, uint8_t year, uint8_t month, uint8_t day, uint8_t dow, uint8_t hour, uint8_t minute, uint8_t second, uint8_t activity, uint8_t mood){
	extEEPROMWrite(address--, year);
	extEEPROMWrite(address--, month);
	extEEPROMWrite(address--, day);
	extEEPROMWrite(address--, dow);
	extEEPROMWrite(address--, hour);
	extEEPROMWrite(address--, minute);
	extEEPROMWrite(address--, second);
	extEEPROMWrite(address--, activity);
	extEEPROMWrite(address--, mood);
}

//sets a record
//gets the next available address of external eeprom for data to be stored to
//Gets base address of circular array from eeprom
//runs through circular array looking for first empty location
//once a location is found we write the address in the memory map at the location it memMapPtr is currently pointing to
//We then record the record decrementing the address each time.
//Once complete we update the next available record pointer ready for the next record to be saved
//increment number of records counter stored eeprom
//break out of loop
void setRecord(uint8_t year, uint8_t month, uint8_t day, uint8_t dow, uint8_t hour, uint8_t minute, uint8_t second, uint8_t activity, uint8_t mood){
	uint16_t availableAddress = getUnencryptedRecordPointer();
	uint8_t numberOfRecords = getUnencrpytedRecordCount();
	uint8_t end = getMemMapEndAddress();
	uint8_t start = getMemMapStartAddress();
	
	
	if(numberOfRecords >= maxNumberOfRecords){
		//encryptRecord(getMemMapElement(start))
		//saveEncryptedRecord()
		availableAddress = getMemMapElement(start);
		deleteOldestRecord();
		start = getMemMapStartAddress();
	}
	
	uint16_t tempAdd = availableAddress;
	
	//save record
	extEEPROMWrite(tempAdd--, year);
	extEEPROMWrite(tempAdd--, month);
	extEEPROMWrite(tempAdd--, day);
	extEEPROMWrite(tempAdd--, dow);
	extEEPROMWrite(tempAdd--, hour);
	extEEPROMWrite(tempAdd--, minute);
	extEEPROMWrite(tempAdd--, second);
	extEEPROMWrite(tempAdd--, activity);
	extEEPROMWrite(tempAdd--, mood);		

	setMemMapElement(end, availableAddress);
	end = (end + 1) % maxNumberOfRecords;
	setMemMapEndAddress(end);			
	
	setUnencryptedRecordCount(getUnencrpytedRecordCount()+1);
	
	//check for next available address in external EEPROM
	//if it doesn't exist in the memory map then can save there
	
	start = getMemMapStartAddress();
	
	uint8_t addressTaken = 1;
	uint8_t index = start;
	if(numberOfRecords < maxNumberOfRecords){
		while(addressTaken == 1){
			addressTaken = 0;
			for(uint8_t i = 0; i < numberOfRecords; i++){
				if(tempAdd == getMemMapElement(index)){
					addressTaken = 1;
					index = (index + 1) % maxNumberOfRecords;		
					break;
				}
		
			}
			if(addressTaken == 1)
				tempAdd -= 9;
			if(tempAdd < 0x7EA0)
				tempAdd = 0x7FFF;					
		}	
	}		
	setUnencryptedRecordPointer(tempAdd);
}

void setEncryptedRecord(uint8_t memIndex){
	uint16_t address = getMemMapElement(memIndex);
	uint16_t encryptedRecord[9];
	unsigned char *encryptedString[37];
	uint16_t encryptedRecordCount = getEncryptedRecordCount();
	uint16_t encryptedRecordPointer = getEncryptedRecordPointer();
	
	dataArray[0] = getYear(address);
	dataArray[1] = getMonth(address);
	dataArray[2] = getDate(address);
	dataArray[3] = getDOW(address);
	dataArray[4] = getHour(address);
	dataArray[5] = getMinute(address);
	dataArray[6] = getSecond(address);
	dataArray[7] = getActivity(address);
	dataArray[8] = getMood(address);
		
	
	for(uint8_t i = 0; i < 9; i++){
		encryptedRecord[i] = encryptNumber((uint32_t)dataArray[i]);
		sprintf(&encryptedString[i*2],"%02x%02x",(uint8_t)(encryptedRecord[i]>>8), (uint8_t)encryptedRecord[i]);
	}
	
	//Save record to EEPROM
	extEEPROMWriteString(encryptedRecordPointer,encryptedString);
	
	encryptedRecordPointer += 38;
	encryptedRecordCount++;
	setEncryptedRecordPointer(encryptedRecordPointer);
	setEncrpytedRecordCount(encryptedRecordCount);
}

void writeOutEncryptedRecords(void){
	uint16_t encryptedRecordCount = getEncryptedRecordCount();
	for(uint16_t i = 0; i < encryptedRecordCount; i++){
		USART_Send_string_CRLF(extEEPROMReadString(i*38));
	}		
}

void insertRecord(uint8_t insertIndex, uint8_t year, uint8_t month, uint8_t day, uint8_t dow, uint8_t hour, uint8_t minute, uint8_t second, uint8_t activity, uint8_t mood){
	uint8_t numberOfRecords = getUnencrpytedRecordCount();
	uint16_t address = getUnencryptedRecordPointer();
	uint16_t tempAdd = address;
	if(numberOfRecords >= maxNumberOfRecords){
		//encrypt oldest record
		deleteOldestRecord();
	}
	uint8_t end = getMemMapEndAddress();
	int8_t index = end;
	while(index != insertIndex){
		setMemMapElement(index, getMemMapElement((index-1)%maxNumberOfRecords));
		if(index != 0)
			index--;
		else
			index = maxNumberOfRecords-1;
	}
	//save record
	extEEPROMWrite(tempAdd--, year);
	extEEPROMWrite(tempAdd--, month);
	extEEPROMWrite(tempAdd--, day);
	extEEPROMWrite(tempAdd--, dow);
	extEEPROMWrite(tempAdd--, hour);
	extEEPROMWrite(tempAdd--, minute);
	extEEPROMWrite(tempAdd--, second);
	extEEPROMWrite(tempAdd--, activity);
	extEEPROMWrite(tempAdd--, mood);
	
	setMemMapElement(insertIndex, address);
	
	end = (end + 1) % maxNumberOfRecords;
	setMemMapEndAddress(end);
		
	setUnencryptedRecordCount(numberOfRecords+1);
	
	uint8_t start = getMemMapStartAddress();

	
	uint8_t addressTaken = 1;
	index = start;
	if(numberOfRecords < maxNumberOfRecords){
		while(addressTaken == 1){
			addressTaken = 0;
			for(uint8_t i = 0; i < numberOfRecords; i++){
				if(tempAdd == getMemMapElement(index)){
					addressTaken = 1;
					index = (index + 1) % maxNumberOfRecords;
					break;
				}
				
			}
			if(addressTaken == 1)
				tempAdd -= 9;
			if(tempAdd < 0x7EA0)
				tempAdd = 0x7FFF;
		}
	}
	setUnencryptedRecordPointer(tempAdd);	
	
}

uint8_t findSpaceInMemMap(uint16_t year, uint8_t month, uint8_t day, uint8_t hour,uint8_t minute,uint8_t second){
	uint16_t time_1[6] = {year,month,day,hour,minute,second};
	uint16_t time_2[6];
	
	uint8_t returnState = 0;

	uint8_t start = getMemMapStartAddress();
	uint8_t index = getMemMapEndAddress();
	uint8_t numberOfRecords = getUnencrpytedRecordCount();
	uint8_t memMapFull = 0;
	uint16_t address = 0;
		
	if (numberOfRecords >= maxNumberOfRecords)
		memMapFull = 1;
		
	while (memMapFull == 1 || index != start){
		memMapFull = 0;
		index--;
		address = getMemMapElement(index);
		time_2[0] = getYear(address);
		time_2[1] = getMonth(address);
		time_2[2] = getDate(address);
		time_2[3] = getHour(address);
		time_2[4] = getMinute(address);
		time_2[5] = getSecond(address);
	
		returnState = compareTimes(time_1,time_2);
		if(returnState == 2 || returnState == 0){
			return index + 1;
		}
		//this if for the edge case where there is only one record
		//we need to insert the record at the beginning of the array
		else if(returnState == 1 && numberOfRecords == 1){
			return index;
		}
		
		if(index != 0){
			index--;
		}
		else{
			index = maxNumberOfRecords-1;
		}
	}
	return 0;
}


void setActivityByAddress(uint16_t address, uint8_t activity){
	saveDataRecordToEEPROM(address,7,activity);
}

void setMoodByAddress(uint16_t address, uint8_t mood){
	saveDataRecordToEEPROM(address,8,mood);
}

void setHourByAddress(uint16_t address, uint8_t hour){
	saveDataRecordToEEPROM(address,4,hour);
}

void setMinuteByAddress(uint16_t address, uint8_t minute){
	saveDataRecordToEEPROM(address,5,minute);
	saveDataRecordToEEPROM(address,6,0);
}

void getAvailableRecords(uint8_t *usedElementsCopy){
	for(uint8_t i = 0; i < 200; i++){
		usedElementsCopy[i] = getMemMapElement(i);
	}
}

uint8_t checkIfUsedElementExists(uint8_t recordID){
	return getMemMapElement(recordID);
}

uint8_t getActivityArraySize(){
	//return sizeof(actArr)/sizeof(actArr[0]);
	return 16;
}

uint8_t getMoodArraySize(){
	//return sizeof(moodArr)/sizeof(moodArr[0]);
	return 16;
}

uint8_t getYear(uint16_t address){
	return extEEPROMRead(address);	
}

uint8_t getMonth(uint16_t address){
	return extEEPROMRead(address-1);	
}

uint8_t getDate(uint16_t address){
	return extEEPROMRead(address-2);	
}

uint8_t getDOW( uint16_t address){
	return extEEPROMRead(address-3);
}

uint8_t getHour(uint16_t address){
	return extEEPROMRead(address-4);
}

uint8_t getMinute(uint16_t address){
	return extEEPROMRead(address-5);
}

uint8_t getSecond(uint16_t address){
	return extEEPROMRead(address-6);	
}

uint8_t getActivity(uint16_t address){
	return extEEPROMRead(address-7);
}

uint8_t getMood(uint16_t address){
	return extEEPROMRead(address-8);
}


void loadActivitiesDataHandler(){
	actArrSize = 16;
	ActOrMoodArrayFromEEPROM(0,activityArray,actArrSize);
}

void loadMoodsDataHandler(){
	moodArrSize = 16;
	ActOrMoodArrayFromEEPROM(1,moodArray,moodArrSize);
}

void extEEPROMWriteString (uint16_t addr, char *string){
	uint16_t i = 0;
	while(string[i] != 0x00){
		extEEPROMWrite(addr+i, string[i]);
		i++;
	}
	
	extEEPROMWrite(addr+i, 0x00);
}

char* extEEPROMReadString(uint16_t addr){
	char *s = stringBuffer;
	
	for(;;){
		*s = extEEPROMRead(addr);
		if(*s == 0x00)
		break;
		s++;
		addr++;
	}
	return stringBuffer;
}

void extEEPROMWriteDatarecord_E(uint8_t year, uint8_t month, uint8_t day, uint8_t dow, uint8_t hour, uint8_t minute, uint8_t activity, uint8_t mood){
	uint8_t index = getEncryptedRecordPointer();
	extEEPROMWrite(index++,year);
	extEEPROMWrite(index++,month);
	extEEPROMWrite(index++,day);
	extEEPROMWrite(index++,dow);
	extEEPROMWrite(index++,hour);
	extEEPROMWrite(index++,minute);
	extEEPROMWrite(index++,activity);
	extEEPROMWrite(index++,mood);
	setEncryptedRecordPointer(index);
}

void extEEPROMReadDatarecord_E(uint16_t index){
	uint16_t address = index*9;
	USART_Sendbyte(address >> 8);
	USART_Sendbyte(address);
	//for(uint8_t i = 0; i < 10; i++)
	//USART_Sendbyte(extEEPROMread(address--));
}

//void encryptOldRecords(index){
	//uint32_t currentTime; 
	//convertToEpoch(getTimeYear(),getTimeMonth(),getTimeDate(),getTimeHour(),getTimeMinute(),getTimeSecond(),&currentTime);
	//uint32_t recordTime = 0;
	//uint8_t address = 0;
	//for(uint8_t i = 0; i < 200; i++){
		//address = getMemMapElement(i);
		//convertToEpoch(getYear(address), getMonth(address), getDate(address), getHour(address), getMinute(address), getSecond(address), &recordTime);
		//if(fortyEightHoursApart(currentTime,recordTime));
			////encrypt record
			////save record to encrypted space on EEPROM
			////delete unencrypted record 
	//}
//}

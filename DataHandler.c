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
	#include "external.c"
	#include "dataHandler.h"
	#include "EEPROM.h"
	#include "EpochTime.h"
	#include "encryption.h"

	static char **activityArray[16];
	static char **moodArray[16];

	uint8_t actArrSize = 16;
	uint8_t moodArrSize = 16;

	extern uint8_t maxRecNum;

	uint32_t dataArray[9];

	uint8_t *stringBuffer;

	//0's the memory map, esentially clearing all records
	void resetRecords(){
		for (uint8_t index = 0; index < maxRecNum; index++){
			setMemMapElement(index,0x0000);
		}
		setUnencryptedRecordCount(0);
		setEncrpytedRecordCount(0);
		setEncryptedRecordPointer(0);
		setMemMapStartIndex(0);
		setMemMapEndIndex(0);
		setUnencryptedRecordPointer(0x7FFF);
	}

	//removes record from memory map
	//if record being deleted is located at the base address of mem map,
	//move base address and set mem location to 0 in mem map
	//otherwise, delete record and shift other records into its place
	void deleteRecord(uint8_t memMapIndex){
		
		uint8_t numberOfRecords = getUnencrpytedRecordCount();
		
		if(numberOfRecords == 1){
			setMemMapElement(memMapIndex, 0x0000);
		}
		
		else{
			uint8_t end = getMemMapEndIndex();
			
			while(memMapIndex != end){
				if(memMapIndex == maxRecNum-1)
				setMemMapElement(memMapIndex,getMemMapElement(0));
				else
				setMemMapElement(memMapIndex,getMemMapElement(memMapIndex+1));
				memMapIndex = (memMapIndex + 1) % maxRecNum;
			}
			
			setMemMapElement(end, 0x0000);
			
			if(end != 0)
			setMemMapEndIndex(end-1);
			else
			setMemMapEndIndex(maxRecNum-1);
		}
		setUnencryptedRecordCount(numberOfRecords-1);
		
		USART_Sendbyte(getMemMapStartIndex());
		USART_Sendbyte(getMemMapEndIndex());
		USART_Sendbyte(0);
		
		for(uint8_t i = 0; i < maxRecNum; i++){
			USART_Sendbyte(getMemMapElement(i));
		}
	}

	void deleteOldestRecord(void){
		uint8_t start = getMemMapStartIndex();
		setEncryptedRecord(start);
		setMemMapElement(start,0x0000);
		start = (start + 1) % maxRecNum;
		setMemMapStartIndex(start);
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
		uint8_t end = getMemMapEndIndex();
		uint8_t start = getMemMapStartIndex();
		
		
		if(numberOfRecords >= maxRecNum){
			//encryptRecord(getMemMapElement(start))
			//saveEncryptedRecord()
			availableAddress = getMemMapElement(start);
			deleteOldestRecord();
			start = getMemMapStartIndex();
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
		end = (end + 1) % maxRecNum;
		setMemMapEndIndex(end);
		
		setUnencryptedRecordCount(getUnencrpytedRecordCount()+1);
		
		//check for next available address in external EEPROM
		//if it doesn't exist in the memory map then can save there
		
		start = getMemMapStartIndex();
		
		uint8_t addressTaken = 1;
		uint8_t index = start;
		if(numberOfRecords < maxRecNum){
			while(addressTaken == 1){
				addressTaken = 0;
				for(uint8_t i = 0; i < numberOfRecords; i++){
					if(tempAdd == getMemMapElement(index)){
						addressTaken = 1;
						index = (index + 1) % maxRecNum;
						break;
					}
					
				}
				if(addressTaken == 1)
				tempAdd -= 9;
				if(tempAdd <= (0x7FFF - (9*maxRecNum)))
				tempAdd = 0x7FFF;
				//USART_Send_string_CRLF("in ze loopy");
			}
		}
		setUnencryptedRecordPointer(tempAdd);
		
	}

	void setEncryptedRecord(uint8_t memIndex){
		//USART_Sendbyte(memIndex);
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

	void insertRecord(uint8_t year, uint8_t month, uint8_t day, uint8_t dow, uint8_t hour, uint8_t minute, uint8_t second, uint8_t activity, uint8_t mood){
		//if array is full, delete oldest record (increments start address too)
		uint8_t numberOfRecords = getUnencrpytedRecordCount();
		if(numberOfRecords >= maxRecNum){
			deleteOldestRecord();
			numberOfRecords--;
		}
		
		// Find a valid place in memory to save the record
		// check for all already taken memory locations so we know what not to use
		uint8_t startIndex = getMemMapStartIndex();
		uint8_t endIndex = getMemMapEndIndex();
		int8_t index = startIndex;
		
		uint16_t memAddress = 0;
		uint8_t valid = 1;
		uint16_t lowerMemIndexLimit = 0x7FFF - (9*maxRecNum);
		
		if(numberOfRecords == 0)
		memAddress = 0x7FFF;
		else{
			for(uint16_t j = 0x7FFF; j > lowerMemIndexLimit; j -= 9){
				index = startIndex;
				valid = 1;
				for(uint8_t i = 0; i < numberOfRecords; i++){
					memAddress = getMemMapElement(index);
					if(memAddress == j){
						valid = 0;
						break;
					}
					else
					index = (index + 1) % maxRecNum;
				}
				if(valid == 1){
					memAddress = j;
					break;
				}
			}
		}
		
		//if the memory map is not empty, find correct position to place new record
		//based on time.
		
		uint8_t incrementFromFront = 0;
				
		int8_t placementIndex = endIndex;
		
		uint16_t time_1[6] = {year,month,day,hour,minute,second};
		uint16_t time_2[6];
		uint16_t address;
		
		uint8_t compareTimeResult = 0;
		
		if(numberOfRecords == 0)
			placementIndex = startIndex;
		else{
			for(uint8_t i = 0; i < numberOfRecords; i++){
				address = getMemMapElement(placementIndex);
				time_2[0] = getYear(address);
				time_2[1] = getMonth(address);
				time_2[2] = getDate(address);
				time_2[3] = getHour(address);
				time_2[4] = getMinute(address);
				time_2[5] = getSecond(address);
				
				// if time return 0 or 2, then the time being inserted
				// comes after the time it's being compared to we have found the right location
				compareTimeResult = compareTimes(time_1,time_2);
				if(compareTimeResult != 1){
					incrementFromFront = i;
					break;
				}
				if(--placementIndex < 0)
					placementIndex = maxRecNum - 1;
				incrementFromFront = i+1;
			}
			placementIndex = (placementIndex + 1) % maxRecNum;
		}		
		
		// if the record being inserted comes before a record already present
		// records that come after the insertion point must be shifted.
		index = (endIndex + 1) % maxRecNum;
	
		for(uint8_t i = 0; i < incrementFromFront; i++){
			if(index != 0){
				setMemMapElement(index, getMemMapElement(index-1));
				index--;
			}
			else{
				setMemMapElement(index, getMemMapElement(maxRecNum-1));
				index = maxRecNum-1;
			}
		}				
			
					
		setMemMapElement(placementIndex, memAddress);
		
		// memAddress now holds a valid mem location address
		extEEPROMWrite(memAddress--, year);
		extEEPROMWrite(memAddress--, month);
		extEEPROMWrite(memAddress--, day);
		extEEPROMWrite(memAddress--, dow);
		extEEPROMWrite(memAddress--, hour);
		extEEPROMWrite(memAddress--, minute);
		extEEPROMWrite(memAddress--, second);
		extEEPROMWrite(memAddress--, activity);
		extEEPROMWrite(memAddress--, mood);
		
		setUnencryptedRecordPointer(memAddress);
		
		numberOfRecords++;
		setUnencryptedRecordCount(numberOfRecords);
		
		if(numberOfRecords > 1){
			endIndex = (endIndex + 1) % maxRecNum;
			setMemMapEndIndex(endIndex);
		}		

		USART_Sendbyte(getMemMapStartIndex());
		USART_Sendbyte(getMemMapEndIndex());
		USART_Sendbyte(placementIndex);
		
		for(uint8_t i = 0; i < maxRecNum; i++){
			USART_Sendbyte(getMemMapElement(i));
		}
		
	}

	uint8_t findSpaceInMemMap(uint16_t year, uint8_t month, uint8_t day, uint8_t hour,uint8_t minute,uint8_t second){
		//USART_Sendbyte(0xaa);
		uint16_t time_1[6] = {year,month,day,hour,minute,second};
		uint16_t time_2[6];
		
		uint8_t returnState = 0;

		uint8_t start = getMemMapStartIndex();
		uint8_t index = getMemMapEndIndex();
		uint8_t numberOfRecords = getUnencrpytedRecordCount();
		uint8_t memMapFull = 0;
		uint16_t address = 0;
		
		//USART_Sendbyte(numberOfRecords);
		
		//if the memory map is full
		if (numberOfRecords >= maxRecNum)
		memMapFull = 1;
		
		
		while (memMapFull == 1 || index >= start){
			memMapFull = 0;
			address = getMemMapElement(index);
			time_2[0] = getYear(address);
			time_2[1] = getMonth(address);
			time_2[2] = getDate(address);
			time_2[3] = getHour(address);
			time_2[4] = getMinute(address);
			time_2[5] = getSecond(address);
			
			returnState = compareTimes(time_1,time_2);
			if(returnState == 2 || returnState == 0){
				if(numberOfRecords >= maxRecNum)
				return index;
				else
				return (index + 1) % maxRecNum;
			}
			//this if for the edge case where there is only one record
			//we need to insert the record at the beginning of the array
			else if(returnState == 1 && numberOfRecords == 1){
				//USART_Sendbyte(index);
				return index;
			}
			
			if(index != 0){
				index--;
			}
			else{
				index = maxRecNum-1;
			}
			//USART_Sendbyte(0xAA);
			//USART_Sendbyte(index);
			//USART_Sendbyte(0xAB);
			//USART_Sendbyte(start);
		}
		return 0;
	}


	void setActivityByAddress(uint16_t address, uint8_t activity){
		saveDataRecordToEEPROM(address,7,activity);
	}

	void setMoodByAddress(uint16_t address, uint8_t mood){
		saveDataRecordToEEPROM(address,8,mood);
	}

	void setYearByAddress(uint16_t address, uint8_t year){
		saveDataRecordToEEPROM(address,0,year);
	}

	void setMonthByAddress(uint16_t address, uint8_t month){
		saveDataRecordToEEPROM(address,1,month);
	}

	void setDayByAddress(uint16_t address, uint8_t day){
		saveDataRecordToEEPROM(address,2,day);
	}

	void setDOWByAddress(uint16_t address, uint8_t dow){
		saveDataRecordToEEPROM(address,3,dow);
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
		//USART_Sendbyte(address >> 8);
		//USART_Sendbyte(address);
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

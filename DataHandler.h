/*
 * dataHandler.h
 *
 * Created: 12/26/2012 5:21:47 PM
 *  Author: Andrew Evans
 */ 


#ifndef DATAHANDLER_H
#define DATAHANDLER_H

//0's the memory map 
void resetRecords();

//deletes record located at particular position within memory map
void deleteRecord(uint8_t memMapLocation);
void deleteOldestRecord(void);

//set record and allocate it a space in memory or set a record with a specified address
void setRecord(uint8_t year, 
				uint8_t month, 
				uint8_t day, 
				uint8_t dow, 
				uint8_t hour, 
				uint8_t minute, 
				uint8_t second, 
				uint8_t activity, 
				uint8_t mood);
				
void setEncryptedRecord(uint8_t memIndex);
void writeOutEncryptedRecords(void);

void setRecordByAddress(uint16_t address, 
						uint8_t year, 
						uint8_t month, 
						uint8_t day, 
						uint8_t dow, 
						uint8_t hour, 
						uint8_t minute, 
						uint8_t second, 
						uint8_t activity, 
						uint8_t mood);

void insertRecord(		uint8_t year,
						uint8_t month,
						uint8_t day,
						uint8_t dow,
						uint8_t hour,
						uint8_t minute,
						uint8_t second,
						uint8_t activity,
						uint8_t mood);


uint8_t findSpaceInMemMap(uint16_t year,
						uint8_t month,
						uint8_t day,
						uint8_t hour,
						uint8_t minute,
						uint8_t second);


void setActivityByAddress(uint16_t address, uint8_t activity);
void setMoodByAddress(uint16_t address, uint8_t mood);
void setYearByAddress(uint16_t address, uint8_t year);
void setMonthByAddress(uint16_t address, uint8_t month);
void setDayByAddress(uint16_t address, uint8_t day);
void setDOWByAddress(uint16_t address, uint8_t dow);
void setHourByAddress(uint16_t address, uint8_t hour);
void setMinuteByAddress(uint16_t address, uint8_t minute);


//getters for each element that makes up a record
uint8_t getYear(uint16_t address);
uint8_t getMonth(uint16_t address);
uint8_t getDate(uint16_t address);
uint8_t getDOW( uint16_t address);
uint8_t getHour(uint16_t address);
uint8_t getMinute(uint16_t address);
uint8_t getSecond(uint16_t address);

uint8_t getActivity(uint16_t address);
uint8_t getMood(uint16_t address);
void getAvailableRecords(uint8_t *usedElements);
uint8_t checkIfUsedElementExists(uint8_t recordID);
uint8_t getActivityArraySize();
uint8_t getMoodArraySize();

void loadActivitiesDataHandler();
void loadMoodsDataHandler();

void extEEPROMWriteString(uint16_t addr, char *string);
char* extEEPROMReadString(uint16_t aadr);

//void encryptOldRecords(index);

#endif /* DATAHANDLER_H_ */
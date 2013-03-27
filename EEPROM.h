/*
 * EEPROM.h
 *
 * Created: 2/14/2013 3:02:00 PM
 *  Author: Andrew Evans & Shawn Stoute
 */ 

#ifndef EEPROM_H
#define EEPROM_H

#define ACTIVITY 0
#define MOOD 1
#define FACES 2
#define eepromwrite 0b10100000
#define eepromread 0b10100001

//Read and write functions for external eeprom
void extEEPROMWrite(uint16_t addr, uint8_t data);
uint8_t extEEPROMRead (uint16_t addr);

//Stores Activites/Moods/Faces to eeprom
//These are set by the configuration tool and read into SRAM on startup
void ActivitiesToEEPROM(char *charPtr);
void MoodsToEEPROM(char  *charPtr);
void FacesToEEPROM(char *charPtr);

//These are getters for the activities/moods/faces
void ActOrMoodArrayFromEEPROM(uint8_t arrType, char **stringArr[], uint8_t arrSize);
void FaceArrayFromEEPROM(uint8_t arrType, uint8_t *intArr, uint8_t arrSize);

//saves records to the external eeprom and updates the memory map table 
void saveDataRecordToEEPROM(uint16_t index, uint8_t cell, uint8_t data);

//checks if memory map location is available or taken, returns 1 in taken, 0 if not taken
uint16_t getMemMapElement(uint8_t index);

//sets the base address of the mem map circular array
void setMemMapStartAddress(uint8_t);
void setMemMapEndAddress(uint8_t);

//returns the base address of the mem map circular array
uint8_t getMemMapStartAddress(void);
uint8_t getMemMapEndAddress(void);

//saves address to memory map in location specified by index 
void setMemMapElement(uint8_t index, uint16_t data);

//setters for Record pointers
void setUnencryptedRecordPointer(uint16_t address);
void setEncryptedRecordPointer(uint16_t address);

//getters for Record Pointers
uint16_t getUnencryptedRecordPointer(void);
uint16_t getEncryptedRecordPointer(void);

//setters for record counts
void setUnencryptedRecordCount( uint8_t count);
void setEncrpytedRecordCount( uint16_t count);

//getters for retrieving record counts
uint8_t getUnencrpytedRecordCount(void);
uint16_t getEncryptedRecordCount(void);

//setters for device ID and modulus
void setDeviceID(char* device_id_string);
void setModulus(char* modulus_string);

//getters for device ID and modulus
uint16_t getDeviceID(void);
uint16_t getModulus(void);

void setStartDate(char* start);
void setEndDate(char* end);

uint8_t getStartDateYear(void);
uint8_t getStartDateMonth(void);
uint8_t getStartDateDay(void);

uint8_t getEndDateYear(void);
uint8_t getEndDateMonth(void);
uint8_t getEndDateDay(void);

//getters & setters for Settings
uint8_t getInvertColourSetting(void);
void setInvertColourSetting(uint8_t);

#endif /* EEPROM_H_ */
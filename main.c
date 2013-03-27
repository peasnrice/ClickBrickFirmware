/*
* main.c
*
* Created: 2/14/2013 2:57:52 PM
*  Author: Andrew Evans
*/

#include <avr/io.h>
#include <math.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdint.h>
#include <avr/sleep.h>
#include "external.c"
#include "DataHandler.h"
#include "Buttons.h"
#include "SPI.h"
#include "SharpMemoryLCD.h"
#include "MenuUI.h"
#include "I2C.h"
#include "ds1307.h"
#include "usart.h"
#include "EEPROM.h"
#include "encryption.h"
//#include "NES.h"

#define SCREENWIDTH 144
#define SCREENHEIGHT 168

void putToSleep(void);
void researchPeriodCheck(void);
void checkAgeAndEncrypt(void);

extern uint8_t researchPeriod;
extern uint8_t modifiedTimeFlag;
extern volatile uint8_t breakFromUSARTFlag;

static uint16_t startDateArray[6];
static uint16_t endDateArray[6];
static uint16_t currentTimeArray[6];

//year,month,day,dow,hour,minute,second,activity,mood

//13,01,01,01,01,01,01,01,01,01


int main(void)
{
	USART_Init(MYUBRR);
	I2CInit();
	//resetRecords();
	initMenuUI();
	initButtons();
	//initNES();
	initLCD();
	initSPI();
	clearLCD();
	
	breakFromUSARTFlag = 0;
	researchPeriod = 0;
	modifiedTimeFlag = 0;
	invertColours = getInvertColourSetting();
	
	loadMoodsDataHandler();
	loadActivitiesDataHandler();
	loadMoodsMenu();
	loadActivitiesMenu();
	loadFacesMenu();
	
	startDateArray[0] = getStartDateYear();
	startDateArray[1] = getStartDateMonth();
	startDateArray[2] = getStartDateDay();
	startDateArray[3] = 0;
	startDateArray[4] = 0;
	startDateArray[5] = 0;
	
	endDateArray[0] = getEndDateYear();
	endDateArray[1] = getEndDateMonth();
	endDateArray[2] = getEndDateDay();
	endDateArray[3] = 0;
	endDateArray[4] = 0;
	endDateArray[5] = 0;
	
	//setTime(12,58,0,6,23,3,13);
	//setRecord(123,57,5,98,102,255,144,24,211);
	//setRecord(1,2,3,4,5,6,7,8,9);
	//setRecord(10,11,12,13,14,15,16,17,18);
	//
	//setEncryptedRecord(0);
	//setEncryptedRecord(1);
	//setEncryptedRecord(2);
	//writeOutEncryptedRecords();

	updateUI();
	
	uint8_t checksum = 0;
	uint8_t timeprevious = 0;
	uint8_t timenow = 0;
	uint8_t nesInput = 0;
	uint8_t buttonInput = 0;
	
	while(1)
	{
		
		//update screen on the minute
		timenow = getTimeMinute();
		if(timenow != timeprevious){
			researchPeriodCheck();
			updateUI();
			timeprevious = timenow;
			checkAgeAndEncrypt();
		}
		

		//check for button inputs
		//issue screen commands accordingly
		//USART_Sendbyte(checkForNESButtons());
		
		//nesInput = checkForNESButtons();
		buttonInput = checkForButtons();
		
		//nesInput = 0;
		//researchPeriod = 0;
		
		if(buttonInput){
			//up
			if (buttonInput == 1){
				if(getCurrentMenuID() != 0){
					decrementMenuCursor();
					updateUI();
				}
			}
			//down
			else if (buttonInput == 2){
				if(getCurrentMenuID() != 0){
					incrementMenuCursor();
					updateUI();
				}
			}
			//forward
			else if (buttonInput == 3){
				incrementMenuID();
				if(modifiedTimeFlag == 1){
					researchPeriodCheck();
					checkAgeAndEncrypt();	
				}					
				modifiedTimeFlag = 0;
				updateUI();
			}
			//back
			else if (buttonInput == 4){
				if(getCurrentMenuID() != 0){
					decrementMenuID();
					if(modifiedTimeFlag == 1){
						researchPeriodCheck();
						checkAgeAndEncrypt();	
					}						
					modifiedTimeFlag = 0;
					updateUI();
				}
				else{
					putToSleep();
					timenow = getTimeMinute();
					if(timenow != timeprevious){
						updateUI();
						timeprevious = timenow;
						checkAgeAndEncrypt();
					}
					updateUI();
				}
			}
			while(buttonInput){
				nesInput = checkForNESButtons();
				buttonInput = checkForButtons();
				//if the user is holding down the button,
				//we still want to refresh screen every minute on the minute
				timenow = getTimeMinute();
				if(timenow != timeprevious){
					updateUI();
					checkAgeAndEncrypt();
					timeprevious = timenow;
				}
			}
		}
		
		if(getCurrentMenuID() == 80){
			EIMSK |= (1<<INT2);
			sei();
			char* tempString = USART_Receive_CRLF();
			if(breakFromUSARTFlag == 0){
				if(strcmp(tempString,"INFO") == 0){
					USART_Send_string_CRLF("INFO");
				}
				else if(strcmp(tempString,"CONFIG") == 0){
					resetRecords();
					USART_Send_string_CRLF("CONFIG");

					//Device ID
					printStringOnLine(2,"                  ",0,0);
					printStringOnLine(3,"                  ",0,0);
					printStringOnLine(4,"  Configuration   ",0,0);
					printStringOnLine(5,"      Tool        ",0,0);
					printStringOnLine(6,"                  ",0,0);
					printStringOnLine(7,"Loading Device ID ",0,0);
				
					tempString = USART_Receive_CRLF();
					setDeviceID(tempString);
					USART_Send_string_CRLF("device ID recorded");

					//Modulus
					printStringOnLine(2,"                  ",0,0);
					printStringOnLine(3,"                  ",0,0);
					printStringOnLine(4,"  Configuration   ",0,0);
					printStringOnLine(5,"      Tool        ",0,0);
					printStringOnLine(6,"                  ",0,0);
					printStringOnLine(7,"Loading modulus   ",0,0);
				
					tempString = USART_Receive_CRLF();
					setModulus(tempString);
					USART_Send_string_CRLF("device Modulus recorded");
				
					//start Date
					printStringOnLine(2,"                  ",0,0);
					printStringOnLine(3,"                  ",0,0);
					printStringOnLine(4,"  Configuration   ",0,0);
					printStringOnLine(5,"      Tool        ",0,0);
					printStringOnLine(6,"                  ",0,0);
					printStringOnLine(7,"Loading StartDate ",0,0);
				
					tempString = USART_Receive_CRLF();
					setStartDate(tempString);
					//load startDate from EEPROM
					USART_Send_string_CRLF("StartDate Recorded");

					//end Date
					printStringOnLine(2,"                  ",0,0);
					printStringOnLine(3,"                  ",0,0);
					printStringOnLine(4,"  Configuration   ",0,0);
					printStringOnLine(5,"      Tool        ",0,0);
					printStringOnLine(6,"                  ",0,0);
					printStringOnLine(7,"Loading endDate   ",0,0);
				
					tempString = USART_Receive_CRLF();
					setEndDate(tempString);
					//load EndDate from EEPROM
					USART_Send_string_CRLF("EndDate Recorded");
				
					//activities
					printStringOnLine(2,"                  ",0,0);
					printStringOnLine(3,"                  ",0,0);
					printStringOnLine(4,"  Configuration   ",0,0);
					printStringOnLine(5,"      Tool        ",0,0);
					printStringOnLine(6,"                  ",0,0);
					printStringOnLine(7,"Loading Activities",0,0);
				
					tempString = USART_Receive_CRLF();
					ActivitiesToEEPROM(tempString);
					loadActivitiesMenu();
					USART_Send_string_CRLF("activities recorded");

					//moods
					printStringOnLine(2,"                  ",0,0);
					printStringOnLine(3,"                  ",0,0);
					printStringOnLine(4,"  Configuration   ",0,0);
					printStringOnLine(5,"      Tool        ",0,0);
					printStringOnLine(6,"                  ",0,0);
					printStringOnLine(7,"  Loading Moods   ",0,0);
				
					tempString = USART_Receive_CRLF();
					MoodsToEEPROM(tempString);
					loadMoodsMenu();
					USART_Send_string_CRLF("moods recorded");

					//faces
					printStringOnLine(2,"                  ",0,0);
					printStringOnLine(3,"                  ",0,0);
					printStringOnLine(4,"  Configuration   ",0,0);
					printStringOnLine(5,"      Tool        ",0,0);
					printStringOnLine(6,"                  ",0,0);
					printStringOnLine(7,"  Loading Faces   ",0,0);

					tempString = USART_Receive_CRLF();
					FacesToEEPROM(tempString);
					loadFacesMenu();
					USART_Send_string_CRLF("faces recorded");

					setMenuID(10);
					updateUI();
				}
				else if(strcmp(tempString,"BACKUP") == 0){

					uint8_t numOfUnencryptedRec = getUnencrpytedRecordCount();
					uint8_t numofEncryptedRecords = getEncryptedRecordCount();
					uint8_t totNumRec = numofEncryptedRecords + numOfUnencryptedRec;
				
					if(totNumRec == 0){
						setMenuID(10);
						updateUI();
						break;
					}
				
					uint8_t s = getMemMapStartAddress();
					uint8_t tempIndex = s;
					for(uint8_t i = s; i < numOfUnencryptedRec; i++){
						setEncryptedRecord(tempIndex);
						tempIndex = (tempIndex + 1) % 40;
					};
					
					uint16_t devID = getDeviceID();
					totNumRec++; //accounts for first string, sending public key
					char *numOfRecString[4];
					char *devIDString[4];
					memset(numOfRecString,0,4);
					snprintf(numOfRecString, 4, "%d", totNumRec);
				
					memset(devIDString,0,4);
					snprintf(devIDString, 4, "%d", devID);
				
					printStringOnLine(2,"                  ",0,0);
					printStringOnLine(3,"    Uploading     ",0,0);
					printStringOnLine(4,"     Records      ",0,0);
					printStringOnLine(5,"                  ",0,0);
					printStringOnLine(6,"     Please       ",0,0);
					printStringOnLine(7,"      Wait        ",0,0);
				
				
					USART_Send_string_CRLF(numOfRecString);
					USART_Send_string_CRLF(devIDString);
				
					uint16_t encryptedRecordCount = getEncryptedRecordCount();
					for(uint16_t i = 0; i < encryptedRecordCount; i++){
						USART_Send_string_CRLF(extEEPROMReadString(i*38));
					}
				
					setMenuID(10);
					updateUI();
				}
				else
					USART_Send_string_CRLF("Unknown Command");
			}
			cli();
			breakFromUSARTFlag = 0;	
			setMenuID(10);
			updateUI();	
			buttonInput = checkForButtons();
			while(buttonInput){
				buttonInput = checkForButtons();
			}
		}
	}
}

void putToSleep(void){
	printStringOnLine(0,"                  ",1,0);
	printStringOnLine(1,"                  ",1,0);
	printStringOnLine(2,"                  ",1,0);
	printStringOnLine(3,"                  ",1,0);
	printStringOnLine(4,"                  ",1,0);
	printStringOnLine(5,"                  ",1,0);
	printStringOnLine(6,"                  ",1,0);
	printStringOnLine(7,"                  ",1,0);
	EIMSK |= (1<<INT5);
	set_sleep_mode(SLEEP_MODE_PWR_SAVE);
	cli();
	sleep_enable();
	sei();
	sleep_cpu();
	sleep_disable();
}

void researchPeriodCheck(void){
	currentTimeArray[0] = getTimeYear();
	currentTimeArray[1] = getTimeMonth();
	currentTimeArray[2] = getTimeDate();
	currentTimeArray[3] = getTimeHour();
	currentTimeArray[4] = getTimeMinute();
	currentTimeArray[5] = getTimeSecond();
				
	uint8_t comparisonStartResult = compareTimes(currentTimeArray,startDateArray);
	uint8_t comparisonEndResult = compareTimes(currentTimeArray,endDateArray);
	if(comparisonStartResult == 1){
		researchPeriod = 1; //too early
	}
	else if(comparisonEndResult == 2){
		researchPeriod = 2; //too late
	}
	else{
		researchPeriod = 0; //correct time
	}
}

void checkAgeAndEncrypt(void){
	uint8_t comparisonResult = 0;
	uint8_t numOfRec = getUnencrpytedRecordCount();
	if(numOfRec != 0){
		currentTimeArray[0] = getTimeYear();
		currentTimeArray[1] = getTimeMonth();
		currentTimeArray[2] = getTimeDate();
		currentTimeArray[3] = getTimeHour();
		currentTimeArray[4] = getTimeMinute();
		currentTimeArray[5] = getTimeSecond();

		uint8_t startIndex = getMemMapStartAddress();	
		uint8_t index = startIndex;
		uint8_t endIndex = getMemMapEndAddress();
		uint16_t recordTimeArray[6];
		
		
		for(uint8_t i = index; i < numOfRec; i++){
			uint16_t address = getMemMapElement(index);
			recordTimeArray[0] = getYear(address);
			recordTimeArray[1] = getMonth(address);
			recordTimeArray[2] = getDate(address);
			recordTimeArray[3] = getHour(address);
			recordTimeArray[4] = getMinute(address);
			recordTimeArray[5] = getSecond(address);
			
			comparisonResult = fortyEightHoursApart(&currentTimeArray, &recordTimeArray);
			
			if(comparisonResult == 1){
				setEncryptedRecord(index);
				USART_Sendbyte(0xFF);
				deleteRecord(index);
			}			
			index = (index+1)%40;
		}
	}		
}

ISR(INT5_vect){
	cli();
	EIMSK &= ~(1<<INT5);
}

ISR(INT2_vect){
	breakFromUSARTFlag = 1;
	EIMSK &= ~(1<<INT2);
}
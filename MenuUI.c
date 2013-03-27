/*
 * MenuUI.c
 *
 * Created: 1/12/2013 11:04:55 AM
 *  Author: Andrew Evans
 */ 

#include <avr/io.h>
#include <stdlib.h>
#include <stdio.h>
#include "external.c"
#include "string.h"
#include "MenuUI.h"
#include "SharpMemoryLCD.h"
#include "DataHandler.h"
#include "ds1307.h"
#include "EEPROM.h"
#include "EpochTime.h"

#define NONE 0
#define BOXEDMENU 1
#define NOBOXMENU 2
#define TOPBAND 0
#define BOTTOMBAND 1
#define BOTTOMBANDMODIFY 2
#define LOG 0
#define ADD 1
#define EDIT 2
#define REMOVE 3
#define HOUR 0
#define MINUTE 1
#define DAY 2
#define MONTH 3
#define YEAR 4
#define NEUTRAL 0
#define INCREMENT 1
#define DECREMENT 2

uint8_t selectionInFrame;	//Keeps track of where to display items in a scrolling menu
uint8_t SelectionInFramePrevious;
uint8_t selectionInMenu;	//Keeps track of current position in the menu
uint8_t cellsPerFrame;
uint8_t currentMenuLength;
uint8_t yOffset;
uint8_t previousMenuID;
uint8_t currentMenuID;
uint8_t selectedActivity;
uint8_t selectedMood;
uint8_t selectedYear;
uint8_t selectedMonth;
uint8_t selectedDate;
uint8_t selectedDay;
uint8_t selectedHour;
uint8_t selectedMinute;
uint8_t selectedSecond;
uint8_t selectedDay;
uint8_t selectedDOW;
uint8_t selectedMonth;
uint8_t selectedYear;
uint8_t selectedRecord;
uint8_t firstPass;
uint8_t timeSelection;
uint8_t manipulateTime;
uint8_t actArrSize;
uint8_t moodArrSize;
uint8_t faceArrSize;
uint8_t availableIndex;
uint8_t selectedIndex;
uint8_t start;
uint8_t end;
uint8_t iter;
uint16_t address;

uint8_t activityInRecord;
uint8_t moodInRecord;

static char **activityArray[16];
static char **moodArray[16];
uint8_t faceArray[16];
char *mainMenuArray[] = { "Log", "View", "Edit", "Settings", "Sleep", "Upload" };
char *modifyMenuArray[] = { "Add Record", "Edit Record", "Delete Record" };
char *moodOrActivityArray[] = {"Activity", "Mood" };
char *settingsArray[] = {"Clock", "Invert", "Reset" };

uint8_t monthDays[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
char *monthNames[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sept", "Oct", "Nov", "Dec"};


void initMenuUI(void){
selectionInFrame = 0;	//Keeps track of where to display items in a scrolling menu
SelectionInFramePrevious = 0;
selectionInMenu = 0;	//Keeps track of current position in the menu
cellsPerFrame = 4;
currentMenuLength = 5;
yOffset = 3;	
currentMenuID = 0;
previousMenuID = 0;
selectedActivity = 0;
selectedMood = 0;
selectedRecord = 0;
firstPass = 0;
timeSelection = 0;
manipulateTime = 0;	
}

void loadActivitiesMenu(){
	actArrSize = 16;
	ActOrMoodArrayFromEEPROM(ACTIVITY,activityArray,actArrSize);
}

void loadMoodsMenu(){
	moodArrSize = 16;
	ActOrMoodArrayFromEEPROM(MOOD,moodArray,moodArrSize);
}

void loadFacesMenu(){
	faceArrSize = 16;
	FaceArrayFromEEPROM(FACES,faceArray,faceArrSize);
}

void incrementMenuCursor(){
	if(currentMenuID != 42 && currentMenuID != 52 && currentMenuID != 71){
		if(currentMenuID == 30 || currentMenuID == 50 || currentMenuID == 41 || currentMenuID == 60)
			currentMenuLength = getUnencrpytedRecordCount();
		if (selectionInMenu < currentMenuLength - 1){
			selectionInMenu++;
			if (selectionInFrame >= cellsPerFrame - 1)
				selectionInFrame = 0;
			else
				selectionInFrame++;	
		}
	}
	else		
		manipulateTime = INCREMENT;
}

void decrementMenuCursor(){
	if(currentMenuID != 42 && currentMenuID != 52 && currentMenuID != 71){
		if (selectionInMenu > 0){
			selectionInMenu--;
			if (selectionInFrame <= 0)
				selectionInFrame = cellsPerFrame - 1;
			else
				selectionInFrame--;	
		}				
	}
	else
		manipulateTime = DECREMENT;
}

//advance to next menu screen with current selection
void incrementMenuID(){
	clearLCD();
	previousMenuID = currentMenuID;
	//Coming From...
	//[Sleep]
	if (currentMenuID == 0 || currentMenuID == 1)
		currentMenuID = 10;
	
	//[Main Menu]
	else if (currentMenuID == 10){
		if(researchPeriod == 0){
			if (selectionInMenu == 0)
				currentMenuID = 20;
			else if (selectionInMenu == 1)
				currentMenuID = 30;
			else if (selectionInMenu == 2){
				currentMenuID = 40;
			}			
			else if (selectionInMenu == 3)
				currentMenuID = 70;
			else if (selectionInMenu == 4)
				currentMenuID = 0;
			else if (selectionInMenu == 5)
				currentMenuID = 80;
		}	
		else{
			if(selectionInMenu == 3 || selectionInMenu == 4 || selectionInMenu == 5){
				if (selectionInMenu == 3)
					currentMenuID = 70;
				else if(selectionInMenu == 4)
					currentMenuID = 0;
				else if(selectionInMenu == 5)
					currentMenuID = 80;
			}					
			else
				currentMenuID = 11;	
		}						
	}
	//[Log - Choose Activity]
	else if (currentMenuID == 20){
		selectedActivity = selectionInMenu;
		selectedYear = getTimeYear();
		selectedMonth = getTimeMonth();
		selectedDate = getTimeDate();
		selectedDay = getTimeDay();
		selectedHour = getTimeHour();
		selectedMinute = getTimeMinute();
		selectedSecond = getTimeSecond();
		currentMenuID++;
	}	
	//[Log - Choose Mood]	
	else if (currentMenuID == 21){
		selectedMood = selectionInMenu;
		currentMenuID++;		
	}
	//[Log - Confirm]
	else if (currentMenuID == 22){
		uint8_t numberOfRecords = getUnencrpytedRecordCount();
		if(numberOfRecords != 0){
			availableIndex = findSpaceInMemMap(selectedYear,selectedMonth,selectedDate,selectedHour,selectedMinute,selectedSecond);
		}
		else{
			availableIndex = getMemMapStartAddress();
		}
		insertRecord(availableIndex,selectedYear,selectedMonth,selectedDate, selectedDay, selectedHour,selectedMinute,selectedSecond,selectedActivity,selectedMood);
		currentMenuID = 10;		
	}
	//[View Timeline]
	else if (currentMenuID == 30){
		currentMenuID = 10;
	}
	
	//[Edit,Add,Remove]
	else if (currentMenuID == 40){
		uint8_t numberOfRecords = getUnencrpytedRecordCount();
		if (selectionInMenu == 0){
			if(numberOfRecords == 0){
				selectedYear = getTimeYear();
				selectedMonth = getTimeMonth();
				selectedDate = getTimeDate();
				selectedDay = getTimeDay();
				currentMenuID = 42;
			}	
			else{
				currentMenuID = 41;
			}							
		}
		else{				
			if(numberOfRecords == 0)
				currentMenuID = 43;
			else{
				if (selectionInMenu == 1)
				currentMenuID = 50;
				else if (selectionInMenu == 2)
				currentMenuID = 60;
			}				
		}				
	}
	
	//[Add]
	else if (currentMenuID == 41){
		uint8_t numberOfRecords = getUnencrpytedRecordCount();
		if(numberOfRecords != 0){
			iter = getMemMapStartAddress();
			selectedRecord = (selectionInMenu + iter) % 40;
			address = getMemMapElement(selectedRecord);
		}
		else{		
			selectedYear = getTimeYear();
			selectedMonth = getTimeMonth();
			selectedDate = getTimeDate();
			selectedDay = getTimeDay();
			selectedHour = getTimeHour();
			selectedMinute = getTimeMinute();
			selectedSecond = getTimeSecond();
		}		
		firstPass = 0;
		currentMenuID = 42;
	}
	
	//[Add - choose time]
	else if (currentMenuID == 42){
		if(timeSelection == HOUR)
			timeSelection = MINUTE;
		else
			currentMenuID = 44;	
	}
	
	//[Edit - NO RECORDS]
	else if (currentMenuID == 43){
		currentMenuID = 10;
	}

	//[Add - choose Activity]
	else if (currentMenuID == 44){
		selectedActivity = selectionInMenu;
		currentMenuID = 45;
	}
	
	//[Add - choose Mood]
	else if (currentMenuID == 45){
		selectedMood = selectionInMenu;
		currentMenuID = 46;
	}
	
	//[Add - Confirm]
	else if (currentMenuID == 46){
		uint8_t numberOfRecords = getUnencrpytedRecordCount();
		if(numberOfRecords != 0){
			availableIndex = findSpaceInMemMap(selectedYear,selectedMonth,selectedDate,selectedHour,selectedMinute,0);
		}			
		else{
			availableIndex = getMemMapStartAddress();
		}			
		insertRecord(availableIndex, selectedYear,selectedMonth,selectedDate, selectedDay, selectedHour,selectedMinute,0,selectedActivity,selectedMood);
		currentMenuID = 10;
	}
	
	//[Edit - choose time]
	else if (currentMenuID == 50){
		start = getMemMapStartAddress();
		selectedIndex = (selectionInMenu + start) % 40;
		address = getMemMapElement(selectedIndex);
		activityInRecord = getActivity(address);
		moodInRecord = getMood(address);
		selectedHour = getHour(address);
		selectedMinute = getMinute(address);
		currentMenuID = 51;
	}
	
	//[Edit - Mood or Act]
	else if (currentMenuID == 51){
		if (selectionInMenu == 0){
			firstPass = 0;
			currentMenuID = 52;
		}			
		else if (selectionInMenu == 1)
			currentMenuID = 56;
	}

	//[Edit - Act - Choose Time]
	else if (currentMenuID == 52){
		if(timeSelection == HOUR)
			timeSelection = MINUTE;
		else
		currentMenuID = 54;
	}

	//[Edit - Act - Choose Act]
	else if (currentMenuID == 54){
		selectedActivity = selectionInMenu;
		selectedMood = moodInRecord;
		currentMenuID = 55;
	}

	//[Edit - Act - Confirm]
	else if (currentMenuID == 55){
		setHourByAddress(address,selectedHour);
		setMinuteByAddress(address,selectedMinute);
		setActivityByAddress(address,selectedActivity);		
		currentMenuID = 10;
	}

	//[Edit - Mood - Choose Mood]
	else if (currentMenuID == 56){
		selectedMood = selectionInMenu;
		selectedActivity = activityInRecord;
		currentMenuID = 57;
	}

	//[Edit - Mood - Confirm]
	else if (currentMenuID == 57){
		setMoodByAddress(address,selectedMood);
		currentMenuID = 10;
	}
	
	//[Remove - choose record]
	else if (currentMenuID == 60){
		iter = getMemMapStartAddress();
		selectedRecord = (selectionInMenu + iter) % 40;
		address = getMemMapElement(selectedRecord);
		selectedActivity = getActivity(address);
		selectedMood = getMood(address);
		selectedHour = getHour(address);
		selectedMinute = getMinute(address);
		currentMenuID = 61;
	}

	//[Remove - confirm]
	else if (currentMenuID == 61){
		deleteRecord(selectedRecord);
		currentMenuID = 10;
	}

	//[Settings]
	else if (currentMenuID == 70){
		if(selectionInMenu == 0){
			firstPass = 0;
			currentMenuID = 71;	
		}			
		else if(selectionInMenu == 1)
			currentMenuID = 72;
		else if(selectionInMenu == 2)
			currentMenuID = 73;
	}
	
	//[Settings - modify Time]
	else if (currentMenuID == 71){
		if(timeSelection == HOUR)
			timeSelection = MINUTE;
		else if (timeSelection == MINUTE)
			timeSelection = DAY;
		else if (timeSelection == DAY)
			timeSelection = MONTH;
		else if (timeSelection == MONTH)
			timeSelection = YEAR;
		else{
			currentMenuID = 70;
			modifiedTimeFlag = 1;
		}			
	}

	//[Settings - Invert Screen]
	else if (currentMenuID == 72){
		invertColours = getInvertColourSetting();
		setInvertColourSetting(!invertColours);
		invertColours = !invertColours;
	}
	
	//[Settings - Reset]
	else if (currentMenuID == 73){
		currentMenuID = 74;
	}
	
	//[Settings - Reset]
	else if (currentMenuID == 74){
		resetRecords();
		currentMenuID = 70;
	}	
			
	//[Upload]
	else if (currentMenuID == 80){
		//currentMenuID = 10;
	}
	
	selectionInFrame = 0;
	selectionInMenu = 0;
}	

//Return to previous screen
void decrementMenuID(){
	selectionInFrame = 0;
	selectionInMenu = 0;
	clearLCD();
	//Coming From...
	//[Sleep]
	
	//[Main Menu]
	if (currentMenuID == 10)
		currentMenuID = 0;

	//[Invalid research period]
	if (currentMenuID == 11)
		currentMenuID = 10;

	//[Log - Choose Activity]
	else if (currentMenuID == 20){
		currentMenuID = 10;
	}	
	//[Log - Choose Mood]	
	else if (currentMenuID == 21){
		currentMenuID--;		
	}
	//[Log - Confirm]
	else if (currentMenuID == 22)
		currentMenuID--;		
	//[View Timeline]
	else if (currentMenuID == 30){
		currentMenuID = 10;
	}
	//[Edit,Add,Remove]
	else if (currentMenuID == 40){
		currentMenuID = 10;
	}
	
	//[Choose Timeslot]
	else if (currentMenuID == 41 || currentMenuID == 50 || currentMenuID == 60){
		currentMenuID = 40;
	}
	
	//[Add - Choose Timeslot]
	else if (currentMenuID == 41){
		currentMenuID = 40;
	}
	
	//[Add - choose time]
	else if (currentMenuID == 42){
		uint8_t numberOfRecords = getUnencrpytedRecordCount();
		if(timeSelection == MINUTE)
			timeSelection = HOUR;
		else{
			if(numberOfRecords == 0)
				currentMenuID = 40;
			else
				currentMenuID = 41;
		}				
		firstPass = 1;
		timeSelection = 0;
	}
	
	//[Add - Empty Records]
	else if (currentMenuID == 43){
		currentMenuID = 10;
	}

	//[Add - choose Activity]
	else if (currentMenuID == 44){
		firstPass = 1;
		timeSelection = 0;
		currentMenuID = 42;
	}
	
	//[Add - choose Mood]
	else if (currentMenuID == 45){
		currentMenuID = 44;
	}
	
	//[Add - Confirm]
	else if (currentMenuID == 46){
		currentMenuID = 45;
	}
	
	//[Edit - choose timeslot]
	else if (currentMenuID == 50){
		currentMenuID = 40;
	}
	
	//[Edit - Mood or Act]
	else if (currentMenuID == 51){
		currentMenuID = 50;
	}

	//[Edit - Act - Choose Time or Mood]
	else if (currentMenuID == 52 || currentMenuID == 56){
		if(timeSelection == MINUTE)
			timeSelection = HOUR;
		else
			currentMenuID = 51;	
		currentMenuID = 51;
	}

	//[Edit - Act - Choose Act]
	else if (currentMenuID == 54){
		firstPass = 0;
		currentMenuID = 52;
	}

	//[Edit - Act - Confirm]
	else if (currentMenuID == 55){
		currentMenuID = 54;
	}

	//[Edit - Act - Confirm]
	else if (currentMenuID == 57){
		currentMenuID = 56;
	}
	
	//[Remove]
	else if (currentMenuID == 60){
		selectedRecord = selectionInMenu;
		currentMenuID = 40;
	}

	//[Remove]
	else if (currentMenuID == 61){
		//deleteRecord();
		currentMenuID = 60;
	}
	
	//[Settings]
	else if (currentMenuID == 70){
		currentMenuID = 10;
	}
	
	//[Settings - change time]
	else if (currentMenuID == 71){
		if(timeSelection == YEAR)
			timeSelection = MONTH;
		else if (timeSelection == MONTH)
			timeSelection = DAY;
		else if (timeSelection == DAY)
			timeSelection = MINUTE;
		else if (timeSelection == MINUTE)
			timeSelection = HOUR;
		else{
			currentMenuID = 70;
			modifiedTimeFlag = 1;	
		}			
	}
	
	//[Settings - Invert]
	else if (currentMenuID == 72){
		currentMenuID = 70;
	}	

	//[Settings - Reset 1]
	else if (currentMenuID == 73){
		currentMenuID = 70;
	}

	//[Settings - Reset 2]
	else if (currentMenuID == 74){
		currentMenuID = 73;
	}

	//[Upload]
	else if (currentMenuID == 80){
		currentMenuID = 10;
	}
}	

void printTime(void){
	char timeString[18];
	memset(timeString,0,sizeof(timeString));
		
	char time[6];
	memset(time,0,sizeof(time));	
	
	getTimeString(time);
	
	 snprintf(timeString, sizeof(timeString), "%s%s%s", "      ", time, "                  ");
		timeString[17] = ' ';
	
	printStringOnLine(0,timeString, 1,NONE);
}	

//Draws Black screen to LCD
void drawSleep(void){		
	char timeString[6];
	memset(timeString,0,sizeof(timeString));
		
	char time[6];
	memset(time,0,sizeof(time));	
	
	getTimeString(time);
	
	snprintf(timeString, sizeof(timeString), "%s%s", time, " ");
	timeString[5] = ' ';
	printStringOnLine(0,"                  ",1,NONE);
	printStringOnLine(1,"                  ",1,NONE);
	printStringOnLine(2,"                  ",1,NONE);
	printStringOnLine(3,"                  ",1,NONE);
	printStringOnLine(4,"                  ",1,NONE);
	printStringOnLine(5,"                  ",1,NONE);
	printStringOnLine(6,"                  ",1,NONE);
	printStringOnLine(7,"                  ",1,NONE);
	printBigTime(2,timeString, 1);		
}

void drawMenuBands(uint8_t menuPage, uint8_t totalPages, uint8_t position){
	if (position == TOPBAND){
		if (menuPage == 0)
			printStringOnLine(2,"~~~~~~~TOP~~~~~~~~",0,NONE);			
		else
			printStringOnLine(2,"^^^^^^^MORE^^^^^^^",0,NONE);			
	}	
	else if (position == BOTTOMBAND){
		if (menuPage == totalPages){
			printStringOnLine(7,"~~~~~~~END~~~~~~~~",0,NONE);
		}
		else
			printStringOnLine(7,"_______MORE_______",0,NONE);
	}
	else if (position == BOTTOMBANDMODIFY){
		if (menuPage == totalPages){
			printStringOnLine(5,"~~~~~~~END~~~~~~~~",0,NONE);
		}
		else
			printStringOnLine(5,"_______MORE_______",0,NONE);
	}
}

//Draws the menu dynamically to specified LCD. 
void drawMainMenu(void){	

	currentMenuLength = sizeof(mainMenuArray)/sizeof(mainMenuArray[0]);
	uint8_t index = 0;
	uint8_t menuPage = 0;
	uint8_t totalPages = 0;
	
	char mainMenuString[18];
	memset(mainMenuString,0,sizeof(mainMenuString));
	
	printTime();
	printStringOnLine(1,"    Main Menu     ", 1,NONE);
	
	for (uint8_t i = 0; i < cellsPerFrame; i++){
		totalPages = ((currentMenuLength-1)/cellsPerFrame);
		menuPage = (selectionInMenu/cellsPerFrame);	
		index = (menuPage*cellsPerFrame)+i;
		
		if (index < currentMenuLength){
 			snprintf(mainMenuString, sizeof(mainMenuString), "%s%s%s", " ", mainMenuArray[index], "                  ");
			mainMenuString[17] = ' ';
		}
		
		drawMenuBands(menuPage,totalPages,TOPBAND);			
				
		//Draw Menu Contents
		if (i == selectionInFrame)
			printStringOnLine(i+yOffset,mainMenuString, 1,NOBOXMENU);
		else if (index < currentMenuLength)
			printStringOnLine(i+yOffset,mainMenuString, 0,NOBOXMENU);
		else
			printStringOnLine(i+yOffset,"                  ", 0,NOBOXMENU);
			
		drawMenuBands(menuPage,totalPages,BOTTOMBAND);	
			
	}		
	
}

void drawInavlidResearchPeriod(void){
	char *time[9];
	char *buffer[18];
	memset(time,0,9);
	memset(buffer,0,9);
	if(researchPeriod == 1){
		snprintf(time, 9, "%d%s%d%s%d", getStartDateDay(),"/",getStartDateMonth(),"/",getStartDateYear());
		snprintf(buffer, 19, "%s%s%s", "      ", time, "                  ");
		buffer[17] = ' ';
		printTime();
		printStringOnLine(1,"    Too Early!    ",1,0);
		printStringOnLine(2,"                  ",0,0);
		printStringOnLine(3,"      Sorry!      ",0,0);
		printStringOnLine(4,"   the research   ",0,0);
		printStringOnLine(5," period begins on ",0,0);
		printStringOnLine(6,buffer,0,0);
		printStringOnLine(7,"                  ",0,0);
	}
	else if(researchPeriod == 2){
		snprintf(time, 9, "%d%s%d%s%d", getEndDateDay(),"/",getEndDateMonth(),"/",getEndDateYear());
		snprintf(buffer, 19, "%s%s%s", "      ", time, "                  ");
		buffer[17] = ' ';
		printTime();
		printStringOnLine(1,"    Too Late!     ",1,0);
		printStringOnLine(2,"                  ",0,0);
		printStringOnLine(3,"      Sorry!      ",0,0);
		printStringOnLine(4,"   the research   ",0,0);
		printStringOnLine(5," period ended on  ",0,0);
		printStringOnLine(6,buffer,0,0);
		printStringOnLine(7,"                  ",0,0);
	}	
}

//Draws the menu dynamically to specified LCD. 
void drawChooseActivity(void){	
	
	currentMenuLength = actArrSize;
	uint8_t index = 0;
	uint8_t menuPage = 0;
	uint8_t totalPages = 0;
	
	char activityString[18];
	

	printTime();
	printStringOnLine(1," Choose Activity  ", 1,NONE);
	
	for (uint8_t i = 0; i < cellsPerFrame; i++){
		memset(activityString,0,sizeof(activityString));
		totalPages = ((currentMenuLength-1)/cellsPerFrame);
		menuPage = (selectionInMenu/cellsPerFrame);	
		index = ((menuPage)*cellsPerFrame)+i;
		
		if(index < currentMenuLength){
			snprintf(activityString, sizeof(activityString), "%s%s%s", " ", activityArray[index], "                  ");
			activityString[17] = ' ';
		}

		drawMenuBands(menuPage,totalPages,TOPBAND);			
				
		//Draw Menu Contents
		if (i == selectionInFrame)
			printStringOnLine(i+yOffset,activityString, 1,NOBOXMENU);
		else if (index < currentMenuLength)
			printStringOnLine(i+yOffset,activityString, 0,NOBOXMENU);
		else
			printStringOnLine(i+yOffset,"                  ", 0,NOBOXMENU);
			
		drawMenuBands(menuPage,totalPages,BOTTOMBAND);	
	}											
}

void drawChooseMood(void){
	
	currentMenuLength = moodArrSize;
	uint8_t index = 0;
	uint8_t menuPage = 0;
	uint8_t totalPages = 0;
	
	char moodString[18];
	
	
	printTime();
	printStringOnLine(1,"   Choose Mood    ", 1,NONE);
	
	for (uint8_t i = 0; i < cellsPerFrame; i++){
		memset(moodString,0,sizeof(moodString));
		totalPages = ((currentMenuLength-1)/cellsPerFrame);
		menuPage = (selectionInMenu/cellsPerFrame);	
		index = ((menuPage)*cellsPerFrame)+i;
		
		if(index < currentMenuLength){
			snprintf(moodString, sizeof(moodString), "%s%s%s", " ", moodArray[index], "                  ");
			moodString[17] = ' ';
		}

		drawMenuBands(menuPage,totalPages,TOPBAND);			
				
		//Draw Menu Contents
		if (i == selectionInFrame)
			printStringOnLine(i+yOffset,moodString, 1,NOBOXMENU);
		else if (index < currentMenuLength)
			printStringOnLine(i+yOffset,moodString, 0,NOBOXMENU);
		else
			printStringOnLine(i+yOffset,"                  ", 0,NOBOXMENU);
			
		drawMenuBands(menuPage,totalPages,BOTTOMBAND);	
			
	}				
}

void drawLogConfirm(uint8_t screenType){
	
	printTime();
	printStringOnLine(1,"Confirm Selection ", 1,NONE);
	
	
	char timeString[18];
	memset(timeString, 0, sizeof timeString);
	char activityString[18];
	memset(activityString, 0, sizeof activityString);
	char moodString[18];
	memset(moodString, 0, sizeof moodString);
	char hourString[3] = {"0"};
	char minuteString[3] = {"0"};
	char tempString[3];
	
	itoa(selectedHour, tempString, 10);
	if (selectedHour < 10)
		strcat(hourString, tempString);
	else{
		strcpy(hourString, tempString);
	}
	
	itoa(selectedMinute, tempString, 10);
	if (selectedMinute < 10)
		strcat(minuteString, tempString);
	else{
		strcpy(minuteString, tempString);
	}
	memset(timeString,' ',18);
	snprintf(timeString, sizeof timeString, "%s%s%s%s%s", "Time:", hourString, ":", minuteString, "                  ");	
	timeString[17] = ' ';
	
	snprintf(activityString, sizeof activityString, "%s%s%s", "Activity:", activityArray[selectedActivity], "                  ");
	activityString[17] = ' ';
	
	snprintf(moodString, sizeof moodString, "%s%s%s", "Mood:", moodArray[selectedMood], "                  ");
	moodString[17] = ' ';
	
	printStringOnLine(2,timeString,0,NONE);	
	printStringOnLine(3,activityString,0,NONE);	
	printStringOnLine(4,moodString,0,NONE);	
	
	if(screenType == LOG){
		printStringOnLine(5,"Confirm Log       ", 1, NONE);	
		printStringOnLine(6,"Are you sure?     ", 1, NONE);
	}
	else if(screenType == ADD){
		printStringOnLine(5,"Confirm Add       ", 1, NONE);	
		printStringOnLine(6,"Are you sure      ", 1, NONE);
	}		
	else if(screenType == EDIT){
		printStringOnLine(5,"Confirm Edit      ", 1, NONE);	
		printStringOnLine(6,"Are you sure?     ", 1, NONE);
	}		
	else if(screenType == REMOVE){
		printStringOnLine(5,"Confirm Removal   ", 1, NONE);	
		printStringOnLine(6,"Are you sure      ", 1, NONE);
	}
	printStringOnLine(7,"                  ", 1, NONE);	
	
}

void drawTimeline(void){
	uint8_t menuPage = 0;
	uint8_t totalPages = 0;
	char tempString[3];
	char timeBuf[18];
	char buf[18];

	printTime();
	printStringOnLine(1,"    Timeline      ", 1,NONE);

	
	uint8_t index = 0;
	
	start = getMemMapStartAddress();
	end = getMemMapEndAddress();
	iter = start;
		
	uint8_t currentMenuLength = getUnencrpytedRecordCount();
	
	for (uint8_t i = 0; i < cellsPerFrame; i++){
		char hourString[3] = {"0"};
		char minuteString[3] = {"0"};
		
		if (currentMenuLength == 0){
			printTimeLineIsEmpty();
			break;
		}
		
		totalPages = ((currentMenuLength-1)/cellsPerFrame);
		menuPage = (selectionInMenu/cellsPerFrame);	
		index = ((menuPage)*cellsPerFrame)+i;
		
		iter = (index + start) % 40;
		
		selectedHour = getHour(getMemMapElement(iter));
		itoa(selectedHour, tempString, 10);
		if (selectedHour < 10)
			strcat(hourString, tempString);
		else{
			strcpy(hourString, tempString);
		}
	
		selectedMinute = getMinute(getMemMapElement(iter));
		itoa(selectedMinute, tempString, 10);
		if (selectedMinute < 10)
			strcat(minuteString, tempString);
		else{
			strcpy(minuteString, tempString);
		}
		
		if(iter != end){
			snprintf(timeBuf, sizeof(timeBuf), "%s%s%s%s", hourString, ":", minuteString, "                  "); 
			snprintf(buf, sizeof(buf), "%s%s%s%s", activityArray[getActivity(getMemMapElement(iter))], "-", moodArray[getMood(getMemMapElement(iter))], "                  "); 	
			timeBuf[17] = ' ';
			buf[17] = ' ';
	
			drawMenuBands(menuPage,totalPages,TOPBAND);			
				
			//Draw Menu Contents
			if (i == selectionInFrame){
				printStringOnLine(i*2+yOffset,timeBuf, 1,NOBOXMENU);
				printTimelineStringOnLine(i*2+yOffset+1,buf, 1,faceArray[getMood(getMemMapElement(iter))]);
			}			
			else if (index < currentMenuLength){
				printStringOnLine(i*2+yOffset,timeBuf, 0,NOBOXMENU);
				printTimelineStringOnLine(i*2+yOffset+1,buf, 0,faceArray[getMood(getMemMapElement(iter))]);
			}
		}						
		else{
			printStringOnLine(i*2+yOffset,"                  ", 0,NOBOXMENU);
			printStringOnLine(i*2+yOffset+1,"                  ", 0,NOBOXMENU);
		}				
		drawMenuBands(menuPage,totalPages,BOTTOMBAND);			
	}		
	
}

void drawModifyMenu(void){
	currentMenuLength = sizeof(modifyMenuArray)/sizeof(modifyMenuArray[0]);
	uint8_t index = 0;
	uint8_t menuPage = 0;
	uint8_t totalPages = 0;
	
	char modifyMenuString[18];
	memset(modifyMenuString,0,sizeof(modifyMenuArray));
	
	printTime();
	printStringOnLine(1,"  Modify Records  ", 1,NONE);
	
	for (uint8_t i = 0; i < cellsPerFrame; i++){
		totalPages = ((currentMenuLength-1)/cellsPerFrame);
		menuPage = (selectionInMenu/cellsPerFrame);	
		index = (menuPage*cellsPerFrame)+i;
		
		if (index < currentMenuLength){
 			snprintf(modifyMenuString, sizeof(modifyMenuString), "%s%s%s", " ", modifyMenuArray[index], "                  ");
			modifyMenuString[17] = ' ';
		}
		
		drawMenuBands(menuPage,totalPages,TOPBAND);			
				
		//Draw Menu Contents
		if (i == selectionInFrame)
			printStringOnLine(i+yOffset,modifyMenuString, 1,NOBOXMENU);
		else if (index < currentMenuLength)
			printStringOnLine(i+yOffset,modifyMenuString, 0,NOBOXMENU);
		else
			printStringOnLine(i+yOffset,"                  ", 0,NOBOXMENU);
			
		drawMenuBands(menuPage,totalPages,BOTTOMBAND);	
			
	}		
}

void drawModifyTimeslot(uint8_t screenType){
//Copy the list of used elements from datahandler.cpp (from memory)
//go through usedElements list to see which memory cells have valid data
//make a note of the index values that contain valid data
//now we know what indexes to look at when we want to print data to the screen.
	char tempString[3];
	
	uint8_t menuPage = 0;
	uint8_t totalPages = 0;
	
	char timeBuf[18];
	char buf[18];
	memset(buf, 0, sizeof(buf));
	memset(timeBuf, 0, sizeof(timeBuf));
	printTime();
	if(screenType == ADD){
		printStringOnLine(1,"   Add Timeslot   ", 1,NONE);
		printStringOnLine(6,"Choose nearby time", 1,NOBOXMENU);	
		printStringOnLine(7,"to one being added", 1,NOBOXMENU);	
	}
	else if(screenType == EDIT){
		printStringOnLine(1,"   Edit Timeslot  ", 1,NONE);
		printStringOnLine(6,"Choose a timeslot ", 1,NOBOXMENU);	
		printStringOnLine(7,"to edit           ", 1,NOBOXMENU);			
	}
	else if(screenType == REMOVE){
		printStringOnLine(1," Remove Timeslot  ", 1,NONE);
		printStringOnLine(6,"Choose a timeslot ", 1,NOBOXMENU);	
		printStringOnLine(7,"to remove         ", 1,NOBOXMENU);					
	}	
	
	
	uint8_t index = 0;
		
	start = getMemMapStartAddress();
	end = getMemMapEndAddress();
	iter = start;
		
	uint8_t currentMenuLength = getUnencrpytedRecordCount();
		
	for (uint8_t i = 0; i < cellsPerFrame; i++){
	char hourString[3] = {"0"};
	char minuteString[3] = {"0"};

	if (currentMenuLength == 0){
		printTimeLineIsEmpty();
		break;
	}

	totalPages = ((currentMenuLength-1)/cellsPerFrame);
	menuPage = (selectionInMenu/cellsPerFrame);
	index = ((menuPage)*cellsPerFrame)+i;

	iter = (index + start) % 40;
		
	selectedHour = getHour(getMemMapElement(iter));
	itoa(selectedHour, tempString, 10);
	if (selectedHour < 10)
		strcat(hourString, tempString);
	else{
		strcpy(hourString, tempString);
	}
	
	selectedMinute = getMinute(getMemMapElement(iter));
	itoa(selectedMinute, tempString, 10);
	if (selectedMinute < 10)
		strcat(minuteString, tempString);
	else{
		strcpy(minuteString, tempString);
	}

	if(iter != end){
		snprintf(buf, sizeof(buf), "%s%s%s%s", hourString, ":", minuteString, "|"); 
		snprintf(buf, sizeof(buf), "%s%s%s%s%s", buf, activityArray[getActivity(getMemMapElement(iter))], "-", moodArray[getMood(getMemMapElement(iter))], "                  "); 	
		timeBuf[17] = ' ';
		buf[17] = ' ';
	
		drawMenuBands(menuPage,totalPages,TOPBAND);			
				
		//Draw Menu Contents
		if (i == selectionInFrame){
			printStringOnLine(i+yOffset,buf, 1,NOBOXMENU);
		}			
		else if (index < currentMenuLength){
			printStringOnLine(i+yOffset,buf, 0,NOBOXMENU);
		}
	}					
	else
		printStringOnLine(i+yOffset,"                  ", 0,NOBOXMENU);
		
	drawMenuBands(menuPage,totalPages,BOTTOMBANDMODIFY);
					
	}		
}

void drawChooseStartTime(void){
	uint8_t modMinute = 0;
	uint8_t divMinute = 0;
	uint8_t numberOfRecords = getUnencrpytedRecordCount();
	iter = 0;
	printTime();
	printStringOnLine(1,"     Timeline     ", 1,NOBOXMENU);	
	printStringOnLine(2,"                  ", 0,NOBOXMENU);		
	
	iter = getMemMapStartAddress();
	iter = (iter + selectedRecord) % 40;
	
	if (firstPass == 0){
		timeSelection = 0;
		
		if(numberOfRecords != 0){
			//find closest time to selected time in time slot
			selectedHour = getHour(getMemMapElement(iter));
			selectedMinute = getMinute(getMemMapElement(iter));
		}			
		
		modMinute = selectedMinute % 15;
		if(modMinute != 0){
			divMinute = selectedMinute/15;
			if(modMinute <=8)
				selectedMinute = divMinute*15;
			else
				selectedMinute = divMinute*15 + 15; 
			if(selectedMinute >= 60)
				selectedMinute = 0;
		}
		firstPass = 1;
	}
	if (timeSelection == HOUR){
		if (manipulateTime == INCREMENT){
			if ((selectedHour + 1) >= 24)
				selectedHour = 0;
			else
				selectedHour += 1;
			manipulateTime = NEUTRAL;
		}
		if (manipulateTime == DECREMENT){
			if ((selectedHour - 1) < 0)
				selectedHour = 23;
			else
				selectedHour -= 1;
			manipulateTime = NEUTRAL;		
		}
	}		
	else if (timeSelection == MINUTE){
		if (manipulateTime == INCREMENT){
			if ((selectedMinute + 15) >= 60)
				selectedMinute = 0;
			else
				selectedMinute += 15;
			manipulateTime = NEUTRAL;
		}
		else if (manipulateTime == DECREMENT){
			if ((selectedMinute - 15) < 0)
				selectedMinute = 45;
			else
				selectedMinute -= 15;
			manipulateTime = NEUTRAL;		
		}
	}	
	
	printTimeSelectionOnLine(3,selectedHour,selectedMinute,0,timeSelection);	
	
	printStringOnLine(4,"                  ", 0,NOBOXMENU);	
	printStringOnLine(5,"                  ", 1,NOBOXMENU);	
	printStringOnLine(6,"Choose Start Time ", 1,NOBOXMENU);	
	printStringOnLine(7,"                  ", 1,NOBOXMENU);	
	
}

void drawNoRecords(void){
	printTime();
	printStringOnLine(1,"  Modify Records  ", 1,NONE);
	printStringOnLine(2,"                  ", 0,NOBOXMENU);	
	printStringOnLine(3,"     Sorry        ", 0,NOBOXMENU);
	printStringOnLine(4,"  There are no    ", 0,NOBOXMENU);
	printStringOnLine(5,"records to modify ", 0,NOBOXMENU);
	printStringOnLine(6,"                  ", 0,NOBOXMENU);
	printStringOnLine(7,"                  ", 0,NOBOXMENU);
}

void drawMoodOrActivity(void){

	currentMenuLength = sizeof(moodOrActivityArray)/sizeof(moodOrActivityArray[0]);
	uint8_t index = 0;
	uint8_t menuPage = 0;
	uint8_t totalPages = 0;
	
	char moodOrActString[18];
	memset(moodOrActString,0,sizeof(moodOrActString));
	
	printTime();
	printStringOnLine(1," Mood or Activity ", 1,NONE);
	
	for (uint8_t i = 0; i < cellsPerFrame; i++){
		totalPages = ((currentMenuLength-1)/cellsPerFrame);
		menuPage = (selectionInMenu/cellsPerFrame);	
		index = (menuPage*cellsPerFrame)+i;
		
		if (index < currentMenuLength){
 			snprintf(moodOrActString, sizeof(moodOrActString), "%s%s%s", " ", moodOrActivityArray[index], "                  ");
			moodOrActString[17] = ' ';
		}
		
		drawMenuBands(menuPage,totalPages,TOPBAND);			
				
		//Draw Menu Contents
		if (i == selectionInFrame)
			printStringOnLine(i+yOffset,moodOrActString, 1,NOBOXMENU);
		else if (index < currentMenuLength)
			printStringOnLine(i+yOffset,moodOrActString, 0,NOBOXMENU);
		else
			printStringOnLine(i+yOffset,"                  ", 0,NOBOXMENU);
			
		drawMenuBands(menuPage,totalPages,BOTTOMBAND);	
			
	}	
}

//void drawChooseEndTime(void){}

void drawSettings(void){
	currentMenuLength = sizeof(settingsArray)/sizeof(settingsArray[0]);
	uint8_t index = 0;
	uint8_t menuPage = 0;
	uint8_t totalPages = 0;
	
	char settingsString[18];
	memset(settingsString,0,sizeof(settingsString));
	
	printTime();
	printStringOnLine(1,"    Main Menu     ", 1,NONE);
	
	for (uint8_t i = 0; i < cellsPerFrame; i++){
		totalPages = ((currentMenuLength-1)/cellsPerFrame);
		menuPage = (selectionInMenu/cellsPerFrame);	
		index = (menuPage*cellsPerFrame)+i;
		
		if (index < currentMenuLength){
 			snprintf(settingsString, sizeof(settingsString), "%s%s%s", " ", settingsArray[index], "                  ");
			settingsString[17] = ' ';
		}
		
		drawMenuBands(menuPage,totalPages,TOPBAND);
				
		//Draw Menu Contents
		if (i == selectionInFrame)
			printStringOnLine(i+yOffset,settingsString, 1,NOBOXMENU);
		else if (index < currentMenuLength)
			printStringOnLine(i+yOffset,settingsString, 0,NOBOXMENU);
		else
			printStringOnLine(i+yOffset,"                  ", 0,NOBOXMENU);
			
		drawMenuBands(menuPage,totalPages,BOTTOMBAND);	
			
	}		
}

void drawChangeTime(void){
	selectedYear = getTimeYear();
	selectedMonth = getTimeMonth();
	selectedDay = getTimeDate();
	selectedDOW = getTimeDay();	
	selectedHour = getTimeHour();
	selectedMinute = getTimeMinute();
	selectedSecond = getTimeSecond();
	
	if(firstPass == 0){
		timeSelection = 0;
		firstPass = 1;
	}		
	
	if (timeSelection == HOUR){
		if (manipulateTime == INCREMENT){
			if ((selectedHour + 1) >= 24)
				selectedHour = 0;			
			else
				selectedHour += 1;
			setTimeHour(selectedHour);			
			manipulateTime = NEUTRAL;
		}
		else if (manipulateTime == DECREMENT){
			if ((selectedHour - 1) < 0)
				selectedHour = 23;
			else
				selectedHour -= 1;
			setTimeHour(selectedHour);
			manipulateTime = NEUTRAL;
		}
	}
	else if (timeSelection == MINUTE){
		if (manipulateTime == INCREMENT){
			if ((selectedMinute + 1) >= 60)
				selectedMinute = 0;
			else
				selectedMinute += 1;
			setTimeMinute(selectedMinute);
			manipulateTime = NEUTRAL;
		}
		else if (manipulateTime == DECREMENT){
			if ((selectedMinute - 1) < 0)
				selectedMinute = 59;
			else
				selectedMinute -= 1;
			setTimeMinute(selectedMinute);
			manipulateTime = NEUTRAL;
		}
	}		
	else if (timeSelection == DAY){
		if (manipulateTime == INCREMENT){
			if ((selectedDay + 1) >= monthDays[selectedMonth-1]+1)
				selectedDay = 1;
			else
				selectedDay += 1;
			setTimeDay(selectedDay);
			manipulateTime = NEUTRAL;
		}
		else if (manipulateTime == DECREMENT){
			if ((selectedDay - 1) < 1)
				selectedDay = monthDays[selectedMonth-1];
			else
				selectedDay -= 1;
			setTimeDay(selectedDay);
			manipulateTime = NEUTRAL;
		}
	}		
	else if (timeSelection == MONTH){
		if (manipulateTime == INCREMENT){
			if ((selectedMonth + 1) >= 13)
				selectedMonth = 1;
			else
				selectedMonth += 1;
			setTimeMonth(selectedMonth);
			manipulateTime = NEUTRAL;
		}
		else if (manipulateTime == DECREMENT){
			if ((selectedMonth - 1) < 1)
				selectedMonth = 12;
			else
				selectedMonth -= 1;
			setTimeMonth(selectedMonth);
			manipulateTime = NEUTRAL;
		}
		//selectedDay = getTimeDate();
		if(monthDays[selectedMonth-1] < selectedDay){
			selectedDay = monthDays[selectedMonth-1];
			setTimeDay(selectedDay);
		}			
	}		
	else if (timeSelection == YEAR){
		if (manipulateTime == INCREMENT){
			if ((selectedYear + 1) >= 100)
				selectedYear = 0;
			else
				selectedYear += 1;
			setTimeYear(selectedYear);
			manipulateTime = NEUTRAL;
		}
		else if (manipulateTime == DECREMENT){
			if ((selectedYear - 1) < 0)
				selectedYear = 99;
			else
				selectedYear -= 1;
			manipulateTime = NEUTRAL;
			setTimeYear(selectedYear);
		}
	}
	
	printTime();
	printStringOnLine(1,"  Time Settings   ", 1,NOBOXMENU);
	printStringOnLine(2,"                  ", 0,NOBOXMENU);		
	printTimeSelectionOnLine(3,selectedHour,selectedMinute,0,timeSelection);
	printDateSelectionOnLine(4,selectedDay,selectedMonth,selectedYear,0,timeSelection);
	printStringOnLine(5,"                  ", 1,NOBOXMENU);
	printStringOnLine(6,"  Set the Time    ", 1,NOBOXMENU);
	printStringOnLine(7,"                  ", 1,NOBOXMENU);		
}

void drawInvertColourScheme(void){
	
	uint8_t invSet = getInvertColourSetting();
	printTime();
	printStringOnLine(1,"     Timeline     ", 1,NOBOXMENU);
	printStringOnLine(2,"                  ", 0,NOBOXMENU);
	if(invSet != 0)
		printStringOnLine(3,"   Invert: On     ", 0,NOBOXMENU);
	else
		printStringOnLine(3,"   Invert: Off    ", 0,NOBOXMENU);		
	printStringOnLine(4,"                  ", 0,NOBOXMENU);
	printStringOnLine(5,"                  ", 0,NOBOXMENU);
	printStringOnLine(6,"                  ", 0,NOBOXMENU);
	printStringOnLine(7,"                  ", 0,NOBOXMENU);	
}

void drawResetRecords(uint8_t stage){
	if(stage == 0){
		printTime();
		printStringOnLine(1,"  Reset Records   ", 1,NONE);
		printStringOnLine(2,"                  ", 0,NONE);
		printStringOnLine(3,"  Are you Sure?   ", 0,NONE);
		printStringOnLine(4,"                  ", 0,NONE);
		printStringOnLine(5," This will erase  ", 0,NONE);
		printStringOnLine(6,"  all records on  ", 0,NONE);
		printStringOnLine(7,"  the ClickBrick! ", 0,NONE);
	}
	else if(stage == 1){
		printTime();
		printStringOnLine(1,"  Reset Records   ", 1,NONE);
		printStringOnLine(2,"                  ", 0,NONE);
		printStringOnLine(3,"  Are you Really  ", 0,NONE);
		printStringOnLine(4,"      Sure?       ", 0,NONE);
		printStringOnLine(6,"                  ", 0,NONE);
		printStringOnLine(6," You can't be too ", 0,NONE);
		printStringOnLine(7,"      careful     ", 0,NONE);	
	}		
}

void drawUpload(void){
	printTime();
	printStringOnLine(1,"      Upload      ", 1,NONE);
	printStringOnLine(2," Connect the      ", 0,NONE);	
	printStringOnLine(3," ClickBrick to PC ", 0,NONE);	
	printStringOnLine(4," Go to ClickBrick ", 0,NONE);	
	printStringOnLine(5," website and      ", 0,NONE);	
	printStringOnLine(6," follow on screen ", 0,NONE);	
	printStringOnLine(7," instructions     ", 0,NONE);
}


void updateUI(void){
    if (currentMenuID == 0 || currentMenuID == 1)
		drawSleep();		
		
	else if (currentMenuID == 10){
		cellsPerFrame = 4;
		drawMainMenu();
	}		
	
	else if (currentMenuID == 11){
		cellsPerFrame = 4;
		drawInavlidResearchPeriod();
	}
		
    else if (currentMenuID == 20 || currentMenuID == 44 || currentMenuID == 54){
		cellsPerFrame = 4;
		drawChooseActivity();
	}		
		
	else if (currentMenuID == 21 || currentMenuID == 45 || currentMenuID == 56){
		cellsPerFrame = 4;
		drawChooseMood();
	}		
		
	else if (currentMenuID == 22 || currentMenuID == 46 || currentMenuID == 55 || currentMenuID == 57 || currentMenuID == 61){
		cellsPerFrame = 4;
		if(currentMenuID == 22)
			drawLogConfirm(LOG);
		else if(currentMenuID == 46)
			drawLogConfirm(ADD);
		else if(currentMenuID == 55 || currentMenuID == 57)
			drawLogConfirm(EDIT);
		else if(currentMenuID == 61)
			drawLogConfirm(REMOVE);
	}		
		
    else if (currentMenuID == 30){
		cellsPerFrame = 2;
		drawTimeline();
	}		
		
    else if (currentMenuID == 40){
		cellsPerFrame = 4;
		drawModifyMenu();
	}
    else if (currentMenuID == 41 || currentMenuID == 50 ||  currentMenuID == 60){
		cellsPerFrame = 2;
		if(currentMenuID == 41)
			drawModifyTimeslot(ADD);
		else if(currentMenuID == 50)
			drawModifyTimeslot(EDIT);
		else if(currentMenuID == 60)
			drawModifyTimeslot(REMOVE);
	}		
		
	else if (currentMenuID == 42 || currentMenuID == 52){
		drawChooseStartTime();
		manipulateTime = NEUTRAL;
	}	

	else if (currentMenuID == 43){
		drawNoRecords();
	}	
		
	else if (currentMenuID == 51)
		drawMoodOrActivity();
	
    else if (currentMenuID == 70)
		drawSettings();
		
    else if (currentMenuID == 71){
		drawChangeTime();
		manipulateTime = NEUTRAL;
	}		
		
    else if (currentMenuID == 72)
		drawInvertColourScheme();

    else if (currentMenuID == 73){
		drawResetRecords(0);
	}

    else if (currentMenuID == 74){
	    drawResetRecords(1);
    }		
		
	else if (currentMenuID == 80)
		drawUpload();
		
}	

uint8_t getCurrentMenuID(void){
	return currentMenuID;
}

void setMenuID(uint8_t id){
	currentMenuID = id;
}


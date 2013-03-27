/*
 * MenuUI.h
 *
 * Created: 1/12/2013 11:05:08 AM
 *  Author: Andrew Evans
 */ 


#ifndef MENUUI_H
#define MENUUI_H

extern uint8_t researchPeriod;
extern uint8_t modifiedTimeFlag;

void initMenuUI(void);
void incrementMenuCursor(void);
void decrementMenuCursor(void);
void incrementMenuID(void);
void decrementMenuID(void);
void setMenuID(uint8_t id);

void drawMenuBands(uint8_t menuPage, uint8_t totalPages, uint8_t position);

void printTime(void);
void drawSleep(void);
void drawMainMenu(void);
void drawInavlidResearchPeriod(void);
void drawChooseActivity(void);
void drawChooseMood(void);
void drawLogConfirm(uint8_t screenType);
void drawTimeline(void);
void drawModifyMenu(void);
void drawModifyTimeslot(uint8_t screenType);
void drawChooseStartTime(void);
void drawChooseEndTime(void);
void drawSettings(void);
void drawChangeTime(void);
void drawInvertColourScheme(void);
void drawResetRecords(uint8_t);
void drawUpload(void);
void drawMoodOrActivity(void);
void drawNoRecords(void);

void loadActivitiesMenu(void);
void loadMoodsMenu(void);
void loadFacesMenu(void);

void updateUI(void);
uint8_t getCurrentMenuID(void);
	
#endif /* MENUUI_H_ */
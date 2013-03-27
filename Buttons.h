/*
 * Buttons.h
 *
 * Created: 1/12/2013 8:27:44 PM
 *  Author: Andrew Evans
 */ 


#ifndef BUTTONS_H
#define BUTTONS_H

#define UP 3	
#define DOWN 4	
#define BACK 5
#define SELECT 2

void initButtons();
uint8_t checkForButtons();

uint8_t upButton;
uint8_t downButton;
uint8_t backButton;
uint8_t selectButton;
uint8_t nesButtons;

#endif /* BUTTONS_H_ */
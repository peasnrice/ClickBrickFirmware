/*
 * Buttons.c
 *
 * Created: 1/12/2013 8:26:38 PM
 *  Author: Andrew Evans
 */ 

#include <avr/io.h>
#include "Buttons.h"

void initButtons(void){
	//DEVELOPMENT
	
	DDRD &= ~(1<<PD2);	//INT2 Select Button
	DDRD &= ~(1<<PD3);	//INT3 Up Button
	DDRE &= ~(1<<PE4);	//INT4 Down Button
	DDRE &= ~(1<<PE5);	//INT5 back button
	
	
	//DEPLOYMENT
	/*
	DDRD &= ~(1<<PE7);	//INT7 Up Button
	DDRD &= ~(1<<PE6);	//INT6 Back Button
	DDRE &= ~(1<<PE5);	//INT5 Down Button
	DDRE &= ~(1<<PE4);	//INT4 Select button	
	*/
	upButton = downButton = backButton = selectButton = 0;
}

uint8_t checkForButtons(){
	//DEVELOPMENT!
	
	upButton = PIND & (1<<UP);
	downButton = PINE & (1<<DOWN);
	backButton = PINE & (1<<BACK);
	selectButton = PIND & (1<<SELECT);
	
	
	//DEPLOYMENT
	/*
	upButton = PINE & (1<<UP);
	downButton = PINE & (1<<DOWN);
	backButton = PINE & (1<<BACK);
	selectButton = PINE & (1<<SELECT);	
	*/
	if(upButton == 0){
		return 1;
	}
	else if(downButton == 0){
		return 2;
	}
	else if(backButton == 0){
		return 3;
	}
	else if(selectButton == 0){
		return 4;
	}
	else 
		return 0;
}
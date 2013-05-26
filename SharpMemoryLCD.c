/*
* SharpMemoryLCD.c
*
* Created: 1/10/2013 3:35:33 PM
*  Author: Andrew Evans
*/

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "SPI.h"
#include "external.c"


//#include "RockwellCondensedFont.c"
#include "SharpMemoryLCD.h"
#include "ClickBrick.h"

char lineHistory[8][18];

void initLCD(void){
	set_output(DDRB, DISP);
	set_output(DDRH, EXTC);
	set_output(DDRB, EXTM);
	set_output(DDRB, SI);
	set_output(DDRH, SCS);
	set_output(DDRB, SCLK);
	set_output(DDRB, SS);
	
	//Setup Pulsing PWM on EXTC
	//page 187
	//WGM21 = 1, WGM20 = 1: Fast PWM (Top 0xFF, Update OCRx @ Bottom, TOV flag set on MAX)
	//COM2B1 = 1, COM2B0 = 1: Set OC2B on Compare Match, clear OC2B at BOTTOM (inverting mode)
	//OCR2B = 0x7F: 50% duty cycle
	/*
	TCCR2A = (1<<WGM21)|(1<<WGM20)|(1<<COM2B1)|(1<<COM2B0);
	TCCR2B = TCCR2B & 0xFF;
	output_high(PORTH,EXTC);
	OCR2B = 0x7F;
	*/
	//turn on display
	output_high(PORTB, EXTM);
	output_high(PORTB, DISP);
	
	output_high(PORTB, SS);
}

void clearLCD(void){
	output_high(PORTH, SCS); //Set Chip Select HIGH
	
	SPI_TX(0x04);
	SPI_TX(0x00);
	
	output_low(PORTH, SCS); //Set Chip Select HIGH
}

void printTimeLineIsEmpty(void){
	printStringOnLine(3,"Oops, Looks like  ", 0,0);
	printStringOnLine(4,"your timeline     ", 0,0);
	printStringOnLine(5,"is empty          ", 0,0);
}

void printTimeSelectionOnLine(uint8_t line, uint8_t timeSelectionHour, uint8_t timeSelectionMinute, uint8_t invert, uint8_t currentSelection){
char hourString[3] = {"0"};
	char minuteString[3] = {"0"};
	char tempString[3];
	char timeString[18];	
	
	itoa(timeSelectionHour, tempString, 10);
	if (timeSelectionHour < 10)
		strcat(hourString, tempString);
	else{
		strcpy(hourString, tempString);
	}

	itoa(timeSelectionMinute, tempString, 10);
	if (timeSelectionMinute < 10)
		strcat(minuteString, tempString);
	else{
		strcpy(minuteString, tempString);
	}	
	
	
	snprintf(timeString, sizeof(timeString), "%s%s%s%s%s", "      ", hourString, ":", minuteString, "       " ); 	
	timeString[17] = ' ';
	
	//prints time highlighting necessary selection
	if (currentSelection == 0)
		printStringOnLine(line,timeString,0,3);
	else if(currentSelection == 1)
		printStringOnLine(line,timeString,0,4);
	else
		printStringOnLine(line,timeString,0,0);
}	

void printDateSelectionOnLine(uint8_t line, uint8_t dateSelectionDay, uint8_t dateSelectionMonth, uint8_t dateSelectionYear, uint8_t invert, uint8_t currentSelection){
	char dayString[3] = {"0"};
	char monthString[3] = {"0"};
	char yearString[3] = {"0"};
	char tempString[3];
	char dateString[18];

	itoa(dateSelectionDay, tempString, 10);
	if (dateSelectionDay < 10)
		strcat(dayString, tempString);
	else{
		strcpy(dayString, tempString);
	}

	itoa(dateSelectionMonth, tempString, 10);
	if (dateSelectionMonth < 10)
		strcat(monthString, tempString);
	else{
		strcpy(monthString, tempString);
	}

	itoa(dateSelectionYear, tempString, 10);
	if (dateSelectionYear < 10)
		strcat(yearString, tempString);
	else{
		strcpy(yearString, tempString);
	}
	memset(dateString,0,18);
	snprintf(dateString, sizeof(dateString), "%s%s%s%s%s%s%s", "     ", dayString, "/", monthString, "/", yearString, "     " );
	dateString[17] = ' ';

	//prints time highlighting necessary selection
	if (currentSelection == 2)
		printStringOnLine(line,dateString,0,5);
	else if(currentSelection == 3)
		printStringOnLine(line,dateString,0,6);
	else if(currentSelection == 4)
		printStringOnLine(line,dateString,0,7);
	else
		printStringOnLine(line,dateString,0,0);	
}

void printStringOnLine(uint8_t line, char lineText[], uint8_t invert, uint8_t option){
	
	if(invertColours != 0)
		invert = !invert;
	
	uint8_t charLine = 0;
	line *= 21;
	line++;
	
	output_high(PORTH, SCS); //Set Chip Select HIGH
	//write command
	SPI_TX(0x01);
	//Line Address
	for (unsigned char l = line; l < line + 21; l++){
		if(l != line)
			SPI_TX(0x00);
		SPI_TX(l);	
		//LineBits
		
		//NONE
		if (option == 0){
			for(uint8_t i = 0; i < 18; i++){
				if (invert == 0)
					SPI_TX(pgm_read_byte(font8x21+(((unsigned char)lineText[i]-32)*21+charLine)));
					//SPI_TX(font8x16[(((unsigned char)lineText[i]-32)*16+charLine)]);
				else
					SPI_TX(~(pgm_read_byte(font8x21+(((unsigned char)lineText[i]-32)*21+charLine))));
					//SPI_TX(~(font8x16[(((unsigned char)lineText[i]-32)*16+charLine)]));
			}	
		}		
		
		//BORDERED MENU	
		else if (option == 1){
			for(uint8_t i = 0; i < 18; i++){
				if (invert == 0)
					SPI_TX(pgm_read_byte(font8x21+(((unsigned char)lineText[i]-32)*21+charLine)));
				else{
					if(i == 0)
						SPI_TX(pgm_read_byte(font8x21+(((unsigned char)lineText[i]-32)*21+charLine)));
					else if (i == 17)
						SPI_TX(pgm_read_byte(font8x21+(((unsigned char)lineText[i]-32)*21+charLine)));
					else
						SPI_TX(~(pgm_read_byte(font8x21+(((unsigned char)lineText[i]-32)*21+charLine))));
				}
			}	
		}	
		
		//NO BORDER
		else if (option == 2){
			for(uint8_t i = 0; i < 18; i++){
				if (invert == 0)
					SPI_TX(pgm_read_byte(font8x21+(((unsigned char)lineText[i]-32)*21+charLine)));
				else{
					SPI_TX(~(pgm_read_byte(font8x21+(((unsigned char)lineText[i]-32)*21+charLine))));
				}
			}	
		}		

		//HIGHLIGHT HOUR
		else if (option == 3){
			for(uint8_t i = 0; i < 18; i++){
				if (invert == 0){
					if(i == 6 || i == 7)
						SPI_TX(~pgm_read_byte(font8x21+(((unsigned char)lineText[i]-32)*21+charLine)));
					else
						SPI_TX(pgm_read_byte(font8x21+(((unsigned char)lineText[i]-32)*21+charLine)));
				}						
				else{
					if(i == 6 || i == 7)
						SPI_TX(pgm_read_byte(font8x21+(((unsigned char)lineText[i]-32)*21+charLine)));
					else
						SPI_TX(~pgm_read_byte(font8x21+(((unsigned char)lineText[i]-32)*21+charLine)));
				}
				
			}	
		}
		
		//HIGHLIGHT MINUTE
		else if (option == 4){
			for(uint8_t i = 0; i < 18; i++){
				if (invert == 0){
					if(i == 9 || i == 10)
						SPI_TX(~pgm_read_byte(font8x21+(((unsigned char)lineText[i]-32)*21+charLine)));
					else
						SPI_TX(pgm_read_byte(font8x21+(((unsigned char)lineText[i]-32)*21+charLine)));
				}						
				else{
					if(i == 9 || i == 10)
						SPI_TX(pgm_read_byte(font8x21+(((unsigned char)lineText[i]-32)*21+charLine)));
					else
						SPI_TX(~pgm_read_byte(font8x21+(((unsigned char)lineText[i]-32)*21+charLine)));
				}
			}	
		}
		//HIGHLIGHT DAY
		else if (option == 5){
			for(uint8_t i = 0; i < 18; i++){
				if (invert == 0){
					if(i == 5 || i == 6)
					SPI_TX(~pgm_read_byte(font8x21+(((unsigned char)lineText[i]-32)*21+charLine)));
					else
					SPI_TX(pgm_read_byte(font8x21+(((unsigned char)lineText[i]-32)*21+charLine)));
				}
				else{
					if(i == 5 || i == 6)
					SPI_TX(pgm_read_byte(font8x21+(((unsigned char)lineText[i]-32)*21+charLine)));
					else
					SPI_TX(~pgm_read_byte(font8x21+(((unsigned char)lineText[i]-32)*21+charLine)));
				}
			}
		}
		//HIGHLIGHT MONTH
		else if (option == 6){
			for(uint8_t i = 0; i < 18; i++){
				if (invert == 0){
					if(i == 8 || i == 9)
					SPI_TX(~pgm_read_byte(font8x21+(((unsigned char)lineText[i]-32)*21+charLine)));
					else
					SPI_TX(pgm_read_byte(font8x21+(((unsigned char)lineText[i]-32)*21+charLine)));
				}
				else{
					if(i == 8 || i == 9)
					SPI_TX(pgm_read_byte(font8x21+(((unsigned char)lineText[i]-32)*21+charLine)));
					else
					SPI_TX(~pgm_read_byte(font8x21+(((unsigned char)lineText[i]-32)*21+charLine)));
				}
			}
		}	
		
		//HIGHLIGHT YEAR
		else if (option == 7){
			for(uint8_t i = 0; i < 18; i++){
				if (invert == 0){
					if(i == 11 || i == 12)
					SPI_TX(~pgm_read_byte(font8x21+(((unsigned char)lineText[i]-32)*21+charLine)));
					else
					SPI_TX(pgm_read_byte(font8x21+(((unsigned char)lineText[i]-32)*21+charLine)));
				}
				else{
					if(i == 11 || i == 12)
					SPI_TX(pgm_read_byte(font8x21+(((unsigned char)lineText[i]-32)*21+charLine)));
					else
					SPI_TX(~pgm_read_byte(font8x21+(((unsigned char)lineText[i]-32)*21+charLine)));
				}
			}
		}	
	charLine++;
	}	
	SPI_TX(0x00);
	SPI_TX(0x00);

	output_low(PORTH, SCS); //Set Chip Select LOW	
}

//prints 16 characters + face of current mood
void printTimelineStringOnLine(uint8_t line, char lineText[], uint8_t invert, uint8_t face){

	if(invertColours != 0)
		invert = !invert;

	uint8_t charLine = 0;
	//line *= 16;
	line *= 21;
	line++;
	
	output_high(PORTH, SCS); //Set Chip Select HIGH
	//write command
	SPI_TX(0x01);
	//Line Address
	//for (unsigned char l = line; l < line + 16; l++){
	for (unsigned char l = line; l < line + 21; l++){
		if(l != line)
		SPI_TX(0x00);
		SPI_TX(l);
		//LineBits
			
		for(uint8_t i = 0; i < 18; i++){
			if (invert == 0)
			SPI_TX(pgm_read_byte(font8x21+(((unsigned char)lineText[i]-32)*21+charLine)));
			//SPI_TX(font8x16[(((unsigned char)lineText[i]-32)*16+charLine)]);
			else
			SPI_TX(~(pgm_read_byte(font8x21+(((unsigned char)lineText[i]-32)*21+charLine))));
			//SPI_TX(~(font8x16[(((unsigned char)lineText[i]-32)*16+charLine)]));
		}
		/*
		if(face == TIRED){
			if (invert == 0){
				SPI_TX(pgm_read_byte(tiredFace+(charLine*2)));
				SPI_TX(pgm_read_byte(tiredFace+(charLine*2)+1));
			}		
			else{
				SPI_TX(~pgm_read_byte(tiredFace+(charLine*2)));
				SPI_TX(~pgm_read_byte(tiredFace+(charLine*2)+1));
			}	
		}			
		else if(face == EXHAUSTED){
			if (invert == 0){
				SPI_TX(pgm_read_byte(exhaustedFace+(charLine*2)));
				SPI_TX(pgm_read_byte(exhaustedFace+(charLine*2)+1));
			}
			else{
				SPI_TX(~pgm_read_byte(exhaustedFace+(charLine*2)));
				SPI_TX(~pgm_read_byte(exhaustedFace+(charLine*2)+1));
			}
		}		
		else if(face == CONFUSED){
			if (invert == 0){
				SPI_TX(pgm_read_byte(confusedFace+(charLine*2)));
				SPI_TX(pgm_read_byte(confusedFace+(charLine*2)+1));
			}
			else{
				SPI_TX(~pgm_read_byte(confusedFace+(charLine*2)));
				SPI_TX(~pgm_read_byte(confusedFace+(charLine*2)+1));
			}
		}
		else if(face == MISERABLE){
			if (invert == 0){
				SPI_TX(pgm_read_byte(miserableFace+(charLine*2)));
				SPI_TX(pgm_read_byte(miserableFace+(charLine*2)+1));
			}
			else{
				SPI_TX(~pgm_read_byte(miserableFace+(charLine*2)));
				SPI_TX(~pgm_read_byte(miserableFace+(charLine*2)+1));
			}
		}
		else if(face == HAPPY){
			if (invert == 0){
				SPI_TX(pgm_read_byte(happyFace+(charLine*2)));
				SPI_TX(pgm_read_byte(happyFace+(charLine*2)+1));
			}
			else{
				SPI_TX(~pgm_read_byte(happyFace+(charLine*2)));
				SPI_TX(~pgm_read_byte(happyFace+(charLine*2)+1));
			}
		}
		else if(face == SAD){
			if (invert == 0){
				SPI_TX(pgm_read_byte(sadFace+(charLine*2)));
				SPI_TX(pgm_read_byte(sadFace+(charLine*2)+1));
			}
			else{
				SPI_TX(~pgm_read_byte(sadFace+(charLine*2)));
				SPI_TX(~pgm_read_byte(sadFace+(charLine*2)+1));
			}
		}
		else if(face == MEH){
			if (invert == 0){
				SPI_TX(pgm_read_byte(mehFace+(charLine*2)));
				SPI_TX(pgm_read_byte(mehFace+(charLine*2)+1));
			}
			else{
				SPI_TX(~pgm_read_byte(mehFace+(charLine*2)));
				SPI_TX(~pgm_read_byte(mehFace+(charLine*2)+1));
			}
		}
		else if(face == ANGRY){
			if (invert == 0){
				SPI_TX(pgm_read_byte(angryFace+(charLine*2)));
				SPI_TX(pgm_read_byte(angryFace+(charLine*2)+1));
			}
			else{
				SPI_TX(~pgm_read_byte(angryFace+(charLine*2)));
				SPI_TX(~pgm_read_byte(angryFace+(charLine*2)+1));
			}
		}
		*/
		charLine++;
	}			

	SPI_TX(0x00);
	SPI_TX(0x00);

	output_low(PORTH, SCS); //Set Chip Select LOW	
}	

void printBigTime(uint8_t line, char lineText[], uint8_t invert){

	if(invertColours != 0)
		invert = !invert;	
	
	uint8_t charLine = 0;
	line *= 21;
	line++;
	
	output_high(PORTH, SCS); //Set Chip Select HIGH
	//write command
	SPI_TX(0x01);
	//Line Address
	//for (unsigned char l = line; l < line + 16; l++){
	for (unsigned char l = line; l < line + 63; l++){
		if(l != line)
			SPI_TX(0x00);
		SPI_TX(l);	
		//LineBits
		for(uint8_t i = 0; i < 6; i++){
			if (lineText[i] == ' '){
				if (invert == 0){
					SPI_TX(pgm_read_byte(white_space+(charLine*3)));
					SPI_TX(pgm_read_byte(white_space+(charLine*3)+1));
					SPI_TX(pgm_read_byte(white_space+(charLine*3)+2));
				}					
				else{
					SPI_TX(~pgm_read_byte(white_space+(charLine*3)));
					SPI_TX(~pgm_read_byte(white_space+(charLine*3)+1));
					SPI_TX(~pgm_read_byte(white_space+(charLine*3)+2));
				}	
			}
			else if (lineText[i] == '0'){
				if (invert == 0){
					SPI_TX(pgm_read_byte(number_0+(charLine*3)));
					SPI_TX(pgm_read_byte(number_0+(charLine*3)+1));
					SPI_TX(pgm_read_byte(number_0+(charLine*3)+2));
				}					
				else{
					SPI_TX(~pgm_read_byte(number_0+(charLine*3)));
					SPI_TX(~pgm_read_byte(number_0+(charLine*3)+1));
					SPI_TX(~pgm_read_byte(number_0+(charLine*3)+2));
				}	
			}
			else if (lineText[i] == '1'){
				if (invert == 0){
					SPI_TX(pgm_read_byte(number_1+(charLine*3)));
					SPI_TX(pgm_read_byte(number_1+(charLine*3)+1));
					SPI_TX(pgm_read_byte(number_1+(charLine*3)+2));
				}					
				else{
					SPI_TX(~pgm_read_byte(number_1+(charLine*3)));
					SPI_TX(~pgm_read_byte(number_1+(charLine*3)+1));
					SPI_TX(~pgm_read_byte(number_1+(charLine*3)+2));
				}	
			}
			else if (lineText[i] == '2'){
				if (invert == 0){
					SPI_TX(pgm_read_byte(number_2+(charLine*3)));
					SPI_TX(pgm_read_byte(number_2+(charLine*3)+1));
					SPI_TX(pgm_read_byte(number_2+(charLine*3)+2));
				}					
				else{
					SPI_TX(~pgm_read_byte(number_2+(charLine*3)));
					SPI_TX(~pgm_read_byte(number_2+(charLine*3)+1));
					SPI_TX(~pgm_read_byte(number_2+(charLine*3)+2));
				}	
			}	
			else if (lineText[i] == '3'){
				if (invert == 0){
					SPI_TX(pgm_read_byte(number_3+(charLine*3)));
					SPI_TX(pgm_read_byte(number_3+(charLine*3)+1));
					SPI_TX(pgm_read_byte(number_3+(charLine*3)+2));
				}					
				else{
					SPI_TX(~pgm_read_byte(number_3+(charLine*3)));
					SPI_TX(~pgm_read_byte(number_3+(charLine*3)+1));
					SPI_TX(~pgm_read_byte(number_3+(charLine*3)+2));
				}	
			}
			else if (lineText[i] == '4'){
				if (invert == 0){
					SPI_TX(pgm_read_byte(number_4+(charLine*3)));
					SPI_TX(pgm_read_byte(number_4+(charLine*3)+1));
					SPI_TX(pgm_read_byte(number_4+(charLine*3)+2));
				}					
				else{
					SPI_TX(~pgm_read_byte(number_4+(charLine*3)));
					SPI_TX(~pgm_read_byte(number_4+(charLine*3)+1));
					SPI_TX(~pgm_read_byte(number_4+(charLine*3)+2));
				}	
			}
			else if (lineText[i] == '5'){
				if (invert == 0){
					SPI_TX(pgm_read_byte(number_5+(charLine*3)));
					SPI_TX(pgm_read_byte(number_5+(charLine*3)+1));
					SPI_TX(pgm_read_byte(number_5+(charLine*3)+2));
				}					
				else{
					SPI_TX(~pgm_read_byte(number_5+(charLine*3)));
					SPI_TX(~pgm_read_byte(number_5+(charLine*3)+1));
					SPI_TX(~pgm_read_byte(number_5+(charLine*3)+2));
				}	
			}	
			else if (lineText[i] == '6'){
				if (invert == 0){
					SPI_TX(pgm_read_byte(number_6+(charLine*3)));
					SPI_TX(pgm_read_byte(number_6+(charLine*3)+1));
					SPI_TX(pgm_read_byte(number_6+(charLine*3)+2));
				}					
				else{
					SPI_TX(~pgm_read_byte(number_6+(charLine*3)));
					SPI_TX(~pgm_read_byte(number_6+(charLine*3)+1));
					SPI_TX(~pgm_read_byte(number_6+(charLine*3)+2));
				}	
			}
			else if (lineText[i] == '7'){
				if (invert == 0){
					SPI_TX(pgm_read_byte(number_7+(charLine*3)));
					SPI_TX(pgm_read_byte(number_7+(charLine*3)+1));
					SPI_TX(pgm_read_byte(number_7+(charLine*3)+2));
				}					
				else{
					SPI_TX(~pgm_read_byte(number_7+(charLine*3)));
					SPI_TX(~pgm_read_byte(number_7+(charLine*3)+1));
					SPI_TX(~pgm_read_byte(number_7+(charLine*3)+2));
				}	
			}
			else if (lineText[i] == '8'){
				if (invert == 0){
					SPI_TX(pgm_read_byte(number_8+(charLine*3)));
					SPI_TX(pgm_read_byte(number_8+(charLine*3)+1));
					SPI_TX(pgm_read_byte(number_8+(charLine*3)+2));
				}					
				else{
					SPI_TX(~pgm_read_byte(number_8+(charLine*3)));
					SPI_TX(~pgm_read_byte(number_8+(charLine*3)+1));
					SPI_TX(~pgm_read_byte(number_8+(charLine*3)+2));
				}	
			}	
			else if (lineText[i] == '9'){
				if (invert == 0){
					SPI_TX(pgm_read_byte(number_9+(charLine*3)));
					SPI_TX(pgm_read_byte(number_9+(charLine*3)+1));
					SPI_TX(pgm_read_byte(number_9+(charLine*3)+2));
				}					
				else{
					SPI_TX(~pgm_read_byte(number_9+(charLine*3)));
					SPI_TX(~pgm_read_byte(number_9+(charLine*3)+1));
					SPI_TX(~pgm_read_byte(number_9+(charLine*3)+2));
				}	
			}	
			else if (lineText[i] == ':'){
				if (invert == 0){
					SPI_TX(pgm_read_byte(colon+(charLine*3)));
					SPI_TX(pgm_read_byte(colon+(charLine*3)+1));
					SPI_TX(pgm_read_byte(colon+(charLine*3)+2));
				}					
				else{
					SPI_TX(~pgm_read_byte(colon+(charLine*3)));
					SPI_TX(~pgm_read_byte(colon+(charLine*3)+1));
					SPI_TX(~pgm_read_byte(colon+(charLine*3)+2));
				}	
			}							
		}	
	
	charLine++;
	}	
	SPI_TX(0x00);
	SPI_TX(0x00);

	output_low(PORTH, SCS); //Set Chip Select LOW	
}

void printImage(){
	uint8_t invert = 0;
	
	if(invertColours != 0)
		invert = !invert;

	uint8_t imageLine = 0;
	uint8_t line = 1;
	
	output_high(PORTH, SCS); //Set Chip Select HIGH
	//write command
	SPI_TX(0x01);
	//Line Address
	//for (unsigned char l = line; l < line + 16; l++){
	for (unsigned char l = line; l < line + 167; l++){
		if(l != line)
			SPI_TX(0x00);
		SPI_TX(l);
		//LineBits
		for(uint8_t i = 0; i < 18; i++){
			if (invert == 0){
				SPI_TX(pgm_read_byte(ClockFaceImage+(i+imageLine*18)));					
			}
			else{
				SPI_TX(~pgm_read_byte(ClockFaceImage+(i+imageLine*18)));
			}				
		}
		imageLine++;
	}
	SPI_TX(0x00);
	SPI_TX(0x00);

	output_low(PORTH, SCS); //Set Chip Select LOW	
}

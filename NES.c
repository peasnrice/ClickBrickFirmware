/*
* NES.c
*
* Created: 2/22/2013 8:26:30 PM
*  Author: Andrew Evans
*/

#include <avr/io.h>
#include <util/delay.h>
#include "NES.h"
#include "usart.h"

void initNES(void){
	DDRF |= (1<<CLOCK);
	DDRF |= (1<<LATCH);
	DDRF &= ~(1<<DATA);
}

uint8_t NESRead(void){
	for(uint8_t i = 0; i < 5; i++){
		PORTF |= (1<<LATCH);
		_delay_ms(5);
		PORTF &= ~(1<<LATCH);
		
		for(uint8_t i = 0; i < 8; i++){
			buttonsPressed |= (PINF & (1<<DATA)) & i;
			if (PINF & (1<<DATA))
			buttonsPressed |= (1<<i);
			else
			buttonsPressed &= ~(1<<i);
			PORTF |= (1<<CLOCK);
			PORTF &= ~(1<<CLOCK);
		}
	}		
	return buttonsPressed;
}

uint8_t checkForNESButtons(void){
	nesButtons = NESRead();

	if(!((1 << NORTH) & nesButtons)){
		//USART_Sendbyte(1);
		return 1;
	}
	else if(!((1 << SOUTH) & nesButtons)){
		//USART_Sendbyte(2);
		return 2;
	}
	else if(!((1 << EAST) & nesButtons) || !((1 << START) & nesButtons) || !((1 << A) & nesButtons)){
		//USART_Sendbyte(3);
		return 3;
	}
	else if(!((1 << WEST) & nesButtons) || !((1 << SELECT) & nesButtons) || !((1 << B) & nesButtons)){
		//USART_Sendbyte(4);
		return 4;
	}
	else
		return 0;
}
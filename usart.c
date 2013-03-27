/*
 * usart.cpp
 *
 * Created: 07/12/2011 15:17:35
 *  Author: Boomber
 */ 
#include "usart.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>			// Conversions
#include "external.c"

char* stringBuffer;

extern volatile uint8_t breakFromUSARTFlag;

void USART_Init( unsigned int ubrr)
{	
	stringBuffer = calloc(sizeof(char), 256);
	
	/*Set baud rate */
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;
	//Enable receiver and transmitter */
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	/* Set frame format: 8data, 2stop bit */
	UCSR0C = (1<<USBS0)|(3<<UCSZ00);
}

void enableUSART(void){
	UCSR0B |= (1<<RXEN0);
	UCSR0B |= (1<<TXEN0);
}

void disableUSART(void){
	UCSR0B &= ~(1<<RXEN0);
	UCSR0B &= ~(1<<TXEN0);	
}

void USART_Sendbyte(unsigned char data )
{
	while (( UCSR0A & (1 << UDRE0 )) == 0) {}; // Do nothing until UDR is ready for more data to be written to it

	UDR0 = data ; // Echo back the received byte back to the computer
}

void USART_Send_string(const char *str)
{
	  while (*str) { USART_Sendbyte(*str++); }	
}

void USART_Send_string_CRLF(const char *str)
{
	while (*str) { USART_Sendbyte(*str++); }
	USART_Sendbyte('\r');
	USART_Sendbyte('\n');
}

void USART_Send_int(unsigned int d )
{
	char str[10];
	sprintf(str,"%u",d);
	USART_Send_string(str);
}


char USART_Receive( void )
{
	while (( UCSR0A & (1 << RXC0 )) == 0) {}; // Do nothing until data have been received and is ready to be read from UDR
	return UDR0 ; // Fetch the received byte value into the variable " ByteReceived "
}

char* USART_Receive_CRLF( void )
{
	disableUSART();
	enableUSART();
	char* s = stringBuffer;
	for(;;) {
		while (( UCSR0A & (1 << RXC0 )) == 0 && breakFromUSARTFlag == 0) {}; // Do nothing until data have been received and is ready to be read from UDR
		if(breakFromUSARTFlag != 0)
			return 0xff;
		*s = UDR0;
		if(*s == '\r'){
			while (( UCSR0A & (1 << RXC0 )) == 0 && breakFromUSARTFlag == 0) {}; // Do nothing until data have been received and is ready to be read from UDR
			if(breakFromUSARTFlag != 0)
				return 0xff;
			s++;
			*s = UDR0;
			if(*s == '\n'){
				s++;
				*s = 0;
				break;			
			}
		}							
		s++;				
	}
	
	// Turn the \r into a 0x00 so that the string doesn't include the \r\n
	*(s-2) = 0;
	return stringBuffer; // Fetch the received byte value into the variable " ByteReceived "
}

void USARTECHO(void)
{
	char ReceivedByte;
	{
	while (( UCSR0A & (1 << RXC0 )) == 0 && breakFromUSARTFlag == 0) {}; // Do nothing until data have been received and is ready to be read from UDR
	ReceivedByte = UDR0 ; // Fetch the received byte value into the variable " ByteReceived "
	
	
	while (( UCSR0A & (1 << UDRE0 )) == 0 && breakFromUSARTFlag == 0) {}; // Do nothing until UDR is ready for more data to be written to it
	UDR0 = ReceivedByte ; // Echo back the received byte back to the computer
	}
}
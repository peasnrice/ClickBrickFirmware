/*
 * usart.h
 *
 * Created: 07/12/2011 15:16:27
 *  Author: Boomber
 */ 


#ifndef USART_H
#define USART_H


#define FOSC 16000000UL // Clock Speed
//#define FOSC 8000000UL
#define USART_BAUDRATE 9600
//#define BAUD_PRESCALE 16
#define MYUBRR (((FOSC / (USART_BAUDRATE * 16UL))) - 1 )
//#define MYUBRR (((((FOSC * 10) / (8L * USART_BAUDRATE)) + 5) / 10) - 1)

void USART_Init( unsigned int ubrr);
void enableUSART(void);
void disableUSART(void);

void USART_Sendbyte(unsigned char data );
void USART_Send_string(const char *str);
void USART_Send_int(unsigned int d);

char USART_Receive(void);
char* USART_Receive_CRLF(void);
void USART_Send_string_CRLF(const char *str);

void USARTECHO(void);

//uint8_t makeHandshake(void);

#endif /* USART_H_ */
#ifndef __uart_h__
#define __uart_h__

#include "intrins.h"
#include "mcu_definitions.h"


typedef unsigned char BYTE;
typedef unsigned int WORD;

#define FOSC 11059200L      //System frequency
#define BAUD 9600           //UART baudrate

/*Define UART parity mode*/
#define NONE_PARITY     0   //None parity
#define ODD_PARITY      1   //Odd parity
#define EVEN_PARITY     2   //Even parity
#define MARK_PARITY     3   //Mark parity
#define SPACE_PARITY    4   //Space parity

#define PARITYBIT NONE_PARITY   //Testing even parity


void startUART();
void Uart_Isr();
void SendData(BYTE dat);
void SendString(char *s);


#endif

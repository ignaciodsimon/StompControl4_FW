#include "uart.h"
#include "mcu_definitions.h"

void mainUI(void);

void main(void)
{
		// This is needed for the "tuner" relay to work
		P4SW = 0x70;
	
		mainUI();	
}

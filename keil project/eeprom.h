#ifndef __eeprom_h__
#define __eeprom_h__

/*------------------------------------------------------------------*/
/* --- STC MCU Limited ---------------------------------------------*/
/* --- STC10/11xx Series MCU ISP/IAP/EEPROM Demo -------------------*/
/* --- Mobile: (86)13922805190 -------------------------------------*/
/* --- Fax: 86-0513-55012956,55012947,55012969 ---------------------*/
/* --- Tel: 86-0513-55012928,55012929,55012966----------------------*/
/* --- Web: www.STCMCU.com -----------------------------------------*/
/* --- Web: www.GXWMCU.com -----------------------------------------*/
/* If you want to use the program or the program referenced in the  */
/* article, please specify in which data and procedures from STC    */
/*------------------------------------------------------------------*/

#include "reg51.h"
#include "intrins.h"

//typedef unsigned char BYTE;
//typedef unsigned int WORD;

/*Declare SFR associated with the IAP */
sfr IAP_DATA    =   0xC2;           //Flash data register
sfr IAP_ADDRH   =   0xC3;           //Flash address HIGH
sfr IAP_ADDRL   =   0xC4;           //Flash address LOW
sfr IAP_CMD     =   0xC5;           //Flash command register
sfr IAP_TRIG    =   0xC6;           //Flash command trigger
sfr IAP_CONTR   =   0xC7;           //Flash control register

/*Define ISP/IAP/EEPROM command*/
#define CMD_IDLE    0               //Stand-By
#define CMD_READ    1               //Byte-Read
#define CMD_PROGRAM 2               //Byte-Program
#define CMD_ERASE   3               //Sector-Erase

/*Define ISP/IAP/EEPROM operation const for IAP_CONTR*/
//#define ENABLE_IAP 0x80           //if SYSCLK<30MHz
//#define ENABLE_IAP 0x81           //if SYSCLK<24MHz
#define ENABLE_IAP  0x82            //if SYSCLK<20MHz
//#define ENABLE_IAP 0x83           //if SYSCLK<12MHz
//#define ENABLE_IAP 0x84           //if SYSCLK<6MHz
//#define ENABLE_IAP 0x85           //if SYSCLK<3MHz
//#define ENABLE_IAP 0x86           //if SYSCLK<2MHz
//#define ENABLE_IAP 0x87           //if SYSCLK<1MHz

//Start address for STC10/11xx EEPROM
#define IAP_ADDRESS 0x0000

void Delay(unsigned char n);
void IapIdle();
unsigned char IapReadByte(int addr);
void IapProgramByte(int addr, unsigned char dat);
void IapEraseSector(int addr);

#endif

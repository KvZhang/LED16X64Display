// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _LEDdisplay16X64_H_
#define _LEDdisplay16X64_H_
#include "Arduino.h"
#include "dispText.h"
#include "Defined.h"
#include "TimerOne.h"
#include "EEPROM.h"
#include "LaputaComm.h"
//add your includes for the project LEDdisplay16X64 here
//end of add your includes here
#ifdef __cplusplus
extern "C" {
#endif
void loop();
void setup();
#ifdef __cplusplus
} // extern "C"
#endif

//add your function definitions for the project LEDdisplay16X64 here
/**********************
 * Pin definition
 */
const int RCKPin=8;//PB0
const int colSCKPin=9;//PB1
const int rowDataPin=10;//PB2
const int colDatePin=11;//PB3
const int rowSCKPin=12;//PB4

#define RCK_HIGH() PORTB|=(1<<PORTB0)
#define RCK_LOW() PORTB&=~(1<<PORTB0)
#define COL_SCK_HIGH() PORTB|=(1<<PORTB1)
#define COL_SCK_LOW() PORTB&=~(1<<PORTB1)
#define ROW_DATA_HIGH() PORTB|=(1<<PORTB2)
#define ROW_DATA_LOW() PORTB&=~(1<<PORTB2)
#define COL_DATA_HIGH() PORTB|=(1<<PORTB3)
#define COL_DATA_LOW() PORTB&=~(1<<PORTB3)
#define ROW_SCK_HIGH() PORTB|=(1<<PORTB4)
#define ROW_SCK_LOW() PORTB&=~(1<<PORTB4)
/**************************
 * constant
 */
#define BUFFER_SIZE (128)
#define FULL   		(0)
#define LOW_HALF	(1)
#define HIGH_HALF	(2)
#define LED_HIGH	(16)
#define LED_LENGTH	(64)
const int hanziSize=32;
const int charSize=16;
const int scrollSpeedDefault=70;
const int visitorCountAddr=0x00;//MSB,xx,xx,LSB
/**************************
 * Globe variables
 */
volatile unsigned int dispBuffer[BUFFER_SIZE]={};
volatile unsigned char startPtr;
volatile unsigned char rowPtr;
volatile unsigned char columnPtr;
volatile unsigned int loadPtr;
volatile unsigned char scrollSpeedCounter;
volatile unsigned char scrollSpeed;
volatile unsigned long visitorCount;
volatile unsigned char numDisp[6]={};//{MSB,...,LSB}DEC
volatile unsigned char halfPtr;
/**************************
 * global flag
 */
struct FLAG_BITES{
	unsigned char isNeedScroll:1;
	unsigned char isScrollingEnded:1;
	unsigned char isBufferEmpty:1;
	unsigned char isLastCycle:1;
	unsigned char isLowHalfBufferEmpty:1;
	unsigned char isHighHalfBufferEmpty:1;
	unsigned char bit6:1;
	unsigned char bit7:1;
};
union SYS_FLAG{
	unsigned char all;
	FLAG_BITES bit;
};
volatile SYS_FLAG gFlag;
/**********************
 * Func definition
 */
void Init_IO();
void Init_Variables();
void TimingISR();
void refreshDispNum();
void clearBuffer(unsigned char selec);
void loadBufferFromFlash(unsigned char addr1,uint8_t *addr2,
	unsigned char length);
void stateIdle();
void stateActing2();
void stateActing1();
unsigned int gState;
unsigned int batteryVolt;
unsigned int gError;
//unsigned int gError=NONE_ERROR;

//Do not add code below this line
#endif /* _LEDdisplay16X64_H_ */

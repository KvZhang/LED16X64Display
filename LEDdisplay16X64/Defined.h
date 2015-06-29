/*
 * Defined.h
 *
 *  Created on: 2015Äê4ÔÂ10ÈÕ
 *      Author: ZhangYu
 */

#ifndef DEFINED_H_
#define DEFINED_H_

typedef enum
{
	NEW_ID_ACTING=0x10,OLD_ID_ACTING=0x11,TRICK_ACTING=0x12,
}_ME_CMD;
typedef enum
{
	STATE_IDLE, STATE_ACTING1, STATE_ACTING2,
}_ME_STATE;
typedef enum
{KEY1='A',KEY2='B',KEY3='C',KEY4='D',KEY5='E',LEFT_ARROW=0xD8,RIGHT_ARROW=0xD7} KEY_DEFINE;
#endif /* DEFINED_H_ */

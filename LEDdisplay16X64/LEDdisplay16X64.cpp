// Do not remove the include below
#include "LEDdisplay16X64.h"

//The setup function is called once at startup of the sketch
LaputaComm myComm(0x10,MACHINE_NUMBER+SCREEN,PASSIVE_NO_BATTERY);
void setup() {
	Serial.begin(9600);
	Init_IO();

	Init_Variables();

// Add your initialization code here
	Timer1.initialize(1200); //timing for 1.2ms
	Timer1.attachInterrupt(TimingISR); //declare the interrupt serve routine:TimingISR
	/*	for (unsigned char i = 0; i < 64; i++) {
	 dispBuffer[i] = (pgm_read_byte(&string2[i/16][((i%16)<<1)]) )
	 + (pgm_read_byte(&string2[i / 16][((i % 16) << 1) + 1])<< 8);

	 }*/
	/*	for (unsigned char i = 0; i < BUFFER_SIZE; i++) {
	 dispBuffer[i] = (pgm_read_byte(&string2[i << 1]))
	 + (pgm_read_byte(&string2[(i<<1)+1]) << 8);
	 }*/

}

// The loop function is called in an endless loop
void loop() {
//Add your repeated code here
	switch (gState) {
	case STATE_IDLE:
		stateIdle();
		break;
	case STATE_ACTING2:
		stateActing2();
		break;
	case STATE_ACTING1:
		stateActing1();
		break;
	default:
		gState = STATE_IDLE;
		break;
	}
	myComm.receiveCMD();//processing communicating
//	Serial.println(myComm.rxRegister.dataL);
	if(myComm.rxRegister.CMD==0x88){
		for(unsigned char i=0;i<sizeof(numDisp);i++){
			Serial.write(numDisp[i]);
		}
		for(unsigned char i=0;i<4;i++){
			Serial.write((unsigned char)((visitorCount>>(24-3*i))&0xff));
		}
		Serial.write(visitorCount);
		myComm.rxRegister.CMD=0x00;
	}

}
void Init_IO() {

	pinMode(RCKPin, OUTPUT);
	pinMode(colSCKPin, OUTPUT);
	pinMode(rowDataPin, OUTPUT);
	pinMode(colDatePin, OUTPUT);
	pinMode(rowSCKPin, OUTPUT);
	pinMode(13, OUTPUT);
	digitalWrite(13, LOW);
}
void Init_Variables() {
	gFlag.all=0;
	rowPtr = 0;
	columnPtr = 0;
	startPtr = 0;
	loadPtr = 0;
	scrollSpeedCounter = 0;
	halfPtr = 0;
	scrollSpeed = scrollSpeedDefault;
	if (EEPROM.read(visitorCountAddr) == 0xff) {
		EEPROM.write(visitorCountAddr, 0);
		EEPROM.write(visitorCountAddr + 1, 0);
		EEPROM.write(visitorCountAddr + 2, 0);
		EEPROM.write(visitorCountAddr + 3, 1);

	}

	for (unsigned char i = 0; i < 4; i++) {
		visitorCount = (visitorCount << 8) + EEPROM.read(visitorCountAddr + i);
	}

	clearBuffer(FULL);

	refreshDispNum();

	gState = STATE_IDLE;
	batteryVolt = 0;
	gError = NONE_ERROR;
}
/*void TimingISR() {//逐列扫描
 RCK_LOW();
 unsigned char realCol = ((columnPtr >> 3) << 3) + 7 - columnPtr % 8;

 for (unsigned i = 0; i < 64; i++) {
 COL_SCK_LOW();
 if (realCol == i)
 COL_DATA_LOW();
 else
 COL_DATA_HIGH();
 COL_SCK_HIGH();
 }
 //	COL_SCK_LOW();
 //	if (columnPtr == 0)
 //		COL_DATA_LOW();
 //	else
 //		COL_DATA_HIGH();
 //	COL_SCK_HIGH();

 for (unsigned char i = 0; i < 16; i++) {
 ROW_SCK_LOW();
 //		if (dispBuffer[startPtr + columnPtr] & (1 << i))
 if (dispBuffer[startPtr + realCol] & (1 << i))
 ROW_DATA_LOW();
 else
 ROW_DATA_HIGH();
 ROW_SCK_HIGH();
 }
 RCK_HIGH();
 //	ROW_SCK_LOW();
 //	COL_SCK_LOW();
 //	RCK_LOW();
 columnPtr++;
 if (columnPtr >= 64) {
 columnPtr = 0;
 }
 }*/
void TimingISR() { //逐行扫描
	RCK_LOW();
	/*********Scan row***************/
	ROW_SCK_LOW();
	if (rowPtr == 0)
		ROW_DATA_LOW();
	else
		ROW_DATA_HIGH();
	ROW_SCK_HIGH();
	/*********Scan col***************/
	for (unsigned char i = 0; i < 8; i++) {
		unsigned char temp1 = startPtr + halfPtr * LED_LENGTH + (i << 3) + 7;
		unsigned int temp2 = 1 << rowPtr;
		for (unsigned char j = 0; j < 8; j++) {
			COL_SCK_LOW();
			if (dispBuffer[(temp1 - j) % BUFFER_SIZE] & (temp2))
				COL_DATA_LOW();
			else
				COL_DATA_HIGH();
			COL_SCK_HIGH();
		}

	}

	RCK_HIGH();
	/**********Scan end*************/
	rowPtr++;
	if (rowPtr >= LED_HIGH)
		rowPtr = 0;
	/*********Processing scrolling*********************/
	if (gFlag.bit.isNeedScroll) {
		scrollSpeedCounter++;
		if (scrollSpeedCounter > scrollSpeed) {
			scrollSpeedCounter = 0;
			startPtr++;
		}
		if ((startPtr == LED_LENGTH) && gFlag.bit.isScrollingEnded == 0) {
			startPtr = 0;
			if (halfPtr)
				halfPtr = 0;
			else
				halfPtr = 1;
			gFlag.bit.isScrollingEnded = 1;
//			digitalWrite(13, HIGH); //for test
			gFlag.bit.isNeedScroll = 0; //stop scrolling and waiting for refresh buffer
		}
	}

}
void stateIdle() {
	unsigned char cmdTemp=0;
	if(myComm.flag.bit.isNeedProcess){
		cmdTemp=myComm.cmdHandler();
//		dispBuffer[0]=cmdTemp;//debug
//		Serial.println(cmdTemp);
		if(cmdTemp==NEW_ID_ACTING){
			visitorCount++;
			Serial.println(visitorCount);
			refreshDispNum();
//			unsigned long temp=visitorCount;

			for(unsigned char i=0;i<4;i++){
				EEPROM.write(visitorCountAddr+i,(visitorCount>>(8*(3-i)))&0xff);
			}
			gState=STATE_ACTING1;
		}else if(cmdTemp==OLD_ID_ACTING){
			gState=STATE_ACTING2;
		}
	}

}
void stateActing1() {
	unsigned char count1 = 3; //the number of Hanzi before visitor numbers
	unsigned char count2 = 6; //the disp size of vistor numbers
	unsigned char count3 = 2; //the number of Hanzi after visitor numbers
	unsigned char count4 = 2; //last things
	if (gFlag.bit.isBufferEmpty) { //first cycle of disp "您是第xxxxxx位闯"
		gFlag.bit.isBufferEmpty = 0;

		for (unsigned char i = 0; i < hanziSize / 2 * count1; i++) { //"您是第"
			dispBuffer[i] = (pgm_read_byte(&welcomeString[i << 1]))
					+ (pgm_read_byte(&welcomeString[(i<<1)+1]) << 8);
		}
		for (unsigned char j = 0; j < count2; j++) { //visitor's number
			for (unsigned char i = 0; i < charSize / 2; i++) {
				dispBuffer[hanziSize / 2 * count1 + i + j * charSize / 2] =
						(pgm_read_byte(
								&numString[numDisp[j] * charSize + i*2]))
								+ (pgm_read_byte(
										&numString[numDisp[j] * charSize
														+ i*2 + 1]) << 8);
			}
		}
		for (unsigned char i = 0; i < hanziSize / 2 * count3; i++) { //"位闯"
			dispBuffer[i + hanziSize / 2 * count1 + charSize / 2 * count2] =
					(pgm_read_byte(
							&welcomeString[(i + hanziSize / 2 * count1) << 1]))
							+ (pgm_read_byte(
									&welcomeString[((i
													+ hanziSize / 2 * count1) << 1)
									+ 1]) << 8);
		}
		gFlag.bit.isNeedScroll = 1;
	}

	if (gFlag.bit.isScrollingEnded) {

		gFlag.bit.isScrollingEnded = 0;
		if (halfPtr) {
			clearBuffer(LOW_HALF);
			gFlag.bit.isLowHalfBufferEmpty = 0;
			for (unsigned char i = 0; i < hanziSize / 2 * count4; i++) { //"关者"
				dispBuffer[i] =
						(pgm_read_byte(
								&welcomeString[(i
										+ hanziSize / 2 * (count1 + count3))
										<< 1]))
								+ (pgm_read_byte(&welcomeString[((i + hanziSize / 2 * (count1+count3)) << 1)
										+ 1]) << 8);
			}
			for (unsigned char i = hanziSize / 2 * count4; i < BUFFER_SIZE / 2;
					i++) {
				dispBuffer[i] = 0;
			}
			gFlag.bit.isNeedScroll = 1;

		} else {
			delay(1000);
			gState = STATE_ACTING2;
			clearBuffer(FULL);
		}
	}

//	digitalWrite(13, HIGH);

}
void stateActing2() {
	while (1) {
		if (gFlag.bit.isBufferEmpty) {
			gFlag.bit.isBufferEmpty = 0;
			loadBufferFromFlash(0, (unsigned char*) &string, BUFFER_SIZE);
			gFlag.bit.isNeedScroll = 1;
		}

		if (gFlag.bit.isScrollingEnded) {
			gFlag.bit.isScrollingEnded = 0;
			if (gFlag.bit.isLastCycle) {
				delay(5000);
				Init_Variables();
				gFlag.bit.isLastCycle=0;
				gState = STATE_IDLE;
				return;
			}
			if (halfPtr) {
				clearBuffer(LOW_HALF);
				loadBufferFromFlash(0, (unsigned char*) &string,
						BUFFER_SIZE / 2);
				gFlag.bit.isNeedScroll = 1;
			} else {
				clearBuffer(HIGH_HALF);
				loadBufferFromFlash(BUFFER_SIZE / 2, (unsigned char*) &string,
						BUFFER_SIZE / 2);
				gFlag.bit.isNeedScroll = 1;
			}

		}
	}

}
void refreshDispNum() {
	unsigned long temp = visitorCount;
	for (unsigned char i = 0; i < 6; i++) {
		numDisp[5-i] = temp % 10;
		temp = temp / 10;
	}
}
void clearBuffer(unsigned char selec) {
	if (selec == FULL) {
		for (unsigned char i = 0; i < BUFFER_SIZE; i++) {
			dispBuffer[i] = 0;
		}
		gFlag.bit.isBufferEmpty = 1;
	} else if (selec == LOW_HALF) {
		for (unsigned char i = 0; i < BUFFER_SIZE / 2; i++) {
			dispBuffer[i] = 0;
		}
		gFlag.bit.isLowHalfBufferEmpty = 1;
	} else if (selec == HIGH_HALF) {
		for (unsigned char i = BUFFER_SIZE / 2; i < BUFFER_SIZE; i++) {
			dispBuffer[i] = 0;
		}
		gFlag.bit.isHighHalfBufferEmpty = 1;
	}
}
void loadBufferFromFlash(unsigned char addr1, uint8_t * addr2,
		unsigned char length) {
	unsigned int flashReadTemp = 0;
	for (unsigned char i = 0; i < length; i++) {
		flashReadTemp = pgm_read_byte(addr2 + (i + loadPtr) * 2)
				+ (pgm_read_byte(addr2+(i+loadPtr)*2+1) << 8);
		if (flashReadTemp == 0xBBAA) {
			for (unsigned char j = i; j < length; j++) {
				dispBuffer[addr1 + j] = 0;
			}
			gFlag.bit.isLastCycle = 1;
			break;
		} else {
			dispBuffer[addr1 + i] = flashReadTemp;
		}
	}
	loadPtr += length;
//	Serial.println(loadPtr);
}

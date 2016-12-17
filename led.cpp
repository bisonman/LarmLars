//**********************************************************************************
//
//
//**********************************************************************************

#include "led.h"
#include <TimeLib.h>
#include <Arduino.h>

//**********************************************************************************
int
LED::setMode(int mode)
{
	switch (mode) {
		case OFF:
		case ON:
		case FLASH:
			break;
			
		default:
			return -1;
	}
	ledMode = mode;
	return 0;
}

//**********************************************************************************
int
LED::getMode()
{
	return ledMode;
}

//**********************************************************************************
void
LED::loop()
{
	switch (ledMode) {
		case OFF:
			digitalWrite(ledPIN, HIGH);
			break;
				
		case ON:
			digitalWrite(ledPIN, LOW);
			break;
					 
		case FLASH:
			int sekund = second();
			
			if (oldSekund != sekund) {
				oldSekund = sekund;
				if (digitalRead(ledPIN)) {
					digitalWrite(ledPIN, LOW);
				}
				else {
					digitalWrite(ledPIN, HIGH);
				}
			}
			break;
	}
}


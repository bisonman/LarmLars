//**********************************************************************************
//
//
//**********************************************************************************

#include "led.h"
#include <TimeLib.h>
#include <Arduino.h>

//**********************************************************************************
void
LED::setAction(int mode)
{
  digitalWrite(ledPIN, mode);
}

//**********************************************************************************
int
LED::setMode(int mode)
{
	switch (mode) {
		case OFF:
      ledMode = mode;
      break;
		case ON:
      ledMode = mode;
      break;
		case FLASH:
      ledMode = mode;
      break;
    case FLASH_COUNT:
			break;
			
		default:
			return -1;
	}
	ledMode = mode;
	return 0;
}

//**********************************************************************************
int
LED::setMode(int mode, int cnt)
{
  switch (mode) {
    case FLASH_COUNT:
      count = cnt;
      break;
      
    default:
      return -1;
  }
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
  int sekund = second();
  
	switch (ledMode) {
		case OFF:
			digitalWrite(ledPIN, HIGH);
			break;

    //---------------------------------
		case ON:
			digitalWrite(ledPIN, LOW);
			break;
      
    //---------------------------------
    case FLASH:    
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

    //---------------------------------
		case FLASH_COUNT:
      if (oldSekund != sekund) {
        oldSekund = sekund;
        
        if (digitalRead(ledPIN)) {
          count--;
          digitalWrite(ledPIN, LOW);
        }
        else {
          digitalWrite(ledPIN, HIGH);
        }
        if (count == 0) {
          ledMode = oldLedMode;
        }
      }
      break;

    default:
      break;
	}
}

//**********************************************************************************
void
LED::flashCount(int n)
{
#if 0
  int oldStatus = digitalRead(ledPIN);
  while (n-- > 0) {
    digitalWrite(ledPIN, LOW);
    delay(150);
    digitalWrite(ledPIN, HIGH);
    delay(150);
  }
  digitalWrite(ledPIN, oldStatus);
#endif
}


/****************************************************************************
 * Project : LarmLars
 * Date : 2016-11-19
 * 
 * Arduino Mini Pro 16MHz
 * 
 * ---- LED ---- 
 * Green:
 *     Off = (power off), On = (power on, switch armed), flashing = (power on, switch active) 
 *     
 * Eed:
 *     Off = (Alarm not active), On = (Alarm active), Flashing = (Alarm has timedout)
 * 
 */

#include <TimeLib.h>
#include <EEPROM.h>
#include <limits.h>
#include <RC5.h>
#include "led.h"

using namespace std;

#define TIMEOUT_STEPS  40       // Number of seconds for each step (1-9) 0 no timeout

//#####  I/O pins
int IR_PIN = 5;
int GREEN_LED_PIN = 6;
int RED_LED_PIN = 7;
int INTERRUPT_PIN = 3;
int RELAY_PIN = 4;
int INTERNAL_LED_PIN = 13;

RC5 rc5(5);

int maxTime = 10;					          // Ställer in Larmtimeout * TIMEOuT_STEPS i antal sekunder
int EEPROM_AdrMaxTime = 0;          // Adress i EEPROM d�r "maxTime" lagras
int EEPROM_AdrPowerOnStatus = 2;    // Adress i EEPROM d�r flagga som s�tter auto power vid boot

//#####  Status flags
boolean FlagPowerOn = false;
boolean FlagFlashRedLED = false;
//## boolean switchStatus = false;
boolean FlagGreenLed = false;
boolean FlagRedLed = false;
boolean FlagRelay = false;
boolean FlagLarm = false;
//## boolean FlagTimeOverflow = false;

int Second = 0;
int oldSecond = 0;
unsigned char oldToggle = 0;		// Används vid fjärrkontroll avläsning

unsigned long TimeLarm = 0;         // avläsning av millis() vid larm
unsigned long TimeLarmTimeout = 0;  // Hur l�ng tid rel�et skall vara aktiverat vid larm

LED greenLED(GREEN_LED_PIN);
LED redLED(RED_LED_PIN);

//**********************************************************************************
void
switchInterrupt()
{
	if (digitalRead(INTERRUPT_PIN) == HIGH) {
		if (FlagPowerOn == true) {
			FlagLarm = 1;
			TimeLarm = millis();
			TimeLarmTimeout = (maxTime * TIMEOUT_STEPS);
			FlagRelay = true;
      redLED.setMode(ON);
		}
	}
  else {
    
  }
}

//**********************************************************************************
void
flashGreenLed(int n)
{
  int oldMode = greenLED.getMode();
  Serial.print(n);   Serial.println(", T1");
  greenLED.setMode(OFF);
  delay(1000);
  while (n > 0) {
    greenLED.setAction(ON);
    delay(1000);
    greenLED.setAction(OFF);
    delay(1000);
    n--;
    Serial.println("T2");
  }
  greenLED.setMode(oldMode);
  Serial.println("T3");
}

//**********************************************************************************
void
setup()
{
	FlagLarm = 0;
	TimeLarm = 0;
  
	maxTime = EEPROM.read(EEPROM_AdrMaxTime);

	if (!(maxTime >= 0 && maxTime <= 9)) {
		maxTime = 2;
		EEPROM.write(EEPROM_AdrMaxTime, maxTime);      // Write default value
	}
	FlagPowerOn = EEPROM.read(EEPROM_AdrPowerOnStatus);
  
	Serial.begin(115200);
	pinMode(INTERRUPT_PIN, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), switchInterrupt, CHANGE);

	pinMode(RELAY_PIN, OUTPUT);
	pinMode(INTERNAL_LED_PIN, OUTPUT);
	digitalWrite(RELAY_PIN, LOW);
	FlagRedLed = false;
 
  if (digitalRead(INTERRUPT_PIN) == LOW) {
    greenLED.setMode(FLASH);
  }
  else {
     greenLED.setMode(ON);
  }
	Serial.println("ATOK");
}

//**********************************************************************************
void
showInformation()
{
	int i;
	int numberOfFlashes = maxTime;

	Serial.print("FlagPowerOn="); Serial.println(FlagPowerOn);
	//Serial.print("EEPROM FlagPowerOn="); Serial.println(EEPROM.read(EEPROM_AdrPowerOnStatus));
	Serial.print("FlagLarm="); Serial.println(FlagLarm);
	Serial.print("maxtime="); Serial.println(maxTime);
	Serial.print("FlagFlashRedLED="); Serial.println(FlagFlashRedLED);
	Serial.print("FlagRedLed="); Serial.println(FlagRedLed);
  
	if (maxTime == 0) {
		numberOfFlashes = 10;
	}
  flashGreenLed(numberOfFlashes);
  //greenLED.flashCount(numberOfFlashes);
  Serial.print("GreenMode="); Serial.println(greenLED.getMode());
}

//**********************************************************************************
void
handleRemoteController()
{
	unsigned char toggle;
	unsigned char address;
	unsigned char command = 0;

	if (rc5.read(&toggle, &address, &command)) {
		Serial.print("a: "); Serial.print(address);
		Serial.print(", c: "); Serial.print(command);
		Serial.print(", t: "); Serial.println(toggle);

		if (toggle != oldToggle) {
			
			switch ((int) command) {
				case 0:
				case 1:
		    case 2:
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
				case 8:
				case 9:
          Serial.println("Write eeprom");
          greenLED.flashCount(2);
					maxTime = command;
					EEPROM.write(EEPROM_AdrMaxTime, maxTime);
					break;

				case 12:
          greenLED.flashCount(3);
					
					if (FlagPowerOn) {
						FlagPowerOn = false;
						FlagGreenLed = false;
						Serial.println("OFF");
					}
					else {
						FlagPowerOn = true;
						FlagGreenLed = true;
						Serial.println("ON");

						if (!digitalRead(INTERRUPT_PIN)) {
							 redLED.setMode(FLASH);
						}
					}
					break;

				case 15:
        case 18:
          greenLED.flashCount(4);
					showInformation();
					break;
			
				case 43:
					greenLED.flashCount(5);
					if (EEPROM.read(EEPROM_AdrPowerOnStatus)) {
						EEPROM.write(EEPROM_AdrPowerOnStatus, 0);
						redLED.flashCount(2);
					}
					else {
						EEPROM.write(EEPROM_AdrPowerOnStatus, 1);
            redLED.flashCount(2);
					}
					break;
			}
		}
		oldToggle = toggle;
	}
}

//**********************************************************************************
void
handleSecondChangeAction()
{
	if (FlagPowerOn == true) {
		if (FlagLarm == true) {
			if (maxTime > 0) {					// Om maxTime = 0 ingen timeout
				Serial.println(TimeLarmTimeout);
		  
				if (TimeLarmTimeout > 0L) {
					TimeLarmTimeout--;
			
					if (TimeLarmTimeout == 0L) {
            redLED.setMode(FLASH);
						FlagRelay = false;		        	// Tid att stänga av larm signalen
						FlagLarm = false;	
					}
				}
			}
		}
		FlagGreenLed = true;
	}
	else {
    greenLED.setMode(OFF);
	}
}

//**********************************************************
void
loop()
{
	handleRemoteController();

  if (FlagPowerOn) {
    if (greenLED.getMode() != FLASH) {
      greenLED.setMode(ON);
    }
  }
	if (digitalRead(INTERRUPT_PIN)) {			// Används vid installations test
    greenLED.setMode(ON);
		digitalWrite(INTERNAL_LED_PIN, HIGH);
	}
	else {
		digitalWrite(INTERNAL_LED_PIN, LOW);
	}
	Second = second();
  
	if (Second != oldSecond) {
		handleSecondChangeAction();
    greenLED.loop();
    redLED.loop();
		oldSecond = Second;
	}
	if (FlagRelay == true) {
		digitalWrite(RELAY_PIN, HIGH);
	}
	else {
		digitalWrite(RELAY_PIN, LOW);
	}
}


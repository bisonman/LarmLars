/****************************************************************************
 * Project : LarmLars
 * Date : 2016-11-19
 * 
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

int maxTime = 1;					// St�ller in Larmtimeout * TIMEOuT_STEPS i antal sekunder
int EEPROM_AdrMaxTime = 0;          // Adress i EEPROM d�r "maxTime" lagras
int EEPROM_AdrPowerOnStatus = 2;    // Adress i EEPROM d�r flagga som s�tter auto power vid boot

//#####  Status flags
boolean FlagPowerOn = false;
boolean FlagFlashRedLED = false;
boolean FlagSwitch = false;
//## boolean switchStatus = false;
boolean FlagGreenLed = false;
boolean FlagRedLed = false;
boolean FlagRelay = false;
boolean FlagLarm = false;
//## boolean FlagTimeOverflow = false;

int Second = 0;
int oldSecond = 0;
unsigned char oldToggle = 0;		// Anv�nds vid fj�rrkontroll avl�sning

unsigned long TimeLarm = 0;         // avl�sning av millis() vid larm
unsigned long TimeLarmTimeout = 0;  // Hur l�ng tid rel�et skall vara aktiverat vid larm

LED greenLED(GREEN_LED_PIN);
LED redLED(RED_LED_PIN);

//**********************************************************************************
void
switchInterrupt()
{
	FlagSwitch = true;
  
	if (digitalRead(INTERRUPT_PIN) == 0) {
		if (FlagPowerOn == true) {
			FlagLarm = 1;
			TimeLarm = millis();
			TimeLarmTimeout = (maxTime * TIMEOUT_STEPS);
			FlagRelay = true;
      redLED.setMode(ON);
		}
	}
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
	attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), switchInterrupt, FALLING);

	pinMode(RELAY_PIN, OUTPUT);
	pinMode(INTERNAL_LED_PIN, OUTPUT);
	digitalWrite(RELAY_PIN, LOW);

	FlagRedLed = false;
	Serial.println("ATOK");
}

//**********************************************************************************
void
showInformation()
{
	int i;
	int numberOfFlashes = maxTime;
  int tmp = greenLED.getMode();
  
	Serial.print("FlagPowerOn="); Serial.println(FlagPowerOn);
	Serial.print("EEPROM FlagPowerOn="); Serial.println(EEPROM.read(EEPROM_AdrPowerOnStatus));
	Serial.print("FlagLarm="); Serial.println(FlagLarm);
	Serial.print("maxtime="); Serial.println(maxTime);
	Serial.print("FlagFlashRedLED="); Serial.println(FlagFlashRedLED);
	Serial.print("FlagRedLed="); Serial.println(FlagRedLed);
	delay(2000);
  
	if (maxTime == 0) {
		numberOfFlashes = 10;
	}
	for (i = 0; i < numberOfFlashes; i++) {
		delay(200);
    greenLED.setMode(ON);
		delay(200);
    greenLED.setMode(OFF);
	}
	delay(2000);
  greenLED.setMode(tmp);
}

//**********************************************************************************
void
flashGreenLED(int n)
{
	int tmp = greenLED.getMode();

  greenLED.setMode(OFF);
	delay(200);

	for (int i=0; i<n; i++) {
    greenLED.setMode(ON);
		delay(100);
    greenLED.setMode(OFF);
		delay(100);
	}
  greenLED.setMode(tmp);
}

//**********************************************************************************
void
flashRedLED(int n)
{
	int tmp = redLED.getMode();

	redLED.setMode(OFF);
	delay(200);
	
	for (int i=0; i<n; i++) {
    redLED.setMode(ON);
		delay(100);
    redLED.setMode(OFF);
		delay(100);
	}	
  redLED.setMode(tmp);
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
		Serial.print(" c: "); Serial.print(command);
		Serial.print(" t: "); Serial.println(toggle);

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
					flashGreenLED(2);
					maxTime = command;
					EEPROM.write(EEPROM_AdrMaxTime, maxTime);
					break;

				case 12:
					flashGreenLED(3);
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
					flashGreenLED(4);
					showInformation();
					break;
			
				case 43:
					flashGreenLED(5);
					if (EEPROM.read(EEPROM_AdrPowerOnStatus)) {
						EEPROM.write(EEPROM_AdrPowerOnStatus, 0);
						flashRedLED(2);
					}
					else {
						EEPROM.write(EEPROM_AdrPowerOnStatus, 1);
						flashRedLED(2);
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
		digitalWrite(INTERNAL_LED_PIN, HIGH);
	}
	else {
		digitalWrite(INTERNAL_LED_PIN, LOW);
	}
	Second = second();
  
	if (Second != oldSecond) {
		handleSecondChangeAction();
		oldSecond = Second;
	}
	if (FlagRelay == true) {
		digitalWrite(RELAY_PIN, HIGH);
	}
	else {
		digitalWrite(RELAY_PIN, LOW);
	}
  greenLED.loop();
  redLED.loop();
}


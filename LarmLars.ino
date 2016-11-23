
#include <EEPROM.h>
#include <limits.h>
#include <RC5.h>

using namespace std;

#define TIMEOUT_STEPS  20       // Number of seconds for each step (1-9) 0 no timeout

boolean switchFlag = false;
boolean switchStatus = false;
int IR_PIN = 5;
int GREEN_LED_PIN = 6;
int RED_LED_PIN = 7;
int INTERRUPT_PIN = 3;
int RELAY_PIN = 4;
unsigned long t0;

RC5 rc5(5);

int maxTime = 1;
int maxTimeAdrEEPROM = 0;

boolean powerOn = false;
boolean flashRedLED = false;

unsigned long currentTime = 0;
unsigned long oldCurrentTime = 0;
unsigned char oldToggle = 0;

boolean larmFlag = false;
boolean timeOverflow = false;
unsigned long larmTime = 0;
unsigned long larmTimeout = 0;

//**********************************************************
void
switchInterrupt()
{
  switchFlag = true;
  
  if (digitalRead(INTERRUPT_PIN)) {
    switchStatus = true;
  }
  else {
    switchStatus = false;    
    
    if (!powerOn) {
      return;
    }
    larmFlag = 1;
    larmTime = millis();
    larmTimeout = (maxTime * TIMEOUT_STEPS);
    digitalWrite(RELAY_PIN, HIGH);
    digitalWrite(RED_LED_PIN, LOW);
    flashRedLED = false;            // Endast efter timeout
  }
}

//**********************************************************
void
setup()
{
  larmFlag = 0;
  larmTime = 0;
  maxTime = EEPROM.read(maxTimeAdrEEPROM);
  
  if (maxTime < 0 || maxTime > 9) {
    maxTime = 2;
    EEPROM.write(maxTimeAdrEEPROM, maxTime);      // Write default value
  }
  
  Serial.begin(115200);
  pinMode(INTERRUPT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), switchInterrupt, FALLING);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(GREEN_LED_PIN, HIGH);
  digitalWrite(RED_LED_PIN, HIGH);
  Serial.println("ATOK");
  
}

//**********************************************************
void
visaMaxtime()
{
  int i;
  int numberOfFlashes = maxTime;
  Serial.print("maxtime="); Serial.println(maxTime);
  delay(2000);
  if (maxTime == 0) {
    numberOfFlashes = 10;
  }
  for (i=0; i<numberOfFlashes; i++) {
    delay(200);
    digitalWrite(GREEN_LED_PIN, LOW);
    delay(200);
    digitalWrite(GREEN_LED_PIN, HIGH);
  }
  delay(2000);
}

//**********************************************************
void
flashGreenLED()
{
  digitalWrite(GREEN_LED_PIN, HIGH);
  delay(300);
  digitalWrite(GREEN_LED_PIN, LOW);  
  delay(200);
  digitalWrite(GREEN_LED_PIN, HIGH);
  delay(200);
  digitalWrite(GREEN_LED_PIN, LOW);  
  delay(200);
  digitalWrite(GREEN_LED_PIN, HIGH);
}

//**********************************************************
void
handleRemoteController()
{
  unsigned char toggle;
  unsigned char address;
  unsigned char command = 0;

  if (rc5.read(&toggle, &address, &command)) {
    Serial.print("a: "); Serial.print(address);
    Serial.print(" c: "); Serial.print(command);
    Serial.print(" ot: "); Serial.print(oldToggle);
    Serial.print(" t: "); Serial.println(toggle);
    flashGreenLED();
    
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
          maxTime = command;
          EEPROM.write(maxTimeAdrEEPROM, maxTime);
          break;
      
        case 15:
          flashGreenLED();
          visaMaxtime();
          break;

        case 12:
          if (powerOn) {
            powerOn = false;
            Serial.println("OFF");
          }
          else {
            powerOn = true;
            Serial.println("ON");
            if (digitalRead(INTERRUPT_PIN)) {
              flashRedLED = true;
            }
          }
      }
    }
    oldToggle = toggle;
  }
}

//**********************************************************
void
handleSecondAction()
{
  if (powerOn) {
    digitalWrite(GREEN_LED_PIN, LOW);
  }
  else {
    digitalWrite(GREEN_LED_PIN, HIGH);
  }
  if (flashRedLED) {
    if (digitalRead(RED_LED_PIN)) {
      digitalWrite(RED_LED_PIN, LOW);  
    }
    else {
      digitalWrite(RED_LED_PIN, HIGH);
    }
  }

  if (switchFlag == true) {
    if (switchStatus == true) {
      Serial.println("Switch HIGH");
    }
    else {
      Serial.println("Switch LOW");
    }
    switchFlag = false;
  }
}

//**********************************************************
void
loop()
{
  unsigned long triggTime;
  
  handleRemoteController();

  currentTime = millis();

  if (currentTime < oldCurrentTime) {   //## Timer overflow
    oldCurrentTime = 0;
  }
  if ((currentTime - oldCurrentTime) > 1000) {
    handleSecondAction();
    
    if (larmFlag && powerOn) {
      if (larmTimeout > 0) {
        larmTimeout--;
      
        if (larmTimeout == 0) {
          digitalWrite(RELAY_PIN, LOW);
          digitalWrite(RED_LED_PIN, HIGH);
          flashRedLED = true;
          larmFlag = false;
        }
      }
      else {
        digitalWrite(RED_LED_PIN, HIGH);
        flashRedLED = true;
      }
    }
  }
  if (!powerOn) {
    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(RED_LED_PIN, HIGH);
    digitalWrite(GREEN_LED_PIN, HIGH);
    larmTime = 0L;
    larmFlag = false;
  }
  oldCurrentTime = currentTime;
}


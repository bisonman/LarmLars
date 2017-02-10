//************************************************************
//
//
//************************************************************

#include <Arduino.h>
#include <TimeLib.h>


#define OFF   0
#define ON    1
#define FLASH 2

//************************************************************
//************************************************************
class LED {
	//********************************************************
	public:
		LED(int pin)
		{
			ledPIN = pin;
			pinMode(ledPIN, OUTPUT);
			digitalWrite(ledPIN, HIGH);
		}
		~LED(){};

		//********************************************************
		int setMode(int mode);

		//********************************************************
		int getMode();
		
		//********************************************************
		void loop();
		
	//********************************************************
	private:
		int ledMode;
		int ledPIN;
		int oldSekund;

};

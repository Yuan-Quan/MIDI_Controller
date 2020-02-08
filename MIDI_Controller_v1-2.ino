#include <MIDI.h>
#include "Controller.h"
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

MIDI_CREATE_DEFAULT_INSTANCE();

// Turn on debug statements to the serial output
#define DEBUG 0

unsigned long currentMillis = millis();

#if DEBUG
#define PRINT(s, x) { Serial.print(F(s)); Serial.print(x); }
#define PRINTS(x) Serial.print(F(x))
#define PRINTX(x) Serial.println(x, HEX)
#else
#define PRINT(s, x)
#define PRINTS(x)
#define PRINTX(x)
#endif

// Define the number of devices we have in the chain and the hardware interface
// NOTE: These pin numbers will probably not work with your hardware and may
// need to be adapted
#define HARDWARE_TYPE MD_MAX72XX::ICSTATION_HW
#define MAX_DEVICES 4
#define CLK_PIN   13
#define DATA_PIN  11
#define CS_PIN    10

bool flag = true;


// HARDWARE SPI
MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
// SOFTWARE SPI
//MD_Parola P = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

uint8_t scrollSpeed = 25;    // default frame delay value
textEffect_t scrollEffect = PA_SCROLL_LEFT;
textPosition_t scrollAlign = PA_LEFT;
uint16_t scrollPause = 1000; // in milliseconds

// Global message buffers shared by Serial and Scrolling functions
#define	BUF_SIZE	128
char curMessage[BUF_SIZE] = { "" };
char newMessage[BUF_SIZE] = { "Init...Read Profile_A" };
bool newMessageAvailable = true;
const char* StdBy = "Bank_A";


void dsplMessage(String str)
{
	static char* cp = newMessage;
	
	for (int count = 0; str[count] != '\0'; count++)//我日你妈foreach 编译报错
	{
		*cp = str[count];
		if ((*cp == '\n') || (cp - newMessage >= BUF_SIZE - 2)) // end of message character or full buffer
		{
			*cp = '\0'; // end the string
			// restart the index for next filling spree and flag we have a message waiting
			cp = newMessage;
			newMessageAvailable = true;
		}
		else
		{
			// move char pointer to next position
			cp++;
		}
	}
}



byte NUMBER_BUTTONS = 4;

byte NUMBER_POTS = 2;

//Pot (Pin Number, Command, CC Control, Channel Number)
//**Command parameter is for future use**

Pot PO1(A0, 0, 1, 1);
Pot PO2(A1, 0, 10, 1);
//Pot PO3(A2, 0, 22, 1);
//Pot PO4(A3, 0, 118, 1);
//Pot PO5(A4, 0, 30, 1);
//Pot PO6(A5, 0, 31, 1);
//*******************************************************************
//Add pots used to array below like this->  Pot *POTS[] {&PO1, &PO2, &PO3, &PO4, &PO5, &PO6};
Pot *POTS[] { &PO1, &PO2 };
//*******************************************************************


//***DEFINE DIRECTLY CONNECTED BUTTONS*******************************
//Button (Pin Number, Command, Note Number, Channel, Debounce Time)
//** Command parameter 0=NOTE  1=CC  2=Toggle CC **

Button BU1(2, 2, 120, 1, 5 );
Button BU2(3, 1, 121, 1, 5 );
Button BU3(4, 1, 122, 1, 5 );
Button BU4(5, 1, 123, 1, 5 );
Button BU5(6, 0, 64, 1, 5 );
Button BU6(7, 0, 65, 1, 5 );
Button BU7(8, 1, 66, 1, 5 );
Button BU8(9, 2, 67, 1, 5 );
//*******************************************************************
//Add buttons used to array below like this->  Button *BUTTONS[] {&BU1, &BU2, &BU3, &BU4, &BU5, &BU6, &BU7, &BU8};
Button *BUTTONS_BankA[] { &BU1, &BU2, &BU3, &BU4 };
Button *BUTTONS_BankB[] { &BU5, &BU6, &BU7, &BU8 };
Button *BUTTONS_BankC[] { &BU5, &BU6, &BU7, &BU8 };
Button *BUTTONS_BankD[] { &BU5, &BU6, &BU7, &BU8 };

//*******************************************************************
Button *BUTTONS[4];
String CurrentBank = "Bank_A";




void setup() {
  MIDI.begin(MIDI_CHANNEL_OFF);
  P.begin();
  P.displayText(curMessage, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);
}

void loop() {
  updateButtons();
  //updatePots();
  updateLeds();
  //if (NUMBER_MUX_BUTTONS != 0) updateMuxButtons();
  //if (NUMBER_MUX_POTS != 0) updateMuxPots();
  updateScreen();
  if (isToConfig)//but1 and 3/4 were pressed at the same time, enter the config.
  {
	  updateBanks();
  }
  
}
bool isToConfig()
{
	return (BUTTONS[0]->getValue == 0 && BUTTONS[2]->getValue == 0) || (BUTTONS[1]->getValue == 0 && BUTTONS[3]->getValue == 0);
}

void updateBanks()
{
	//TODO
	if (CurrentBank == "Bank_A")
	{
		switchBank(BUTTONS, BUTTONS_BankB);
		CurrentBank = "Bank_B";
		StdBy = "Bank_B";
	}
	else if (CurrentBank == "Bank_B")
	{
		switchBank(BUTTONS, BUTTONS_BankC);
		CurrentBank = "Bank_C";
		StdBy = "Bank_C";
	}
	else if (CurrentBank == "Bank_C")
	{
		switchBank(BUTTONS, BUTTONS_BankD);
		CurrentBank = "Bank_D";
		StdBy = "Bank_D";
	}
	else if (CurrentBank == "Bank_D")
	{
		switchBank(BUTTONS, BUTTONS_BankA);
		CurrentBank = "Bank_A";
		StdBy = "Bank_A";
	}
	P.displayText(StdBy, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);
}

void switchBank(Button* org[], Button *target[])
{
	for (size_t i = 0; i < NUMBER_BUTTONS; i++)
	{
		org[i] = target[i];
	}
}

void updateScreen()
{
	if (P.displayAnimate())
	{
		if (newMessageAvailable)
		{
			strcpy(curMessage, newMessage);
			newMessageAvailable = false;
		}
		P.displayReset();
	}
}
//*****************************************************************
void updateButtons() {

  // Cycle through Button array
  for (int i = 0; i < NUMBER_BUTTONS; i = i + 1) {
    byte message = BUTTONS[i]->getValue();

    //  Button is pressed
    if (message == 0) {
		//P.displayText("btpsd", scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);
		//dsplMessage("btpsd");
      switch (BUTTONS[i]->Bcommand) {
        case 0: //Note
          MIDI.sendNoteOn(BUTTONS[i]->Bvalue, 127, BUTTONS[i]->Bchannel);
		  BUTTONS[i]->LedState = HIGH;
		  P.displayText("V_127 ON Ch1 cc#", scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);
		  P.displayText((const char*)BUTTONS[i]->Bchannel, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);
		  P.displayText(StdBy, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);
		  break;
        case 1: //CC
			MIDI.sendControlChange(BUTTONS[i]->Bvalue, 127, BUTTONS[i]->Bchannel);
		  BUTTONS[i]->LedState = HIGH;
		  P.displayText("V_127 ON Ch1 cc#", scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);
		  P.displayText((const char*)BUTTONS[i]->Bchannel, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);
		  P.displayText(StdBy, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);
		  break;
        case 2: //Toggle
          if (BUTTONS[i]->Btoggle == 0) {
            MIDI.sendControlChange(BUTTONS[i]->Bvalue, 127, BUTTONS[i]->Bchannel);
            BUTTONS[i]->Btoggle = 1;
			BUTTONS[i]->LedState = HIGH;
			P.displayText("V_127 ON Ch1 cc#", scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);
			P.displayText((const char*)BUTTONS[i]->Bchannel, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);
			P.displayText(StdBy, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);

          }
          else if (BUTTONS[i]->Btoggle == 1) {
            MIDI.sendControlChange(BUTTONS[i]->Bvalue, 0, BUTTONS[i]->Bchannel);
            BUTTONS[i]->Btoggle = 0;
			BUTTONS[i]->LedState = LOW;
			P.displayText("V_0 ON Ch1 cc#", scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);
			P.displayText((const char*)BUTTONS[i]->Bchannel, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);
			P.displayText(StdBy, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);
		  }
          break;
      }
    }

    //  Button is not pressed
    if (message == 1) {
      switch (BUTTONS[i]->Bcommand) {
        case 0:
          MIDI.sendNoteOff(BUTTONS[i]->Bvalue, 0, BUTTONS[i]->Bchannel);
		  BUTTONS[i]->LedState = LOW;
		  P.displayText("V_0 ON Ch1 cc#", scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);
		  P.displayText((const char*)BUTTONS[i]->Bchannel, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);
		  P.displayText(StdBy, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);
		  break;
        case 1:
          MIDI.sendControlChange(BUTTONS[i]->Bvalue, 0, BUTTONS[i]->Bchannel);
		  BUTTONS[i]->LedState = LOW;
		  P.displayText("V_0 ON Ch1 cc#", scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);
		  P.displayText((const char*)BUTTONS[i]->Bchannel, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);
		  P.displayText(StdBy, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);
		  break;
      }
    }
  }
}
//***********************************************************************
void updatePots() {
	for (int i = 0; i < NUMBER_POTS; i++)
	{
		byte potmessage = POTS[i]->getValue();
		if (potmessage != 255) MIDI.sendControlChange(POTS[1]->Pcontrol, potmessage, POTS[1]->Pchannel);
	}
}

void updateLeds()
{
	for (int i = 0; i < NUMBER_BUTTONS; i = i + 1)
	{
		digitalWrite(BUTTONS[i]->LedPin, BUTTONS[i]->LedState);
	}

}



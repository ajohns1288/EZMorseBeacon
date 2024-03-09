/*
  EZMorse.h
  Author: A. Johnson
  Date: Nov 30 2022
  This is a library to enable one to send morse code without blocking the main function. It does this by running timers in the background and actuating pins when needed.
  The default setup is to use a PTT pin and PWM pin to output audio such as to a HT for a easy fox hunt. For true CW use, set the PTT pin to zero and the PWM pin will function as a on or off signal.
    To use this library, construct the object, then use the begin method. Use sendStr(String) to load a string into memory to send. Call loop() each loop to update the timers/pins.

	This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.

*/
#ifndef _CHAR2MORSE_H
#define _CHAR2MORSE_H

#include <Arduino.h>

#define DEBUG 0
#define USE_CW_MODE -1


//Length of dit in ms (dah is 3x this value, space is 7x)
#define DEFAULT_DIT_LENGTH 100
//Time between PTT activating and myCall transmitting
#define DEFAULT_PTT_DELAY 250

class EZMorse
{
public:
EZMorse(byte, byte);
void begin();
void setDitLength(int);
void setDitLengthWPM(int);
void dontWait(bool);
void setPTTDelay(int);
int sendStr(String);
bool isDone();
void loop();
void setPTT();
void clearPTT();
void calcTmrs(char);
char getMorse(char);
void setTone(int);
int sendToneFor(int);


private:
//String to send
String str2Send;

//HW variables
byte _pinPTT;
byte _pinAud;

//duration variables
int dit2use;
int pttDelay;

//PWM DC to use for tone
int toneDC;

//Timer variables
unsigned long delayTimer=0;
unsigned long delayStart=0;
unsigned long toneStart=0;
unsigned long toneTimer=30000;
unsigned long pttTimer=0;
byte charIdx=0;

//State variables
bool toneDone=true;
bool charDone=true;
bool forceTone=false;
bool allDone;
bool okToSend=true;
bool spaceNeeded=false;
bool quiet=false;
bool pttOK=false;
bool dontWaitTilDone;
};
#endif

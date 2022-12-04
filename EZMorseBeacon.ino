/*
	EZMorseBeacon.ino
	Copyright 2022 A. Johnson
	
	This program is for arduino type microcontrollers and is designed to act as a beacon when paired with a radio
	It does this by outputing morse code on a PWM pin and keying PTT on another pin. This way, you can use it as a beacon, fox, etc.
	Please ensure your hardware design prevents damage to the radio or micro. It is strongly recommended the outputs are isolated from the radio
	by means of an optocoupler (PTT) or isolation/blocking capacitor (Audio). A low pass filter (capacitor between pin and ground) is recommended
	on the audio line to reduce the higher harmonics of the square wave.
	
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


#include "char2morse.h"

//Hardware defines
//Note: AUD_PIN must be a PWM capable pin
#define AUD_PIN 9
#define PTT_PIN 13

//Timer defines

//Length of dit in ms (dah is 3x this value)
#define DIT_LENGTH 100
//Time between transmissions in ms
#define TONE_DELAY 3000
//Time between PTT activating and myCall transmitting
#define PTT_DELAY 250


//Put your call here, this will be broadcast every TONE_DELAY ms
char myCall[]="W1AW"; //Note: Right now letters, numbers, stroke, and most puntuation is supported. Spaces are not.

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
bool okToSend=true;
bool spaceNeeded=false;
bool quiet=false;
bool pttOK=false;

void setup() {
  Serial.begin(9600);
  pinMode(AUD_PIN,OUTPUT);
  pinMode(PTT_PIN,OUTPUT);

}

void loop() {
	
	//See if the PTT delay has expired, if it has, latch until cleared
  pttOK=((millis()-pttTimer>PTT_DELAY)||pttOK);
  
  //If we are not done with all the letters and we are OK to send
  if(charIdx<=strlen(myCall)&&okToSend)
  {
    
	//Calculate the timer values for each element of the character
	sendChar(getMorse(myCall[charIdx]));
	
	//If the tone has played long enough, we are done
    if(millis()-toneStart>toneTimer)
    {
      toneDone=true;
    }
	
	//For spaces in between elements (one dit) and letters (one dah), we want to make sure its quiet
    if(quiet)
    {
      analogWrite(AUD_PIN,0);
    }
    else
    {
      analogWrite(AUD_PIN,127);	
    }
  }
  //If we are done with all the letters, clear the okToSend and reset the character index and delay timer
  else if(okToSend)
  {
    okToSend=false;
    charIdx=0;
    delayStart=millis();
    analogWrite(AUD_PIN,0);
    clearPTT();

  }
  //If the delay timer is expired, we are ok to send, so key PTT
  else if(millis()-delayStart>TONE_DELAY&&!okToSend)
  {
    okToSend=true;
    setPTT();
  }

}

void setPTT()
{
    //Turn PTT pin on and start the PTT delay timer
	digitalWrite(PTT_PIN,HIGH);
    pttTimer=millis();
}

void clearPTT()
{
    //Turn PTT pin off and clear pttOK flag
	digitalWrite(PTT_PIN,LOW);
    pttOK=false;
}

void sendChar(char val)
{
  static char length=0;
  static char reg=0;

  if(charDone)
  {
    //If this is a new character, get the length (number of elements)
	if(val&0xE0==0xC0)
    {
	    //If first two bits of length are set, its 6, 3rd length bit is used for data
		length=6;
    }
    else
    {
	    //Otherwise we use three bits for length
		length=(val&0xE0)>>5;
    }
    
    //Load value into register only once and clear done flag
	reg=val;
	charDone=false;
    //Inc to next character
    charIdx++;
  }


//Here is where we update the tone timers. There are three cases, end of letter, end of element, and next element.
//If none of these are true, we do nothing. (The timer is still running though)
 if(length<0)
  {
  	//End of the character, so we need a quiet period of 1 dah/3 dits
	toneTimer=3*DIT_LENGTH;
	
	//Start tone timer and set state variables
	toneStart=millis();
	toneDone=false;
    quiet=true;
    charDone=true;
	spaceNeeded=false;
  }
  else if(spaceNeeded&&toneDone)
  {
  	//End of the element, so we need a quiet period of 1 dit
	toneTimer=DIT_LENGTH;
	
	//Start tone timer and set state variables
	toneStart=millis();
	toneDone=false;
    quiet=true;
	spaceNeeded=false;
  }
  else if(toneDone)
  {
    //If the current tone is done playing, determine next element based on 1 or 0 in reg
	(reg&1)?toneTimer=3*DIT_LENGTH:toneTimer=DIT_LENGTH;

	  //Shift register over for next element, dec the length since we sent an element
	  reg=reg>>1;
	  length--;
	  
	  //Start tone timer and set state variables
	  toneStart=millis();
	  toneDone=false;
      quiet=false;
      spaceNeeded=true;
  }



}

/*
  EZMorse.cpp
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


/*

  Low storage method for morse characters instead of using strings. (8bits to store a morse char)
  As the most elements a character has is 6, we can use the first three bits as a length and the last five as dits and dahs
  Since 0b110 is six, if the first two MSBs are 1, we use two bits for the size and use the third size bit as an additional character; Dahs are 1s, Dits are 0s.
  The array is sorted by ascii order, alphabet first, followed by ASCII 43 to 59 (+ to ;) followed by non-contigouse characters.
  This makes it easy to find an index based on the ascii value

These bytes are formated as follows: xxyz zzzz
xxy: size of the morse character (number of dits/dahs)
      If xx=0b11 (6), then yz zzzz are used as elements
      If xx is less than 6, xxy is the size and z zzzz are the elements
      1 is a dash and 0 is a dit, starting from the right (Example: A (.-) would be 10)
*/

#include "EZMorse.h"


const char morse[]={
0b01000010,
0b10000001,
0b10000101,
0b01100001,
0b00100000,
0b10000100, 
0b01100011,
0b10000000,
0b01000000,
0b10001110,
0b01100101,
0b10000010,
0b01000011,
0b01000001,
0b01100111,
0b10000110,
0b10001011,
0b01100010,
0b01100000,
0b00100001,
0b01100100,
0b10001000,
0b01100110,
0b10001001,
0b10001101,
0b10000011,
0b10101010,
0b11110011,
0b11100001,
0b11101010,
0b10101001,
0b10111111,
0b10111110,
0b10111100,
0b10111000,
0b10110000,
0b10100000,
0b10100001,
0b10100011,
0b10100111,
0b10101111,
0b11000111,
0b11010101,
0b10110001,
0b11001100,
0b11110101};

EZMorse::EZMorse(byte ptt, byte aud)
{
    _pinAud=aud;
    _pinPTT=ptt;

    if(_pinPTT==USE_CW_MODE)// CW mode: PTT pin=-1, otherwise use PTT+Audio mode
    {
        toneDC=255; //100% Duty cycle
    }
    else
    {
        toneDC=127; //50% Duty cycle
    }
        
    dit2use=DEFAULT_DIT_LENGTH ;
    pttDelay=DEFAULT_PTT_DELAY;
    dontWaitTilDone=false;
    allDone=true;
   
    
}

void EZMorse::begin()
{
  pinMode(_pinAud,OUTPUT);
  pinMode(_pinPTT,OUTPUT);
    
}

void EZMorse::setDitLength(int d)
//Sets length of dit in time
{
  dit2use=d;
}

void EZMorse::setDitLengthWPM(int wpm)
//Sets length of dit as WPM
{
  dit2use=(int)(60000/(50*wpm));
}

void EZMorse::dontWait(bool b)
{
  dontWaitTilDone=b;
}

void EZMorse::setPTTDelay(int t)
{
  pttDelay=t;
}

int EZMorse::sendStr(String s)
{
    //We won't honor a new string until existing one is done sending     
    if(allDone||dontWaitTilDone)
    {
        str2Send=s; // Set string

        //Turn off pins      
        analogWrite(_pinAud,0);
        clearPTT();  

        //Reset state variables
        charIdx=0;
        allDone=false;
        charDone=true;
        setPTT();

        return 1;
    }
    else
    {
        return 0;
    }
}

bool EZMorse::isDone()
{
    return allDone;
}

int EZMorse::sendToneFor(int i)
{
  if(allDone)
  {
    clearPTT();
    setPTT();
  forceTone=true;
  allDone=false;
  toneStart=millis();
  toneTimer=i;
  okToSend=false;
  quiet=false;
  charIdx=99;
  return 1;
  }
  else
  {
    return 0;
  }
}

void EZMorse::loop()
{
  //See if the PTT delay has expired, if it has, latch until cleared (delay allows for XMIT to key up)
  pttOK=((millis()-pttTimer>pttDelay)||pttOK)&&!allDone;
  
  //If we are not done with all the letters and we are OK to send
  if((charIdx<str2Send.length()||forceTone)&&okToSend)
  {
    
	//Calculate the timer values for each element of the character
	if(forceTone)
	{
	  //do nothing, we already set the timers.
	}
 else
 {
  calcTmrs(getMorse(str2Send[charIdx]));
 }
    
    
	//For spaces in between elements (one dit) and letters (one dah), we want to make sure its quiet
    if(quiet)
    {
      analogWrite(_pinAud,0);
    }
    else
    {
      analogWrite(_pinAud, toneDC);	
    }

	//If the tone has played long enough, we are done
    if(millis()-toneStart>toneTimer)
    {           
        toneDone=true;
        if(forceTone)
        {
          forceTone=false;
          okToSend=false;
          allDone=true;
          clearPTT();
        }
        
        analogWrite(_pinAud,0);
    }
  }
  //If we are done with all the letters, clear the okToSend and reset the character index and delay timer
  else if(okToSend)
  {
    okToSend=false;
    allDone=true;
    charIdx=0;
    analogWrite(_pinAud,0);
    clearPTT();
    
  }
  else if(pttOK&&!okToSend)
  {
    okToSend=true;
  }
}

void EZMorse::setPTT()
{
    if(_pinPTT!=USE_CW_MODE)
    {     
    //Turn PTT pin on and start the PTT delay timer
	digitalWrite(_pinPTT,HIGH);
    pttTimer=millis();
    
    }
    else
    {
        pttOK=true; //CW mode, allow transmision instantly
        
    }
}

void EZMorse::clearPTT()
{
    if(_pinPTT!=USE_CW_MODE)
    {    
    //Turn PTT pin off and clear pttOK flag
	digitalWrite(_pinPTT,LOW);
    
    }
    pttOK=false;
}

void EZMorse::setTone(int t)
{
    toneDC=t;
}

void EZMorse::calcTmrs(char val)
{
  static char length=0;
  static char reg=0;

  if(val==0) //Space or other error
  {
    //Space is 7x Dit length
	toneTimer=dit2use*7;
	
	//Start tone timer and set state variables
	toneStart=millis();
	toneDone=false;
    charDone=true;
    quiet=true;
	spaceNeeded=false;
    //Inc to next character
    charIdx++;
    return;
  }

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
  }


//Here is where we update the tone timers. There are three cases, end of letter, end of element, and next element.
//If none of these are true, we do nothing. (The timer is still running though)
 if(length<0)
  {
  //End of the character, so we need a quiet period of 1 dah/3 dits
	toneTimer=3*dit2use;
	
	//Start tone timer and set state variables
	toneStart=millis();
	toneDone=false;
    quiet=true;
    charDone=true;
	spaceNeeded=false;
        //Inc to next character
    charIdx++;
  }
  else if(spaceNeeded&&toneDone)
  {
  	//End of the element, so we need a quiet period of 1 dit
	toneTimer=dit2use;
	
	//Start tone timer and set state variables
	toneStart=millis();
	toneDone=false;
    quiet=true;
	spaceNeeded=false;
  }
  else if(toneDone)
  {
    //If the current tone is done playing, determine next element based on 1 or 0 in reg
	if(reg&1)
    {
toneTimer=3*dit2use;
    
}
    else
    {
toneTimer=dit2use;
}

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

char EZMorse::getMorse(char c)
{

char index=0;

//convert lower to uppercase
if(c>=97&&c<=122)
{
	c-=32;
}

//Capital letters
if(c>=65&&c<=90)
{
   index=c-65;
}
//Numbers/punctuation
else if(c>=43&&c<=59)
{
	index=(c-17);
}
else if(c==61)
{
	index=42;
}
else if(c==63)
{
	index=43;
}
else if(c==33)
{
	index=44;
}
else
{
    //ie: send a space	
    return 0;
}

return morse[index];

}

/*
  char2morse.cpp
  Author: A. Johnson
  This is a low storage method for morse characters instead of using strings.
  As the most elements a character has is 6, we can use the first three bits as a length and the last five as dits and dahs
  Since 0b110 is six, if the first two MSBs are 1, we use two bits for the size and use the third size bit as an additional character; Dahs are 1s, Dits are 0s.
  The array is sorted by ascii order, alphabet first, followed by ASCII 43 to 59 (+ to ;) followed by non-contigouse characters.
  This makes it easy to find an index based on the ascii value
*/


/*
These bytes are formated as follows: xxyz zzzz
xxy: size of the morse character (number of dits/dahs)
      If xx=0b11 (6), then yz zzzz are used as elements
      If xx is less than 6, xxy is the size and z zzzz are the elements
      1 is a dash and 0 is a dit, starting from the right (Example: A (.-) would be 10)
*/

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

char getMorse(char c)
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
	return 0;
}

return morse[index];

}
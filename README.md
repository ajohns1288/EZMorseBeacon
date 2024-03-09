# EZMorseBeacon
This program is for Arduino type microcontrollers and is designed to act as a beacon when paired with a radio.

It does this by outputing morse code on a PWM pin and keying PTT on another pin. This way, you can use it as a beacon, fox, etc. Please ensure your hardware design prevents damage to the radio or micro. It is strongly recommended the outputs are isolated from the radio by means of an optocoupler (PTT) or isolation/blocking capacitor (Audio). A low pass filter (capacitor between pin and ground) is recommended on the audio line to reduce the higher harmonics of the square wave.

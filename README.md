# EZMorseBeacon
This program is for Arduino type microcontrollers and is designed to act as a beacon when paired with a radio.

It does this by outputing morse code on a PWM pin and keying PTT on another pin. This way, you can use it as a beacon, fox, etc. Please ensure your hardware design prevents damage to the radio or micro. It is strongly recommended the outputs are isolated from the radio by means of an optocoupler (PTT) or isolation/blocking capacitor (Audio). A low pass filter (capacitor between pin and ground) is recommended on the audio line to reduce the higher harmonics of the square wave.

Also included in the hardware folder is a KiCad project of a 3.3V coin cell powered AtTiny85 PCB that can be used as the hardware portion of this project. I have not built it yet so I cannot guarantee it works.

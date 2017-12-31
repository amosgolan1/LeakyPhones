/*
 * IRremote: IRsendDemo - demonstrates sending IR codes with IRsend
 * An IR LED must be connected to Arduino PWM pin 3.
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 * JVC and Panasonic protocol added by Kristian Lauszus (Thanks to zenwheel and other people at the original blog post)
 */
#include <IRremote.h>
IRsend irsend;

//ada radio setting//
#include <Wire.h>
#include <Adafruit_Si4713.h>

#define RESETPIN 12
#define FMSTATION 9740     // 9800=98.00Hz

Adafruit_Si4713 radio = Adafruit_Si4713(RESETPIN);


void setup()
{
  Serial.begin(9600);
  if (! radio.begin()) {  // begin with address 0x63 (CS high default)
    Serial.println("Couldn't find radio?");
    while (1);}
    radio.setTXpower(115);
    radio.tuneFM(FMSTATION); // 98.00 mhz
    Serial.println("tuned to FM STATION");
}


/** 
 *  convert FCC Channel number [200..300] into 'tens of khz' number as used in the SI4713 FM Library * 
 */
int freqToChannel(int freq) {
  return (freq - 8800) / 20;
}

/** 
 *  convert ferquency in 'tens of khz' number as used in the SI4713 FM Library into FCC Channel number [200..300] 
 */
int channelToFreq(int channel) {
  return 8800 + channel * 20;
}


/** 
 *  gets a number of a channel from 200..300 and transmits its AMOS-Code over IR
 */
void sendChannel(int freq) {
  //do i need 8 bit or 32 bit here? is the numbr the length of the NEC code (always 32) or the lenght of the massage?
  irsend.sendNEC(freqToChannel(freq), 16);
}

void loop() { 
    sendChannel(FMSTATION);
    //Serial.println(FMSTATION);
    delay(42);


}



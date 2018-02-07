/*
 * IRremote: IRsendDemo - demonstrates sending IR codes with IRsend
 * An IR LED must be connected to Arduino PWM pin 3.
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 * JVC and Panasonic protocol added by Kristian Lauszus (Thanks to zenwheel and other people at the original blog post)
 */
#include <IRremote.h>
#define DELAY_TIME 42

IRsend irsend;
//transmitter setting
#include <Wire.h>
#include <Adafruit_Si4713.h>

#define RESETPIN 12
//#define CHANNEL 50//107FM 
# define FMSTATION 10700 //107FM

Adafruit_Si4713 radio = Adafruit_Si4713(RESETPIN);

void setup()
{
    Serial.begin(9600);
    Serial.println("AMOS_FM TX V0.3");
    Serial.println("---------------");
    Serial.println("Setting up radio...");
    
    if (! radio.begin()) {  // begin with address 0x63 (CS high default)
    Serial.println("Couldn't find radio?");
    while (1);}
    radio.setTXpower(115);  // dBuV, 88-115 max
    
    // radio.tuneFM(channelToFreq(CHANNEL)); // 102.3 mhz
    //Serial.println(channelToFreq(CHANNEL));
    radio.tuneFM(FMSTATION); // 107.00 mhz
    Serial.print("IR TX Delay set to: ");
    Serial.println(DELAY_TIME);
    Serial.println("Started.");

    //remember that i had an issue with reset. perhaps worth resetiing a couple of times upon turn on
}


/** 
 *  convert FCC Channel number [200..300] into 'tens of khz' number as used in the SI4713 FM Library * 
 */
int16_t channelToFreq(int16_t channel) {
  return 8790 + 20*(channel-200);
}

/** 
 *  convert ferquency in 'tens of khz' number as used in the SI4713 FM Library into FCC Channel number [200..300] 
 */
int16_t freqToChannel(int16_t freq) {
  return 200 + (freq-8790) / 20;
}


/** 
 *  gets a number of a channel from 200..300 and transmits its Code over IR
 */
void sendChannel(int freq) {
  irsend.sendNEC(freqToChannel(freq), 16); 
}

void loop() {
  sendChannel(FMSTATION);
  Serial.println();
  long lastSentTime = millis();
  //Serial.println("sent");
  delay(DELAY_TIME);
}

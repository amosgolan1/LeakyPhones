/*
 * IRremote: IRsendDemo - demonstrates sending IR codes with IRsend
 * An IR LED must be connected to Arduino PWM pin 3.
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 * JVC and Panasonic protocol added by Kristian Lauszus (Thanks to zenwheel and other people at the original blog post)
 */
#include <IRremote.h>
 
#define PanasonicAddress      0x4004     // Panasonic address (Pre data) 
#define PanasonicPower        0x100BCBD  // Panasonic Power button

#define JVCPower              0xC5E8

IRsend irsend;

void setup()
{
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
 *  gets a number of a channel from 200..300 and converts it to a 16-bit AMOS-Code.
 */
int16_t channelToAMOS(int16_t channel) {
  int16_t base = (channel-200);
  int16_t result = base + (base << 8);  
  return result;
}

/** 
 *  gets a number of a channel from 200..300 and transmits its AMOS-Code over IR
 */
void sendChannel(int16_t channel) {
  irsend.sendNEC(channelToAMOS(channel), 16); 
}

void loop() {
//  irsend.sendPanasonic(PanasonicAddress,PanasonicPower); // This should turn your TV on and off
  
//  irsend.sendJVC(JVCPower, 16,0); // hex value, 16 bits, no repeat

  //irsend.sendNEC(0xFEEE, 16); // hex value, 16 bits, no repeat
  sendChannel(225);
//  delayMicroseconds(50); // see http://www.sbprojects.com/knowledge/ir/jvc.php for information
//  irsend.sendJVC(JVCPower, 16,1); // hex value, 16 bits, repeat
//  delayMicroseconds(50);

  delay(100);
  irsend.sendNEC(0xFEFE, 16); // hex value, 16 bits, no repeat
  delay(400);

}

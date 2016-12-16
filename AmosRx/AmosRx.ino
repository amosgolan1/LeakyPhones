#include <boarddefs.h>
#include <ir_Lego_PF_BitStreamEncoder.h>
#include <IRremote.h>
#include <IRremoteInt.h>
#include <Si4703_Breakout.h>
#include <Wire.h>

int resetPin = 2;
int SDIO = A4;
int SCLK = A5;

#define DEFAULT_STATION 9290
#define DEFAULT_CHANNEL freqToChannel(DEFAULT_STATION)
#define MILLIS_BEFORE_RETURN_TO_DEFUALT 1400

Si4703_Breakout radio(resetPin, SDIO, SCLK);
int radiochannel;
int volume;
long lastValidReceived;

/*
 * IRremote: IRrecvDemo - demonstrates receiving IR codes with IRrecv
 * An IR detector/demodulator must be connected to the input RECV_PIN.
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 */

int RECV_PIN = 11;

#define AMOS_INVALID 0

IRrecv irrecv(RECV_PIN);

decode_results results;

void setup()
{
  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver

  radio.powerOn();
  radio.setVolume(7);
}


void dump(decode_results *results) {
  // Dumps out the decode_results structure.
  // Call this after IRrecv::decode()
  int count = results->rawlen;
  if (results->decode_type == UNKNOWN) {
    Serial.print("Unknown encoding: ");
  }
  else if (results->decode_type == NEC) {
    Serial.print("Decoded NEC: ");

  }
  else if (results->decode_type == SONY) {
    Serial.print("Decoded SONY: ");
  }
  else if (results->decode_type == RC5) {
    Serial.print("Decoded RC5: ");
  }
  else if (results->decode_type == RC6) {
    Serial.print("Decoded RC6: ");
  }
  else if (results->decode_type == PANASONIC) {
    Serial.print("Decoded PANASONIC - Address: ");
    Serial.print(results->address, HEX);
    Serial.print(" Value: ");
  }
  else if (results->decode_type == LG) {
    Serial.print("Decoded LG: ");
  }
  else if (results->decode_type == JVC) {
    Serial.print("Decoded JVC: ");
  }
  else if (results->decode_type == AIWA_RC_T501) {
    Serial.print("Decoded AIWA RC T501: ");
  }
  else if (results->decode_type == WHYNTER) {
    Serial.print("Decoded Whynter: ");
  }
  Serial.print(results->value, HEX);
  Serial.print(" (");
  Serial.print(results->bits, DEC);
  Serial.println(" bits)");
  Serial.print("Raw (");
  Serial.print(count, DEC);
  Serial.print("): ");

  for (int i = 1; i < count; i++) {
    if (i & 1) {
      Serial.print(results->rawbuf[i]*USECPERTICK, DEC);
    }
    else {
      Serial.write('-');
      Serial.print((unsigned long) results->rawbuf[i]*USECPERTICK, DEC);
    }
    Serial.print(" ");
  }
  Serial.println();
}

/**
 * convert channel from SI4713 library format (adafruit) to SI4703 format (sparkfun)
 */
int16_t adafruitToSparkfun(int16_t channel) {
  return channel/10;
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
 *  gets a 16-bit AMOS-Code and converts it to a number of a channel from 200..300, or returns AMOS_INVALID if not a valid amos code.
 */
int16_t amosToChannel(int16_t amos) {
  if (isValidAmos(amos)) {
    int16_t amosBase = amos & 0x00FF;
    int16_t result = amosBase + 200;
    return result;
  }
  else 
  {
    return AMOS_INVALID;
  }
}

/**
 * return true iff the code is a valid AMOS-Code...
 */
bool isValidAmos(int16_t amos) {
  return (((0xFF00 & amos) >> 8) == (0x00FF & amos)) 
          && ((0x00FF & amos) < 101);
}

/**
 * retune SI4703 to given FCC channel, iff not already at that channel;
 */
void tuneIfNeeded(int16_t channel) {
  int16_t newRadioChannel = adafruitToSparkfun(channelToFreq(channel));
  if (radiochannel != newRadioChannel) {
    radiochannel = newRadioChannel;
     radio.setChannel(radiochannel);
  }
}

void printStatus() {
  Serial.println("----");
  Serial.print("Current radio channel: ");
  Serial.println(radiochannel);
  Serial.print("Time since last valid receive: ");
  Serial.println(timeSinceLastValidReceive());
  Serial.println("----");
}

long timeSinceLastValidReceive() {
  return millis() - lastValidReceived;
}

//void 

void loop() {
  
   
  if (irrecv.decode(&results)) {
    //we receieved something
    if (results.decode_type == JVC) {
      //it is of the correct encoding
      
      //Serial.println(results.value, HEX);
      int16_t decodedAmos = amosToChannel(results.value);
      Serial.println(decodedAmos, DEC);
      if (decodedAmos != AMOS_INVALID) {
        //it is a valid amos code
        tuneIfNeeded(decodedAmos);

        //record this moment as the last valid receive
        lastValidReceived=millis();
        
      }//if valid amos
       
    }//correct encoding
    
    //dump(&results);
    irrecv.resume(); // Receive the next value
    
  }// received some ir

  if ( timeSinceLastValidReceive() > MILLIS_BEFORE_RETURN_TO_DEFUALT) {
    tuneIfNeeded(DEFAULT_CHANNEL);
  }

  printStatus();
  delay(500);
}

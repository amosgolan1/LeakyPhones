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

#define TIME_PER_IR_SEND 108
#define MILLIS_BEFORE_RETURN_TO_DEFUALT 3*TIME_PER_IR_SEND

Si4703_Breakout radio(resetPin, SDIO, SCLK);
int radioChannel;
int radioVolume;

int16_t lastValidAmos_Code;
long lastValidAmos_Time;


//utility variables for changing radio channels
int rcChange_DestChannel;
long rcChange_Time;

//utility variables for changing volume
long volChange_StartTime;
long volChange_DurationMs;
int volChange_StartVol;
int volChange_EndVol;

#define NEXTCHANNEL_NONE 0
int nextChannel = NEXTCHANNEL_NONE;

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

  Serial.println("AMOS FM RX V0.2");

//  rcChange_DestChannel;
//  rcChange_Time;

  volChange_StartTime = 0;
  volChange_DurationMs = 0;
  volChange_StartVol = 0;
  volChange_EndVol = 7;

  radio.powerOn();
  setVolumeIfNeeded(7);
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
 * schedule immediate tune of SI4703 to given FCC channel, iff not already at that channel;
 */
void tuneIfNeeded(int16_t channel) {
  int16_t newRadioChannel = adafruitToSparkfun(channelToFreq(channel));
  if (radioChannel != newRadioChannel) {
    rcChange_DestChannel = newRadioChannel;
    rcChange_Time = millis(); //now
  }
}

/**
 * make sure we're tuned to the currently scheduled station, must be periodically called from the loop.
 */
void tuneIfScheduled() {
  if (radioChannel != rcChange_DestChannel) {
    //channel change is needed
    if (millis() > rcChange_Time) {
      //it's past the time for the channel change
      radioChannel = rcChange_DestChannel;
      radio.setChannel(radioChannel);
    }
  }
}

/**
 * sets the radio volume to the given vol [0..15], and takes care of housekeeping of our internal variables
 */
void setVolumeIfNeeded(int vol) {
  if (radioVolume != vol) {
    radioVolume = vol;
    radio.setVolume(radioVolume);
  }
}

void setVolumeByTime() {
  long currentTime = millis();
  if (currentTime < volChange_StartTime) {
    return; //nothing to do yet;
  }

  long endTime = volChange_StartTime + volChange_DurationMs; 
  Serial.print("T:");
  Serial.print(volChange_StartTime);
  Serial.print("\t");
  Serial.print(volChange_DurationMs);
  Serial.print("\t");
  Serial.print(endTime);
  
  if (currentTime >= endTime) {
    //we're past the end time... make sure volume is set.
    setVolumeIfNeeded(volChange_EndVol);
    
  } else {
    
    //we know we're after the start time and before the end time...
    float newVolume = 
        ((currentTime - volChange_StartTime)/(1.0f*volChange_DurationMs)) * volChange_EndVol
        +
        ((endTime - currentTime)/(1.0f*volChange_DurationMs)) * volChange_StartVol;
    Serial.print("New Volume:");
    Serial.print(newVolume);
    setVolumeIfNeeded((int)newVolume);           
  }

}

void printStatus() {

  Serial.print("nc:");
  Serial.print(nextChannel);
  Serial.print("rChan:");
  Serial.print(radioChannel);
  //Serial.print("\tLast received valid code: ");
  //Serial.print(lastValidAmos_Code,HEX);
  Serial.print("\t Vol:");
  Serial.print(radioVolume);
  Serial.print("\ttLVR:");
  Serial.print(timeSinceLastValidReceive());

  Serial.println();
  //Serial.println("----");
}

long timeSinceLastValidReceive() {
  return millis() - lastValidAmos_Time;
}

/**
 * set the channel to switch within given ms
 */
void setTuneFuture(int channel, long ms) {
  rcChange_DestChannel = channel;
  rcChange_Time = millis() + ms;
}

/**
 * set the volume to fade to a certain level, with given pace in ms/step, starting immediate
 */
void setVolumeFuture(int vol, long msPerStep) {
 volChange_StartTime = millis();
 volChange_DurationMs = msPerStep * abs(radioVolume-vol);
 volChange_StartVol = radioVolume;
 volChange_EndVol = vol;
}


void loop() {

  ////>>> HOUSEKEEPING -- MUST RUN FREQUENTLY
  tuneIfScheduled();
  setVolumeByTime();
  
  if (0==radioVolume && (nextChannel != NEXTCHANNEL_NONE)) {
    Serial.println("RETUNING");
    tuneIfNeeded(nextChannel);
    setVolumeFuture(7,100);
    
    nextChannel = NEXTCHANNEL_NONE;
  }
  ///<<<<

  static bool tunedToIR = false;
     
  if (irrecv.decode(&results)) {
    //we receieved something
    if (results.decode_type == JVC) {
      //it is of the correct encoding
      
      //Serial.println(results.value, HEX);
      int16_t decodedAmos = amosToChannel(results.value);
      //Serial.println(decodedAmos, DEC);
      if (decodedAmos != AMOS_INVALID) {
        //it is a valid amos code
        if (!tunedToIR) {
          tunedToIR = true;
          
          //tuneIfNeeded(decodedAmos);//setTuneFuture(decodedAmos,0);
          //setVolumeFuture(7,333); 
          Serial.println("Switching by IR code!");

          if (radioChannel == adafruitToSparkfun(channelToFreq(decodedAmos))) {
            setVolumeFuture(7,100);
          } else {
            nextChannel = decodedAmos;
            setVolumeFuture(0,100);
          }
        }

        //record this moment as the last valid receive
        lastValidAmos_Code=results.value;
        lastValidAmos_Time=millis();
        
      }//if valid amos
       
    }//correct encoding
    
    //dump(&results);
    irrecv.resume(); // Receive the next value
    
  } // received some ir

  
  if ( timeSinceLastValidReceive() > MILLIS_BEFORE_RETURN_TO_DEFUALT) {
    //tuneIfNeeded(DEFAULT_CHANNEL);
    if (tunedToIR) {
      Serial.println("Switching by LACK OF IR code!");
      tunedToIR = false;
      
      if (radioChannel == adafruitToSparkfun(channelToFreq(DEFAULT_CHANNEL))) {
        setVolumeFuture(7,100);
      } else {
        nextChannel = DEFAULT_CHANNEL;
        setVolumeFuture(0,100);
      }
    }
  }

  printStatus();
  delay(50);
}

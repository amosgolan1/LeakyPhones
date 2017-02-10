//IR LED must be connected to Arduino PWM pin 3.
// IR receiver connected to pin 11

#include <Wire.h>
#include <IRremote.h>

#define RECV_PIN 7

IRsend irsend;
IRrecv irrecv(RECV_PIN);
decode_results results;

//variables to track and control mixing
int mixPosition = 0; //0 - self, 255 - other

const int steps = 256;//number of steps in the digipot
long duration = 2560;//mixing duration from volume 0 to volume 255
long step_dureation = duration / steps;//duration of each digipot step

//addition
int noSignalCount = 0;
#define HOW_MANY_SIGNALS_TO_MISS_BEFORE_MIXING_BACK_TO_SELF 20

//end addition

//mcp42010 variables
#include "AH_MCP41xxx.h"
//#define DATAOUT  11   //uno MOSI , IC SI
//#define SPICLOCK 13   //uno SCK  , IC SCK
#define CS   10   //chipselect pin
#define SHDN 9   //shutdown pin
#define RS   8   //reset pin

#define POTIOMETER_SELF 0
#define POTIOMETER_OTHER 1

AH_MCP41xxx mcp42010;

int irTarget; // The IR we are currently tuned to as 'other'. Can only change if mix is at 100% self.
long timeSinceLastSeenValidIR; //updated every time an IR signal is received AND equals the irTarget

/**
 * Set the potentiometers to the desired mix setting. [0..255]
 */
int __lastMixTo = -1; //initialized to impossible value
void setMix(int mixTo) {
  //TODO: right now this works because the mixTo range [0..255] is the same as number of steps in the pots. In the future, this should be dynamic...

  if (__lastMixTo!=mixTo) {
    mcp42010.setValue(255-mixTo, POTIOMETER_SELF);
    mcp42010.setValue(mixTo, POTIOMETER_OTHER);
    __lastMixTo = mixTo;
  } 
}

int getMix() {
  return __lastMixTo;
}

void setup()
{
  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
  Serial.println("receiver on");
  mcp42010.init_MCP42xxx (CS, SHDN, RS);  //initialisation
  delay(100);
}


/**
    convert ferquency in 'tens of khz' number as used in the SI4713 FM Library into a number in the range [0..100]
    returns 0 for 8800 and 100 for 10800
*/
int freqToChannel(int freq) {
  return (freq - 8800) / 20;
}

/**
    convert channel in the range [0..100]
    into a frequancy in 'tens of khz' ranging from 8800 to 10800
*/
int channelToFreq(int channel) {
  return 8800 + channel * 20;
}
/**
    gets a frequancy number of a channel from 0..100 and transmits it over IR using NEC protocol (32 bits, 8 adress, 8 inverse of adress, 8 data, 8 inverse of data)
*/
void sendChannel(int freq) {
  //do i need 8 bit or 32 bit here? is the numbr the length of the NEC code (always 32) or the lenght of the massage?
  irsend.sendNEC(freqToChannel(freq), 8);
}

/**
 * General principle of operation:
 * Each loop:
 * 
 * receiving IR signal?
 * Yes:
 *  am I at MIX=SELF 100%? 
 *    Yes: 
 *      set IR Target.
 *      set last valid IR time.
 *    No:
 *      is IR equal to IR Target?
 *        Yes:
 *          set last valid IR time.
 *        No: 
 *          no need to do anything, the mixing back towards 'self' should occur based on last-valid-IR-time expiration.
 * No: 
 *  no need to do anything, mixing should behave appropriately based on last-valid-IR-time expiration. 
 * 
 * -------------
 * 
 * //Should we adjust target based on expiration time having passed/not-passed
 * if (last valid IR time expired && mixTarget != SELF)
 *      Set mixTarget to 'SELF';
 *      Compute & set mix start and end goal-times.;
 * if (last valid IR time NOT expired && mixTarget != OTHER)
 *      Set mixTarget to 'OTHER';
 *      Compute & set mix start and end goal-times.;
 * 
 * Should we be mixing? (if mix != mixTarget) 
 *  set mix accroding to start and end goal times
 */

/**
 * return channel value by IR received (if valid), or -1 otherwise
 */
int IR_Received() {
  int result = -1;
  
  if (irrecv.decode(&results) == true) {
    if (results.decode_type == JVC) {
      int channel = results.value;
      if (channel >= 0 && channel <= 100) {
        result = channel; //otherwise -1 will be returned (see bottom)
      }          
    }
    irrecv.resume();
  } 
  return result;
}

#define OTHER 255
#define SELF 0
int mixTarget = SELF;
int IRTarget = -1;
long lastValidIRTime = 0;

#define IR_EXPIRATION_MS 300

long startMixTime;
int startMixValue;
double mixRateMsPerStep;

void loop() {
  //take care of IR
  int currentMix = getMix();
  int channelReceived = IR_Received();
  long currentTime = millis();

//  Serial.print(channelReceived);
//  Serial.print("\t");
//  Serial.println(currentMix);
  
  if (-1!=channelReceived) {
    if (channelReceived == IRTarget || SELF==currentMix) {
      IRTarget = channelReceived;
      lastValidIRTime = currentTime;
    } 
  }

  if ((currentTime-lastValidIRTime)>IR_EXPIRATION_MS && mixTarget != SELF) {
    mixTarget = SELF;
    startMixTime = currentTime;
    startMixValue = currentMix;
    mixRateMsPerStep = 5.0; 
  }

  if ((currentTime-lastValidIRTime)<=IR_EXPIRATION_MS && mixTarget != OTHER) {
    mixTarget = OTHER;
    startMixTime = currentTime;
    startMixValue = currentMix;
    mixRateMsPerStep = 10.0; 
  }

  if (currentMix!=mixTarget) {
    int mixDirection = (mixTarget==SELF) ? -1 : 1; 
    int mixNewValueRaw = startMixValue + mixDirection * ((currentTime - startMixTime) / mixRateMsPerStep);
    int mixNewValue = max(SELF,min(OTHER,mixNewValueRaw));
    Serial.print(mixDirection);
    Serial.print("\t");
    Serial.print(mixNewValueRaw);
    Serial.print("\t");
    Serial.println(mixNewValue);
    
    setMix(mixNewValue);
  }
  
}


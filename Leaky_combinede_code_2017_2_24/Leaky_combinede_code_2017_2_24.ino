/*
   DO NOT FORGET TO CONNECT FM RECEIVER TO 3.3V AND NOT TO 5V

  Pin connection:
  IR receiver:
  IR receiver connection: from left to right when it is facing you:
  1. out connected to pin 7 , 2. GND, 3.Vs
  IR transmitter:
  positive goes to pin 3, negative to ground
  radio transmitter Si4713:
  Clock to A5, Data to A4, Vin 3.3V, RST to 12
  radio receiver:
  Clock to A5, Data to A4, Vin to 3.3V  RST to 2
  /**
   General principle of operation:
   Each loop:

   receiving IR signal?
   Yes:
    am I at MIX=SELF 100%?
      Yes:
        set IR Target.
        set last valid IR time.
      No:
        is IR equal to IR Target?
          Yes:
            set last valid IR time.
          No:
            no need to do anything, the mixing back towards 'self' should occur based on last-valid-IR-time expiration.
   No:
    no need to do anything, mixing should behave appropriately based on last-valid-IR-time expiration.

   -------------

   //Should we adjust target based on expiration time having passed/not-passed
   if (last valid IR time expired && mixTarget != SELF)
        Set mixTarget to 'SELF';
        Compute & set mix start and end goal-times.;
   if (last valid IR time NOT expired && mixTarget != OTHER)
        Set mixTarget to 'OTHER';
        Compute & set mix start and end goal-times.;

   Should we be mixing? (if mix != mixTarget)
    set mix accroding to start and end goal times
*/

/**
   return channel value by IR received (if valid), or -1 otherwise
*/
//TODO need to check how to reduce the send delay
//need to make transition more smooth and control length of ttransition more accuratly
#include <IRLibSendBase.h>
#include <IRLibDecodeBase.h>
#include <IRLib_P01_NEC.h>
#include <IRLibCombo.h>
#include <IRLibRecv.h>

#include <Wire.h>
#include <Si4703_Breakout.h>//receiver
#include <Adafruit_Si4713.h>//trasmitter

//Si4713 (blue) transmitter setup. clock=A5, Data=A4 reset=12
#define MYFMSTATION 10700//change this according to this user's  TRANSMITION frquancy
#define DELAY_TIME 100//added delay time for IR transmitter. should be longer then 68 ms for NEC protocol
#define RESETPIN 12//reset pin for the radio transmitter
Adafruit_Si4713 radioTransmitter = Adafruit_Si4713(RESETPIN);

//tracking time between IR transmittions
unsigned long previousMillis = 0;

//Si4703 (red) radio  receiver
int resetPin = 2;
int SDIO = A4;
int SCLK = A5;
Si4703_Breakout radio(resetPin, SDIO, SCLK);

//IR receiver
#define RECV_PIN 7
IRrecv myReceiver(RECV_PIN);
IRdecode myDecoder;   //create decoder in the future i can use:"IRdecodeNEC My_Decoder; and this will save space on on arduino when decoding

//IR transmitter pin 3 connects to left
IRsend mySender;

//decode_results results;

//variables to track and control mixing
int mixPosition = 0; //0 - self, 255 - other
const int steps = 256;//number of steps in the digipot

//mcp42010 variables
#include "AH_MCP41xxx.h"
//#define DATAOUT  11   //uno MOSI , IC SI from librery
//#define SPICLOCK 13   //uno SCK  , IC SCK
#define CS_1   10   //chipselect pin for pot 1
#define SHDN    9   //shutdown pin
#define RS    8   //reset pin
#define CS_2    5   //chipselect pin for pot 2

#define POTIOMETER_SELF 0
#define POTIOMETER_OTHER 1

AH_MCP41xxx mcp42010_1;
AH_MCP41xxx mcp42010_2;


const int LOOKUP[] =
{ 250, 249, 248, 247, 246, 245, 244, 243,
  239, 233, 230, 227, 223, 215, 210, 198,
  191, 184, 175, 165, 154, 142, 128, 112,
  95, 75, 53, 28, 8, 0
};

//The number of steps of the transition is the number of datapoints in the lookup table:
int lookupSteps = sizeof(LOOKUP) / sizeof(int);

/**
   Set the potentiometers to the desired mix setting. [0..255]
*/
int __lastMixTo = -1; //initialized to impossible value.

void setMix(int mixTo) {
  if (__lastMixTo != mixTo) {
    if (mixTo == 0) {
      //volume value is 0 make sure digipot 1 is on, and 2 is off
      mcp42010_1.shutdown(1);
      mcp42010_2.shutdown(0);

      mcp42010_1.setValue(255, POTIOMETER_SELF);
      mcp42010_2.setValue(0, POTIOMETER_OTHER);
      mcp42010_1.setValue(255, POTIOMETER_SELF);
      mcp42010_2.setValue(0, POTIOMETER_OTHER);
    }
    else if (mixTo == 255) {
      //volume value is 255 make sure digipot 1 is off, and 2 is on
      mcp42010_1.shutdown(0);
      mcp42010_2.shutdown(1);

      mcp42010_1.setValue(0, POTIOMETER_OTHER);
      mcp42010_2.setValue(255, POTIOMETER_SELF);
      mcp42010_1.setValue(0, POTIOMETER_OTHER);
      mcp42010_2.setValue(255, POTIOMETER_SELF);
    }
    else {
      //volume value is 1-254 make sure digipots are on
      mcp42010_1.shutdown(1);
      mcp42010_2.shutdown(1);

      mcp42010_1.setValue(255 - LOOKUP[mixTo], POTIOMETER_SELF);
      mcp42010_2.setValue(LOOKUP[mixTo], POTIOMETER_OTHER);
      mcp42010_1.setValue(255 - LOOKUP[mixTo], POTIOMETER_SELF);
      mcp42010_2.setValue(LOOKUP[mixTo], POTIOMETER_OTHER);
    }
    __lastMixTo = mixTo;
  }
}

int getMix() {
  return __lastMixTo;//__lastMixTo is a value 0-255
}

void setup()
{
  Serial.begin(9600);

  myReceiver.enableIRIn(); // Start the receiver
  Serial.println("IR receiver on");

  mcp42010_1.init_MCP42xxx (CS_1, SHDN, RS);  //mcp42010_1 initialisation
  Serial.println("mixer 1 on");

  mcp42010_2.init_MCP42xxx (CS_2, SHDN, RS);  //mcp42010_2 initialisation
  Serial.println("mixer 2 on");

  //from TX code: setting up the radio transmittion
  if (! radioTransmitter.begin()) {  // begin with address 0x63 (CS high default)
    Serial.println("Couldn't find Adafruit radioTransmitter");
    while (1);
  }
  //Serial.println("transmitter setup done");
  radioTransmitter.setTXpower(115);  // dBuV, 88-115 max
  radioTransmitter.tuneFM(MYFMSTATION); // 107.00 mhz
  Serial.println("transmitter setup done");

  //receiver connections: clk=A5, SDA=A4, rst=7, V=3.3V
  radio.powerOn();//si4703 radio receiver setup
  radio.setVolume(0);// set volume to mute. volume can be 0-15
  Serial.println("radio receiver on");
}


/**
    convert ferquency in 'tens of khz' number as used in the SI4713 FM Library into a number in the range [0..100]
    returns 0 for 8800 and 100 for 10800
*/
int32_t freqToChannel(int freq) {
  return (freq - 8800) / 20;
}

/**
    convert channel in the range [0..100]
    into a frequancy in 'tens of khz' ranging from 8800 to 10800
*/
int32_t channelToFreq(int channel) {
  return 8800 + channel * 20;
}
/**
    gets a frequancy number of a channel from 0..100 and transmits it over IR using NEC protocol (32 bits, 8 adress, 8 inverse of adress, 8 data, 8 inverse of data)
*/
void sendChannel(int freq) {
  mySender.send(NEC, freqToChannel(freq)); ///this is the problem. can't gt this to send code and refreash the receiver
  myReceiver.enableIRIn();
}

int IR_Received() {
  int result = -1;
  if (myReceiver.getResults()) {
    myDecoder.decode();
    if (myDecoder.protocolNum == NEC) {
      int channel = myDecoder.value;
      if (channel >= 0 && channel <= 100) {
        result = channel; //otherwise -1 will be returned (see bottom)
        Serial.println(result, DEC);
      }
    }
  }
  myReceiver.enableIRIn();
  Serial.println("IR received results:");
  Serial.println(result);
  Serial.println("----");
  return result;
}

//was #define 255
#define OTHER  lookupSteps -1 //the index of other is the number of members in the arrey
#define SELF 0
int mixTarget = SELF;
int IRTarget = -1;
int TargetFrequancy = -1;
long lastValidIRTime = 0;

#define IR_EXPIRATION_MS 500//was 300 was 500

long startMixTime = 0; //added =0
int startMixIndex = -1;//added =-1
double mixRateMsPerStep;

void loop() {

  //record time for LED transmitter
  unsigned long currentMillis = millis();
  //check if time has passed to send again
  if (currentMillis - previousMillis >= DELAY_TIME) {
    //Serial.println("send");
    previousMillis = currentMillis;
    sendChannel(MYFMSTATION);
    delay(75);//was 100. couldn't get below 75-dosn't get all the way to index 29
   myReceiver.enableIRIn();
  }
  //myReceiver.enableIRIn();
  //take care of IR receiver
  int currentMixIndex = getMix();// returns an index
  Serial.print("currentmindex:");
  Serial.println(currentMixIndex);
  int channelReceived = IR_Received();
  Serial.print("channel received:");
  Serial.println(channelReceived);
  Serial.print("mixTarget:");
  Serial.println(mixTarget);
  myReceiver.enableIRIn();
  long currentTime = millis();

  if (-1 != channelReceived &&  freqToChannel(MYFMSTATION) != channelReceived ) {//if i got something and that thing is different fron my own
    if (channelReceived == IRTarget || SELF == currentMixIndex) {  //was SELF==currentMixIndex ...if the receiverd channel is alreday my target o I'm at the target
      IRTarget = channelReceived;
      Serial.println("IRTarget:");
      Serial.println(IRTarget);

      TargetFrequancy = (channelToFreq(IRTarget)) / 10; //convert channel to frequancy and devide by 10 to match the si4703's range
      //Serial.println(TargetFrequancy);
      if (SELF == currentMixIndex) {
        Serial.println("SELF == currentMixIndex");
        radio.setChannel(TargetFrequancy);//set channel to target frequancy
        radio.setVolume(7);//set radio volume range is 0-15
      }
      lastValidIRTime = currentTime;
    }
  }

  if ((currentTime - lastValidIRTime) > IR_EXPIRATION_MS && mixTarget != SELF) {//...if I am not listening to self and time has past the "lastValidIRTime" i need to start mixing back to my own music
    mixTarget = SELF; //...set *self* as the target
    startMixTime = currentTime;//...record time

    ///problem need the index from the value
    startMixIndex = currentMixIndex;//...this is -1 or the current index of the value of the pot where we stopped seeing the IR
    mixRateMsPerStep = 75.0;//was 100
  }

  if ((currentTime - lastValidIRTime) <= IR_EXPIRATION_MS && mixTarget != OTHER) {
    mixTarget = OTHER;//set *other* as the target
    startMixTime = currentTime;//record time

    startMixIndex = currentMixIndex;
    mixRateMsPerStep = 75.0; //was
  }

  if (currentMixIndex != mixTarget) {
    int mixDirection = (mixTarget == SELF) ? -1 : 1; //check what is the mix direction
    int mixNewRawIndex = startMixIndex + mixDirection * ((currentTime - startMixTime) / mixRateMsPerStep);//is the problem here?
    int mixNewIndex = max(SELF, min(OTHER, mixNewRawIndex)); //value between 0 and sizeof(lookuptable)-1

    Serial.print(mixDirection);
    Serial.print("\t");
    Serial.print(mixNewRawIndex);
    Serial.print("\t");
    Serial.println(mixNewIndex);

    setMix(mixNewIndex);
    //delay(10);
  }
}


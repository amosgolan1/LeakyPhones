//IR LED must be connected to Arduino PWM pin 3.
// IR receiver connected to pin 11

#include <Wire.h>
#include <IRremote.h>
int RECV_PIN = 11;
IRsend irsend;
IRrecv irrecv(RECV_PIN);
decode_results results;

//debug: 0 - nothing, 1 - numerical for graphing, 2 - full text
#define DEBUG 1

//variables to track and control mixing
long duration = 2560;//mixing duration from volume 0 to volume 255
int step_direction = 1;// used to change the mixing direction
const int steps = 256;//number of steps in the digipot
long step_dureation = duration / steps;//duration of each digipot step
long last_step = 0;

long  time_from_last_change = 0;
long last_change = 0;

int current_frequancy = -1;
int previous_frequancy = -1;

int volume_channel_1 = 255;//channel_1 is my music
int volume_channel_2 = 0;//channel_2 is other's music

//variables to track sending time
long lastSendTime = 0;
long timeFromLastSend = 0;
int DELAY_TIME = 42;//delay between IR transmissions

//transmition frequancy
int freq = 10300;

//state No. and names
const int RECEIVING = 0;
const int MIX = 1;

int noSignalCount = 0;
#define HOW_MANY_SIGNALS_TO_MISS_BEFORE_MIXING_BACK_TO_SELF 40






void setup()
{
  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
  Serial.println("receiver on");

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

static int state = RECEIVING;
void loop() {





  //case TRANSMITTING_RECEIVING:
  //transmit IR as 0-100 code representing the personal radio frequancy
  //do so every "DELAY_TIME" milliseconds
  //timeFromLastSend = millis();
  //if (timeFromLastSend - lastSendTime >= DELAY_TIME) {
  //lastSendTime = millis();
  //sendChannel(freq);//send frequancy as a channel 0-100 using NEC protocol
  //Serial.println(freqToChannel(freq));
  //irrecv.enableIRIn(); // Re-enable receiver
  //}


  //new code

  static int receiveType = 0;
  switch (state)
  {
    case RECEIVING:



      // received valid ir?
      //yes
      if (DEBUG>=2) { Serial.println("receiving"); }
      if (irrecv.decode(&results) == true) {
        if (DEBUG>=2) { Serial.println("waiting for signal"); }
        if (results.decode_type == JVC) {
          if (DEBUG>=2) { Serial.println("received NEC code    "); }
          int channel = results.value;
          if (DEBUG>=2) { Serial.println(channel); }
          if (channel >= 0 && channel <= 100) {
            noSignalCount = 0; //reset counter for 'mix back'
            receiveType = 30;
            
            ////addition
//            time_from_last_change = millis() - last_change;
//            if (time_from_last_change >= 30) {
//              last_change = millis();
              ///end addition
              
              current_frequancy = channelToFreq(channel);
              if (DEBUG>=2) {
                Serial.print("frequancy:");
                Serial.println(current_frequancy);
                Serial.print("previous frequancy:");
                Serial.println(previous_frequancy);
              }
              //is current =previous?
              //yes
              if (current_frequancy == previous_frequancy) {
                //previous_frequancy = current_frequancy;
                //am I at the maximum of channel 2
                //yes
                if (volume_channel_2 == 255) {
                  if (DEBUG>=2) {
                    Serial.println("ive reached the end of channel 2");
                  }
                  state = RECEIVING;
                }
                //no
                else {
                  //previous_frequancy = current_frequancy;
                  if (DEBUG>=2) {
                    Serial.println("stil didn't reach 255 in channel 2");
                  }
                  step_direction = -1;
                  state = MIX;
                }
              }
              //no, current!=previous
              else {
                if (DEBUG>=2) {
                  Serial.println("previous is -1, current is somthing new");
                }
                previous_frequancy = current_frequancy;
                step_direction = -1;
                state = MIX;
                }
              //}
            
            
          }
        }
        irrecv.resume(); // Receive the next value
      }

      //didn't receive anything
      //(irrecv.decode(&results)==false)
      else {
        receiveType = 50;
        time_from_last_change = millis() - last_change;
        if (time_from_last_change >= 200) {


          
          if (DEBUG>=2) {
            Serial.println("didn't receive anything, current frequancy=-1");
          }
          current_frequancy = -1;
          //is current =previous?
          //yes
          if (current_frequancy == previous_frequancy) {
            if (DEBUG>=2) {
              Serial.println("previous frequancy= -1");
            }
            previous_frequancy = current_frequancy;
            //am I at the maximum of channel 1?
            //yes
            if (volume_channel_1 == 255) {
            if (DEBUG>=2) {
              Serial.println("reached 255 in channel 1");
            }
              state = RECEIVING;
              //break;

            }
            //no
            else {
              step_direction = 1;
              if (DEBUG>=2) {
                Serial.println("stil didn't reach 255 in channel 1");
              }
              state = MIX;
//              Serial.println("Not recievd, still mixing");
              //break;
            }
          }
          //no
          else {
            if (DEBUG>=2) {
              Serial.println("previous frequancy!=-1");
            }
            step_direction = 1;
            
            if (noSignalCount > HOW_MANY_SIGNALS_TO_MISS_BEFORE_MIXING_BACK_TO_SELF) {
              state = MIX;   
              previous_frequancy = current_frequancy;         
            } else {
              noSignalCount++;
            }

            //break;

          }
          if (DEBUG>=2) {
            Serial.println("last change was more then 100ms ago");
          }
        }
      }
      if (DEBUG>=2) {
        Serial.println("finished the receiveing loop");
      }
      //irrecv.resume(); // Receive the next value

      break;



    case MIX:
      if (DEBUG>=2) {
        Serial.println("mixing");
      }
      //record the current time
      long time_from_last_step = millis() - last_step;
      //is it time to do another mixing step?
      if (time_from_last_step >= step_dureation) {
        //yes
        last_step = millis();
        volume_channel_1 += step_direction * 5;
        volume_channel_2 -= step_direction * 5;

        //to fix/prevent any over-range bugs:
        volume_channel_1 = min(max(volume_channel_1,0),255);
        volume_channel_2 = min(max(volume_channel_2,0),255);

        if (DEBUG>=2) {
          Serial.print("channel_1_volume is: "); 
          Serial.println(volume_channel_1);
          Serial.print("channel_2_volume is: ");
          Serial.println(volume_channel_2);
        //Serial.println(previous_frequancy);
        //Serial.println(current_frequancy);
        }
        last_change = millis();
        state = RECEIVING;
      }
      else {
        //no
       state = MIX;
      }
      break;
  }
        if (DEBUG>=1) {
          Serial.print(volume_channel_1);
          Serial.print(" ");
          Serial.print(volume_channel_2);  
          if (MIX == state) {
            Serial.print(" 200 ");   
          } else {
            Serial.print(" 100 ");
          }
          Serial.print(receiveType);
          Serial.print(" ");
          Serial.println(noSignalCount*10);             
        }

  delay(120); //TAL HACK: TEST DELAY EFFECT ON DECISION MAKING INSIDE STATE MACHINE
}


//Crazy Audio code modified by me
//http://www.crazy-audio.com/projects/quad-volume-control/controlling-quadvol-with-arduino/
#define CS_NOT 4
#define SDI 3
#define SCLK 2

#define MIN_VOL  191 // +0db,  was 0
#define MAX_VOL 255 // +32.5db was 191
int cycle = 1; // 1ms clock cycle

void setup() {
  // initialize outputs
  Serial.begin(9600);
  pinMode(SCLK, OUTPUT);
  pinMode(CS_NOT, OUTPUT);
  pinMode(SDI, OUTPUT);
}

// set the volume on 4 channels. channels 0&1 (self music) are the same and so is 2&3 (other music)
// 0 = mute
// 255: +31.5db
//0x80 is 0b 1000 0000 and is used to write the volume as a byte to the chip

void setVolume (int selfVol, int otherVol) {

  digitalWrite(CS_NOT, LOW);
  digitalWrite(SCLK, LOW);

  // set volume for the 4 channels
  for (int channel = 0; channel < 4; channel++) {
    int targetVolume = (channel < 2) ? selfVol : otherVol;
    //move bits one by one
    for (int bit = 0; bit < 8; bit++) {
      
      if (targetVolume & 0x80) {
        digitalWrite(SDI, HIGH);
        //Serial.print("1");
      }
      else {
        digitalWrite(SDI, LOW);
        //Serial.print("0");
      }

      delayMicroseconds(1);
      digitalWrite(SCLK, HIGH);
      delayMicroseconds(1);
      
      targetVolume = targetVolume << 1;
      digitalWrite(SCLK, LOW);

    }//for-every-bit
    
  }//for-every-channel

  // initialize all pins to basic state again
  digitalWrite(CS_NOT, HIGH);
//  digitalWrite(SDI, LOW);
  digitalWrite(SCLK, LOW);
}


//loop a 9 sec shift between my music to other
void loop() {
  int delta;
  for (int i=MIN_VOL; i<=MAX_VOL; i+=delta){
    int self=i;
    int other=MAX_VOL-i;
    setVolume(self,other);
    Serial.print("self=");
    Serial.println(self);
    Serial.print("other=");
    Serial.println(other);
    if(i==MIN_VOL) delta=1;
    if(i==MAX_VOL) delta=-1;
    delay(100);
    }
 /* setVolume(MIN_VOL, MAX_VOL);
  Serial.print("MIN - MAX\n");
  delay(2000);
  setVolume(round((MIN_VOL + MAX_VOL) / 1.25), round((MIN_VOL + MAX_VOL) / 1.25));
  Serial.print("50 - 50\n");
  delay(2000);
  setVolume(MAX_VOL, MIN_VOL);
  Serial.print("MAX - MIN\n");
  delay(2000);
  setVolume(round((MIN_VOL + MAX_VOL) / 1.25), round((MIN_VOL + MAX_VOL) / 1.25));
  Serial.print("50 - 50\n");
  delay(2000);*/
}

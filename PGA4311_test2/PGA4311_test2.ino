//check zero crossing at 5V
//mute should be connected to 5V or
#define CS_NOT 4
#define SDI 3
#define SCLK 2

#define MIN_VOL 1
#define MAX_VOL 192

void setup() {

  Serial.begin(9600);
  Serial.println("Welcome, Volume Control Test V1");
  pinMode(CS_NOT,OUTPUT);
  pinMode(SDI,OUTPUT);
  pinMode(SCLK,OUTPUT);

  digitalWrite(CS_NOT,HIGH);
  digitalWrite(SCLK, LOW);
}

void setAllVolume(int vol) {
   
  digitalWrite(SCLK, LOW);
  digitalWrite(CS_NOT,LOW);

  for (int i = 0; i<4; ++i) {//for each one of the 
  
    for (int j = 0; j<8; ++j) {
        int bitCheck = 1<<(7-j);//moves integer 1 7-j places to the left (j=0--> 7-j=7-->10000000)
        int serialValue = (vol & bitCheck )==bitCheck ? 1 : 0;
        //& is bitwise and operator. 
        //in the previous line, I am comparing the bit in the jth location with 1, for example is vol is 168-->01111111
        // and j=0-->7-j=7-->10000000 therefore 01111111 & 01000000, the bit wise adding of these is 01000000 which is 
        //equal to the value of bitcheck (true!).  
        //Serial.print(serialValue);
        //TODO: Delay?
        delayMicroseconds(1);
        digitalWrite(SDI, serialValue==1?HIGH:LOW  );//write the bit value (0 or 1) to the SDI pin. this will now correspond to the 
        //bit value to indicate the volume for the pga

        digitalWrite(SCLK, HIGH);
        delayMicroseconds(1);        
        //delayMicros(1); //TODO: check value; needs to be at least 20 ns
        digitalWrite(SCLK, LOW);      
        
    }
  }
  Serial.println();

  //housekeeping
  digitalWrite(CS_NOT,HIGH);
  digitalWrite(SCLK, LOW);

}

void loop() {
  setAllVolume(191);
  Serial.println(191);
  delay(2000);
  setAllVolume(95);
  Serial.println(95);
  delay(2000);
  setAllVolume(0);
  Serial.println(0);
  delay(2000);
}

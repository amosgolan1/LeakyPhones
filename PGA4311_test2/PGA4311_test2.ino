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

  for (int i = 0; i<4; ++i) {
  
    for (int j = 0; j<8; ++j) {
        int bitCheck = 1<<(7-j);
        int serialValue = (vol & bitCheck )==bitCheck ? 1 : 0;
        //Serial.print(serialValue);
        //TODO: Delay?
        delayMicroseconds(1);
        digitalWrite(SDI, serialValue==1?HIGH:LOW  );

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

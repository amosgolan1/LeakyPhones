#define CS_NOT 4
#define SDI 3
#define SCLK 2

#define MIN_VOL 0
#define MAX_VOL 45

#define UPDATE_INTERVAL 25

void setup() {

  Serial.begin(115200);
  Serial.println("Welcome, Volume Control Test V1");
  pinMode(CS_NOT,OUTPUT);
  pinMode(SDI,OUTPUT);
  pinMode(SCLK,OUTPUT);

  digitalWrite(CS_NOT,HIGH);
  digitalWrite(SCLK, LOW);
}

void loop() {
  static int vol = MIN_VOL;
  static bool increase = true;

  if (increase) {
    if (vol<MAX_VOL) {
      vol+=1;
    } else {
      increase = false;
      vol = MAX_VOL;
    }
  } else {
    if (vol>MIN_VOL) {
      vol-=1;
    } else {
      increase = true;
      vol = MIN_VOL;
    }
  }
  
  Serial.print("Setting volume to: ");
  Serial.println(vol);
  
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

  
  delay(UPDATE_INTERVAL);
}


/********************************************************
**  Downloaded from:                                   **
**  http://www.arduino-projekte.de                     **
********************************************************/

#include "AH_MCP41xxx.h"
//#include <SPI.h>

//#define DATAOUT  11   //uno MOSI , IC SI
//#define SPICLOCK 13   //uno SCK  , IC SCK
#define CS   10   //chipselect pin
#define SHDN 9   //shutdown pin
#define RS   8   //reset pin
#define CS_1   7   //chipselect pin
//#define CS_2   6   //chipselect pin

#define POTIOMETER_0 0
#define POTIOMETER_1 1

byte resistance = 0;
int testIN1 = A0;
int testIN2 = A1;
const int VOL_LOOKOUT[] =  {255,254,250,243,235,
224,211,196,180,163,146,128,109,
92,75,59,44,31,20,12,5,
1,0, 300};
const int LOG_LOOKOUT[] =  {250,249,248,247,246,245,244,243,
239,233,230,227,223,215,210,198,
191,184,175,165,154,142,128,112,
95,75,53,28,0,300};


AH_MCP41xxx mcp42010_1;
AH_MCP41xxx mcp42010_2;
 
void setup()
{  
 Serial.begin(9600);
 Serial.println("Setup ready");
 mcp42010_1.init_MCP42xxx (CS_1, SHDN, RS);  //initialisation
 mcp42010_2.init_MCP42xxx (CS, SHDN, RS);  //initialisation

}

void loop()
{
 
if (Serial.available() > 0) {
int Num = Serial.parseInt();
Serial.println(Num);

if(Num==300){
  Serial.println(300);
mcp42010_1.shutdown(0);
mcp42010_2.shutdown(0);

}

else if (Num==400){
mcp42010_1.shutdown(1);
mcp42010_2.shutdown(1);
Serial.println("swipe sin");
for (int i=0; i<23; i++){
mcp42010_1.setValue(VOL_LOOKOUT[i], POTIOMETER_0);       
mcp42010_1.setValue(VOL_LOOKOUT[i], POTIOMETER_1); 
mcp42010_2.setValue(VOL_LOOKOUT[23-i], POTIOMETER_0);       
mcp42010_2.setValue(VOL_LOOKOUT[23-i], POTIOMETER_1); 
Serial.println(VOL_LOOKOUT[i]);
delay(50);
  }
digitalWrite(SHDN,0); 
}
else if (Num==600){
mcp42010_1.shutdown(1);
mcp42010_2.shutdown(1);
Serial.println("swipe sin");
for (int i=0; i<255; i++){
mcp42010_1.setValue(i, POTIOMETER_0);       
mcp42010_1.setValue(i, POTIOMETER_1); 
mcp42010_2.setValue(255-i, POTIOMETER_0);       
mcp42010_2.setValue(255-i, POTIOMETER_1); 
Serial.println(i);
delay(100);
  } 
}
else if (Num==500){
mcp42010_1.shutdown(1);
mcp42010_2.shutdown(1);
Serial.println("swipe log");
for (int i=0; i<29; i++){
mcp42010_1.setValue(LOG_LOOKOUT[i], POTIOMETER_0);       
mcp42010_1.setValue(LOG_LOOKOUT[i], POTIOMETER_1);
mcp42010_2.setValue(LOG_LOOKOUT[29-i], POTIOMETER_0);       
mcp42010_2.setValue(LOG_LOOKOUT[29-i], POTIOMETER_1);  
Serial.println(LOG_LOOKOUT[i]);
delay(100);
  }
digitalWrite(SHDN,0); 
  
}

else if (Num>=0 && Num<=255){
mcp42010_1.shutdown(1);
mcp42010_2.shutdown(1);
mcp42010_1.setValue(Num, POTIOMETER_0);       
mcp42010_1.setValue(Num, POTIOMETER_1);
mcp42010_2.setValue(Num, POTIOMETER_0);       
mcp42010_2.setValue(Num, POTIOMETER_1);  
  }

else{
  }
}
}



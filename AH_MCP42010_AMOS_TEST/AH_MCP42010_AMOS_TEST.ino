
/********************************************************
** based on librery and example from : 
*                                                      **
**  http://www.arduino-projekte.de                     **
********************************************************/
///This is the TEST version that works////
#include "AH_MCP41xxx.h"
//#include <SPI.h>

//#define DATAOUT  11   //uno MOSI , IC SI
//#define SPICLOCK 13   //uno SCK  , IC SCK
#define CS_1   10   //chipselect pin
#define SHDN_1 5   //shutdown pin for ic 1
#define SHDN_2 9   //shutdown pin for ic 2
#define CS_2   7   //chipselect pin for ic 2
#define RS   8   //reset pin

#define POTIOMETER_1 0
#define POTIOMETER_2 1

byte resistance = 0;
int testIN1 = A0;
int testIN2 = A1;
const int VOL_LOOKUP[] =  {255,254,250,243,235,
                            224,211,196,180,163,146,128,109,
                            92,75,59,44,31,20,12,5,1,0, 300};
                            
const int SIN_LOOKUP[] = {0,13,25,37,50,62,74,86,
                             98,109,120,131,142,152,162,171,
                             180,189,197,205,212,219,225,230,
                             236,240,244,247,250,252,254,255};
                            
const int LOG_LOOKUP[] =  {250,249,248,247,246,245,244,243,
                            239,233,230,227,223,215,210,198,
                            191,184,175,165,154,142,128,112,
                             95,75,53,28,0,300};


AH_MCP41xxx mcp42010_1;
AH_MCP41xxx mcp42010_2;
 
void setup()
{  
 Serial.begin(9600);
 Serial.println("Setup ready");
 mcp42010_1.init_MCP42xxx (CS_1, SHDN_1, RS);  //initialisation
 mcp42010_2.init_MCP42xxx (CS_2, SHDN_2, RS);  //initialisation
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
//Serial.println("swipe sin");
for (int i=0; i<23; i++){
mcp42010_1.setValue(VOL_LOOKUP[i], POTIOMETER_1);       
mcp42010_1.setValue(VOL_LOOKUP[i], POTIOMETER_2); 
mcp42010_2.setValue(VOL_LOOKUP[23-i], POTIOMETER_1);       
mcp42010_2.setValue(VOL_LOOKUP[23-i], POTIOMETER_2); 
Serial.println(VOL_LOOKUP[i]);
delay(200);
  }
mcp42010_1.shutdown(0);
mcp42010_2.shutdown(1);

//digitalWrite(SHDN_1,0);
//digitalWrite(SHDN_2,1);

}
else if (Num==600){
mcp42010_1.shutdown(1);
mcp42010_2.shutdown(1);
//Serial.println("swipe sin");
for (int i=0; i<255; i++){
mcp42010_1.setValue(i, POTIOMETER_1);       
mcp42010_1.setValue(i, POTIOMETER_2); 
mcp42010_2.setValue(255-i, POTIOMETER_1);       
mcp42010_2.setValue(255-i, POTIOMETER_2); 
Serial.println(i);
delay(200);
  } 
digitalWrite(SHDN_1,0);
//digitalWrite(SHDN_2,1);  
}

else if (Num==500){
mcp42010_1.shutdown(1);
mcp42010_2.shutdown(1);
Serial.println("swipe log");
for (int i=0; i<29; i++){
mcp42010_1.setValue(LOG_LOOKUP[i], POTIOMETER_1);       
mcp42010_1.setValue(LOG_LOOKUP[i], POTIOMETER_2);
mcp42010_2.setValue(LOG_LOOKUP[29-i], POTIOMETER_1);       
mcp42010_2.setValue(LOG_LOOKUP[29-i], POTIOMETER_2);  
Serial.println(LOG_LOOKUP[i]);
delay(200);
  }
mcp42010_1.shutdown(0);
//digitalWrite(SHDN_1,0);
//digitalWrite(SHDN_2, 1); 
  
}

else if (Num>0 && Num<255){
mcp42010_1.shutdown(1);
mcp42010_2.shutdown(1);
mcp42010_1.setValue(Num, POTIOMETER_1);       
mcp42010_1.setValue(Num, POTIOMETER_2);
mcp42010_2.setValue(255-Num, POTIOMETER_1);       
mcp42010_2.setValue(255-Num, POTIOMETER_2);  
  }
else if(Num==0){
mcp42010_1.shutdown(0);
mcp42010_2.shutdown(1);
mcp42010_2.setValue(255, POTIOMETER_1);       
mcp42010_2.setValue(255, POTIOMETER_2);  
mcp42010_1.setValue(0, POTIOMETER_1);       
mcp42010_1.setValue(0, POTIOMETER_2);
  }
  else if(Num==255){
mcp42010_1.shutdown(1);
mcp42010_2.shutdown(0);
mcp42010_1.setValue(255, POTIOMETER_1);       
mcp42010_1.setValue(255, POTIOMETER_2);  
mcp42010_2.setValue(0, POTIOMETER_1);       
mcp42010_2.setValue(0, POTIOMETER_2); 
  }     

else{
  }
}
}



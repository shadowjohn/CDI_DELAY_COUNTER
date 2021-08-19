#include <Arduino.h>
//數位模組
#include <TM1637.h>
const int ToPin = D3;  //凸台
const int FirePin = D1;  //點火
#define CLK D5 
#define DIO D4 
TM1637 tm1637(CLK,DIO);
volatile unsigned long C=micros(), rpm=0,RPM_DELAY=0,duty=499999,isShowCount=0,RPM_START_TIME=0,CDI_DELAY=0;
volatile bool CFLAG=false;
void ICACHE_RAM_ATTR countup() {  //For newest version
  //收到CDI點火，扣掉偵測到凸台RISING時間
  //只要是Rising就是Fire 
        
  if(CFLAG==false)
  {
    C=micros();
    RPM_DELAY=0;
    CDI_DELAY=0;
    //rpm = 99999;
    //RPM_START_TIME = 0;      
  }
  else
  {
    RPM_DELAY=micros()-C;
    rpm = 60000000UL / RPM_DELAY;    
    //CDI_DELAY = micros() - RPM_START_TIME;    
    
  }
  CFLAG=!CFLAG;    
}
void ICACHE_RAM_ATTR count_delay() {
  //if(CFLAG){
    CDI_DELAY = micros()-C;
    if(isShowCount > 100==0 ){
      display_rpm();    
      isShowCount=0;
    }
    isShowCount++;
  //}
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(250000);
  Serial.println("Counting...");  
  pinMode(ToPin, INPUT_PULLUP);
  pinMode(FirePin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ToPin), countup, RISING);  
  attachInterrupt(digitalPinToInterrupt(FirePin), count_delay, RISING);  
}

void loop() {
  // put your main code here, to run repeatedly:
  //if(isShowCount > 100==0 )
  {
  //  display_rpm();    
  //  isShowCount=0;
  }
  //isShowCount++;
}
void display_rpm() {  
  if(rpm > 30000) return;
  //if(duty>=500000) return;
  Serial.print("rpm: ");
  Serial.print(rpm); 
  Serial.print(" , CDI_DELAY: ");
  Serial.println(CDI_DELAY);  
  //Serial.print(" , duty: ");  
  //Serial.println(duty);  
}

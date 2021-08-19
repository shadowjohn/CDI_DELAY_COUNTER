#include <Arduino.h>
//數位模組
#include <TM1637.h>
const int SensorPin = D3;  //Define Interrupt Pin (2 or 3 @ Arduino Uno)
const int Purse_Pin = D6;  //產生訊號角
#define CLK D5 
#define DIO D4 
TM1637 tm1637(CLK,DIO);
//#define myPin D6
//#define myPinBit (1<<myPin)
volatile unsigned long C=micros(), rpm=0,RPM_DELAY=0, VR_RPM=100,duty=499999,isShowCount=0,RPM_START_TIME=0,CDI_DELAY=0;
volatile bool CFLAG=false;
volatile unsigned int VR_LOW = 999;
volatile unsigned int VR_HIGH = 0;
volatile unsigned long test = 0;
//From : https://forum.arduino.cc/t/arduino-esp8266-faster-direct-port-write/407251/3
#define myPinBit (1<<15)
void ICACHE_RAM_ATTR countup() {  //For newest version
  //收到CDI點火，扣掉偵測到凸台RISING時間
  //只要是Rising就是Fire 
        
  if(CFLAG==false)
  {
    C=micros();
    RPM_DELAY=0;
    rpm = 99999;  
    CDI_DELAY = micros() - RPM_START_TIME;        
  }
  else
  {
    RPM_DELAY=micros()-C;
    rpm = 60000000UL / RPM_DELAY;    
  }
  CFLAG=!CFLAG;  
  
}

void setup(){
  //delay(1000);

  Serial.begin(250000);
  Serial.println("Counting...");  
  pinMode(SensorPin, INPUT_PULLUP);
  pinMode(Purse_Pin, OUTPUT);
  //analogWriteFreq(22);
  //int o = 512;//map(22,0,233,0,255);
  //analogWriteResolution(13);
  //analogWrite(D6,o);  
  attachInterrupt(digitalPinToInterrupt(SensorPin), countup, RISING);  
}
void loop() {
  //test = (((1*1000*1000)/(1500/60)) ) / 2.0;
  //Serial.println(test);
  //return;
  //Serial.println(o);
  uint16_t  VR = analogRead(A0);
  VR = (VR > 1023)?1023:VR;
  VR_LOW = (VR<VR_LOW)?VR:VR_LOW;
  VR_HIGH = (VR>VR_HIGH)?VR:VR_HIGH;
  
  int HZ = 0;
  if(VR_LOW!=999 && VR_HIGH!=0)
  {
    //自動修正VR上下限值
    HZ = map(VR,VR_LOW,VR_HIGH,1,250); //15000
    //HZ = map(VR,VR_LOW,VR_HIGH,1,500); //30000
  }
  else
  {
    HZ = map(VR,8,1023,1,250); //初值 8~1023 , 15000rpm
    //HZ = map(VR,9,1023,1,500); //初值 30000rpm
  }
  VR_RPM = map(HZ,1,250,0,15000); //0~15000RPM
  //VR_RPM = map(HZ,1,500,0,30000); //0~15000RPM
  
  /*
  參考：http://stm32-learning.blogspot.com/2014/05/arduino.html
  轉速 100 轉 = 每分鐘 100 轉，每秒 1.67 轉，1轉多少秒呢，一轉 = 0.598802 秒 = 598.802 ms = 598802us
  轉速 200 轉 = 每分鐘 200 轉，每秒 3.3 轉，1轉多少秒呢，一轉 = 0.300003 秒 = 300.003 ms = 300003us
  轉速 600 轉 = 每分鐘 600 轉，每秒 10 轉，1轉多少秒呢，一轉 = 0.1 秒 = 100.000 ms = 100000us
  轉速 1000 轉 = 每分鐘 1000 轉，每秒 16.67 轉，1轉多少秒呢，一轉 = 0.059989 秒 = 59.989 ms = 59989us
  轉速 1500 轉 = 每分鐘 1500 轉，每秒 25 轉，1轉多少秒呢，一轉 = 0.04 秒 = 40.000 ms = 40000us
  轉速 6000 轉 = 每分鐘 6000 轉，每秒 60 轉，1轉多少秒呢，一轉 = 0.01 秒 = 10.000 ms = 10000us
  轉速 12000 轉 = 每分鐘 12000 轉，每秒 120 轉，1轉多少秒呢，一轉 = 0.005 秒 = 5.000 ms = 5000us
  轉速 14000 轉 = 每分鐘 14000 轉，每秒 233.3 轉，1轉多少秒呢，一轉 = 0.0042863 秒 = 4.286 ms = 4286us
  轉速 15000 轉 = 每分鐘 15000 轉，每秒 250 轉，1轉多少秒呢，一轉 = 0.004 秒 = 4.000 ms = 4000us
  轉速 16000 轉 = 每分鐘 16000 轉，每秒 266.6 轉，1轉多少秒呢，一轉 = 0.0037500 秒 = 3.750 ms = 3750us
  轉速 20000 轉 = 每分鐘 20000 轉，每秒 333.333 轉，1轉多少秒呢，一轉 = 0.003000003 秒 = 3.000 ms = 3000us
  轉速 30000 轉 = 每分鐘 30000 轉，每秒 500 轉，1轉多少秒呢，一轉 = 0.002 秒 = 2.000 ms = 2000us
  */
  //Serial.println(HZ);
  //analogWriteFreq(HZ);
  //analogWrite(D6,512); 
    

  if(isShowCount > 100==0 && RPM_DELAY == 0 )
  {
    display_rpm();    
    isShowCount=0;
  }

  if(VR_RPM>35000)
  {
    return;
  }
  duty = ((1000000/(VR_RPM/60.0)) )/2; //一正一負 (us) on 10% off 90%
  if(duty>500000) return; //WTF
  //From : https://roboticsbackend.com/arduino-fast-digitalwrite/  
  //開始了
  digitalWrite(Purse_Pin,LOW);
  RPM_START_TIME=micros();
  digitalWrite(Purse_Pin,HIGH); //一個 digitalWrite 會 delay 3.4us  
  //WRITE_PERI_REG( 0x60000304, myPinBit ); //on
  delayMicroseconds((long)duty*((360-180)/360.0)); //29度~=30度 = 330/360 = 11/12
  digitalWrite(Purse_Pin,LOW);
  //WRITE_PERI_REG( 0x60000308, myPinBit ); //off
  delayMicroseconds((long)duty*(180.0/360.0));
  isShowCount++;
}




void display_rpm() {  
  //if(rpm > 30000 || VR_RPM > 30000) return;
  if(duty>=500000) return;
  Serial.print("VR_LOW: ");
  Serial.print(VR_LOW);
  Serial.print(" , VR_HIGH: ");
  Serial.print(VR_HIGH);
  Serial.print(" , CDI_DELAY: ");
  Serial.print(CDI_DELAY);
  Serial.print(" , VR_RPM: ");  
  Serial.print(VR_RPM);
  Serial.print(" , duty: ");  
  Serial.println(duty);  
}

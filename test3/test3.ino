#include <Arduino.h>
//數位模組
#include <TM1637.h>
const int ToPin = D3;  //凸台
const int FirePin = D1;  //點火
//0~14000
volatile const int degree[15] = {12,12,12,23,29,27,25,23,20,17,13,10,8,8,8}; 
volatile int rdegree[15] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; //34 - degree
volatile const int fullAdv = 34;
volatile int fireTimes = 0;
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
    digitalWrite(FirePin,LOW);
    if(rpm!=0)
    {
      int index = (int)rpm/1000;
      //if(index>=15) index=14;
      index = (index>=15)? 14:index;
      index = (index<=0) ?  1:index;
      int r = rdegree[index];
      CDI_DELAY =(int)((1000000)/(rpm/60.0))*(r/360.0);            
      //display_rpm();    
      delayMicroseconds(CDI_DELAY);
      //diaplayOnLed(r);
      //rpm=0;
      digitalWrite(FirePin,HIGH);
      //點火持續時間
      //fireTimes = (int)(map(rpm,600,14000,350,150));
      delayMicroseconds(200);
      digitalWrite(FirePin,LOW);
      
    }
          
  }
  else
  {
    RPM_DELAY=micros()-C;
    rpm = 60000000UL / RPM_DELAY;    
    //CDI_DELAY = micros() - RPM_START_TIME;    
    
  }
  CFLAG=!CFLAG;    
}



void setup() {
  // put your setup code here, to run once:
  Serial.begin(250000);
  Serial.println("Counting...");  
  pinMode(ToPin, INPUT_PULLUP);
  pinMode(FirePin, OUTPUT);
  //degree to rdegree
  for(int i=0;i<15;i++)
  {
    rdegree[i] = fullAdv - degree[i];
  }
  
  
  attachInterrupt(digitalPinToInterrupt(ToPin), countup, RISING);   
  tm1637.init();  
  tm1637.set(BRIGHT_TYPICAL); //BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;
  playFirstTime();
  diaplayOnLed(0);
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
void playFirstTime()
{
  // 0000~9999 跑二次
  for(int i=0;i<= 9;i++)
  {
    for(int j=0;j<4;j++)
    {
      tm1637.display(j,i);    
    }
    delay(100);  
  }
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
void diaplayOnLed(int show_rpm)
{
  //將轉速，變成顯示值
  //只顯示 萬千百十
  //如果要顯示 千百十個，就不用除了
  //太多數位有點眼花
  //String rpm_str = String(show_rpm/10);
  String rpm_str = String(show_rpm);
  if(rpm_str.length()<=3)
  {
    rpm_str = lpad(rpm_str,4,"X"); // 變成如 "XXX0"
  }
  //Serial.print("\nAfter lpad:");
  //Serial.println(rpm_str);
  for(int i=0;i<4;i++)
  { 
      if(rpm_str[i]=='X')
      {
        tm1637.display(i,-1); //-1 代表 blank 一片黑    
      }
      else
      {
        // Serial.println(rpm_str[i]);
        // 腦包直接轉回 String 再把單字轉 int
        // From : https://www.arduino.cc/en/Tutorial.StringToIntExample
        tm1637.display(i,String(rpm_str[i]).toInt());    
      }
  }
}
String lpad(String temp , byte L , String theword){
  //用來補LED左邊的空白
  byte mylen = temp.length();
  if(mylen > (L - 1))return temp.substring(0,L-1);
  for (byte i=0; i< (L-mylen); i++) 
    temp = theword + temp;
  return temp;
}

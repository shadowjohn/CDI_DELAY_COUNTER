//CDI_DELAY_COUNTER
//Require Arduino Version 1.6.23
//CDI 脈衝點火時間差顯示器

//Author：
//        田同學  2021/08/12 夜
//        羽山 2021/08/13
//接角說明
//D_CDI 接 CDI 點火訊號<->高壓線圈之間 (D1)
//D_INT 接 脈衝訊號 (凸台訊號) (D3) 
//ESP8266 使用 D3
//Arduino Uno使用D2
//D8 TM1637 CLK
//D7 TM1637 DIO

#include <TM1637.h>
//用來偵測 D1 腳抓脈衝凸台的訊號

volatile unsigned long p0_t = 0;
volatile unsigned long p0_t1= 0;
volatile unsigned long fire_time= 0;
volatile unsigned short rpm = 0;
volatile bool p0_flag = true;
volatile bool start_wait_fire = false;
//volatile bool p1_flag = false;
volatile unsigned long duty = 0;
const int MY_D7 = D7;
const int MY_D8 = D8;

const int MY_D_INT = D3;
const int MY_D_CDI = D1;

#define CLK MY_D8
#define DIO MY_D7

 
TM1637 tm1637(CLK,DIO);


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
void diaplayOnLed(int show_rpm)
{
  //將show_rpm，變成顯示值
  //例如 1500 轉 = 每分鐘  1500 轉，每秒 25   轉，1轉 = 0.04 秒 =  40.000 ms =  40000us = 顯示為 4000
  //例如 6000 轉，                 每秒 60   轉，1轉 = 0.01666... 秒 =  16.667 ms = 16667us = 顯示為 1666
  //例如 14000 轉 = 每分鐘 14000 轉，每秒 233.3 轉，1轉 = 0.0042863. 秒 =   4.286 ms =   4286us = 顯示為 428
  //只顯示 萬千百十
  //如果要顯示 千百十個，就不用除了
  //太多數位有點眼花
  String rpm_str = String(show_rpm/10);
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
void p0_ISR() {  
  if (p0_flag) {
    p0_t = micros();
    start_wait_fire = true;
  }
  else {
    p0_t1 = micros() - p0_t;
    rpm = 60000000UL / p0_t1;
  }
  p0_flag = !p0_flag;
}
void setup() {
  
  Serial.begin(115200);
  // 4 Digit-Display
  tm1637.init();  
  tm1637.set(BRIGHT_TYPICAL); //BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;
  playFirstTime();
  //playFirstTime();
  //回歸零
  //tm1637.display(0,-1); //-1 for blank
  //tm1637.display(1,-1); //-1 for blank
  //tm1637.display(2,-1); //-1 for blank
  //tm1637.display(3,0);
  diaplayOnLed(0);
  
  pinMode(MY_D_CDI,INPUT);
  pinMode(MY_D_INT,INPUT);
  
  
  attachInterrupt(digitalPinToInterrupt(MY_D_INT), p0_ISR, CHANGE); //接訊號 //偵測點火 (凸台) //脈衝訊號  
  //下版再試
  //attachInterrupt(digitalPinToInterrupt(MY_D_CDI), p0_ISR, RISING); //D3 
  
  Serial.println("");
  Serial.println("Program Start:");
  Serial.println(digitalPinToInterrupt(MY_D_INT));
  //Serial.println(digitalPinToInterrupt(MY_D3));
  //noInterrupts();
  //interrupts();
}

void loop() {  
  if (start_wait_fire) // digitalRead(MY_D_CDI)==HIGH) 
  {
    //p1_flag = false;
    start_wait_fire = false;
    //fire_time = micros();        
    //duty
    duty = fire_time - p0_t;    
    Serial.print("RPM: ");
    Serial.print(rpm);        
    //Serial.print(" , ");
    //Serial.print(fire_time);
    //Serial.print(" - ");
    //Serial.print(p0_t);
    //Serial.print(" = ");
    //Serial.print(duty);
    Serial.println();
    /*
     * 參考：http://stm32-learning.blogspot.com/2014/05/arduino.html   
     * 轉速  1500 轉 = 每分鐘  1500 轉，每秒  25   轉，1轉 = 0.04       秒 =  40.000 ms =  40000us
     * 轉速  6000 轉 = 每分鐘  6000 轉，每秒  60   轉，1轉 = 0.01666... 秒 =  16.667 ms =  16667us
     * 轉速 14000 轉 = 每分鐘 14000 轉，每秒 233.3 轉，1轉 = 0.0042863. 秒 =   4.286 ms =   4286us
     * 轉速 16000 轉 = 每分鐘 16000 轉，每秒 266.6 轉，1轉 = 0.0037500. 秒 =   3.750 ms =   3750us
    */             
  }
 
}
/*
  //讓 LED 不要刷的太快，太快眼睛跟不上字會黏在一起
  //if (micros()%10000==0) 
  //{       
    //diaplayOnLed(duty);        
  //}  
  //digitalWrite(MY_D3,HIGH);
  //t=!t;
  //delay(1);
 */

/*void p1_ISR() {
  p1_t = micros();
  p1_flag = true;
}
*/
